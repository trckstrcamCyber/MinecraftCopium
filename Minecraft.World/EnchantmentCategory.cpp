#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "EnchantmentCategory.h"
#include "Mac10Item.h"
#include "MaceItem.h"
#include "CrossbowItem.h"

const EnchantmentCategory *EnchantmentCategory::all = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::armor = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::armor_feet = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::armor_legs = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::armor_torso = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::armor_head = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::weapon = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::digger = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::bow = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::fishing_rod = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::zoro_sword = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::firearm = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::mace = new EnchantmentCategory();
const EnchantmentCategory *EnchantmentCategory::crossbow = new EnchantmentCategory();

bool EnchantmentCategory::canEnchant(Item *item) const
{
	if (this == all) return true;
	if (this == firearm) return dynamic_cast<Mac10Item *>(item) != nullptr;
	if (this == mace) return dynamic_cast<MaceItem *>(item) != nullptr;
	if (this == crossbow) return dynamic_cast<CrossbowItem *>(item) != nullptr;
	// the Crossbow is not a Weapon, Digger or Bow. Which enchantments it really takes is decided by CrossbowItem::isEnchantmentAllowed
	// (checked in Enchantment::canEnchant and the table roll), so the weapon / durability categories pass here (Looting, Unbreaking).
	if (dynamic_cast<CrossbowItem *>(item) != nullptr) return this == weapon || this == digger;
	// the Mace is not a WeaponItem. Which enchantments it really takes is decided by MaceItem::isEnchantmentAllowed (checked in
	// Enchantment::canEnchant and the table roll), so the weapon / durability categories pass here.
	if (dynamic_cast<MaceItem *>(item) != nullptr) return this == weapon || this == digger;

	if (dynamic_cast<ArmorItem *>( item ) != nullptr)
	{
		if (this == armor) return true;
		ArmorItem *ai = static_cast<ArmorItem *>(item);
		if (ai->slot == ArmorItem::SLOT_HEAD) return this == armor_head;
		if (ai->slot == ArmorItem::SLOT_LEGS) return this == armor_legs;
		if (ai->slot == ArmorItem::SLOT_TORSO) return this == armor_torso;
		if (ai->slot == ArmorItem::SLOT_FEET) return this == armor_feet;
		return false;
	}
	else if (dynamic_cast<WeaponItem *>(item) != nullptr)
	{
		// every sword takes the normal weapon enchantments; the Zoro-only ones need a Zoro sword
		if (this == weapon) return true;
		if (this == zoro_sword) return static_cast<WeaponItem *>(item)->isZoroSword();
		return false;
	}
	else if (dynamic_cast<DiggerItem *>(item) != nullptr)
	{
		return this == digger;
	}
	else if (dynamic_cast<BowItem *>(item) != nullptr)
	{
		return this == bow;
	}
	else if (dynamic_cast<FishingRodItem*>(item) != nullptr)
	{
		return this == fishing_rod;
	}
	return false;
}