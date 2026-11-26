#include "stdafx.h"
#include "AmethystDecorativeTile.h"
#include "IconRegister.h"
#include "net.minecraft.world.level.tile.h"

AmethystDecorativeTile::AmethystDecorativeTile(int id, const std::wstring& texture)
    : Tile(id, Material::stone), textureName(texture), icon(nullptr)
{
}

Icon* AmethystDecorativeTile::getTexture(int, int)
{
    return icon;
}

void AmethystDecorativeTile::registerIcons(IconRegister* iconRegister)
{
    icon = iconRegister->registerIcon(textureName);
}

AmethystBrickWallTile::AmethystBrickWallTile(int id, Tile* baseTile)
    : WallTile(id, baseTile)
{
}

Icon* AmethystBrickWallTile::getTexture(int face, int)
{
    return Tile::amethystBricks != nullptr ? Tile::amethystBricks->getTexture(face, 0) : nullptr;
}
