#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "ItemInstance.h"
#include "IconRegister.h"
#include "SignItem.h"
#include "GenericStats.h"

SignItem::SignItem(int id, int woodType) : Item(id)
{
	this->woodType = woodType;
	for (int i = 0; i < 7; i++) woodIcons[i] = nullptr;
	setBaseItemTypeAndMaterial(Item::eBaseItemType_sign, Item::eMaterial_wood);
	// 4J-PB - Changed for TU9
	maxStackSize = 16;
	if (woodType < 0) setStackedByData(true);	// different woods don't stack together
}

static const unsigned int SIGN_NAME_IDS[7] =
{
	IDS_ITEM_SIGN, IDS_ITEM_SIGN_SPRUCE, IDS_ITEM_SIGN_BIRCH, IDS_ITEM_SIGN_JUNGLE, IDS_ITEM_SIGN_ACACIA, IDS_ITEM_SIGN_DARK_OAK, IDS_ITEM_SIGN_BAMBOO
};
static const wchar_t *SIGN_ICON_NAMES[7] =
{
	L"sign", L"signSpruce", L"signBirch", L"signJungle", L"signAcacia", L"signDark", L"bamboo_sign"
};

void SignItem::registerIcons(IconRegister *iconRegister)
{
	if (woodType >= 0)
	{
		Item::registerIcons(iconRegister);
		return;
	}
	for (int i = 0; i < 7; i++) woodIcons[i] = iconRegister->registerIcon(SIGN_ICON_NAMES[i]);
	icon = woodIcons[0];
}

Icon *SignItem::getIcon(int auxValue)
{
	if (woodType >= 0) return Item::getIcon(auxValue);
	if (auxValue < 0 || auxValue > 6) auxValue = 0;
	return woodIcons[auxValue] != nullptr ? woodIcons[auxValue] : icon;
}

unsigned int SignItem::getDescriptionId(int iData)
{
	if (woodType >= 0) return Item::getDescriptionId(iData);
	if (iData < 0 || iData > 6) iData = 0;
	return SIGN_NAME_IDS[iData];
}

unsigned int SignItem::getDescriptionId(shared_ptr<ItemInstance> instance)
{
	if (woodType >= 0) return Item::getDescriptionId(instance);
	return getDescriptionId(instance->getAuxValue());
}

bool SignItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	// 4J-PB - Adding a test only version to allow tooltips to be displayed
	if (face == 0) return false;
	if (!level->getMaterial(x, y, z)->isSolid()) return false;


	if (face == 1) y++;
	if (face == 2) z--;
	if (face == 3) z++;
	if (face == 4) x--;
	if (face == 5) x++;

	if (!player->mayUseItemAt(x, y, z, face, instance)) return false;

	if (!Tile::sign->mayPlace(level, x, y, z)) return false;

	// the wood: the shared item carries it in its aux value, the old per-wood items have it built in
	int wood = (woodType >= 0) ? woodType : instance->getAuxValue();
	if (wood < 0 || wood > 6) wood = 0;

	if (level->isClientSide)
	{
		return true;
	}

	if(!bTestUseOnOnly)
	{
		if (face == 1) 
		{
			int rot = Mth::floor(((player->yRot + 180) * 16) / 360 + 0.5) & 15;
			level->setTileAndData(x, y, z, Tile::standing_sign_Id, rot, Tile::UPDATE_ALL);
		} 
		else
		{
			level->setTileAndData(x, y, z, Tile::wall_standing_sign_Id, face, Tile::UPDATE_ALL);
		}

		instance->count--;
		shared_ptr<SignTileEntity> ste = dynamic_pointer_cast<SignTileEntity>( level->getTileEntity(x, y, z) );
		if (ste != nullptr)
		{
			ste->setWoodType(wood);
			level->sendTileUpdated(x, y, z);	// tell the clients which wood this sign is
			player->openTextEdit(ste);
		}

		// 4J-JEV: Hook for durango 'BlockPlaced' event.
		player->awardStat(
			GenericStats::blocksPlaced((face==1) ? Tile::standing_sign_Id : Tile::wall_standing_sign_Id),
			GenericStats::param_blocksPlaced( 
				(face==1) ? Tile::standing_sign_Id : Tile::wall_standing_sign_Id,
				instance->getAuxValue(),
				1)
			);
	}
	return true;
}
