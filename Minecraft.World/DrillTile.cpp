#include "stdafx.h"
#include "ItemInstance.h"
#include "Material.h"
#include "DrillTile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.material.h"
#include <map>
#include <tuple>
#include <mutex>
#include "Facing.h"
#include "IconRegister.h"
#include "Level.h"
#include "PistonBaseTile.h"

DrillTile::DrillTile(int id)
	: Tile(id, Material::stone, true)
{
	frontIcon = nullptr;
	backIcon = nullptr;
	sideIcon = nullptr;
	topIcon = nullptr;

	setDestroyTime(3.5f);
	setExplodeable(6.0f);
	setSoundType(Tile::SOUND_STONE);
}

void DrillTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	// Same as the Observer: PistonBaseTile::getNewFacing() gives the direction TOWARD the
	// player, so the opposite points the face away from the player, at whatever they aimed at.
	int facing = Facing::getOpposite(PistonBaseTile::getNewFacing(level, x, y, z, by));
	level->setData(x, y, z, facing & FACING_MASK, Tile::UPDATE_ALL);
}

Icon *DrillTile::getTexture(int face, int data)
{
	int facing = getFacing(data);
	int opposite = Facing::getOpposite(facing);

	if (face == facing) return frontIcon;
	if (face == opposite) return backIcon;

	// Same rule as the Observer: the arrow texture ("top") is on the up/down faces of a horizontal drill, and on the north/south
	// faces of a vertical one; TileRenderer turns it so the arrow points at the drill face.
	bool facingIsHorizontal = (facing != Facing::UP && facing != Facing::DOWN);
	if (facingIsHorizontal)
	{
		if (face == Facing::UP || face == Facing::DOWN) return topIcon;
	}
	else
	{
		if (face == Facing::NORTH || face == Facing::SOUTH) return topIcon;
	}

	return sideIcon;
}

void DrillTile::registerIcons(IconRegister *iconRegister)
{
	frontIcon = iconRegister->registerIcon(L"drill_face");
	backIcon = iconRegister->registerIcon(L"observer_back");
	sideIcon = iconRegister->registerIcon(L"observer_side");
	topIcon = iconRegister->registerIcon(L"observer_top");

	icon = sideIcon;
}


// ============================================================================================================================
// neoLegacy - mining, formerly DrillTileEntity::tick(). Same numbers a player with an iron pickaxe gets:
//     progress per tick = (tool speed / block hardness) / 30      (Tile::getDestroyProgress)
// Progress resets when power is lost or the target block changes. State is kept per drill position.
// ============================================================================================================================
namespace
{
	struct DrillState
	{
		float progress;
		int targetState;	// packed (tileId << 4) | data of the block being mined, -1 = none
		int crackStage;		// last crack-overlay stage sent, -1 = none
		DrillState() : progress(0.0f), targetState(-1), crackStage(-1) {}
	};
	// neoLegacy - recursive: DrillTile::mine() breaks blocks, and breaking a block can call straight back into drillForget() (a drill mining
	// another drill, or anything whose removal triggers a drill's onRemove) on the SAME thread. With a plain std::mutex that second lock throws
	// std::system_error(resource_deadlock_would_occur) -> std::terminate (the crash in RandomCreativeCrash3).
	std::recursive_mutex g_drillLock;
	std::map<std::tuple<Level *, int, int, int>, DrillState> g_drillStates;
	shared_ptr<ItemInstance> g_drillPickaxe;

	// one overlay id per drill so two drills never fight over a crack overlay, and it can't collide with a real entity id
	int drillCrackId(int x, int y, int z)
	{
		return 0x40000000 | ((x & 0x3FF) << 20) | ((z & 0x3FF) << 10) | (y & 0x3FF);
	}

	void drillResetState(Level *level, int x, int y, int z, DrillState &st)
	{
		if (st.crackStage >= 0 && level != nullptr)
		{
			level->destroyTileProgress(drillCrackId(x, y, z), x, y, z, -1);
		}
		st.progress = 0.0f;
		st.targetState = -1;
		st.crackStage = -1;
	}

	void drillForget(Level *level, int x, int y, int z)
	{
		std::lock_guard<std::recursive_mutex> g(g_drillLock);
		std::map<std::tuple<Level *, int, int, int>, DrillState>::iterator it = g_drillStates.find(std::make_tuple(level, x, y, z));
		if (it != g_drillStates.end())
		{
			drillResetState(level, x, y, z, it->second);
			g_drillStates.erase(it);
		}
	}
}

void DrillTile::mine(Level *level, int x, int y, int z)
{
	// neoLegacy - the world is only changed AFTER the lock is released and after we are done with `st`. Destroying the target can remove
	// other drills (or this one) and erase their entries from g_drillStates, which would leave `st` dangling if it were used afterwards.
	bool breakNow = false;
	int wx = 0, wy = 0, wz = 0;
	{
		std::lock_guard<std::recursive_mutex> g(g_drillLock);
		DrillState &st = g_drillStates[std::make_tuple(level, x, y, z)];

		int facing = getFacing(level->getData(x, y, z));
		wx = x + Facing::STEP_X[facing];
		wy = y + Facing::STEP_Y[facing];
		wz = z + Facing::STEP_Z[facing];

		int targetId = level->getTile(wx, wy, wz);
		if (targetId <= 0) { drillResetState(level, x, y, z, st); return; }

		Tile *target = Tile::tiles[targetId];
		if (target == nullptr || target->material->isLiquid()) { drillResetState(level, x, y, z, st); return; }

		// same "can't be broken" test as a player: negative hardness (bedrock etc.)
		float hardness = target->getDestroySpeed(level, wx, wy, wz);
		if (hardness < 0) { drillResetState(level, x, y, z, st); return; }

		if (g_drillPickaxe == nullptr) g_drillPickaxe = std::make_shared<ItemInstance>(Item::iron_pickaxe);

		// "up to iron level": only what an iron pickaxe may harvest (so not obsidian)
		bool canHarvest = target->material->isAlwaysDestroyable() || g_drillPickaxe->canDestroySpecial(target);
		if (!canHarvest) { drillResetState(level, x, y, z, st); return; }

		// new target (or the block in front changed under us): start over
		int state = (targetId << 4) | (level->getData(wx, wy, wz) & 0xF);
		if (state != st.targetState)
		{
			drillResetState(level, x, y, z, st);
			st.targetState = state;
		}

		if (hardness <= 0.0f) st.progress = 1.0f;		// tall grass, torches... break immediately
		else st.progress += (g_drillPickaxe->getDestroySpeed(target) / hardness) / 30.0f;

		if (st.progress >= 1.0f)
		{
			st.progress = 0.0f;
			st.targetState = -1;
			st.crackStage = -1;
			breakNow = true;
		}
		else
		{
			int stage = static_cast<int>(st.progress * 10) - 1;	// same 0-9 crack stages a player's mining shows
			if (stage != st.crackStage)
			{
				st.crackStage = stage;
				level->destroyTileProgress(drillCrackId(x, y, z), wx, wy, wz, stage);
			}
		}
	}

	if (breakNow)
	{
		level->destroyTileProgress(drillCrackId(x, y, z), x, y, z, -1);
		level->destroyTile(wx, wy, wz, true);
	}
}

void DrillTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (level->isClientSide) return;

	if (!level->hasNeighborSignal(x, y, z))
	{
		drillForget(level, x, y, z);	// power lost: stop and clear the crack overlay
		return;
	}

	mine(level, x, y, z);
	level->addToTickNextTick(x, y, z, id, 1, 0);	// keep drilling every game tick while powered
}

void DrillTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (level->isClientSide) return;
	if (level->hasNeighborSignal(x, y, z) && !level->isTileToBeTickedAt(x, y, z, id))
	{
		level->addToTickNextTick(x, y, z, id, 1, 0);
	}
}

void DrillTile::onPlace(Level *level, int x, int y, int z)
{
	// also runs when a piston finishes moving the drill: it starts mining fresh if it is powered
	if (level->isClientSide) return;
	if (level->hasNeighborSignal(x, y, z) && !level->isTileToBeTickedAt(x, y, z, id))
	{
		level->addToTickNextTick(x, y, z, id, 1, 0);
	}
}

void DrillTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	if (!level->isClientSide) drillForget(level, x, y, z);
	Tile::onRemove(level, x, y, z, id, data);
}
