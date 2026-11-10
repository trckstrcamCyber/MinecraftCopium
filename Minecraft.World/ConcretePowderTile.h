#pragma once

#include "HeavyTile.h"

// Gravity-affected 16-color concrete powder.
// Tile data is the dye/color index 0..15.
class ConcretePowderTile : public HeavyTile
{
private:
    static const int COLOR_COUNT = 16;
    Icon *icons[COLOR_COUNT];

    bool touchingWater(Level *level, int x, int y, int z) const;
    void harden(Level *level, int x, int y, int z);

public:
    ConcretePowderTile(int id);

    virtual void onPlace(Level *level, int x, int y, int z) override;
    virtual void neighborChanged(Level *level, int x, int y, int z, int type) override;
    virtual void tick(Level *level, int x, int y, int z, Random *random) override;
    virtual void onLand(Level *level, int x, int y, int z, int data) override;

    virtual Icon *getTexture(int face, int data) override;
    virtual int getSpawnResourcesAuxValue(int data) override;
    // NOTE: intentionally no getColor() override -- see ConcreteTile.h.
    virtual void registerIcons(IconRegister *iconRegister) override;
};
