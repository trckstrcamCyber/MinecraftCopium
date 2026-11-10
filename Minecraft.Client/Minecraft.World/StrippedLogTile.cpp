#include "stdafx.h"
#include "StrippedLogTile.h"
#include "IconRegister.h"

const unsigned int StrippedLogTile::NAMES[WOOD_COUNT] = {
	IDS_TILE_STRIPPED_OAK_LOG, IDS_TILE_STRIPPED_SPRUCE_LOG, IDS_TILE_STRIPPED_BIRCH_LOG, IDS_TILE_STRIPPED_JUNGLE_LOG,
	IDS_TILE_STRIPPED_ACACIA_LOG, IDS_TILE_STRIPPED_DARK_OAK_LOG, IDS_TILE_STRIPPED_BAMBOO_BLOCK
};

// Every wood now has its own stripped art. (Flip a wood back to false to make it borrow the normal log texture instead.)
const bool StrippedLogTile::HAS_ART[WOOD_COUNT] = { true, true, true, true, true, true, true };

const wstring StrippedLogTile::SIDE_NAMES[WOOD_COUNT] = {
	L"stripped_oak_log", L"stripped_spruce_log", L"stripped_birch_log", L"stripped_jungle_log",
	L"stripped_acacia_log", L"stripped_dark_oak_log", L"stripped_bamboo_block"
};
const wstring StrippedLogTile::TOP_NAMES[WOOD_COUNT] = {
	L"stripped_oak_log_top", L"stripped_spruce_log_top", L"stripped_birch_log_top", L"stripped_jungle_log_top",
	L"stripped_acacia_log_top", L"stripped_dark_oak_log_top", L"stripped_bamboo_block_top"
};

StrippedLogTile::StrippedLogTile(int id, int firstType) : RotatedPillarTile(id, Material::wood), firstType(firstType)
{
	for (int i = 0; i < 4; ++i) sideIcons[i] = topIcons[i] = nullptr;
}

int StrippedLogTile::woodIndex(int type) const
{
	int wood = firstType + (type & MASK_TYPE);
	if (wood >= WOOD_COUNT) wood = WOOD_COUNT - 1;
	return wood;
}

unsigned int StrippedLogTile::getDescriptionId(int iData)
{
	if (iData < 0) iData = 0;
	return NAMES[woodIndex(iData)];
}

void StrippedLogTile::registerIcons(IconRegister *iconRegister)
{
	for (int type = 0; type < 4; ++type)
	{
		int wood = woodIndex(type);
		if (!HAS_ART[wood]) continue;
		sideIcons[type] = iconRegister->registerIcon(SIDE_NAMES[wood]);
		topIcons[type] = iconRegister->registerIcon(TOP_NAMES[wood]);
	}
}

// a wood without stripped art borrows the normal log's texture
Icon *StrippedLogTile::fallbackIcon(int wood, int face) const
{
	if (wood < 4) return Tile::treeTrunk->getTexture(face, wood);
	return Tile::log2->getTexture(face, wood - 4);
}

Icon *StrippedLogTile::getTypeTexture(int type)
{
	int t = type & MASK_TYPE;
	if (sideIcons[t] != nullptr) return sideIcons[t];
	return fallbackIcon(woodIndex(type), 2);
}

Icon *StrippedLogTile::getTopTexture(int type)
{
	int t = type & MASK_TYPE;
	if (topIcons[t] != nullptr) return topIcons[t];
	return fallbackIcon(woodIndex(type), 1);
}
