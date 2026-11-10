#include "stdafx.h"
#include "BlackStoneTile.h"
#include "IconRegister.h"
#include "net.minecraft.world.h"
#include "net.minecraft.h"

BlackStoneTile::BlackStoneTile(int id)
    : Tile(id, Material::stone)
{
    icons = nullptr;
}

const unsigned int BlackStoneTile::BLACKSTONE_NAMES[BLACKSTONE_NAMES_LENGTH] =
{
    IDS_TILE_BLACKSTONE,
    IDS_TILE_GILDED_BLACKSTONE,
    IDS_TILE_POLISHED_BLACKSTONE,
    IDS_TILE_POLISHED_BLACKSTONE_BRICKS,
    IDS_TILE_CHISELED_POLISHED_BLACKSTONE,
    IDS_TILE_CRACKED_POLISHED_BLACKSTONE_BRICKS
};

const std::wstring BlackStoneTile::TEXTURE_NAMES[] =
{
    L"blackstone",
    L"gilded_blackstone",
    L"polished_blackstone",
    L"polished_blackstone_bricks",
    L"chiseled_polished_blackstone",
    L"cracked_polished_blackstone_bricks"
};

unsigned int BlackStoneTile::getDescriptionId(int data)
{
    if (data < 0 || data >= BLACKSTONE_NAMES_LENGTH)
        data = 0;

    return BLACKSTONE_NAMES[data];
}

int BlackStoneTile::getResource(int data, Random* random, int playerBonusLevel)
{
    if (data < 0 || data >= BLACKSTONE_NAMES_LENGTH)
        data = 0;

    return Tile::blackstone_block_Id;
}

int BlackStoneTile::getSpawnResourcesAuxValue(int data)
{
    if (data < 0 || data >= BLACKSTONE_NAMES_LENGTH)
        data = 0;

    return data;
}

Icon* BlackStoneTile::getTexture(int face, int data)
{
    if (data < 0 || data >= BLACKSTONE_NAMES_LENGTH)
        data = 0;

    return icons[data];
}

void BlackStoneTile::registerIcons(IconRegister* iconRegister)
{
    icons = new Icon*[BLACKSTONE_NAMES_LENGTH];

    for (int i = 0; i < BLACKSTONE_NAMES_LENGTH; i++)
    {
        icons[i] = iconRegister->registerIcon(TEXTURE_NAMES[i]);
    }
}

int BlackStoneTile::getPlacedOnFaceDataValue(
    Level*,
    int,
    int,
    int,
    int,
    float,
    float,
    float,
    int data)
{
    return data;
}