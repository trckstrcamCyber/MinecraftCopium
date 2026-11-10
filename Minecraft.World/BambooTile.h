#pragma once
#include "Tile.h"

class IconRegister;
class Random;

// neoLegacy - the Bamboo plant (Java Edition rules).
//
// data bits:  1 = AGE (0 thin stalk, 1 thick stalk)   2 = STAGE (1 = finished growing)   0xC = LEAVES (0 none, 4 small, 8 large)
//
//  - Grows on dirt, grass, sand, gravel, mycelium (coarse dirt, podzol and red sand are data variants of those) and on other bamboo
//  - The top block needs light level 9+ to grow; a stalk grows to 12-16 blocks
//  - Breaking any block pops everything above it (the support is lost), a piston pushing it breaks it
//  - Material is wood, so flowing water does not wash a stalk away (it does wash away a BambooShootTile)
class BambooTile : public Tile
{
public:
	static const int AGE_BIT = 1;
	static const int STAGE_BIT = 2;
	static const int LEAVES_MASK = 0xC;
	static const int LEAVES_SHIFT = 2;
	static const int LEAVES_NONE = 0;
	static const int LEAVES_SMALL = 1;
	static const int LEAVES_LARGE = 2;
	static const int MAX_HEIGHT = 16;
	static const int MIN_LIGHT_TO_GROW = 9;

private:
	Icon *stalkIcon;
	Icon *smallLeavesIcon;
	Icon *largeLeavesIcon;

public:
	BambooTile(int id);

	// can this tile id carry a bamboo plant or shoot?
	static bool isValidSoil(int tileId);
	// the same small random offset Java gives the stalk, from the position
	static void getOffset(int x, int z, float &offsetX, float &offsetZ);
	Icon *getLeavesIcon(int leaves);

	virtual void registerIcons(IconRegister *iconRegister);
	virtual Icon *getTexture(int face, int data);
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isCubeShaped();
	virtual bool blocksLight();
	virtual int getRenderShape();
	virtual void updateDefaultShape();
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual AABB *getTileAABB(Level *level, int x, int y, int z);
	virtual int getPistonPushReaction();
	virtual int getResource(int data, Random *random, int playerBonusLevel);
	virtual bool mayPlace(Level *level, int x, int y, int z);
	virtual bool canSurvive(Level *level, int x, int y, int z);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual void tick(Level *level, int x, int y, int z, Random *random);

private:
	int countBambooBelow(Level *level, int x, int y, int z);
	void growBamboo(Level *level, int x, int y, int z, Random *random, int height);
};
