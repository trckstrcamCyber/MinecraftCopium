#include "stdafx.h"
#include "net.minecraft.world.h"
#include "ConcreteTile.h"
#include "DyePowderItem.h"

ConcreteTile::ConcreteTile(int id) : Tile(id, Material::stone)
{
    for (int i = 0; i < COLOR_COUNT; ++i)
        icons[i] = nullptr;
}

Icon *ConcreteTile::getTexture(int face, int data)
{
    (void)face;
    data = Mth::clamp(data, 0, COLOR_COUNT - 1);
    return icons[data];
}

int ConcreteTile::getSpawnResourcesAuxValue(int data)
{
    return Mth::clamp(data, 0, COLOR_COUNT - 1);
}

void ConcreteTile::registerIcons(IconRegister *iconRegister)
{
    for (int i = 0; i < COLOR_COUNT; ++i)
        icons[i] = iconRegister->registerIcon(
            L"concrete_" + DyePowderItem::COLOR_TEXTURES[i]);
}
