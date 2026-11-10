#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "Facing.h"
#include "GenericStats.h"
#include "IconRegister.h"
#include "BedTileEntity.h"
#include "BedItem.h"

// Same order as the wool / block color ids, so a bed's variant equals the wool aux it is crafted from.
const wchar_t *const BedItem::VARIANT_NAMES[BedItem::ICON_COUNT] =
{
	L"white", L"orange", L"magenta", L"light_blue", L"yellow", L"lime", L"pink", L"gray", L"light_gray",
	L"cyan", L"purple", L"blue", L"brown", L"green", L"red", L"black", L"straw"
};

const unsigned int BedItem::VARIANT_DESCS[BedItem::ICON_COUNT] =
{
	IDS_ITEM_BED_WHITE, IDS_ITEM_BED_ORANGE, IDS_ITEM_BED_MAGENTA, IDS_ITEM_BED_LIGHT_BLUE, IDS_ITEM_BED_YELLOW, IDS_ITEM_BED_LIME,
	IDS_ITEM_BED_PINK, IDS_ITEM_BED_GRAY, IDS_ITEM_BED_LIGHT_GRAY, IDS_ITEM_BED_CYAN, IDS_ITEM_BED_PURPLE, IDS_ITEM_BED_BLUE,
	IDS_ITEM_BED_BROWN, IDS_ITEM_BED_GREEN, IDS_ITEM_BED_RED, IDS_ITEM_BED_BLACK, IDS_ITEM_BED_STRAW
};

static int clampVariant(int aux)
{
	return (aux >= 0 && aux < BedItem::ICON_COUNT) ? aux : 0;
}

BedItem::BedItem(int id) : Item( id )
{
	setStackedByData(true);	// a white bed and a red bed must not merge, and straw beds stack among themselves
	setBaseItemTypeAndMaterial(Item::eBaseItemType_bed, Item::eMaterial_cloth);	// one crafting-menu slot for all 17 bed recipes (like boats / signs)
	icons = nullptr;
}

Icon *BedItem::getIcon(int auxValue)
{
	return icons[clampVariant(auxValue)];
}

void BedItem::registerIcons(IconRegister *iconRegister)
{
	icons = new Icon *[ICON_COUNT];
	for (int i = 0; i < ICON_COUNT; i++)
	{
		icons[i] = iconRegister->registerIcon(wstring(L"bed_") + VARIANT_NAMES[i]);
	}
}

unsigned int BedItem::getDescriptionId(int iData)
{
	if (iData < 0) return IDS_ITEM_BED;
	return VARIANT_DESCS[clampVariant(iData)];
}

unsigned int BedItem::getDescriptionId(shared_ptr<ItemInstance> instance)
{
	return VARIANT_DESCS[clampVariant(instance->getAuxValue())];
}

int BedItem::getMaxStackSize(const ItemInstance *instance) const
{
	if (instance != nullptr && instance->getAuxValue() == BedTileEntity::VARIANT_STRAW) return 16;
	return getMaxStackSize();
}

bool BedItem::useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	if (level->isClientSide) return true;

	if (face != Facing::UP)
	{
		return false;
	}

	// place on top of tile
	y = y + 1;

	BedTile *tile = static_cast<BedTile *>(Tile::bed);

	int dir = ( Mth::floor(player->yRot * 4 / (360) + 0.5f) ) & 3;
	int xra = 0;
	int zra = 0;

	if (dir == Direction::SOUTH) zra = 1;
	if (dir == Direction::WEST) xra = -1;
	if (dir == Direction::NORTH) zra = -1;
	if (dir == Direction::EAST) xra = 1;

	if (!player->mayUseItemAt(x, y, z, face, itemInstance) || !player->mayUseItemAt(x + xra, y, z + zra, face, itemInstance)) return false;

	if (level->isEmptyTile(x, y, z) && level->isEmptyTile(x + xra, y, z + zra) && level->isTopSolidBlocking(x, y - 1, z) && level->isTopSolidBlocking(x + xra, y - 1, z + zra))
	{
		// 4J-PB - Adding a test only version to allow tooltips to be displayed
		if(!bTestUseOnOnly)
		{
			const int variant = clampVariant(itemInstance->getAuxValue());

			level->setTileAndData(x, y, z, tile->id, dir, Tile::UPDATE_ALL);
			// double-check that the bed was successfully placed
			if (level->getTile(x, y, z) == tile->id)
			{
				// neoLegacy - both halves carry which bed this is (color / straw)
				BedTileEntity::setVariantAt(level, x, y, z, variant);

				// 4J-JEV: Hook for durango 'BlockPlaced' event.
				player->awardStat(GenericStats::blocksPlaced(tile->id), GenericStats::param_blocksPlaced(tile->id,itemInstance->getAuxValue(),1));

				level->setTileAndData(x + xra, y, z + zra, tile->id, dir + BedTile::HEAD_PIECE_DATA, Tile::UPDATE_ALL);
				BedTileEntity::setVariantAt(level, x + xra, y, z + zra, variant);
			}

			itemInstance->count--;
		}
		return true;
	}

	return false;
}
