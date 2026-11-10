#pragma once

#include "Tile.h"

// Solid 16-color concrete. Tile data is the dye/color index 0..15.
class ConcreteTile : public Tile
{
private:
    static const int COLOR_COUNT = 16;
    Icon *icons[COLOR_COUNT];

public:
    ConcreteTile(int id);

    virtual Icon *getTexture(int face, int data) override;
    virtual int getSpawnResourcesAuxValue(int data) override;
    // NOTE: intentionally no getColor() override. The concrete_<color> textures are already
    // colored, so tinting them by DyePowderItem::COLOR_RGB double-applied the color in the
    // inventory (getColor(aux) path) and made it darker than the placed block (the in-world
    // getColor(level,x,y,z) path was never overridden, so it was untinted).
    virtual void registerIcons(IconRegister *iconRegister) override;
};
