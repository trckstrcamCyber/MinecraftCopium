#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.h"
#include "TrapDoorTile.h"
#include "CopperOxidationTileEntity.h"
#include "IconRegister.h"
#include <tuple>

// neoLegacy - Copper Age (review fix, 2026-11-20). Oxidation snapshot
// for blocks about to be removed, keyed by block position (see
// DoorTile.cpp — same design). The player-break path removes the tile
// (and its TE) before spawnResources runs, so onRemoving records the
// oxidation here first and spawnResources falls back to it when the TE
// is already gone. Entries are consumed on read; a stale entry is
// harmless — the next removal at that position overwrites it.
static std::map<std::tuple<int, int, int>, int> s_removedOxidation;


TrapDoorTile::TrapDoorTile(int id, Material *material) : BaseEntityTile(id, material,isSolidRender())
{
	float r = 0.5f;
	float h = 1.0f;
	setLightBlock(0);
	setShape(0.5f - r, 0, 0.5f - r, 0.5f + r, h, 0.5f + r);
}

shared_ptr<TileEntity> TrapDoorTile::newTileEntity(Level *level)
{
	return std::make_shared<CopperOxidationTileEntity>();
}

// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). The copper
// trapdoor's oxidation lives in its CopperOxidationTileEntity, not in
// the data nibble (which holds structural bits only). Item form: the
// item aux IS the oxidation field (0-7), so the stage is aux & 3 —
// same as DoorTile's item form.
Icon *TrapDoorTile::getTexture(int face, int data)
{
	if (id == Tile::copper_trapdoor_Id) return stateIcons[data & 3];
	return icon;
}

Icon *TrapDoorTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	if (id == Tile::copper_trapdoor_Id)
	{
		shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
		shared_ptr<CopperOxidationTileEntity> ox = dynamic_pointer_cast<CopperOxidationTileEntity>(te);
		if (ox != nullptr) return stateIcons[ox->getOxidationStage()];
		return stateIcons[0];
	}
	return icon;
}

void TrapDoorTile::registerIcons(IconRegister *iconRegister)
{
	if (id == Tile::copper_trapdoor_Id)
	{
		static const wchar_t *prefixes[4] = { L"", L"exposed_", L"weathered_", L"oxidized_" };
		for (int i = 0; i < 4; i++)
		{
			wstring name = wstring(prefixes[i]) + getIconName();
			stateIcons[i] = iconRegister->registerIcon(name);
		}
		icon = stateIcons[0];
	}
	else
	{
		Tile::registerIcons(iconRegister);
	}
}

int TrapDoorTile::getSpawnResourcesAuxValue(int data)
{
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20): the
	// oxidation no longer lives in the nibble, so there is nothing to
	// read here — the value below only ever reflects legacy nibble bits
	// (0 for live blocks). Drops go through the spawnResources override,
	// which reads the CopperOxidationTileEntity, and pick-block goes
	// through the cloneTileData override below, which reads the TE too.
	if (id == Tile::copper_trapdoor_Id) return (data >> 4) & 7;
	return Tile::getSpawnResourcesAuxValue(data);
}

// neoLegacy - Copper Age (review fix, 2026-11-20). Snapshot the
// oxidation just before the block is removed. LevelChunk::setTileAndData
// calls onRemoving on the server before the tile is replaced and before
// onRemove drops the TE — but the player-break path
// (ServerPlayerGameMode::destroyBlock) calls spawnResources only AFTER
// that removal, so a TE read there would always miss. Recording the
// oxidation here lets spawnResources fall back to it. Copper trapdoors
// only; other trapdoors have no oxidation to preserve.
void TrapDoorTile::onRemoving(Level *level, int x, int y, int z, int data)
{
	(void)data;
	if (id != Tile::copper_trapdoor_Id) return;
	shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
	shared_ptr<CopperOxidationTileEntity> ox = dynamic_pointer_cast<CopperOxidationTileEntity>(te);
	if (ox != nullptr) s_removedOxidation[std::make_tuple(x, y, z)] = ox->getOxidation();
}

// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). The base
// spawnResources drops the item with getSpawnResourcesAuxValue(data),
// which cannot see the TE — so for copper trapdoors read the TE here
// and drop with its oxidation aux, mirroring DoorTile. The TE is
// usually already gone by the time this runs (the player-break path
// removes the tile before dropping), so fall back to the oxidation
// onRemoving snapshotted just before removal.
void TrapDoorTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel)
{
	if (id == Tile::copper_trapdoor_Id)
	{
		if (level->isClientSide) return;
		int ox = 0;
		shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
		shared_ptr<CopperOxidationTileEntity> oxTe = dynamic_pointer_cast<CopperOxidationTileEntity>(te);
		if (oxTe != nullptr) ox = oxTe->getOxidation();
		else
		{
			auto it = s_removedOxidation.find(std::make_tuple(x, y, z));
			if (it != s_removedOxidation.end())
			{
				ox = it->second;
				s_removedOxidation.erase(it);
			}
		}
		int count = getResourceCountForLootBonus(playerBonusLevel, level->random);
		for (int i = 0; i < count; i++)
		{
			if (level->random->nextFloat() > odds) continue;
			int type = getResource(data, level->random, playerBonusLevel);
			if (type <= 0) continue;
			popResource(level, x, y, z, std::make_shared<ItemInstance>(type, 1, ox));
		}
		return;
	}
	Tile::spawnResources(level, x, y, z, data, odds, playerBonusLevel);
}

int TrapDoorTile::cloneTileData(Level *level, int x, int y, int z)
{
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). Pick-block
	// (middle-click) on a copper trapdoor gives the trapdoor item with
	// the TE oxidation aux.
	if (id == Tile::copper_trapdoor_Id)
	{
		shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
		shared_ptr<CopperOxidationTileEntity> oxTe = dynamic_pointer_cast<CopperOxidationTileEntity>(te);
		if (oxTe != nullptr) return oxTe->getOxidation();
	}
	return Tile::cloneTileData(level, x, y, z);
}

void TrapDoorTile::createBlockStateDefinition()
{
	if (!m_blockStateDefinition)
		m_blockStateDefinition = new BlockStateDefinition(this);
}

int TrapDoorTile::defaultBlockState()
{
	return 0;
}

int TrapDoorTile::convertBlockStateToLegacyData(BlockState *state)
{
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20): legacy
	// data is structural only (dir 0-3, open bit 2, top bit 3) — mask to
	// 0xF. The oxidation lives in the CopperOxidationTileEntity, not in
	// the BlockState value. (Phase 4 masked to 0x7F to carry bits 4-6.)
	return state ? (state->value & 0xF) : 0;
}

Tile::BlockState TrapDoorTile::getBlockState(int data)
{
	return Tile::BlockState(data & 0xF);
}

Tile::BlockState TrapDoorTile::getBlockState(LevelSource *level, int x, int y, int z)
{
	return Tile::BlockState(level->getData(x, y, z) & 0xF);
}

bool TrapDoorTile::blocksLight()
{
	return false;
}


bool TrapDoorTile::isSolidRender(bool isServerLevel)
{
	return false;
}


bool TrapDoorTile::isCubeShaped()
{
	return false;
}

bool TrapDoorTile::isPathfindable(LevelSource *level, int x, int y, int z)
{
	return !isOpen(level->getData(x, y, z));
}

int TrapDoorTile::getRenderShape()
{
	return Tile::SHAPE_BLOCK;
}


AABB *TrapDoorTile::getTileAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	return Tile::getTileAABB(level, x, y, z);
}


AABB *TrapDoorTile::getAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	return Tile::getAABB(level, x, y, z);
}


void TrapDoorTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	setShape(level->getData(x, y, z));
}


void TrapDoorTile::updateDefaultShape()
{
	float r = 3 / 16.0f;
	setShape(0, 0.5f - r / 2, 0, 1, 0.5f + r / 2, 1);
}


void TrapDoorTile::setShape(int data)
{

	float r = 3 / 16.0f;
	if ((data & TOP_MASK) != 0)
	{
		setShape(0, 1 - r, 0, 1, 1, 1);
	}
	else
	{
		setShape(0, 0, 0, 1, r, 1);
	}
	if (isOpen(data))
	{
		if ((data & 3) == 0) setShape(0, 0, 1 - r, 1, 1, 1);
		if ((data & 3) == 1) setShape(0, 0, 0, 1, 1, r);
		if ((data & 3) == 2) setShape(1 - r, 0, 0, 1, 1, 1);
		if ((data & 3) == 3) setShape(0, 0, 0, r, 1, 1);
	}
}


void TrapDoorTile::attack(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	//use(level, x, y, z, player, 0, 0, 0, 0);
}

// 4J-PB - Adding a TestUse for tooltip display
bool TrapDoorTile::TestUse()
{
	return true;
}

bool TrapDoorTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if (material == Material::metal) return true;

	if (soundOnly)
	{
		// 4J - added - just do enough to play the sound
		level->levelEvent(player, LevelEvent::SOUND_OPEN_DOOR, x, y, z, 0);
		return false;
	}

	int dir = level->getData(x, y, z);
	level->setData(x, y, z, dir ^ 4, Tile::UPDATE_CLIENTS);

	level->levelEvent(player, LevelEvent::SOUND_OPEN_DOOR, x, y, z, 0);
	return true;
}


void TrapDoorTile::setOpen(Level *level, int x, int y, int z, bool shouldOpen)
{
	int dir = level->getData(x, y, z);

	bool wasOpen = (dir & 4) > 0;
	if (wasOpen == shouldOpen) return;

	level->setData(x, y, z, dir ^ 4, Tile::UPDATE_CLIENTS);

	level->levelEvent(nullptr, LevelEvent::SOUND_OPEN_DOOR, x, y, z, 0);
}

void TrapDoorTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (level->isClientSide) return;

	int data = level->getData(x, y, z);
	int xt = x;
	int zt = z;
	if ((data & 3) == 0) zt++;
	if ((data & 3) == 1) zt--;
	if ((data & 3) == 2) xt++;
	if ((data & 3) == 3) xt--;


	// neoLegacy - Java behaviour: a trapdoor does not need a supporting block, so it no longer breaks when the block
	// beside it is removed (this also lets floor/ceiling trapdoors exist).

	bool signal = level->hasNeighborSignal(x, y, z);
	if( signal || ((type > 0 && Tile::tiles[type]->isSignalSource())) )
	{
		setOpen(level, x, y, z, signal);
	}
}

HitResult *TrapDoorTile::clip(Level *level, int xt, int yt, int zt, Vec3 *a, Vec3 *b)
{
	updateShape(level, xt, yt, zt);
	return Tile::clip(level, xt, yt, zt, a, b);
}

int TrapDoorTile::getDir(int dir)
{
	if ((dir & 4) == 0)
	{
		return ((dir - 1) & 3);
	}
	else
	{
		return (dir & 3);
	}
}

// the face that was clicked for the trapdoor being placed right now (read back in setPlacedBy to orient floor/ceiling ones)
static thread_local int s_trapdoorPlacedFace = 2;

int TrapDoorTile::getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue)
{
	s_trapdoorPlacedFace = face;
	if (face == Facing::UP) return 0;			// lying on a floor: bottom half
	if (face == Facing::DOWN) return TOP_MASK;	// hanging from a ceiling: top half
	int dir = 0;
	if (face == 2) dir = 0;
	if (face == 3) dir = 1;
	if (face == 4) dir = 2;
	if (face == 5) dir = 3;
	if (face != Facing::UP && face != Facing::DOWN && clickY > 0.5f) dir |= TOP_MASK;
	return dir;
}

bool TrapDoorTile::mayPlace(Level *level, int x, int y, int z, int face)
{
	// neoLegacy - Java: trapdoors can be placed on any face, including floors and ceilings, with no support requirement
	return true;
}

void TrapDoorTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	if (by == nullptr) return;

	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20): the
	// item aux is the oxidation field — write it into the
	// CopperOxidationTileEntity instead of the nibble (the nibble holds
	// structural bits only). setTileAndData already created the TE.
	// Copper only; other trapdoors have no TE oxidation to write.
	if (id == Tile::copper_trapdoor_Id && itemInstance != nullptr)
	{
		shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
		shared_ptr<CopperOxidationTileEntity> ox = dynamic_pointer_cast<CopperOxidationTileEntity>(te);
		if (ox != nullptr)
		{
			ox->setOxidation(itemInstance->getAuxValue() & 7);
			level->broadcastTileEntityUpdate(ox);
		}
	}

	if (s_trapdoorPlacedFace != Facing::UP && s_trapdoorPlacedFace != Facing::DOWN) return;

	// floor/ceiling trapdoor: put the hinge on the side the player is looking toward, so the open flap stands up facing them
	// look direction 0 = south, 1 = west, 2 = north, 3 = east  ->  data 0 = hinge south, 1 = hinge north, 2 = hinge east, 3 = hinge west
	static const int hingeForLook[4] = { 0, 3, 1, 2 };
	int look = Mth::floor(by->yRot * 4.0f / 360.0f + 0.5) & 3;
	int data = level->getData(x, y, z);
	// Structural bits only — strip any oxidation bits (4-6) out of the
	// data being written, so a legacy/old-save value cannot linger.
	level->setData(x, y, z, ((data & 0xF) & ~3) | hingeForLook[look], Tile::UPDATE_ALL);
}

bool TrapDoorTile::isOpen(int data)
{
	return (data & 4) != 0;
}

bool TrapDoorTile::attachesTo(int id)
{
	if (id <= 0)
	{
		return false;
	}
	Tile *tile = Tile::tiles[id];

	return tile != nullptr && (tile->material->isSolidBlocking() && tile->isCubeShaped()) || tile == Tile::glowstone || (dynamic_cast<HalfSlabTile *>(tile) != nullptr) || (dynamic_cast<StairTile *>(tile) != nullptr);
}