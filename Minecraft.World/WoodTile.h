#pragma once
#include "Tile.h"

class ChunkRebuildData;

class WoodTile : public Tile
{
friend class ChunkRebuildData;
public:
	static const int WOOD_NAMES_LENGTH = 8;	// neoLegacy: + bamboo planks (6) and bamboo mosaic (7)
	static const int BAMBOO_PLANKS = 6;
	static const int BAMBOO_MOSAIC = 7;


	static const unsigned int WOOD_NAMES[WOOD_NAMES_LENGTH];
	static const wstring TEXTURE_NAMES[];

private:
	Icon **icons;

public:
	WoodTile(int id);
    virtual unsigned int getDescriptionId(int iData = -1);
	virtual Icon *getTexture(int face, int data) ;
	virtual int getSpawnResourcesAuxValue(int data);

	void registerIcons(IconRegister *iconRegister);
};