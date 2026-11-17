#pragma once

class Item;

class EnchantmentCategory
{
public:
	static const EnchantmentCategory *all;
	static const EnchantmentCategory *armor;
	static const EnchantmentCategory *armor_feet;
	static const EnchantmentCategory *armor_legs;
	static const EnchantmentCategory *armor_torso;
	static const EnchantmentCategory *armor_head;
	static const EnchantmentCategory *weapon;
	static const EnchantmentCategory *digger;
	static const EnchantmentCategory *bow;
	static const EnchantmentCategory *fishing_rod;
	static const EnchantmentCategory *zoro_sword;	// neoLegacy - only the four Zoro swords (table, anvil and books all go through this)
	static const EnchantmentCategory *firearm; // neoLegacy - MAC-10 only
	static const EnchantmentCategory *mace;		// neoLegacy - Mace only (Density, Breach, Wind Burst)
	static const EnchantmentCategory *crossbow;	// neoLegacy - Crossbow only (Quick Charge, Multishot, Piercing, Torched)

	bool canEnchant(Item *item) const;
};