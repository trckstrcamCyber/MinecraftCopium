#include "stdafx.h"
#include "CalciteWallTile.h"
#include "net.minecraft.world.level.tile.h"

CalciteWallTile::CalciteWallTile(int id, Tile* baseTile)
    : WallTile(id, baseTile)
{
}

Icon* CalciteWallTile::getTexture(int face, int data)
{
    return Tile::calcite->getTexture(face);
}