#pragma once

#include "Tile.h"

// Detects changes to the block it is facing and emits a short redstone pulse from the opposite (back/dot) face.
//
// neoLegacy - rewritten to behave like the Java Edition observer:
//   * It is a plain block (no tile entity), so pistons can push/pull it - which flying machines need.
//   * Detection is event driven. Level::setTileAndData / setData call ObserverTile::notifyWatchers() for every
//     change at a position, which schedules the observers that are looking at that position.
//   * Timing is the Java timing: the output turns ON 2 ticks after the change and stays ON for 2 ticks.
//   * A moved observer that arrives powered switches itself off straight away (Java onPlace behaviour).
//
// Data layout:
//   bits 0-2 : facing, one of Facing::{DOWN,UP,NORTH,SOUTH,WEST,EAST}  (the side the observer looks at)
//   bit  3   : powered (the pulse is currently active)
class ObserverTile : public Tile
{
public:
	static const int FACING_MASK = 0x7;
	static const int POWERED_BIT = 0x8;
	static const int PULSE_DELAY = 2;

	ObserverTile(int id);

	virtual bool isCubeShaped() override { return true; }
	virtual bool isSolidRender(bool isServerLevel = false) override { return true; }

	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance) override;
	virtual void onPlace(Level *level, int x, int y, int z) override;
	virtual void onRemove(Level *level, int x, int y, int z, int id, int data) override;

	virtual Icon *getTexture(int face, int data) override;
	virtual void registerIcons(IconRegister *iconRegister) override;

	virtual bool isSignalSource() override { return true; }
	virtual int getSignal(LevelSource *level, int x, int y, int z, int dir) override;
	virtual int getDirectSignal(LevelSource *level, int x, int y, int z, int dir) override;

	// scheduled tick: first call turns the pulse ON (and schedules the OFF), second call turns it OFF
	virtual void tick(Level *level, int x, int y, int z, Random *random) override;

	// Called by Level whenever the block at (x,y,z) changed: wakes every observer that is looking at that position.
	static void notifyWatchers(Level *level, int x, int y, int z);

	// Called when a piston finishes moving an observer to a new position. Java runs updateShape() on the moved block for
	// every neighbour (Block.updateFromNeighbourShapes), so an observer that lands un-powered always starts its own pulse.
	// This is what makes the classic two-observer flying machine keep going. movedData = the data it had while moving.
	static void onMovedByPiston(Level *level, int x, int y, int z, int movedData);

	static int getFacing(int data) { return data & FACING_MASK; }
	static bool isPowered(int data) { return (data & POWERED_BIT) != 0; }

private:
	void updateNeighborsInFront(Level *level, int x, int y, int z, int data);

	Icon *frontIcon;
	Icon *backIcon;
	Icon *backOnIcon;
	Icon *sideIcon;
	Icon *topIcon;
};
