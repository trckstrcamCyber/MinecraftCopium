#include "stdafx.h"
#include "ItemInstance.h"
#include "DrillTileEntity.h"
#include "DrillTile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.material.h"
#include "Facing.h"

DrillTileEntity::DrillTileEntity()
{
	progress = 0.0f;
	targetState = -1;
	crackStage = -1;
}

// The crack overlay is keyed by an id (normally the mining player's entity id). Give each drill
// its own large id derived from its position so two drills never fight over one overlay and it
// can never collide with a real entity id.
int DrillTileEntity::getCrackId()
{
	return 0x40000000 | ((x & 0x3FF) << 20) | ((z & 0x3FF) << 10) | (y & 0x3FF);
}

void DrillTileEntity::stopMining()
{
	if (crackStage >= 0 && level != nullptr)
	{
		level->destroyTileProgress(getCrackId(), x, y, z, -1);
	}
	progress = 0.0f;
	targetState = -1;
	crackStage = -1;
}

void DrillTileEntity::tick()
{
	// neoLegacy - the drill is a plain block now (see DrillTile); this class only remains so worlds saved with the old tile-entity
	// drill still load. It removes itself on the first tick.
	TileEntity::tick();
	if (level == nullptr || level->isClientSide) return;
	level->removeTileEntity(x, y, z);
	setRemoved();
}

void DrillTileEntity::setRemoved()
{
	// Don't leave a crack overlay hanging on the target when the drill is broken/unloaded.
	if (level != nullptr && !level->isClientSide)
	{
		stopMining();
	}
	TileEntity::setRemoved();
}

shared_ptr<TileEntity> DrillTileEntity::clone()
{
	shared_ptr<DrillTileEntity> result = std::make_shared<DrillTileEntity>();
	TileEntity::clone(result);
	// A piston-moved drill starts mining fresh; nothing to carry over.
	return result;
}
