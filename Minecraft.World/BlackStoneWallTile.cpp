#include "stdafx.h"
#include "BlackStoneWallTile.h"
#include "net.minecraft.world.level.tile.h"

BlackStoneWallTile::BlackStoneWallTile(int id, Tile* baseTile)
    : WallTile(id, baseTile)
{
}

Icon* BlackStoneWallTile::getTexture(int face, int data)
{
    return Tile::blackstone->getTexture(face);
}