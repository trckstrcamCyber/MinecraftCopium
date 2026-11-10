#pragma once

#include "Tile.h"

class Random;
class IconRegister;

class BlackStoneTile : public Tile
{
public:
    static const int BLACKSTONE = 0;
    static const int GILDED_BLACKSTONE = 1;
    static const int POLISHED_BLACKSTONE = 2;
    static const int POLISHED_BLACKSTONE_BRICKS = 3;
    static const int CHISELED_POLISHED_BLACKSTONE = 4;
    static const int CRACKED_POLISHED_BLACKSTONE_BRICKS = 5;

    static const int BLACKSTONE_NAMES_LENGTH = 6;

    static const unsigned int BLACKSTONE_NAMES[BLACKSTONE_NAMES_LENGTH];
    static const std::wstring TEXTURE_NAMES[];

private:
    Icon** icons;

public:
    BlackStoneTile(int id);

    virtual Icon* getTexture(int face, int data);
    virtual unsigned int getDescriptionId(int data = -1);
    virtual int getResource(int data, Random* random, int playerBonusLevel);
    virtual int getSpawnResourcesAuxValue(int data);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual int getPlacedOnFaceDataValue(
    Level* level,
    int x,
    int y,
    int z,
    int face,
    float clickX,
    float clickY,
    float clickZ,
    int data);
};

