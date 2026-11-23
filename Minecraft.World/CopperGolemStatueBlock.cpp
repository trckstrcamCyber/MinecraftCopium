#include "stdafx.h"
#include "CopperGolemStatueBlock.h"
#include "CopperGolemStatueBlockEntity.h"
#include "CopperGolem.h"
#include "HatchetItem.h"
#include "IconRegister.h"
#include "Inventory.h"
#include "net.minecraft.world.level.h"
#include "SoundTypes.h"

const unsigned int CopperGolemStatueBlock::COPPER_GOLEM_STATUE_NAMES[8] = {
	IDS_TILE_COPPER_GOLEM_STATUE,
	IDS_TILE_EXPOSED_COPPER_GOLEM_STATUE,
	IDS_TILE_WEATHERED_COPPER_GOLEM_STATUE,
	IDS_TILE_OXIDIZED_COPPER_GOLEM_STATUE,
	IDS_TILE_WAXED_COPPER_GOLEM_STATUE,
	IDS_TILE_WAXED_EXPOSED_COPPER_GOLEM_STATUE,
	IDS_TILE_WAXED_WEATHERED_COPPER_GOLEM_STATUE,
	IDS_TILE_WAXED_OXIDIZED_COPPER_GOLEM_STATUE
};

int CopperGolemStatueBlock::makeData(int oxidation, int pose, int facing)
{
	return (oxidation & 3) | ((pose & 3) << 3) | ((facing & 3) << 5);
}

int CopperGolemStatueBlock::getOxidation(int data)
{
	return data & 3;
}

int CopperGolemStatueBlock::getPose(int data)
{
	return (data >> 3) & 3;
}

int CopperGolemStatueBlock::getFacing(int data)
{
	return (data >> 5) & 3;
}

float CopperGolemStatueBlock::facingToYaw(int facing)
{
	// Minecraft yaw: 0 = south, 90 = west, 180 = north, 270 = east.
	switch (facing & 3)
	{
	case FACING_NORTH:
		return 180.0f;
	case FACING_EAST:
		return 270.0f;
	case FACING_WEST:
		return 90.0f;
	default:
		return 0.0f;
	}
}

// Oxidation workaround: pose/facing live in the tile entity, not the
// nibble. Returns nullptr when the TE is missing (chunk not fully
// loaded, or a level-less TE); callers fall back gracefully.
static shared_ptr<CopperGolemStatueBlockEntity> getStatueTE(Level *level, int x, int y, int z)
{
	if (level == nullptr) return nullptr;
	return dynamic_pointer_cast<CopperGolemStatueBlockEntity>(level->getTileEntity(x, y, z));
}

CopperGolemStatueBlock::CopperGolemStatueBlock(int id) : BaseEntityTile(id, Material::metal, isSolidRender())
{
	for (int i = 0; i < 4; i++) stateIcons[i] = nullptr;
}

bool CopperGolemStatueBlock::isSolidRender(bool)
{
	return false;
}

bool CopperGolemStatueBlock::isCubeShaped()
{
	return false;
}

int CopperGolemStatueBlock::getRenderShape()
{
	// ChestTile precedent: the chunk renderer has no case for this shape,
	// so it draws nothing; CopperGolemStatueRenderer draws the model.
	return Tile::SHAPE_ENTITYTILE_ANIMATED;
}

Icon *CopperGolemStatueBlock::getTexture(int face, int data)
{
	// Waxed (bit 2) reuses the unwaxed texture for the same oxidation state.
	return stateIcons[data & 3];
}

Icon *CopperGolemStatueBlock::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	return stateIcons[level->getData(x, y, z) & 3];
}

void CopperGolemStatueBlock::registerIcons(IconRegister *iconRegister)
{
	// CopperTile pattern (4 oxidation icons from the base name), with the
	// IconRegister definition included directly — CopperTile.cpp itself
	// was missing this include and failed to compile (Phase 3 fix
	// aa7b5fc6); do not repeat that here.
	static const wchar_t *prefixes[4] = { L"", L"exposed_", L"weathered_", L"oxidized_" };
	for (int i = 0; i < 4; i++)
	{
		wstring name = wstring(prefixes[i]) + L"copper_golem_statue";
		stateIcons[i] = iconRegister->registerIcon(name);
	}
	// Keep the base Tile::icon on the unaffected state so anything that reads
	// the icon field directly (particles, pick-block) gets a valid icon.
	icon = stateIcons[0];
}

shared_ptr<TileEntity> CopperGolemStatueBlock::newTileEntity(Level *level)
{
	return std::make_shared<CopperGolemStatueBlockEntity>();
}

void CopperGolemStatueBlock::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	// Java Block.column(10, 0, 14): a 10/16-wide, 14/16-tall column.
	setShape(3 / 16.0f, 0, 3 / 16.0f, 13 / 16.0f, 14 / 16.0f, 13 / 16.0f);
}

void CopperGolemStatueBlock::updateDefaultShape()
{
	setShape(3 / 16.0f, 0, 3 / 16.0f, 13 / 16.0f, 14 / 16.0f, 13 / 16.0f);
}

AABB *CopperGolemStatueBlock::getTileAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z, -1, shared_ptr<TileEntity>());
	return Tile::getTileAABB(level, x, y, z);
}

AABB *CopperGolemStatueBlock::getAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z, -1, shared_ptr<TileEntity>());
	return Tile::getAABB(level, x, y, z);
}

void CopperGolemStatueBlock::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	if (by == nullptr) return;

	// The statue has no item form, so nothing places it by hand — only
	// turnToStatue (via setTileAndData, which does not call setPlacedBy)
	// creates one. Implemented defensively: facing from the placer's yaw,
	// pose STANDING, oxidation (+ waxed) bits preserved.
	// Oxidation workaround: the nibble keeps bits 0-2 ONLY (oxidation +
	// waxed); the old packed pose/facing bits 3-6 are cleared, and the
	// facing goes into the TE instead.
	int data = level->getData(x, y, z);
	int facing = (Mth::floor(by->yRot * 4.0f / 360.0f + 0.5f)) & 3;
	level->setData(x, y, z, data & 7, Tile::UPDATE_ALL);
	shared_ptr<CopperGolemStatueBlockEntity> te = getStatueTE(level, x, y, z);
	if (te != nullptr)
	{
		te->setPose(POSE_STANDING);
		te->setFacing(facing);
		level->broadcastTileEntityUpdate(te);
	}
}

bool CopperGolemStatueBlock::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/)
{
	if (soundOnly)
	{
		// DoorTile pattern: just play the sound, change nothing.
		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_COPPER_STATUE_BECOME_STATUE, 1.0f, 1.0f);
		return false;
	}

	// Axe on a statue: Java reanimates an UNOXIDIZED, unwaxed statue
	// into a copper golem — only once the wax and every oxidation
	// stage have been scraped off. An axe on an oxidized statue just
	// scrapes one stage (the pass-through below); it does NOT reanimate.
	// (The audit had this backwards: it reanimated a fully-oxidized
	// statue on one axe use, and never reanimated a scraped-clean one.)
	shared_ptr<ItemInstance> item = player->inventory->getSelected();
	if (item != nullptr && dynamic_cast<HatchetItem *>(item->getItem()) != nullptr)
	{
		int data = level->getData(x, y, z);
		if (getOxidation(data) == 0 && ((data >> 2) & 1) == 0)
		{
			// Unoxidized and unwaxed: reanimate into a golem. The
			// axe only loses durability (and the click is only
			// consumed) when a golem was actually created, on the
			// server. On the client, predict the reanimate (return
			// true) without damaging the axe — the server applies the
			// 1 durability there, so damaging here too would cost 2.
			if (level->isClientSide) return true;
			shared_ptr<CopperGolemStatueBlockEntity> te = dynamic_pointer_cast<CopperGolemStatueBlockEntity>(level->getTileEntity(x, y, z));
			if (te != nullptr)
			{
				shared_ptr<CopperGolem> golem = te->removeStatue();
				if (golem != nullptr)
				{
					level->addEntity(golem);
					level->removeTile(x, y, z);
					item->hurtAndBreak(1, player);
					return true;
				}
			}
			// No golem could be created (TE missing): pass through so
			// the axe's wax-scrape / de-oxidize applies instead of
			// consuming the click for nothing.
			return false;
		}
		return false;
	}

	// Otherwise: cycle the pose (STANDING -> SITTING -> RUNNING -> STAR ->
	// STANDING; Java getNextPose wraps to ZERO on out-of-bounds) and play
	// the become-statue sound. Oxidation workaround: the pose lives in
	// the TE, not the nibble — cycle it there and broadcast so clients
	// see the new pose live (the renderer reads the TE every frame, so
	// no chunk rebuild is needed; setChanged() inside setPose() also
	// notifies comparators). If the TE is missing, change nothing: the
	// old nibble write would only store a truncated pose that never
	// persisted.
	shared_ptr<CopperGolemStatueBlockEntity> te = getStatueTE(level, x, y, z);
	if (te != nullptr)
	{
		int nextPose = (te->getPose() + 1) & 3;
		te->setPose(nextPose);
		level->broadcastTileEntityUpdate(te);
		// Only play the sound when the pose actually changed — a
		// missing TE changes nothing, so it should not sound like it did.
		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_COPPER_STATUE_BECOME_STATUE, 1.0f, 1.0f);
	}
	return true;
}

bool CopperGolemStatueBlock::hasAnalogOutputSignal()
{
	return true;
}

int CopperGolemStatueBlock::getAnalogOutputSignal(Level *level, int x, int y, int z, int dir)
{
	// Java: comparator output = pose ordinal + 1. Oxidation workaround:
	// the pose lives in the TE, not the nibble.
	shared_ptr<CopperGolemStatueBlockEntity> te = getStatueTE(level, x, y, z);
	if (te != nullptr) return te->getPose() + 1;
	// Fallback: TE missing — the nibble no longer holds the pose (it
	// moved into the TE), so decoding it could report a stale pose
	// (an old-save statue with stale bit 3 would read pose 1 instead
	// of 0). Report no output rather than a wrong pose.
	return 0;
}

int CopperGolemStatueBlock::getResourceCount(Random *random)
{
	// Audit fix (Java): breaking a statue drops the statue as an item.
	// It does NOT spawn a golem on break — reanimation is via an axe
	// on an unoxidized, unwaxed statue (see use()).
	return 1;
}

int CopperGolemStatueBlock::getSpawnResourcesAuxValue(int data)
{
	// Oxidation (+ waxed) lives in the nibble bits 0-2; keep it on
	// break/pick, like every other oxidizable copper block (Cameron:
	// a weathered statue drops as a weathered statue).
	return data & 7;
}

unsigned int CopperGolemStatueBlock::getDescriptionId(int iData)
{
	if (iData < 0) iData = 0;
	return COPPER_GOLEM_STATUE_NAMES[iData & 7];
}
