#pragma once
#include "Tile.h"

class CalciteTile : public Tile
{
private:
    Icon* icon;
public:
    CalciteTile(int id);
    virtual Icon* getTexture(int face, int data);
    virtual void registerIcons(IconRegister* iconRegister);
};
