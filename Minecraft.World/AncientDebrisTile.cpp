#include "stdafx.h"
#include "AncientDebrisTile.h"
#include "IconRegister.h"
#include "net.minecraft.world.h"
#include "net.minecraft.h"

AncientDebrisTile::AncientDebrisTile(int id)
    : OreTile(id)
{
    icons[0] = nullptr;
    icons[1] = nullptr;
}

Icon* AncientDebrisTile::getTexture(int face, int)
{
    if (face == Facing::UP || face == Facing::DOWN)
        return icons[1];
    return icons[0];
}

void AncientDebrisTile::registerIcons(IconRegister* iconRegister)
{
    icons[0] = iconRegister->registerIcon(L"ancient_debris_side");
    icons[1] = iconRegister->registerIcon(L"ancient_debris_top");
}
