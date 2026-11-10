#pragma once
#include "TileEntity.h"

class ObserverTileEntity : public TileEntity
{
public:
    eINSTANCEOF GetType() { return eTYPE_OBSERVERTILEENTITY; }
    static TileEntity *create() { return new ObserverTileEntity(); }

private:
    int lastState;		// packed (watchedTileId << 4) | watchedData
    bool initialized;	// false until the first tick has taken a baseline reading

public:
    ObserverTileEntity();

    virtual void save(CompoundTag *base);
    virtual void load(CompoundTag *base);
    virtual void tick();
    virtual shared_ptr<TileEntity> clone();
};