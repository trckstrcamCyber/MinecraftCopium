#pragma once

#include "Tile.h"

class IconRegister;
class Random;
class Level;

// One legacy tile ID with two metadata variants:
// 0 = Amethyst Block, 1 = Budding Amethyst.
class AmethystTile : public Tile
{
public:
    static const int AMETHYST_BLOCK = 0;
    static const int BUDDING_AMETHYST = 1;

    explicit AmethystTile(int id);

    virtual Icon* getTexture(int face, int data) override;
    virtual unsigned int getDescriptionId(int data = -1) override;
    virtual int getResource(int data, Random* random, int playerBonusLevel) override;
    virtual int getSpawnResourcesAuxValue(int data) override;
    virtual void registerIcons(IconRegister* iconRegister) override;
    virtual void tick(Level* level, int x, int y, int z, Random* random) override;

private:
    Icon* icons[2];
};

// Buds/clusters are a transparent cross-texture tile. Their metadata is the
// growth stage: 0 small, 1 medium, 2 large, 3 cluster.
class AmethystCrystalTile : public Tile
{
public:
    static const int SMALL = 0;
    static const int MEDIUM = 1;
    static const int LARGE = 2;
    static const int CLUSTER = 3;
    static const int STAGE_COUNT = 4;

    explicit AmethystCrystalTile(int id);

    virtual Icon* getTexture(int face, int data) override;
    virtual unsigned int getDescriptionId(int data = -1) override;
    virtual int getResource(int data, Random* random, int playerBonusLevel) override;
    virtual int getResourceCount(Random* random) override;
    virtual void registerIcons(IconRegister* iconRegister) override;
    virtual bool isCubeShaped() override;
    virtual int getRenderShape() override;
    virtual bool isSolidRender(bool isServerLevel = false) override;
    virtual bool blocksLight() override;
    virtual AABB* getAABB(Level* level, int x, int y, int z) override;
    virtual void tick(Level* level, int x, int y, int z, Random* random) override;
    virtual void neighborChanged(Level* level, int x, int y, int z, int type) override;

private:
    Icon* icons[STAGE_COUNT];
    bool hasBuddingSupport(Level* level, int x, int y, int z) const;
};
