#include "stdafx.h"
#include "Item.h"
#include "Player.h"
#include "ItemInstance.h"
#include "Rarity.h"
#include "EnchantmentEncyclopediaItem.h"

EnchantmentEncyclopediaItem::EnchantmentEncyclopediaItem(int id) : Item(id)
{
	setMaxStackSize(1);
}

bool EnchantmentEncyclopediaItem::isFoil(shared_ptr<ItemInstance> itemInstance)
{
	return true;
}

const Rarity *EnchantmentEncyclopediaItem::getRarity(shared_ptr<ItemInstance> itemInstance)
{
	return Rarity::rare;
}

// Same flow as WrittenBookItem / WritingBookItem: Player::openItemInstanceGui() is a no-op on the server, and
// LocalPlayer overrides it to open the book reader (see the Encyclopedia branch in LocalPlayer.cpp).
shared_ptr<ItemInstance> EnchantmentEncyclopediaItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	player->openItemInstanceGui(instance, player);
	return instance;
}

bool EnchantmentEncyclopediaItem::TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	return true;
}
