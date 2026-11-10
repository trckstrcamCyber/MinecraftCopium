#pragma once

#include "WallTile.h"

class CalciteWallTile : public WallTile
{
public:
    CalciteWallTile(int id, Tile* baseTile);

    virtual Icon* getTexture(int face, int data);
};