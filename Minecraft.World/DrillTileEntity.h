#pragma once
#include "TileEntity.h"

class ItemInstance;

// neoLegacy - the mining logic for DrillTile.
//
// Every game tick, while the drill is powered, it advances a break-progress counter on the
// block in front of it using the SAME numbers a player would get:
//
//     progress per tick = (tool speed / block hardness) / 30      (Tile::getDestroyProgress)
//
// where the tool is a real iron pickaxe, so the speed (and which blocks are harvestable at
// all) come straight from the engine's own iron pickaxe rules instead of a copy of them.
// Progress resets whenever power is lost or the target block changes.
class DrillTileEntity : public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_DRILLTILEENTITY; }
	static TileEntity *create() { return new DrillTileEntity(); }

private:
	float progress;						// 0..1 break progress on the current target
	int targetState;					// packed (tileId << 4) | data of the block being mined, -1 = none
	int crackStage;						// last crack-overlay stage sent to clients, -1 = none
	shared_ptr<ItemInstance> pickaxe;	// the "tool" the drill mines with (built on first use)

	int getCrackId();
	void stopMining();

public:
	DrillTileEntity();

	virtual void tick();
	virtual void setRemoved();
	virtual shared_ptr<TileEntity> clone();
};
