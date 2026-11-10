#include "stdafx.h"
#include "ObserverTileEntity.h"
#include "ObserverTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "Facing.h"

ObserverTileEntity::ObserverTileEntity()
{
	lastState = 0;
	initialized = false;
}

void ObserverTileEntity::save(CompoundTag *base)
{
	TileEntity::save(base);
	base->putInt(L"LastState", lastState);
	base->putInt(L"Init", initialized ? 1 : 0);
}

void ObserverTileEntity::load(CompoundTag *base)
{
	TileEntity::load(base);
	if (base->contains(L"LastState")) lastState = base->getInt(L"LastState");
	if (base->contains(L"Init")) initialized = base->getInt(L"Init") != 0;
}

void ObserverTileEntity::tick()
{
	// neoLegacy - the observer is an ordinary event-driven block now (see ObserverTile). This class only
	// remains so worlds saved with the old tile-entity observer still load; it removes itself on first tick.
	if (level == nullptr || level->isClientSide) return;
	level->removeTileEntity(x, y, z);
	setRemoved();
}

shared_ptr<TileEntity> ObserverTileEntity::clone()
{
	shared_ptr<ObserverTileEntity> result = std::make_shared<ObserverTileEntity>();
	TileEntity::clone(result);

	// Carried over so a piston-pushed observer keeps its existing baseline
	// instead of treating its own relocation as "the watched block changed".
	result->lastState = lastState;
	result->initialized = initialized;

	return result;
}