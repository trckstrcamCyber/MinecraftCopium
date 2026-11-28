#pragma once
#include "Tile.h"
class PyreheartAltarTile : public Tile
{
public:
    PyreheartAltarTile(int id) : Tile(id, Material::stone) {}
    virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player,
        int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false);
};
