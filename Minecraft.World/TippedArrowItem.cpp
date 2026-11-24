#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.effect.h"
#include "IconRegister.h"
#include "StringHelpers.h"
#include "TippedArrowItem.h"

TippedArrowItem::TippedArrowItem(int id) : PotionItem(id)
{
	setMaxStackSize(64);
	iconBase = nullptr;
}

bool TippedArrowItem::TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	return false;
}

shared_ptr<ItemInstance> TippedArrowItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	return instance;
}

int TippedArrowItem::getUseDuration(shared_ptr<ItemInstance> itemInstance)
{
	return 0;
}

UseAnim TippedArrowItem::getUseAnimation(shared_ptr<ItemInstance> itemInstance)
{
	return UseAnim_none;
}

Icon *TippedArrowItem::getIcon(int auxValue)
{
	return iconBase;
}

wstring TippedArrowItem::getHoverName(shared_ptr<ItemInstance> itemInstance)
{
	vector<MobEffectInstance *> *effects = getMobEffects(itemInstance);
	if (effects != nullptr && !effects->empty())
	{
		wstring name = app.GetString(IDS_ITEM_TIPPED_ARROW_EFFECT);
		return replaceAll(name, L"{*postfix*}", app.GetString(effects->at(0)->getPostfixDescriptionId()));
	}
	return app.GetString(IDS_ITEM_TIPPED_ARROW);
}

bool TippedArrowItem::isFoil(shared_ptr<ItemInstance> itemInstance)
{
	return false;
}

unsigned int TippedArrowItem::getUseDescriptionId(shared_ptr<ItemInstance> instance)
{
	return IDS_DESC_TIPPED_ARROW;
}

void TippedArrowItem::registerIcons(IconRegister *iconRegister)
{
	PotionItem::registerIcons(iconRegister);
	iconBase = iconRegister->registerIcon(L"tipped_arrow_base");
	iconOverlay = iconRegister->registerIcon(L"tipped_arrow_head");
}
