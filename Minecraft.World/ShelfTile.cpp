// neoLegacy - Copper Age (Phase 6): shelf storage + redstone output.
// Phase 3 texture/aux behaviour preserved verbatim (woodIcons,
// getSpawnResourcesAuxValue = data & 7).
// Round-5 fix (2026-11-20): the facing lives in the ShelfTileEntity,
// NOT in data bits 3-5 — block data is a 4-bit nibble, so the Phase 6
// "(facing << 3)" write could never be read back and every shelf
// rendered with the NORTH shape. See updateShape()/setPlacedBy().
#include "stdafx.h"
#include "ShelfTile.h"
#include "IconRegister.h"
#include "ShelfTileEntity.h"
#include "SoundTypes.h"
#include "AbstractContainerMenu.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "Facing.h"
#include "Direction.h"
#include "ItemInstance.h"

ShelfTile::ShelfTile(int id) : BaseEntityTile(id, Material::wood, isSolidRender())
{
	for (int i = 0; i < 7; i++) woodIcons[i] = nullptr;
	for (int i = 0; i < 7; i++) backIcons[i] = nullptr;
	for (int i = 0; i < 7; i++) poweredIcons[i] = nullptr;
}

bool ShelfTile::isSolidRender(bool)
{
	return false;
}

bool ShelfTile::isCubeShaped()
{
	return false;
}

void ShelfTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	// Round-5 fix: the facing lives in the ShelfTileEntity, NOT in
	// the block data. Block data is a 4-bit nibble
	// (SparseDataStorage::set masks every write with "& 15", and
	// get() masks every read the same way), so the Phase 6 encoding
	// "(data >> 3) & 7" could never yield Facing::SOUTH (3),
	// WEST (4) or EAST (5): setPlacedBy wrote (facing << 3), i.e.
	// 16/24/32/40, which reads back as 0 or 8, so (data >> 3) & 7
	// was 0 or 1 and the switch below always took the NORTH default
	// — every shelf sat against the wrong wall except a
	// north-facing one. The TE is the only store with room for it:
	// wood already takes data bits 0-2, and a facing needs 2 more.
	// Prefer the TE the caller already has (forceEntity); otherwise
	// look it up (LevelChunk::getTileEntity creates it lazily, and
	// returns nullptr while tile entities are disabled). No TE yet
	// means a freshly placed shelf whose setPlacedBy has not run, or
	// a pre-fix shelf — NORTH is the safe default either way.
	int facing = Facing::NORTH;
	shared_ptr<ShelfTileEntity> shelf = dynamic_pointer_cast<ShelfTileEntity>(forceEntity);
	if (shelf == nullptr && level != nullptr)
	{
		shelf = dynamic_pointer_cast<ShelfTileEntity>(level->getTileEntity(x, y, z));
	}
	if (shelf != nullptr)
	{
		facing = shelf->getFacing();
	}
	// Front faces that direction, body sits against the opposite
	// wall. 5/16 deep (model z 11-16), full width/height.
	switch (facing)
	{
	case Facing::SOUTH: setShape(0, 0, 0, 1, 1, 5.0f/16.0f); break;
	case Facing::WEST:  setShape(11.0f/16.0f, 0, 0, 1, 1, 1); break;
	case Facing::EAST:  setShape(0, 0, 0, 5.0f/16.0f, 1, 1); break;
	default:            setShape(0, 0, 11.0f/16.0f, 1, 1, 1); break; // NORTH
	}
}

void ShelfTile::updateDefaultShape()
{
	setShape(0, 0, 11.0f/16.0f, 1, 1, 1);   // NORTH default
}

// ChestTile pattern: the base getTileAABB/getAABB use the thread-local
// shape, which is only the NORTH default unless updateShape() has run
// for this block — so refresh the facing-dependent shape first, or
// collision/selection would use the wrong side's slab.
AABB *ShelfTile::getTileAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z, -1, shared_ptr<TileEntity>());
	return Tile::getTileAABB(level, x, y, z);
}

AABB *ShelfTile::getAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z, -1, shared_ptr<TileEntity>());
	return Tile::getAABB(level, x, y, z);
}

Icon *ShelfTile::getTexture(int face, int data)
{
	int wood = data & 7;
	if (wood > 6) wood = 0;
	return woodIcons[wood];
}

Icon *ShelfTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	return getTexture(face, level->getData(x, y, z));
}

int ShelfTile::getRenderShape()
{
	// COPPER_MODELS_RENDERER: Java-style multi-cuboid model (back panel +
	// two boards + overlay plane) in TileRenderer.
	return Tile::SHAPE_SHELF;
}

Icon *ShelfTile::getFrontIcon(int data)
{
	int wood = data & 7;
	if (wood > 6) wood = 0;
	// Java 1.21.9 powered mode (Phase 7): bit 3 selects the powered
	// front icon (same layout, redstone-red dividers). The in-world
	// model (TileRenderer::tesselateShelfInWorld) calls this with the
	// block data, so a powered shelf shows the powered front on the
	// body, boards AND overlay; the inventory form calls it with the
	// item aux (0-6, bit 3 clear), so it stays unpowered.
	if (isPowered(data)) return poweredIcons[wood];
	return woodIcons[wood];
}

Icon *ShelfTile::getBackIcon(int data)
{
	int wood = data & 7;
	if (wood > 6) wood = 0;
	return backIcons[wood];
}

void ShelfTile::registerIcons(IconRegister *iconRegister)
{
	static const wchar_t *names[7] = {
		L"oak_shelf", L"spruce_shelf", L"birch_shelf", L"jungle_shelf",
		L"acacia_shelf", L"dark_oak_shelf", L"bamboo_shelf"
	};
	static const wchar_t *backNames[7] = {
		L"oak_shelf_back", L"spruce_shelf_back", L"birch_shelf_back", L"jungle_shelf_back",
		L"acacia_shelf_back", L"dark_oak_shelf_back", L"bamboo_shelf_back"
	};
	// Java 1.21.9 powered mode (Phase 7): powered front icons, atlas
	// row 37 cols 0-6 (PreStitchedTextureMap::loadUVs). Same 16x16
	// layout as the front icon; the divider bars are redstone-red.
	static const wchar_t *poweredNames[7] = {
		L"oak_shelf_powered", L"spruce_shelf_powered", L"birch_shelf_powered", L"jungle_shelf_powered",
		L"acacia_shelf_powered", L"dark_oak_shelf_powered", L"bamboo_shelf_powered"
	};
	for (int i = 0; i < 7; i++)
	{
		woodIcons[i] = iconRegister->registerIcon(names[i]);
		backIcons[i] = iconRegister->registerIcon(backNames[i]);
		poweredIcons[i] = iconRegister->registerIcon(poweredNames[i]);
	}
	icon = woodIcons[0];
}

shared_ptr<TileEntity> ShelfTile::newTileEntity(Level *level)
{
	MemSect(50);
	shared_ptr<TileEntity> retval = std::make_shared<ShelfTileEntity>();
	MemSect(0);
	return retval;
}

void ShelfTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	// ChestTile facing convention (front faces the placer).
	int dir = (Mth::floor(by->yRot * 4 / (360) + 0.5)) & 3;
	int facing = Facing::NORTH;
	if (dir == 1) facing = Facing::EAST;
	if (dir == 2) facing = Facing::SOUTH;
	if (dir == 3) facing = Facing::WEST;
	// Round-5 fix: the facing goes in the TileEntity, NOT in the
	// block data. The wood aux stays in data bits 0-2 (the placement
	// data is already just the item aux); ORing (facing << 3) into
	// the 4-bit data nibble could never be read back — and on a
	// pre-clamp build it spilled bit 4 into the neighbour block's
	// nibble. The TE already exists here: Level::setTileAndData
	// (TileItem::useOn) creates it before setPlacedBy is called.
	level->setData(x, y, z, level->getData(x, y, z) & 7, Tile::UPDATE_ALL);
	shared_ptr<ShelfTileEntity> shelf = dynamic_pointer_cast<ShelfTileEntity>(level->getTileEntity(x, y, z));
	if (shelf != nullptr)
	{
		shelf->setFacing(facing);
		shelf->setChanged();
		// Live-sync the facing to clients (ShelfTileEntity::setChanged
		// broadcasts the TE update packet — see the centralization
		// note there). Level::broadcastTileEntityUpdate is a base no-op
		// on the client Level; ServerLevel sends te->getUpdatePacket()
		// to every player. Same call the statue pose cycle / chest
		// oxidation use.
	}
	// Java 1.21.9 powered mode (Phase 7): a shelf placed next to a
	// powered source starts powered. (The setData above strips bit 3,
	// so the power check runs after it.)
	updatePoweredState(level, x, y, z);
}

void ShelfTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	// ChestTile pattern, without the scatter: drop each stored stack
	// whole at the block centre so nothing is lost with the shelf.
	shared_ptr<ShelfTileEntity> shelf = dynamic_pointer_cast<ShelfTileEntity>(level->getTileEntity(x, y, z));
	if (shelf != nullptr)
	{
		for (unsigned int i = 0; i < shelf->getContainerSize(); i++)
		{
			shared_ptr<ItemInstance> item = shelf->getItem(i);
			if (item != nullptr)
			{
				shared_ptr<ItemEntity> itemEntity = std::make_shared<ItemEntity>(level, x + 0.5, y + 0.5, z + 0.5, item);
				level->addEntity(itemEntity);
				shelf->setItem(i, nullptr);
			}
		}
		level->updateNeighbourForOutputSignal(x, y, z, id);
	}
	BaseEntityTile::onRemove(level, x, y, z, id, data);
}

bool ShelfTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly)
{
	if (soundOnly) return true;
	if (level->isClientSide) return true;
	shared_ptr<ShelfTileEntity> shelf = dynamic_pointer_cast<ShelfTileEntity>(level->getTileEntity(x, y, z));
	if (shelf == nullptr) return false;

	// Java 1.21.9 powered mode (Phase 7): using a POWERED shelf does
	// not swap one slot — it swaps the whole powered group's contents
	// with the player's hotbar (1 shelf -> rightmost 3 slots, 2 ->
	// rightmost 6, 3 -> all 9). The clicked slot does not matter, and
	// a powered shelf never does a single-slot swap. Power lives in
	// data bit 3 (see updatePoweredState).
	if (isPowered(level->getData(x, y, z)))
	{
		swapPoweredGroup(level, x, y, z, player);
		return true;
	}

	// Slot = thirds along the shelf front face: clickX for a
	// north/south-facing shelf, clickZ for east/west-facing.
	// Round-5 fix: the facing comes from the ShelfTileEntity, not
	// from the block data (see updateShape() — the data nibble
	// cannot hold it, so the old "(data >> 3) & 7" read was 0 or 1
	// and an east/west-facing shelf picked its slot from clickX).
	int facing = shelf->getFacing();
	float along = (facing == Facing::EAST || facing == Facing::WEST) ? clickZ : clickX;
	int slot = static_cast<int>(along * 3.0f);
	if (slot < 0) slot = 0;
	if (slot > 2) slot = 2;
	// Java parity: slot 0 is the player's LEFT slot. For NORTH and EAST
	// facings the slot order along the front is mirrored (slot 0 sits at
	// low x / low z, which is the player's right), so mirror the clicked
	// slot to match the renderer (which mirrors the same way).
	if (facing == Facing::NORTH || facing == Facing::EAST)
	{
		slot = 2 - slot;
	}

	shared_ptr<ItemInstance> hand = player->inventory->getSelected();
	if (hand == nullptr)
	{
		// Empty hand: take the item in the clicked slot.
		// SHELF FIX (2026-11-22, swap-delete bug): Java takes by a
		// strict 1:1 hand<->slot exchange — the stack goes directly
		// into the hand slot. The old code routed it through
		// Inventory::add(), which drops it in the FIRST free slot
		// (or merges it into an existing partial stack) instead of
		// the hand, and in Creative a full inventory silently
		// DESTROYS it: Inventory::add() sets count = 0 and returns
		// true when instabuild is set (Inventory.cpp, instabuild
		// branch), so the "put it back" fallback below never ran.
		// Direct assignment cannot fail: the hand slot is empty by
		// definition here, so no fallback is needed at all.
		shared_ptr<ItemInstance> stored = shelf->getItem(slot);
		if (stored == nullptr) return false;
		shelf->removeItemNoUpdate(slot);
		player->inventory->setItem(player->inventory->selected, stored);
		player->inventory->setChanged();
		shelf->setChanged();
		// Java 1.21.9 sound mapping (Phase 7): there is NO distinct
		// take-out sound — removal plays single_swap. And activate /
		// deactivate are POWER sounds (played in updatePoweredState),
		// not fill/empty sounds, so the old "deactivate when the last
		// item is taken" call is deleted here.
		// Live-sync: shelf->setChanged() above broadcasts the TE update
		// packet (see ShelfTileEntity::setChanged centralization).
		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_SHELF_SINGLE_SWAP, 0.5f, level->random->nextFloat() * 0.1f + 0.9f);
		return true;
	}

	shared_ptr<ItemInstance> stored = shelf->getItem(slot);
	if (stored == nullptr)
	{
		// Slot empty: place the WHOLE hand stack (Java swaps the whole
		// stack, not one item).
		int handCount = hand->count;
		shared_ptr<ItemInstance> placed = std::make_shared<ItemInstance>(hand->id, handCount, hand->getAuxValue());
		if (hand->hasTag()) placed->setTag(static_cast<CompoundTag *>(hand->getTag()->copy()));
		shelf->setItem(slot, placed);
		player->inventory->removeItem(player->inventory->selected, handCount);
		// Java 1.21.9 sound mapping (Phase 7): activate is a POWER
		// sound (played in updatePoweredState), not a "first item
		// placed" sound, so the old wasEmpty/activate call is deleted.
		// Live-sync: setItem() calls setChanged(), which broadcasts
		// the TE update packet (see ShelfTileEntity::setChanged).
		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_SHELF_PLACE_ITEM, 0.5f, level->random->nextFloat() * 0.1f + 0.9f);
		return true;
	}

	// Slot occupied: swap the WHOLE hand stack for the stored stack
	// (Java swaps the whole stack, not one item).
	// SHELF FIX (2026-11-22, swap-delete bug): Java is a strict
	// 1:1 hand<->slot exchange — the displaced stack goes directly
	// into the hand slot, never into the inventory at large. The
	// old code removed the hand stack, then routed the displaced
	// stack through Inventory::add(): that drops it in the FIRST
	// free slot (or merges it into an existing partial stack)
	// instead of the hand, so the player saw the old item vanish
	// from the interaction — and in Creative a full inventory
	// silently DESTROYS it: Inventory::add() sets count = 0 and
	// returns true when instabuild is set (Inventory.cpp), so the
	// drop fallback below never ran. Direct assignment cannot
	// fail (the hand slot is freed by the exchange itself), so
	// the drop fallback is deleted, not moved.
	int handCount = hand->count;
	shared_ptr<ItemInstance> placed = std::make_shared<ItemInstance>(hand->id, handCount, hand->getAuxValue());
	if (hand->hasTag()) placed->setTag(static_cast<CompoundTag *>(hand->getTag()->copy()));
	shelf->setItem(slot, placed);
	player->inventory->setItem(player->inventory->selected, stored);
	player->inventory->setChanged();
	// Live-sync: setItem() calls setChanged(), which broadcasts the
	// TE update packet (see ShelfTileEntity::setChanged).
	level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_SHELF_SINGLE_SWAP, 0.5f, level->random->nextFloat() * 0.1f + 0.9f);
	return true;
}

bool ShelfTile::hasAnalogOutputSignal()
{
	return true;
}

int ShelfTile::getAnalogOutputSignal(Level *level, int x, int y, int z, int dir)
{
	// Java: the shelf emits a BINARY signal by filled slots (0-7); the
	// left slot (from the player's view of the front) is the LSB.
	// (Not the chest-style container-fullness 0-15.)
	// Java 1.21.9 (Phase 7): the signal is read from the BACK only.
	// `dir` is a Direction (SOUTH=0, WEST=1, NORTH=2, EAST=3) giving
	// the direction FROM the shelf TOWARD the comparator — see
	// ComparatorTile::getInputSignal, which passes
	// DIRECTION_OPPOSITE[its input direction]. So the comparator is
	// on the back side only when dir is the opposite of the shelf's
	// front direction (Facing -> Direction via FACING_DIRECTION).
	// (The old code ignored dir entirely, so a comparator on ANY side
	// read the shelf.)
	shared_ptr<ShelfTileEntity> shelf = dynamic_pointer_cast<ShelfTileEntity>(level->getTileEntity(x, y, z));
	if (shelf == nullptr) return 0;
	int frontDir = Direction::FACING_DIRECTION[shelf->getFacing()];
	int backDir = Direction::DIRECTION_OPPOSITE[frontDir];
	if (dir != backDir) return 0;
	int signal = 0;
	for (unsigned int i = 0; i < ShelfTileEntity::SLOT_COUNT; i++)
	{
		if (shelf->getItem(i) != nullptr) signal |= (1 << i);
	}
	return signal;
}

// Java 1.21.9 powered mode (Phase 7). Power lives in data bit 3
// (wood keeps bits 0-2); the TileEntity holds no power state, so a
// setData(UPDATE_ALL) re-tesselates the chunk and the client renderer
// picks the powered front icon via getFrontIcon.
void ShelfTile::updatePoweredState(Level *level, int x, int y, int z)
{
	int data = level->getData(x, y, z);
	bool powered = level->hasNeighborSignal(x, y, z);
	bool wasPowered = isPowered(data);
	if (powered == wasPowered) return;
	level->setData(x, y, z, powered ? (data | 8) : (data & ~8), Tile::UPDATE_ALL);
	level->playSound(x + 0.5, y + 0.5, z + 0.5, powered ? eSoundType_SHELF_ACTIVATE : eSoundType_SHELF_DEACTIVATE, 0.5f, level->random->nextFloat() * 0.1f + 0.9f);
}

void ShelfTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	// HopperTile pattern: re-check power whenever a neighbour changes.
	updatePoweredState(level, x, y, z);
}

// Is the block at (x,y,z) a powered shelf facing `facing`? (Group-walk
// helper: the neighbour must be a shelf, powered (bit 3), with the same
// TE facing. A missing TE means "not matching".)
static bool isPoweredShelfAt(Level *level, int x, int y, int z, int facing)
{
	if (level->getTile(x, y, z) != Tile::shelf_Id) return false;
	if (!ShelfTile::isPowered(level->getData(x, y, z))) return false;
	shared_ptr<ShelfTileEntity> shelf = dynamic_pointer_cast<ShelfTileEntity>(level->getTileEntity(x, y, z));
	if (shelf == nullptr) return false;
	return shelf->getFacing() == facing;
}

int ShelfTile::getPoweredGroup(Level *level, int x, int y, int z, int facing, int *outX, int *outZ)
{
	// "Left" is the player's left when facing the shelf front: for a
	// NORTH-facing shelf the player looks south, so left is +x (EAST);
	// SOUTH -> -x; WEST -> -z (player looks east, left is north);
	// EAST -> +z.
	int leftDx = 0, leftDz = 0;
	switch (facing)
	{
	case Facing::NORTH: leftDx = 1; break;
	case Facing::SOUTH: leftDx = -1; break;
	case Facing::WEST:  leftDz = -1; break;
	case Facing::EAST:  leftDz = 1; break;
	default: break;
	}
	// Java: up to 3 connected, left priority — extend left first, then
	// right to fill to 3. The clicked shelf is always group member 1.
	int leftCount = 0, rightCount = 0;
	int cx = x, cz = z;
	while (leftCount + rightCount < 2)
	{
		int nx = cx + leftDx, nz = cz + leftDz;
		if (!isPoweredShelfAt(level, nx, y, nz, facing)) break;
		leftCount++;
		cx = nx; cz = nz;
	}
	cx = x; cz = z;
	while (leftCount + rightCount < 2)
	{
		int nx = cx - leftDx, nz = cz - leftDz;
		if (!isPoweredShelfAt(level, nx, y, nz, facing)) break;
		rightCount++;
		cx = nx; cz = nz;
	}
	// Left-to-right: leftmost first.
	int count = 1 + leftCount + rightCount;
	int sx = x + leftCount * leftDx, sz = z + leftCount * leftDz;
	for (int i = 0; i < count; i++)
	{
		outX[i] = sx - i * leftDx;
		outZ[i] = sz - i * leftDz;
	}
	return count;
}

void ShelfTile::swapPoweredGroup(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	shared_ptr<ShelfTileEntity> shelf = dynamic_pointer_cast<ShelfTileEntity>(level->getTileEntity(x, y, z));
	if (shelf == nullptr) return;
	int facing = shelf->getFacing();
	int gx[3], gz[3];
	int count = getPoweredGroup(level, x, y, z, facing, gx, gz);
	// Java: the group's shelves (left to right) swap with the hotbar —
	// 1 shelf -> the 3 rightmost hotbar slots, 2 -> the 6 rightmost,
	// 3 -> all 9. Each shelf's 3 slots map to 3 consecutive hotbar
	// slots in order. Whole stacks, direct slot assignment (the Part 1
	// swap fix: never route through Inventory::add — its instabuild
	// branch silently destroys stacks in Creative).
	int base = (count == 1) ? 6 : (count == 2) ? 3 : 0;
	for (int gi = 0; gi < count; gi++)
	{
		shared_ptr<ShelfTileEntity> gshelf = dynamic_pointer_cast<ShelfTileEntity>(level->getTileEntity(gx[gi], y, gz[gi]));
		if (gshelf == nullptr) continue;
		for (int s = 0; s < 3; s++)
		{
			int h = base + gi * 3 + s;
			shared_ptr<ItemInstance> shelfItem = gshelf->getItem(s);
			shared_ptr<ItemInstance> hotItem = player->inventory->getItem(h);
			gshelf->setItem(s, hotItem);
			player->inventory->setItem(h, shelfItem);
		}
	}
	player->inventory->setChanged();
	level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_SHELF_MULTI_SWAP, 0.5f, level->random->nextFloat() * 0.1f + 0.9f);
}

int ShelfTile::getSpawnResourcesAuxValue(int data)
{
	return data & 7;
}
