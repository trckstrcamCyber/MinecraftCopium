#include "stdafx.h"
#include "ObserverTile.h"
#include "ObserverTileEntity.h"
#include "Facing.h"
#include "IconRegister.h"
#include "Level.h"
#include "PistonBaseTile.h"

ObserverTile::ObserverTile(int id)
	: Tile(id, Material::stone, true)
{
	frontIcon = nullptr;
	backIcon = nullptr;
	backOnIcon = nullptr;
	sideIcon = nullptr;
	topIcon = nullptr;

	setDestroyTime(3.5f);
	setExplodeable(6.0f);
	setSoundType(Tile::SOUND_STONE);
}

void ObserverTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	int facing = Facing::getOpposite(PistonBaseTile::getNewFacing(level, x, y, z, by));
	level->setData(x, y, z, facing & FACING_MASK, Tile::UPDATE_ALL);
}

Icon *ObserverTile::getTexture(int face, int data)
{
	int facing = getFacing(data);
	int opposite = Facing::getOpposite(facing);
	bool powered = isPowered(data);

	if (face == facing) return frontIcon;
	if (face == opposite) return powered ? backOnIcon : backIcon;

	// Java model: the arrow texture ("top") is on the top and bottom of a horizontal observer, and on the north and south
	// faces of a vertical one (the model is rotated); the other two faces use the plain side texture.
	// TileRenderer rotates the arrow so it points toward the side being watched (see TileRenderer::tesselateInWorld).
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

void ObserverTile::registerIcons(IconRegister *iconRegister)
{
	frontIcon = iconRegister->registerIcon(L"observer_front");
	backIcon = iconRegister->registerIcon(L"observer_back");
	backOnIcon = iconRegister->registerIcon(L"observer_back_on");
	sideIcon = iconRegister->registerIcon(L"observer_side");
	topIcon = iconRegister->registerIcon(L"observer_top");

	icon = sideIcon;
}

int ObserverTile::getSignal(LevelSource *level, int x, int y, int z, int dir)
{
	int data = level->getData(x, y, z);
	if (!isPowered(data)) return 0;

	// neoLegacy FIX - this used to compare against the opposite of our own
	// facing, which is backwards for how this engine's getSignal(dir) is
	// called. Callers (see Level::getBestNeighborSignal / DiodeTile's own
	// getInputSignal) pass "dir" as the direction FROM the neighbor asking
	// for power TO this tile, not the direction this tile is pointing. Our
	// front points toward the watched block (getFacing(data)); a neighbor
	// standing on our BACK side is therefore in the direction opposite our
	// front, which is the SAME direction as "front" when queried from that
	// neighbor's side of us. DiodeTile (the repeater) uses this exact
	// pattern -- compare directly to our own facing, not its opposite -- so
	// mirror that here. The old check made the observer pulse out its FRONT
	// (into the watched block) instead of its back, and nowhere else.
	return (dir == getFacing(data)) ? 15 : 0;
}

int ObserverTile::getDirectSignal(LevelSource *level, int x, int y, int z, int dir)
{
	return getSignal(level, x, y, z, dir);
}

void ObserverTile::tick(Level *level, int x, int y, int z, Random *random)
{
	// Java ObserverBlock.tick: when the scheduled tick fires the pulse is either switched OFF (if it was on),
	// or switched ON and a second tick is scheduled to switch it off again.
	// Java: level.setBlock(pos, state.setValue(POWERED, ...), 2) - flag 2 only (clients + shape updates, NO neighbour
	// block updates). The signal is pushed out of the back explicitly by updateNeighborsInFront below.
	int data = level->getData(x, y, z);
	if (isPowered(data))
	{
		level->setData(x, y, z, data & ~POWERED_BIT, Tile::UPDATE_CLIENTS);
	}
	else
	{
		level->setData(x, y, z, data | POWERED_BIT, Tile::UPDATE_CLIENTS);
		level->addToTickNextTick(x, y, z, id, PULSE_DELAY, 0);
	}
	updateNeighborsInFront(level, x, y, z, data);
}

// The signal leaves through the back: refresh the block behind us and everything touching that block, so strong
// power through a solid block (and pistons / dust next to it) update the same tick.
void ObserverTile::updateNeighborsInFront(Level *level, int x, int y, int z, int data)
{
	// Java: level.neighborChanged(oppositePos, this); level.updateNeighborsAtExceptFromFacing(oppositePos, this, direction);
	int facing = getFacing(data);
	int back = Facing::getOpposite(facing);
	int bx = x + Facing::STEP_X[back];
	int by = y + Facing::STEP_Y[back];
	int bz = z + Facing::STEP_Z[back];

	level->neighborChanged(bx, by, bz, id);
	for (int d = 0; d < 6; d++)
	{
		if (d == facing) continue;	// that neighbour is the observer itself
		level->neighborChanged(bx + Facing::STEP_X[d], by + Facing::STEP_Y[d], bz + Facing::STEP_Z[d], id);
	}
}

void ObserverTile::onPlace(Level *level, int x, int y, int z)
{
	if (level->isClientSide) return;

	// A powered observer that gets placed (e.g. a piston just finished moving it) has no pulse to finish -
	// switch it off right away, like Java does, instead of leaving it stuck on.
	int data = level->getData(x, y, z);
	if (isPowered(data) && !level->isTileToBeTickedAt(x, y, z, id))
	{
		int off = data & ~POWERED_BIT;
		level->setData(x, y, z, off, Tile::UPDATE_CLIENTS | Tile::UPDATE_KNOWN_SHAPE);	// Java flags 18
		updateNeighborsInFront(level, x, y, z, off);
	}
}

void ObserverTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	if (level->isClientSide) return;

	// Java affectNeighborsAfterRemoval: removed while powered and still waiting for its off-tick -> the signal is gone
	if (isPowered(data) && level->isTileToBeTickedAt(x, y, z, id))
	{
		updateNeighborsInFront(level, x, y, z, data & ~POWERED_BIT);
	}
	Tile::onRemove(level, x, y, z, id, data);
}

static inline int id_of_observer() { return Tile::observer->id; }

void ObserverTile::onMovedByPiston(Level *level, int x, int y, int z, int movedData)
{
	if (level == nullptr || level->isClientSide) return;
	if (Tile::observer == nullptr || level->getTile(x, y, z) != Tile::observer->id) return;

	// Java ObserverBlock.updateShape: only reacts while NOT powered, and only if no tick is already scheduled
	if (isPowered(movedData)) return;
	if (level->isTileToBeTickedAt(x, y, z, id_of_observer())) return;
	level->addToTickNextTick(x, y, z, id_of_observer(), PULSE_DELAY, 0);
}

void ObserverTile::notifyWatchers(Level *level, int x, int y, int z)
{
	if (level == nullptr || level->isClientSide) return;
	if (Tile::observer == nullptr) return;

	const int observerId = Tile::observer->id;

	for (int d = 0; d < 6; d++)
	{
		// the observer would sit one step from the changed block, looking back at it
		int ox = x + Facing::STEP_X[d];
		int oy = y + Facing::STEP_Y[d];
		int oz = z + Facing::STEP_Z[d];
		if (oy < 0 || oy >= Level::maxBuildHeight) continue;
		if (level->getTile(ox, oy, oz) != observerId) continue;

		int data = level->getData(ox, oy, oz);
		if (getFacing(data) != Facing::getOpposite(d)) continue;

		// Java ObserverBlock.updateShape -> startSignal(): ignored while already on or already scheduled
		if (isPowered(data)) continue;
		if (level->isTileToBeTickedAt(ox, oy, oz, observerId)) continue;

		level->addToTickNextTick(ox, oy, oz, observerId, PULSE_DELAY, 0);
	}
}
