#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "ItemInstance.h"
#include "IconRegister.h"
#include "BoatItem.h"

BoatItem::BoatItem(int id) : Item( id )
{
	maxStackSize = 1;
	setBaseItemTypeAndMaterial(Item::eBaseItemType_boat, Item::eMaterial_wood);
	setStackedByData(true);
	for (int i = 0; i < 7; i++) woodIcons[i] = nullptr;
}

// neoLegacy - names of the six boat kinds (index = WoodTile data)
static const unsigned int BOAT_NAME_IDS[7] =
{
	IDS_ITEM_BOAT, IDS_ITEM_BOAT_SPRUCE, IDS_ITEM_BOAT_BIRCH, IDS_ITEM_BOAT_JUNGLE, IDS_ITEM_BOAT_ACACIA, IDS_ITEM_BOAT_DARK_OAK, IDS_ITEM_RAFT_BAMBOO
};
static const wchar_t *BOAT_ICON_NAMES[7] =
{
	L"boat", L"boatSpruce", L"boatBirch", L"boatJungle", L"boatAcacia", L"boatDark", L"bamboo_raft"
};

void BoatItem::registerIcons(IconRegister *iconRegister)
{
	for (int i = 0; i < 7; i++) woodIcons[i] = iconRegister->registerIcon(BOAT_ICON_NAMES[i]);
	icon = woodIcons[0];
}

Icon *BoatItem::getIcon(int auxValue)
{
	if (auxValue < 0 || auxValue > 6) auxValue = 0;
	return woodIcons[auxValue] != nullptr ? woodIcons[auxValue] : icon;
}

unsigned int BoatItem::getDescriptionId(int iData)
{
	if (iData < 0 || iData > 6) iData = 0;
	return BOAT_NAME_IDS[iData];
}

unsigned int BoatItem::getDescriptionId(shared_ptr<ItemInstance> instance)
{
	return getDescriptionId(instance->getAuxValue());
}

bool BoatItem::TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	// 4J-PB - added for tooltips to test use
	// 4J TODO really we should have the crosshair hitresult telling us if it hit water, and at what distance, so we don't need to do this again
	// if the player happens to have a boat in their hand

	float xRot = player->xRotO + (player->xRot - player->xRotO);
	float yRot = player->yRotO + (player->yRot - player->yRotO);

	double x = player->xo + (player->x - player->xo);
	double y = player->yo + (player->y - player->yo) + 1.62 - player->heightOffset;
	double z = player->zo + (player->z - player->zo);

	Vec3 *from = Vec3::newTemp(x, y, z);

	float yCos = Mth::cos(-yRot * Mth::RAD_TO_GRAD - PI);
	float ySin = Mth::sin(-yRot * Mth::RAD_TO_GRAD - PI);
	float xCos = -Mth::cos(-xRot * Mth::RAD_TO_GRAD);
	float xSin = Mth::sin(-xRot * Mth::RAD_TO_GRAD);

	float xa = ySin * xCos;
	float ya = xSin;
	float za = yCos * xCos;

	double range = 5;
	Vec3 *to = from->add(xa * range, ya * range, za * range);
	HitResult *hr = level->clip(from, to, true);
	if (hr == nullptr) return false;

	if (hr->type == HitResult::TILE)
	{
		delete hr;
		return true;
	}
	delete hr;
	return false;
}
shared_ptr<ItemInstance> BoatItem::use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	float a = 1;

	float xRot = player->xRotO + (player->xRot - player->xRotO) * a;
	float yRot = player->yRotO + (player->yRot - player->yRotO) * a;

	double x = player->xo + (player->x - player->xo) * a;
	double y = player->yo + (player->y - player->yo) * a + 1.62 - player->heightOffset;
	double z = player->zo + (player->z - player->zo) * a;

	Vec3 *from = Vec3::newTemp(x, y, z);

	float yCos = Mth::cos(-yRot * Mth::RAD_TO_GRAD - PI);
	float ySin = Mth::sin(-yRot * Mth::RAD_TO_GRAD - PI);
	float xCos = -Mth::cos(-xRot * Mth::RAD_TO_GRAD);
	float xSin = Mth::sin(-xRot * Mth::RAD_TO_GRAD);

	float xa = ySin * xCos;
	float ya = xSin;
	float za = yCos * xCos;

	double range = 5;
	Vec3 *to = from->add(xa * range, ya * range, za * range);
	HitResult *hr = level->clip(from, to, true);
	if (hr == nullptr) return itemInstance;

	// check entity collision
	Vec3 *b = player->getViewVector(a);
	bool hitEntity = false;
	float overlap = 1;
	vector<shared_ptr<Entity> > *objects = level->getEntities(player, player->bb->expand(b->x * (range), b->y * (range), b->z * (range))->grow(overlap, overlap, overlap));

	for( auto& e : *objects )
	{
		if (!e->isPickable()) continue;

		float rr = e->getPickRadius();
		AABB *bb = e->bb->grow(rr, rr, rr);
		if (bb->contains(from))
		{
			hitEntity = true;
		}
	}
	if (hitEntity)
	{
		return itemInstance;
	}

	if (hr->type == HitResult::TILE)
	{
		int xt = hr->x;
		int yt = hr->y;
		int zt = hr->z;

		if (level->getTile(xt, yt, zt) == Tile::snow_layer_Id) yt--;
		if( level->countInstanceOf(eTYPE_BOAT, true) < Level::MAX_XBOX_BOATS )		// 4J - added limit
		{
			shared_ptr<Boat> boat = std::make_shared<Boat>(level, xt + 0.5f, yt + 1.0f, zt + 0.5f);
			boat->yRot = ((Mth::floor(player->yRot * 4.0F / 360.0F + 0.5) & 0x3) - 1) * 90;
			boat->setWoodType(itemInstance->getAuxValue());	// neoLegacy: wood type comes from the item
			if (!level->getCubes(boat, boat->bb->grow(-.1, -.1, -.1))->empty())
			{
				return itemInstance;
			}
			if (!level->isClientSide)
			{
				level->addEntity(boat);
			}
			if (!player->abilities.instabuild)
			{
				itemInstance->count--;
			}
		}
		else
		{
			// display a message to say max boats has been hit
			player->displayClientMessage(IDS_MAX_BOATS );
		}
	}
	delete hr;

	return itemInstance;
}