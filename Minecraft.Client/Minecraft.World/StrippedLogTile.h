#pragma once
#include "RotatedPillarTile.h"

class IconRegister;

// neoLegacy - ONE tile family for every stripped log, instead of one tile id per wood.
//
// Like the normal logs, a tile has 4 wood types in the low data bits (MASK_TYPE) and the axis in the next two (MASK_FACING),
// so two tiles cover everything:
//     stripped_log   (Tile::strippedLog)   oak, spruce, birch, jungle
//     stripped_log2  (Tile::strippedLog2)  acacia, dark oak, bamboo
// The "global" wood index (0..6) is  firstType + (data & MASK_TYPE).
//
// All 7 woods have stripped art (terrain.png row 31, registered in PreStitchedTextureMap). A wood whose HAS_ART flag is false in
// StrippedLogTile.cpp simply shows its normal log texture instead.
// Axes strip logs / Block of Bamboo in HatchetItem::useOn, keeping the axis the block was placed on.
class StrippedLogTile : public RotatedPillarTile
{
public:
	static const int WOOD_COUNT = 7;
	enum { OAK = 0, SPRUCE, BIRCH, JUNGLE, ACACIA, DARK_OAK, BAMBOO };	// global wood index

	static const unsigned int NAMES[WOOD_COUNT];		// description string ids
	static const bool HAS_ART[WOOD_COUNT];
	static const wstring SIDE_NAMES[WOOD_COUNT];
	static const wstring TOP_NAMES[WOOD_COUNT];

private:
	int firstType;				// 0 for stripped_log, 4 for stripped_log2
	Icon *sideIcons[4];
	Icon *topIcons[4];

	int woodIndex(int type) const;
	Icon *fallbackIcon(int wood, int face) const;

public:
	StrippedLogTile(int id, int firstType);

	virtual unsigned int getDescriptionId(int iData = -1);
	virtual void registerIcons(IconRegister *iconRegister);

protected:
	virtual Icon *getTypeTexture(int type);
	virtual Icon *getTopTexture(int type);
};
