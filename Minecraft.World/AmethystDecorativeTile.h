#pragma once

#include "Tile.h"
#include "WallTile.h"

class IconRegister;

// Single-texture decorative block used by the Better Amethyst set.
class AmethystDecorativeTile : public Tile
{
private:
    std::wstring textureName;
    Icon* icon;

public:
    AmethystDecorativeTile(int id, const std::wstring& texture);
    virtual Icon* getTexture(int face, int data) override;
    virtual void registerIcons(IconRegister* iconRegister) override;
};

class AmethystBrickWallTile : public WallTile
{
public:
    AmethystBrickWallTile(int id, Tile* baseTile);
    virtual Icon* getTexture(int face, int data) override;
};
