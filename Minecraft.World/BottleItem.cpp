#include "stdafx.h"
#include "SoundTypes.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.item.h"
#include "BottleItem.h"
#include "AreaEffectCloud.h"

BottleItem::BottleItem(int id) : Item(id)
{
}

Icon *BottleItem::getIcon(int auxValue)
{
	return Item::potion->getIcon(0);
}

shared_ptr<ItemInstance> BottleItem::use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	vector<shared_ptr<Entity> > *clouds = level->getEntitiesOfClass(typeid(AreaEffectCloud), player->bb->grow(2, 2, 2));
	if (clouds != nullptr)
	{
		shared_ptr<AreaEffectCloud> breath = nullptr;
		for (auto& it : *clouds)
		{
			shared_ptr<AreaEffectCloud> cloud = dynamic_pointer_cast<AreaEffectCloud>(it);
			if (cloud != nullptr && !cloud->removed && cloud->getParticle() == eParticleType_dragonbreath)
			{
				breath = cloud;
				break;
			}
		}
		delete clouds;
		if (breath != nullptr)
		{
			breath->setRadius(breath->getRadius() - 0.5f);
			level->playSound(player->x, player->y, player->z, eSoundType_ITEM_BOTTLE_FILL_DRAGONBREATH, 1.0f, 1.0f);
			itemInstance->count--;
			if (itemInstance->count <= 0)
			{
				return std::make_shared<ItemInstance>(Item::dragon_breath);
			}
			if (!player->inventory->add(std::make_shared<ItemInstance>(Item::dragon_breath)))
			{
				player->drop(std::make_shared<ItemInstance>(Item::dragon_breath_Id, 1, 0));
			}
			return itemInstance;
		}
	}

	HitResult *hr = getPlayerPOVHitResult(level, player, true);
	if (hr == nullptr) return itemInstance;

	if (hr->type == HitResult::TILE)
	{
		int xt = hr->x;
		int yt = hr->y;
		int zt = hr->z;
		int face = hr->f;
		delete hr;

		if (!level->mayInteract(player, xt, yt, zt, 0))
		{
			return itemInstance;
		}
		if (!player->mayUseItemAt(xt, yt, zt, face, itemInstance))
		{
			return itemInstance;
		}
		if (level->getMaterial(xt, yt, zt) == Material::water)
		{
			level->playSound(player->x, player->y, player->z, eSoundType_ITEM_BOTTLE_FILL, 1.0f, 1.0f);
			itemInstance->count--;
			if (itemInstance->count <= 0)
			{
				return std::make_shared<ItemInstance>(static_cast<Item *>(Item::potion));
			}
			else
			{
				if (!player->inventory->add(std::make_shared<ItemInstance>(static_cast<Item *>(Item::potion))))
				{
					player->drop(std::make_shared<ItemInstance>(Item::potion_Id, 1, 0));
				}
			}
		}
	}
	else
	{
		delete hr;
	}

	return itemInstance;
}

// 4J-PB - added to allow tooltips
bool BottleItem::TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	vector<shared_ptr<Entity> > *clouds = level->getEntitiesOfClass(typeid(AreaEffectCloud), player->bb->grow(2, 2, 2));
	if (clouds != nullptr)
	{
		bool nearBreath = false;
		for (auto& it : *clouds)
		{
			shared_ptr<AreaEffectCloud> cloud = dynamic_pointer_cast<AreaEffectCloud>(it);
			if (cloud != nullptr && !cloud->removed && cloud->getParticle() == eParticleType_dragonbreath) nearBreath = true;
		}
		delete clouds;
		if (nearBreath) return true;
	}

	HitResult *hr = getPlayerPOVHitResult(level, player, true);
	if (hr == nullptr) return false;

	if (hr->type == HitResult::TILE)
	{
		int xt = hr->x;
		int yt = hr->y;
		int zt = hr->z;
		int face = hr->f;
		delete hr;

		if (!level->mayInteract(player, xt, yt, zt, 0))
		{
			return false;
		}
		if (!player->mayUseItemAt(xt, yt, zt, face, itemInstance))
		{
			return false;
		}
		if (level->getMaterial(xt, yt, zt) == Material::water)
		{
			return true;
		}
	}
	else
	{
		delete hr;
	}

	return false;
}

void BottleItem::registerIcons(IconRegister *iconRegister)
{
	// We reuse another texture.
}