#pragma once

#include "Enchantment.h"

// 4J neoLegacy Added
// Pickaxe-only digger enchantment (single level, like Silk Touch). Ore blocks
// that still drop their own raw form (currently iron and gold - see
// Tile::getSmeltedResource()) drop the smelted ingot directly instead,
// with a small XP bonus matching typical furnace smelting yield.
// Incompatible with Silk Touch - the two enchantments want contradictory
// things from the same drop (raw block vs. smelted result).
class AutoSmeltEnchantment : public Enchantment
{
public:
	AutoSmeltEnchantment(int id, int frequency);

	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool canEnchant(shared_ptr<ItemInstance> item);
	virtual bool isCompatibleWith(Enchantment *other) const;
	virtual HtmlString getFullname(int level);
};
