#pragma once

#include "Enchantment.h"

// ---------------------------------------------------------------------------------------------------------------------------
// neoLegacy - the first curses. A curse:
//   * is shown in red text (Enchantment::getFullname uses red for anything whose isCurse() is true)
//   * is never offered by the enchanting table or put on random books/loot (it is skipped in
//     EnchantmentHelper::getAvailableEnchantmentResults and left out of Enchantment::validEnchantments)
//   * survives the grindstone (GrindstoneMenu already keeps everything with isCurse())
// They can be put on an item with an anvil + book, /enchant, or taken from the creative menu's enchanted books.
// ---------------------------------------------------------------------------------------------------------------------------

// Fishing rods and books. The inverse of Luck of the Sea: each level lowers the treasure chance and raises the junk chance
// (FishingHook feeds "Luck of the Sea level - Hook of Muddy Waters level" into the loot roll, so the maths is exactly mirrored).
// Cannot be combined with Luck of the Sea.
class HookOfMuddyWatersEnchantment : public Enchantment
{
public:
	HookOfMuddyWatersEnchantment(int id, int frequency);
	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool isCompatibleWith(Enchantment *other) const;
	virtual bool isTreasureEnchantment() const { return true; }
	virtual bool isCurse() const { return true; }
	virtual HtmlString getFullname(int level);
};

// Armor, carved pumpkins, mob heads and elytra. In Survival the item can't be taken off (not by clicking, shift-clicking or
// dropping it from the armor slot) until the player dies or the item breaks. Creative mode can still remove it.
class BindingCurseEnchantment : public Enchantment
{
public:
	BindingCurseEnchantment(int id, int frequency);
	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool canEnchant(shared_ptr<ItemInstance> item);
	virtual bool isTreasureEnchantment() const { return true; }
	virtual bool isCurse() const { return true; }
	virtual HtmlString getFullname(int level);

	// the things that can be worn in an armor slot
	static bool isWearable(shared_ptr<ItemInstance> item);
};

// Anything that can break or be worn, plus compasses. The item is destroyed when its owner dies instead of dropping
// (see Inventory::dropAll).
class VanishingCurseEnchantment : public Enchantment
{
public:
	VanishingCurseEnchantment(int id, int frequency);
	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool canEnchant(shared_ptr<ItemInstance> item);
	virtual bool isTreasureEnchantment() const { return true; }
	virtual bool isCurse() const { return true; }
	virtual HtmlString getFullname(int level);
};
