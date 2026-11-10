#include "stdafx.h"
#include "IceBiome.h"
#include "net.minecraft.world.level.tile.h"
#include "IceSpikeFeature.h"
#include "SpruceFeature.h"
#include "Level.h"
#include "Random.h"

IceBiome::IceBiome(int id, bool isSpikes) : Biome(id)
{
    this->isSpikes = isSpikes;
    if (isSpikes)
    {
        topMaterial = static_cast<byte>(Tile::snow_Id);
    }
    friendlies.clear();

    // neoLegacy - the zombies of the cold biomes are Frostbites (snowy plains, ice spikes, frozen peaks). They follow the normal
    // hostile spawn rules, so they appear at night / in thunderstorms on the surface and in the dark underground.
    for (auto it = enemies.begin(); it != enemies.end();)
    {
        if ((*it)->mobClass == eTYPE_ZOMBIE) it = enemies.erase(it); else ++it;
    }
    enemies.push_back(new MobSpawnerData(eTYPE_FROSTBITE, 80, 4, 4));
    enemies.push_back(new MobSpawnerData(eTYPE_ICEOLOGER, 5, 1, 1));	// neoLegacy - the Iceologer of the FrozenFiend mod haunts snowy biomes
}

void IceBiome::decorate(Level *level, Random *random, int xo, int zo)
{
    if (this->isSpikes)
    {
        IceSpikeFeature iceSpikeFeature;
        for (int i = 0; i < 3; ++i)
        {
            int j = random->nextInt(16) + 8;
            int k = random->nextInt(16) + 8;
            int y = level->getHeightmap(xo + j, zo + k);

            iceSpikeFeature.place(level, random, xo + j, y, zo + k);
        }
    }
    Biome::decorate(level, random, xo, zo);
}

Feature* IceBiome::getTreeFeature(Random* random)
{
    return new SpruceFeature(false);
}