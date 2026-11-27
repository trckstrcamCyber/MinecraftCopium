#include "stdafx.h"
#include "AmethystTile.h"
#include "Item.h"
#include "IconRegister.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.h"

namespace
{
    const int AMETHYST_NEIGHBOURS[6][3] =
    {
        { 1, 0, 0 }, { -1, 0, 0 },
        { 0, 1, 0 }, { 0, -1, 0 },
        { 0, 0, 1 }, { 0, 0, -1 }
    };
}

AmethystTile::AmethystTile(int id)
    : Tile(id, Material::stone)
{
    icons[AMETHYST_BLOCK] = nullptr;
    icons[BUDDING_AMETHYST] = nullptr;
    // The tile tick is used only by metadata 1 (budding amethyst).
    setTicking(true);
}

Icon* AmethystTile::getTexture(int, int data)
{
    if (data != BUDDING_AMETHYST)
        data = AMETHYST_BLOCK;
    return icons[data];
}

unsigned int AmethystTile::getDescriptionId(int data)
{
    return data == BUDDING_AMETHYST
        ? IDS_TILE_BUDDING_AMETHYST
        : IDS_TILE_AMETHYST_BLOCK;
}

int AmethystTile::getResource(int data, Random*, int)
{
    // Budding amethyst is never obtainable, including with Silk Touch.
    return data == BUDDING_AMETHYST ? Tile::air_Id : Tile::amethyst_stone_Id;
}

int AmethystTile::getSpawnResourcesAuxValue(int)
{
    return AMETHYST_BLOCK;
}

void AmethystTile::registerIcons(IconRegister* iconRegister)
{
    icons[AMETHYST_BLOCK] = iconRegister->registerIcon(L"amethyst_block");
    icons[BUDDING_AMETHYST] = iconRegister->registerIcon(L"budding_amethyst");
}

void AmethystTile::tick(Level* level, int x, int y, int z, Random* random)
{
    if (level == nullptr || random == nullptr ||
        level->getData(x, y, z) != BUDDING_AMETHYST)
        return;

    // A random tick has a 1-in-5 chance to attempt a new bud. The crystal
    // tile then advances through the four visual stages on its own ticks.
    if (random->nextInt(5) != 0)
        return;

    const int start = random->nextInt(6);
    for (int i = 0; i < 6; ++i)
    {
        const int* direction = AMETHYST_NEIGHBOURS[(start + i) % 6];
        const int px = x + direction[0];
        const int py = y + direction[1];
        const int pz = z + direction[2];
        if (level->getTile(px, py, pz) == Tile::air_Id)
        {
            level->setTileAndData(px, py, pz, Tile::amethyst_crystal_Id,
                AmethystCrystalTile::SMALL, Tile::UPDATE_ALL);
            return;
        }
    }
}

AmethystCrystalTile::AmethystCrystalTile(int id)
    : Tile(id, Material::plant, false)
{
    for (int i = 0; i < STAGE_COUNT; ++i)
        icons[i] = nullptr;

    setTicking(true);
    updateDefaultShape();
}

Icon* AmethystCrystalTile::getTexture(int, int data)
{
    if (data < SMALL || data >= STAGE_COUNT)
        data = SMALL;
    return icons[data];
}

unsigned int AmethystCrystalTile::getDescriptionId(int)
{
    return IDS_TILE_AMETHYST_CLUSTER;
}

int AmethystCrystalTile::getResource(int data, Random*, int)
{
    // Only fully grown clusters drop shards. Small, medium and large buds stay attached.
    return data == CLUSTER ? Item::amethyst_shard_Id : Tile::air_Id;
}

int AmethystCrystalTile::getResourceCount(Random*)
{
    // Vanilla-style base yield for a fully grown cluster. The engine applies its normal
    // harvest/fortune handling after the tile selects the resource ID.
    return 4;
}

void AmethystCrystalTile::registerIcons(IconRegister* iconRegister)
{
    icons[SMALL] = iconRegister->registerIcon(L"small_amethyst_bud");
    icons[MEDIUM] = iconRegister->registerIcon(L"medium_amethyst_bud");
    icons[LARGE] = iconRegister->registerIcon(L"large_amethyst_bud");
    icons[CLUSTER] = iconRegister->registerIcon(L"amethyst_cluster");
}

bool AmethystCrystalTile::isCubeShaped()
{
    return false;
}

int AmethystCrystalTile::getRenderShape()
{
    return Tile::SHAPE_CROSS_TEXTURE;
}

bool AmethystCrystalTile::isSolidRender(bool)
{
    return false;
}

bool AmethystCrystalTile::blocksLight()
{
    return false;
}

AABB* AmethystCrystalTile::getAABB(Level*, int, int, int)
{
    return nullptr;
}

bool AmethystCrystalTile::hasBuddingSupport(Level* level, int x, int y, int z) const
{
    for (int i = 0; i < 6; ++i)
    {
        const int* direction = AMETHYST_NEIGHBOURS[i];
        const int px = x + direction[0];
        const int py = y + direction[1];
        const int pz = z + direction[2];
        if (level->getTile(px, py, pz) == Tile::amethyst_stone_Id &&
            level->getData(px, py, pz) == AmethystTile::BUDDING_AMETHYST)
            return true;
    }
    return false;
}

void AmethystCrystalTile::tick(Level* level, int x, int y, int z, Random* random)
{
    if (level == nullptr || !hasBuddingSupport(level, x, y, z))
    {
        if (level != nullptr)
            level->setTileAndData(x, y, z, Tile::air_Id, 0, Tile::UPDATE_CLIENTS);
        return;
    }

    if (random == nullptr || random->nextInt(5) != 0)
        return;

    const int stage = level->getData(x, y, z);
    if (stage < CLUSTER)
        level->setData(x, y, z, stage + 1, Tile::UPDATE_ALL);
}

void AmethystCrystalTile::neighborChanged(Level* level, int x, int y, int z, int)
{
    if (!hasBuddingSupport(level, x, y, z))
        level->setTileAndData(x, y, z, Tile::air_Id, 0, Tile::UPDATE_CLIENTS);
}
