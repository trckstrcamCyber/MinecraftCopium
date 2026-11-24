#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "DoorTile.h"
#include "CopperOxidationTileEntity.h"
#include "IconRegister.h"
#include "LevelEvent.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.h"
#include "net.minecraft.h"
#include <tuple>

static std::map<wstring, int> doorItemMap = {
	{ L"doorWood",   Item::wooden_door_Id   },
	{ L"doorIron",   Item::iron_door_Id   },
	{ L"doorSpruce", Item::spruce_door_Id },
	{ L"doorBirch",  Item::birch_door_Id  },
	{ L"doorJungle", Item::jungle_door_Id },
	{ L"doorAcacia", Item::acacia_door_Id },
	{ L"doorDark",   Item::dark_oak_door_Id   },
	{ L"doorBamboo", Item::bamboo_door_Id },
	{ L"doorCopper", Item::copper_door_Id },	// neoLegacy - Copper Age (Phase 3)
	{ L"doorRoseGold", Item::rose_gold_door_Id }	// neoLegacy - Copper Age (Phase 3)
};

// neoLegacy - Copper Age (review fix, 2026-11-20). Oxidation snapshot
// for blocks about to be removed, keyed by block position. The live
// break path removes the tile (and its TE) before spawnResources runs,
// so onRemoving records the oxidation here first and spawnResources
// falls back to it when the TE is already gone. Entries are consumed
// on read; a stale entry (e.g. a Creative break, which never drops) is
// harmless — the next removal at that position overwrites it.
static std::map<std::tuple<int, int, int>, int> s_removedOxidation;

DoorTile::DoorTile(int id, Material *material, const wstring& doorType) : BaseEntityTile(id, material,isSolidRender())
{
	setLightBlock(0);
	this->doorType = doorType;

	float r = 0.5f;
	float h = 1.0f;
	Tile::setShape(0.5f - r, 0, 0.5f - r, 0.5f + r, h, 0.5f + r);
}

shared_ptr<TileEntity> DoorTile::newTileEntity(Level *level)
{
	// Every door half gets one (setTileAndData creates it per half); the
	// LOWER half's is authoritative for oxidation, the upper's is ignored.
	return std::make_shared<CopperOxidationTileEntity>();
}

Icon *DoorTile::getTexture(int face, int data)
{
	// Item / inventory form: the item aux value is the oxidation state.
	return iconBottom[data & 3][TEXTURE_NORMAL];
}

int DoorTile::getOxidation(LevelSource *level, int x, int y, int z, bool isUpper)
{
	if (id != Tile::copper_door_Id) return 0;
	// Oxidation workaround (2026-11-20): the field lives in the LOWER
	// half's CopperOxidationTileEntity, not in the nibble. Resolve the
	// lower half from either half and read its TE (0 if it is missing).
	int lowerY = isUpper ? y - 1 : y;
	// The block below an upper half must still be this door: a copper
	// trapdoor or copper stairs below shares the same TE class, so
	// without this check an orphaned upper half would read the wrong
	// block's oxidation (and getCompositeData would read its data as
	// door data).
	if (level->getTile(x, lowerY, z) != id) return 0;
	shared_ptr<TileEntity> te = level->getTileEntity(x, lowerY, z);
	shared_ptr<CopperOxidationTileEntity> ox = dynamic_pointer_cast<CopperOxidationTileEntity>(te);
	if (ox == nullptr) return 0;
	return ox->getOxidation();
}

bool DoorTile::isHandOpenable()
{
	// Copper + rose-gold doors open by hand (Java); the iron door does not.
	if (id == Tile::copper_door_Id || id == Tile::rose_gold_door_Id) return true;
	return material != Material::metal;
}

Icon *DoorTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	if (face == Facing::UP || face == Facing::DOWN) return iconBottom[getOxidation(level, x, y, z, (level->getData(x, y, z) & C_IS_UPPER_MASK) != 0) & 3][TEXTURE_NORMAL];

	int compositeData = getCompositeData(level, x, y, z);
	int dir = compositeData & C_DIR_MASK;
	bool isOpen = (compositeData & C_OPEN_MASK) != 0;
	bool flip = false;
	bool upper = (compositeData & C_IS_UPPER_MASK) != 0;
	int oxidation = getOxidation(level, x, y, z, upper) & 3;

	if (isOpen)
	{
		if (dir == 0 && face == 2) flip = !flip;
		else if (dir == 1 && face == 5) flip = !flip;
		else if (dir == 2 && face == 3) flip = !flip;
		else if (dir == 3 && face == 4) flip = !flip;
	}
	else
	{
		if (dir == 0 && face == 5) flip = !flip;
		else if (dir == 1 && face == 3) flip = !flip;
		else if (dir == 2 && face == 4) flip = !flip;
		else if (dir == 3 && face == 2) flip = !flip;
		if ((compositeData & C_RIGHT_HINGE_MASK) != 0) flip = !flip;
	}

	if (upper)
	{
		return iconTop[oxidation][flip ? TEXTURE_FLIPPED : TEXTURE_NORMAL];
	}
	else
	{
		return iconBottom[oxidation][flip ? TEXTURE_FLIPPED : TEXTURE_NORMAL];
	}
}

void DoorTile::registerIcons(IconRegister *iconRegister)
{
	// Only the copper door has per-oxidation-state icons; every other door
	// registers the single (unaffected) state exactly as before.
	int states = (id == Tile::copper_door_Id) ? 4 : 1;
	static const wchar_t *prefixes[4] = { L"", L"exposed_", L"weathered_", L"oxidized_" };
	for (int s = 0; s < states; s++)
	{
		wstring upperName = wstring(prefixes[s]) + getIconName() + L"_upper";
		wstring lowerName = wstring(prefixes[s]) + getIconName() + L"_lower";
		iconTop[s][TEXTURE_NORMAL] = iconRegister->registerIcon(upperName);
		iconBottom[s][TEXTURE_NORMAL] = iconRegister->registerIcon(lowerName);
		iconTop[s][TEXTURE_FLIPPED] = new FlippedIcon(iconTop[s][TEXTURE_NORMAL], true, false);
		iconBottom[s][TEXTURE_FLIPPED] = new FlippedIcon(iconBottom[s][TEXTURE_NORMAL], true, false);
	}
	// Point the unused state slots at state 0 so a stray read never derefs null.
	for (int s = states; s < 4; s++)
	{
		iconTop[s][TEXTURE_NORMAL] = iconTop[0][TEXTURE_NORMAL];
		iconBottom[s][TEXTURE_NORMAL] = iconBottom[0][TEXTURE_NORMAL];
		iconTop[s][TEXTURE_FLIPPED] = iconTop[0][TEXTURE_FLIPPED];
		iconBottom[s][TEXTURE_FLIPPED] = iconBottom[0][TEXTURE_FLIPPED];
	}
}

bool DoorTile::blocksLight()
{
	return false;
}

bool DoorTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool DoorTile::isCubeShaped()
{
	return false;
}

int DoorTile::getRenderShape()
{
	return Tile::SHAPE_DOOR;
}

AABB *DoorTile::getTileAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	AABB *retval = Tile::getTileAABB(level, x, y, z);
	return retval;
}

AABB *DoorTile::getAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	AABB *retval = Tile::getAABB(level, x, y, z);
	return retval;
}

void DoorTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	setShape(getCompositeData(level,x, y, z));
}

int DoorTile::getDir(LevelSource *level, int x, int y, int z)
{
	return getCompositeData(level, x, y, z) & C_DIR_MASK;
}

bool DoorTile::isOpen(LevelSource *level, int x, int y, int z)
{
	return (getCompositeData(level, x, y, z) & C_OPEN_MASK) != 0;
}

void DoorTile::setShape(int compositeData)
{
	float r = 3 / 16.0f;
	Tile::setShape(0, 0, 0, 1, 2, 1);
	int dir = compositeData & C_DIR_MASK;
	bool open = (compositeData & C_OPEN_MASK) != 0;
	bool hasRightHinge = (compositeData & C_RIGHT_HINGE_MASK) != 0;
	if (dir == 0)
	{
		if (open)
		{
			if (!hasRightHinge) setShape(0, 0, 0, 1, 1, r);
			else setShape(0, 0, 1 - r, 1, 1, 1);
		}
		else setShape(0, 0, 0, r, 1, 1);
	}
	else if (dir == 1)
	{
		if (open)
		{
			if (!hasRightHinge) setShape(1 - r, 0, 0, 1, 1, 1);
			else setShape(0, 0, 0, r, 1, 1);
		}
		else setShape(0, 0, 0, 1, 1, r);
	}
	else if (dir == 2)
	{
		if (open)
		{
			if (!hasRightHinge) setShape(0, 0, 1 - r, 1, 1, 1);
			else setShape(0, 0, 0, 1, 1, r);
		}
		else setShape(1 - r, 0, 0, 1, 1, 1);
	}
	else if (dir == 3)
	{
		if (open)
		{
			if (!hasRightHinge) setShape(0, 0, 0, r, 1, 1);
			else setShape(1 - r, 0, 0, 1, 1, 1);
		}
		else setShape(0, 0, 1 - r, 1, 1, 1);
	}
}

void DoorTile::attack(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	// Fix for #92957 - TU11: Content: Multiplayer: Wooden Doors splits in half and glitch in open / close motion while being mined.
	// In lastest PC version this is commented out, so do that now to fix bug above
	//use(level, x, y, z, player);
}

// 4J-PB - Adding a TestUse for tooltip display
bool DoorTile::TestUse()
{
	return id == Tile::wooden_door_Id;
}

bool DoorTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if (soundOnly)
	{
		// 4J - added - just do enough to play the sound
		if (isHandOpenable())
		{
			bool willOpen = (getCompositeData(level, x, y, z) & C_OPEN_MASK) == 0;
			level->levelEvent(player, getToggleSound(willOpen), x, y, z, 0);
		}
		return false;
	}

	if (!isHandOpenable()) return true;

	int compositeData = getCompositeData(level, x, y, z);
	int lowerData = compositeData & C_LOWER_DATA_MASK;
	lowerData ^= 4;
	bool nowOpen = (compositeData & C_OPEN_MASK) == 0;
	if ((compositeData & C_IS_UPPER_MASK) == 0)
	{
		level->setData(x, y, z, lowerData, Tile::UPDATE_CLIENTS);
		level->setTilesDirty(x, y, z, x, y, z);
	}
	else
	{
		level->setData(x, y - 1, z, lowerData, Tile::UPDATE_CLIENTS);
		level->setTilesDirty(x, y - 1, z, x, y, z);
	}

	level->levelEvent(player, getToggleSound(nowOpen), x, y, z, 0);
	return true;
}

void DoorTile::setOpen(Level *level, int x, int y, int z, bool shouldOpen)
{
	int compositeData = getCompositeData(level, x, y, z);
	bool isOpen = (compositeData & C_OPEN_MASK) != 0;
	if (isOpen == shouldOpen) return;

	int lowerData = compositeData & C_LOWER_DATA_MASK;
	lowerData ^= 4;
	if ((compositeData & C_IS_UPPER_MASK) == 0)
	{
		level->setData(x, y, z, lowerData, Tile::UPDATE_CLIENTS);
		level->setTilesDirty(x, y, z, x, y, z);
	}
	else
	{
		level->setData(x, y - 1, z, lowerData, Tile::UPDATE_CLIENTS);
		level->setTilesDirty(x, y - 1, z, x, y, z);
	}

	level->levelEvent(nullptr, getToggleSound(shouldOpen), x, y, z, 0);
}

void DoorTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	int data = level->getData(x, y, z);
	if ((data & UPPER_BIT) == 0)
	{
		bool spawn = false;
		if (level->getTile(x, y + 1, z) != id)
		{
			level->removeTile(x, y, z);
			spawn = true;
		}
		if (!level->isTopSolidBlocking(x, y - 1, z))
		{
			level->removeTile(x, y, z);
			spawn = true;
			if (level->getTile(x, y + 1, z) == id)
			{
				level->removeTile(x, y + 1, z);
			}
		}
		if (spawn)
		{
			if (!level->isClientSide)
			{
				spawnResources(level, x, y, z, data, 1, 0);
			}
		}
		else
		{
			bool signal = level->hasNeighborSignal(x, y, z) || level->hasNeighborSignal(x, y + 1, z);
			if ((signal || (type > 0 && Tile::tiles[type]->isSignalSource())) && type != id)
			{
				setOpen(level, x, y, z, signal);
			}
		}
	}
	else
	{
		if (level->getTile(x, y - 1, z) != id)
		{
			level->removeTile(x, y, z);
		}
		if (type > 0 && type != id)
		{
			neighborChanged(level, x, y - 1, z, type);
		}
	}
}

int DoorTile::getResource(int data, Random *random, int playerBonusLevel)
{
	if ((data & 8) != 0) return 0;
	return doorItemMap[doorType];
}

// neoLegacy - Copper Age (review fix, 2026-11-20). Snapshot the
// oxidation just before the block is removed. LevelChunk::setTileAndData
// calls onRemoving on the server before the tile is replaced and before
// onRemove drops the TE — but the player-break path
// (ServerPlayerGameMode::destroyBlock) and the support-loss path
// (neighborChanged) both call spawnResources only AFTER that removal,
// so a TE read there would always miss. Recording the oxidation here
// lets spawnResources fall back to it. Keyed by position: the upper
// half's snapshot (oxidation 0 — its TE is unused) must not overwrite
// the lower half's. Copper doors only; other doors have no oxidation
// to preserve.
void DoorTile::onRemoving(Level *level, int x, int y, int z, int data)
{
	(void)data;
	if (id != Tile::copper_door_Id) return;
	shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
	shared_ptr<CopperOxidationTileEntity> ox = dynamic_pointer_cast<CopperOxidationTileEntity>(te);
	if (ox != nullptr) s_removedOxidation[std::make_tuple(x, y, z)] = ox->getOxidation();
}

// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). The copper
// door's oxidation lives in the LOWER half's CopperOxidationTileEntity,
// but the drop is produced from the LOWER half (getResource returns 0
// for the upper half), so the base spawnResources would always drop an
// unaffected (aux 0) door item. Read the lower half's TE and drop the
// door item with that aux, so the oxidation round-trips (break -> wax
// -> place). The TE is usually already gone by the time this runs —
// the player-break path (ServerPlayerGameMode::destroyBlock) and the
// support-loss path (neighborChanged) both remove the tile before
// dropping — so fall back to the oxidation onRemoving snapshotted just
// before removal. No upper-half guard: the neighbour cascade removes
// the upper half before this runs, so requiring it would force the
// aux-0 base path. Non-copper doors fall through to the base
// implementation unchanged.
void DoorTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel)
{
	if (id == Tile::copper_door_Id && (data & UPPER_BIT) == 0)
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

int DoorTile::cloneTileData(Level *level, int x, int y, int z)
{
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). Pick-block
	// (middle-click) on a copper door gives the door item with the TE
	// oxidation aux. getOxidation resolves the lower half's TE from
	// either half (bit 3 = upper half).
	if (id == Tile::copper_door_Id)
	{
		int data = level->getData(x, y, z);
		return getOxidation(level, x, y, z, (data & 8) != 0);
	}
	return Tile::cloneTileData(level, x, y, z);
}

HitResult *DoorTile::clip(Level *level, int xt, int yt, int zt, Vec3 *a, Vec3 *b)
{
	updateShape(level, xt, yt, zt);
	return Tile::clip(level, xt, yt, zt, a, b);
}

bool DoorTile::mayPlace(Level *level, int x, int y, int z)
{
	if (y >= Level::maxBuildHeight - 1) return false;

	return (level->isTopSolidBlocking(x, y - 1, z) && Tile::mayPlace(level, x, y, z) && Tile::mayPlace(level, x, y + 1, z));
}

bool DoorTile::isOpen(int data)
{
	return (data & 4) != 0;
}

int DoorTile::getPistonPushReaction()
{
	return Material::PUSH_DESTROY;
}

int DoorTile::getCompositeData(LevelSource *level, int x, int y, int z)
{
	int data = level->getData(x, y, z);
	bool isUpper = (data & UPPER_BIT) != 0;
	int lowerData;
	int upperData;
	if (isUpper)
	{
		// Orphaned upper half (the block below is no longer this door):
		// its data is not door data — treat as closed, facing 0.
		lowerData = level->getTile(x, y - 1, z) == id ? level->getData(x, y - 1, z) : 0;
		upperData = data;
	}
	else
	{
		lowerData = data;
		upperData = level->getData(x, y + 1, z);
	}

	// bits: dir, dir, open/closed, isUpper, isRightHinge
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20): bits 5-7
	// still carry the copper door's oxidation in the composite, but now
	// read from the LOWER half's CopperOxidationTileEntity (see
	// getOxidation), NOT from the upper half's nibble — the nibble holds
	// structural bits only. BlockState.value is a plain int (no width
	// limit). Vanilla doors have 0 there, so this is a no-op for them.
	// DoorTile::use(BlockState) only reads bits 0-3, so opening/closing
	// still works and never wipes the oxidation.
	bool isRightHinge = (upperData & 1) != 0;
	int oxidation = getOxidation(level, x, y, z, isUpper);
	return (lowerData & C_LOWER_DATA_MASK) | (isUpper ? 8 : 0) | (isRightHinge ? 16 : 0) | ((oxidation & 7) << 5);
}

int DoorTile::cloneTileId(Level *level, int x, int y, int z)
{
	return doorItemMap[doorType];
}

void DoorTile::playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player)
{
	if (player->abilities.instabuild)
	{
		if ((data & UPPER_BIT) != 0)
		{
			if (level->getTile(x, y - 1, z) == id)
			{
				level->removeTile(x, y - 1, z);
			}
		}
	}
}

void DoorTile::createBlockStateDefinition()
{
	if (!m_blockStateDefinition)
		m_blockStateDefinition = new BlockStateDefinition(this);
}

int DoorTile::defaultBlockState()
{
	return 0; // closed
}

Tile::BlockState DoorTile::getBlockState(LevelSource *level, int x, int y, int z)
{
	int composite = getCompositeData(level, x, y, z);
	return Tile::BlockState(composite);
}

int DoorTile::convertBlockStateToLegacyData(BlockState *state)
{
	if (!state) return 0;
	int composite = state->value;

	if (composite & C_IS_UPPER_MASK)
	{
		int base = (composite & C_RIGHT_HINGE_MASK) ? 9 : 8;
		if (composite & C_OPEN_MASK) base |= 2;
		// neoLegacy - Copper Age (oxidation workaround, 2026-11-20): the
		// oxidation bits (5-7) the composite carries are NOT restored
		// into the legacy data — the nibble holds structural bits only
		// (lower = dir, upper = 8 | hinge) and the oxidation lives in
		// the lower half's CopperOxidationTileEntity.
		return base;
	}
	else
	{
		int base = composite & C_DIR_MASK;
		if (composite & C_OPEN_MASK) base |= 4;
		return base;
	}
}

Tile::BlockState DoorTile::getBlockState(int data)
{
	int composite = 0;
	if ((data & UPPER_BIT) == 0) {
		composite = data & C_LOWER_DATA_MASK;
	} else {
		composite = C_IS_UPPER_MASK;
		if ((data & 1) != 0) composite |= C_RIGHT_HINGE_MASK;
		// neoLegacy - Copper Age (oxidation workaround, 2026-11-20):
		// legacy data carries no oxidation bits — the oxidation lives in
		// the lower half's CopperOxidationTileEntity, so nothing is
		// carried into the BlockState here (see getCompositeData).
	}
	return Tile::BlockState(composite);
}

void DoorTile::fillVirtualBlockStateProperties(Tile::BlockState *state, LevelSource *level, const BlockPos &pos)
{
	if (!state) return;
	int composite = getCompositeData(level, pos.getX(), pos.getY(), pos.getZ());
	state->value = composite;
}

bool DoorTile::use(Level *level, const BlockPos &pos, Tile::BlockState *state, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly)
{
	if (soundOnly)
	{
		if (material != Material::metal)
			level->levelEvent(player, getToggleSound((getCompositeData(level, pos.getX(), pos.getY(), pos.getZ()) & C_OPEN_MASK) == 0), pos.getX(), pos.getY(), pos.getZ(), 0);
		return false;
	}

	if (material == Material::metal) return true;

	int composite = state ? state->value : getCompositeData(level, pos.getX(), pos.getY(), pos.getZ());
	bool nowOpen = (composite & C_OPEN_MASK) == 0;
	int lowerData = composite & C_LOWER_DATA_MASK;
	lowerData ^= 4;

	if ((composite & C_IS_UPPER_MASK) == 0) {
		level->setData(pos.getX(), pos.getY(), pos.getZ(), lowerData, Tile::UPDATE_CLIENTS);
		level->setTilesDirty(pos.getX(), pos.getY(), pos.getZ(), pos.getX(), pos.getY(), pos.getZ());
	} else {
		level->setData(pos.getX(), pos.getY() - 1, pos.getZ(), lowerData, Tile::UPDATE_CLIENTS);
		level->setTilesDirty(pos.getX(), pos.getY() - 1, pos.getZ(), pos.getX(), pos.getY(), pos.getZ());
	}

	level->levelEvent(player, getToggleSound(nowOpen), pos.getX(), pos.getY(), pos.getZ(), 0);
	return true;
}
int DoorTile::getToggleSound(bool open)
{
	if (material == Material::metal) return open ? LevelEvent::SOUND_OPEN_IRON_DOOR : LevelEvent::SOUND_CLOSE_IRON_DOOR;
	return open ? LevelEvent::SOUND_OPEN_WOODEN_DOOR : LevelEvent::SOUND_CLOSE_WOODEN_DOOR;
}
