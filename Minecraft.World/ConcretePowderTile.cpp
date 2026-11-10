#include "stdafx.h"
#include "net.minecraft.world.h"
#include "ConcretePowderTile.h"
#include "DyePowderItem.h"
#include "Level.h"

ConcretePowderTile::ConcretePowderTile(int id)
    : HeavyTile(id, Material::sand)
{
    for (int i = 0; i < COLOR_COUNT; ++i)
        icons[i] = nullptr;
}

bool ConcretePowderTile::touchingWater(Level *level, int x, int y, int z) const
{
    // Material::water covers still and flowing water in this engine.
    if (level->getMaterial(x, y, z) == Material::water)
        return true;

    static const int OFFSETS[6][3] =
    {
        { 0, -1, 0 }, { 0, 1, 0 },
        { 0, 0, -1 }, { 0, 0, 1 },
        {-1, 0, 0 }, { 1, 0, 0 }
    };

    for (int i = 0; i < 6; ++i)
    {
        if (level->getMaterial(
                x + OFFSETS[i][0],
                y + OFFSETS[i][1],
                z + OFFSETS[i][2]) == Material::water)
            return true;
    }

    return false;
}

void ConcretePowderTile::harden(Level *level, int x, int y, int z)
{
    if (level->isClientSide) return;
    if (level->getTile(x, y, z) != id) return;
    if (!touchingWater(level, x, y, z)) return;

    int data = level->getData(x, y, z);
    level->setTileAndData(
        x, y, z, Tile::concrete_Id, data, Tile::UPDATE_ALL);
}

void ConcretePowderTile::onPlace(Level *level, int x, int y, int z)
{
    if (touchingWater(level, x, y, z))
    {
        harden(level, x, y, z);
        return;
    }

    HeavyTile::onPlace(level, x, y, z);
}

void ConcretePowderTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
    if (touchingWater(level, x, y, z))
    {
        harden(level, x, y, z);
        return;
    }

    HeavyTile::neighborChanged(level, x, y, z, type);
}

void ConcretePowderTile::tick(Level *level, int x, int y, int z, Random *random)
{
    if (touchingWater(level, x, y, z))
    {
        harden(level, x, y, z);
        return;
    }

    HeavyTile::tick(level, x, y, z, random);
}

void ConcretePowderTile::onLand(Level *level, int x, int y, int z, int data)
{
    if (touchingWater(level, x, y, z))
    {
        level->setTileAndData(
            x, y, z, Tile::concrete_Id, data, Tile::UPDATE_ALL);
    }
}

Icon *ConcretePowderTile::getTexture(int face, int data)
{
    (void)face;
    data = Mth::clamp(data, 0, COLOR_COUNT - 1);
    return icons[data];
}

int ConcretePowderTile::getSpawnResourcesAuxValue(int data)
{
    return Mth::clamp(data, 0, COLOR_COUNT - 1);
}

void ConcretePowderTile::registerIcons(IconRegister *iconRegister)
{
    for (int i = 0; i < COLOR_COUNT; ++i)
        icons[i] = iconRegister->registerIcon(
            L"concrete_powder_" + DyePowderItem::COLOR_TEXTURES[i]);
}
