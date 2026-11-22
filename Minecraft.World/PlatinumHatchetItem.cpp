#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.level.h"
#include "SharedMonsterAttributes.h"
#include "SoundTypes.h"
#include "PlatinumHatchetItem.h"

PlatinumHatchetItem::PlatinumHatchetItem(int id) : Item(id)
{
	maxStackSize = 1;
	setMaxDamage(DURABILITY);
}

bool PlatinumHatchetItem::hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker)
{
	itemInstance->hurtAndBreak(1, attacker);
	return true;
}

bool PlatinumHatchetItem::isHandEquipped()
{
	return true;
}

int PlatinumHatchetItem::getEnchantmentValue()
{
	return 1;
}

bool PlatinumHatchetItem::isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem)
{
	if (repairItem != nullptr && repairItem->id == Item::platinum_sheet_Id) return true;
	return Item::isValidRepairItem(source, repairItem);
}

attrAttrModMap *PlatinumHatchetItem::getDefaultAttributeModifiers()
{
	attrAttrModMap *result = Item::getDefaultAttributeModifiers();
	result->insert(attrAttrModMap::value_type(SharedMonsterAttributes::ATTACK_DAMAGE->getId(),
		new AttributeModifier(eModifierId_ITEM_BASEDAMAGE, ATTACK_DAMAGE, AttributeModifier::OPERATION_ADDITION)));
	return result;
}

// drawn like a bow
UseAnim PlatinumHatchetItem::getUseAnimation(shared_ptr<ItemInstance> itemInstance)
{
	return UseAnim_bow;
}

int PlatinumHatchetItem::getUseDuration(shared_ptr<ItemInstance> itemInstance)
{
	return 20 * 60 * 60;
}

shared_ptr<ItemInstance> PlatinumHatchetItem::use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	player->startUsingItem(itemInstance, getUseDuration(itemInstance));
	return itemInstance;
}

// Released after at least half a second: the hatchet leaves the hand (Java: HatchetItem.releaseUsing). Not when that throw would break it.
void PlatinumHatchetItem::releaseUsing(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, int durationLeft)
{
	int drawn = getUseDuration(itemInstance) - durationLeft;
	if (drawn < MIN_DRAW_TICKS) return;
	if (itemInstance->getDamageValue() + 1 >= itemInstance->getMaxDamage()) return;
	if (level->isClientSide) return;

	itemInstance->hurtAndBreak(1, player);
	shared_ptr<ItemInstance> thrown = itemInstance->copy();
	thrown->count = 1;

	shared_ptr<Snowball> hatchet = std::make_shared<Snowball>(level, player);
	hatchet->setHatchet(true);
	hatchet->setHatchetStack(player->abilities.instabuild ? nullptr : thrown);	// nothing to pick up in creative: the stack stays in the hand
	level->addEntity(hatchet);
	level->playSound(player, eSoundType_RANDOM_BOW, 1.0f, 1.0f);

	if (!player->abilities.instabuild) player->inventory->setItem(player->inventory->selected, nullptr);
}
