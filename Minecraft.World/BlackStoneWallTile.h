#pragma once

#include "WallTile.h"

class BlackStoneWallTile : public WallTile
{
public:
    BlackStoneWallTile(int id, Tile* baseTile);

    virtual Icon* getTexture(int face, int data);
};