#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "HtmlString.h"
#include "MobEffect.h"
#include "MobEffectInstance.h"
#include "OminousBottleItem.h"

OminousBottleItem::OminousBottleItem(int id) : Item(id)
{
	setMaxStackSize(64);
	setStackedByData(true);		// the data is the level, so bottles of different levels do not stack
}

shared_ptr<ItemInstance> OminousBottleItem::create(int level)
{
	if (level < 1) level = 1;
	if (level > MAX_LEVEL) level = MAX_LEVEL;
	return std::make_shared<ItemInstance>(Item::ominous_bottle_Id, 1, level - 1);
}

int OminousBottleItem::getLevel(shared_ptr<ItemInstance> itemInstance)
{
	int level = (itemInstance != nullptr ? itemInstance->getAuxValue() : 0) + 1;
	if (level < 1) level = 1;
	if (level > MAX_LEVEL) level = MAX_LEVEL;
	return level;
}

int OminousBottleItem::getUseDuration(shared_ptr<ItemInstance> itemInstance)
{
	return DRINK_DURATION;
}

UseAnim OminousBottleItem::getUseAnimation(shared_ptr<ItemInstance> itemInstance)
{
	return UseAnim_drink;
}

shared_ptr<ItemInstance> OminousBottleItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	player->startUsingItem(instance, getUseDuration(instance));
	return instance;
}

shared_ptr<ItemInstance> OminousBottleItem::useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	int amplifier = getLevel(instance) - 1;

	if (!player->abilities.instabuild) instance->count--;

	if (!level->isClientSide)
	{
		// a lower level never replaces a higher one; the same or a higher one does, and starts the 100 minutes again
		MobEffectInstance *existing = player->getEffect(MobEffect::badOmen);
		if (existing == nullptr || existing->getAmplifier() <= amplifier)
		{
			player->removeEffect(MobEffect::badOmen->id);
			player->addEffect(new MobEffectInstance(MobEffect::badOmen->id, BAD_OMEN_TICKS, amplifier));
		}
	}

	return instance;	// no empty bottle comes back
}

void OminousBottleItem::appendHoverText(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, vector<HtmlString> *lines, bool advanced)
{
	static const wchar_t *numerals[MAX_LEVEL] = { L"I", L"II", L"III", L"IV", L"V" };
	wchar_t text[48];
	swprintf(text, 48, L"Bad Omen %ls (100:00)", numerals[getLevel(itemInstance) - 1]);
	lines->push_back(HtmlString(text, eHTMLColor_7));
}
