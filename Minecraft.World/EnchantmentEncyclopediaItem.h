#pragma once

#include "Item.h"

// neoLegacy - Enchantment Encyclopedia.
// A reference book listing every registered enchantment. Using it opens the existing read-only book reader
// (the same scene a signed Written Book uses) on a temporary book that EnchantmentEncyclopedia::createBook()
// builds from the live enchantment registry, so it is always up to date and nothing is stored in the item's NBT.
// The reader is opened client-side from LocalPlayer::openItemInstanceGui(); on the server use() does nothing.
class EnchantmentEncyclopediaItem : public Item
{
public:
	EnchantmentEncyclopediaItem(int id);

	virtual bool isFoil(shared_ptr<ItemInstance> itemInstance);
	virtual const Rarity *getRarity(shared_ptr<ItemInstance> itemInstance);

	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual bool TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
};
