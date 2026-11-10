#include "stdafx.h"
#include "CalciteTile.h"
#include "IconRegister.h"

CalciteTile::CalciteTile(int id)
    : Tile(id, Material::stone)
{
    icon = nullptr;
}

Icon* CalciteTile::getTexture(int face, int data)
{
    return icon;
}

void CalciteTile::registerIcons(IconRegister* iconRegister)
{
    icon = iconRegister->registerIcon(L"calcite");
}