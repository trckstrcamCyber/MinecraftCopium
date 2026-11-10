#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "ItemInstance.h"
#include "WeighedTreasure.h"
#include "CurseEnchantments.h"
#include "CurseLoot.h"

Enchantment *CurseLoot::randomCurse(Random *random)
{
	Enchantment *curses[3] = { Enchantment::hookOfMuddyWaters, Enchantment::bindingCurse, Enchantment::vanishingCurse };
	return curses[random->nextInt(3)];
}

bool CurseLoot::isCompatibleWithAll(shared_ptr<ItemInstance> item, Enchantment *curse)
{
	bool ok = true;
	unordered_map<int, int> *current = EnchantmentHelper::getEnchantments(item);
	if (current != nullptr)
	{
		for (auto &it : *current)
		{
			Enchantment *e = Enchantment::enchantments[it.first];
			if (e == nullptr || e == curse) continue;
			if (!curse->isCompatibleWith(e) || !e->isCompatibleWith(curse))
			{
				ok = false;
				break;
			}
		}
		delete current;
	}
	return ok;
}

bool CurseLoot::addRandomCurse(shared_ptr<ItemInstance> item, Random *random)
{
	if (item == nullptr) return false;
	const bool isBook = item->id == Item::enchanted_book_Id;
	Enchantment *curses[3] = { Enchantment::hookOfMuddyWaters, Enchantment::bindingCurse, Enchantment::vanishingCurse };

	int start = random->nextInt(3);
	for (int i = 0; i < 3; ++i)
	{
		Enchantment *curse = curses[(start + i) % 3];
		if (!isBook && !curse->canEnchant(item)) continue;
		if (!isCompatibleWithAll(item, curse)) continue;

		int level = curse->getMinLevel() + random->nextInt(curse->getMaxLevel() - curse->getMinLevel() + 1);
		if (isBook) Item::enchanted_book->addEnchantment(item, new EnchantmentInstance(curse, level));
		else item->enchant(curse, level);
		return true;
	}
	return false;
}

shared_ptr<ItemInstance> CurseLoot::createCursedBook(Random *random, bool withOther)
{
	Enchantment *curse = randomCurse(random);
	int level = curse->getMinLevel() + random->nextInt(curse->getMaxLevel() - curse->getMinLevel() + 1);
	shared_ptr<ItemInstance> book = Item::enchanted_book->createForEnchantment(new EnchantmentInstance(curse, level));

	if (withOther && !Enchantment::validEnchantments.empty())
	{
		Enchantment *other = Enchantment::validEnchantments[random->nextInt(static_cast<int>(Enchantment::validEnchantments.size()))];
		if (curse->isCompatibleWith(other) && other->isCompatibleWith(curse))
		{
			int otherLevel = other->getMinLevel() + random->nextInt(other->getMaxLevel() - other->getMinLevel() + 1);
			Item::enchanted_book->addEnchantment(book, new EnchantmentInstance(other, otherLevel));
		}
	}
	return book;
}

shared_ptr<ItemInstance> CurseLoot::createCursedItem(Random *random, Category category, bool withOther)
{
	static const int armor[] = {
		Item::iron_helmet_Id, Item::iron_chestplate_Id, Item::iron_leggings_Id, Item::iron_boots_Id,
		Item::chainmail_helmet_Id, Item::chainmail_chestplate_Id, Item::chainmail_leggings_Id, Item::chainmail_boots_Id,
		Item::diamond_helmet_Id, Item::diamond_chestplate_Id, Item::diamond_leggings_Id, Item::diamond_boots_Id
	};
	static const int weapons[] = { Item::iron_sword_Id, Item::diamond_sword_Id, Item::bow_Id };
	static const int tools[] = {
		Item::iron_pickaxe_Id, Item::diamond_pickaxe_Id, Item::iron_axe_Id, Item::diamond_axe_Id,
		Item::iron_shovel_Id, Item::diamond_shovel_Id, Item::fishing_rod_Id
	};

	if (category == ANY) category = static_cast<Category>(1 + random->nextInt(3));
	int id;
	switch (category)
	{
	case ARMOR:  id = armor[random->nextInt(12)]; break;
	case WEAPON: id = weapons[random->nextInt(3)]; break;
	default:     id = tools[random->nextInt(7)]; break;
	}

	shared_ptr<ItemInstance> item = std::make_shared<ItemInstance>(id, 1, 0);
	if (withOther)
	{
		shared_ptr<ItemInstance> enchanted = EnchantmentHelper::enchantItem(random, item, 5 + random->nextInt(25));
		if (enchanted != nullptr) item = enchanted;
	}
	addRandomCurse(item, random);
	return item;
}

WeighedTreasure *CurseLoot::createTreasure(Random *random, int minCount, int maxCount, int weight)
{
	shared_ptr<ItemInstance> item;
	bool withOther = random->nextBoolean();
	if (random->nextInt(5) < 3) item = createCursedBook(random, withOther);
	else item = createCursedItem(random, ANY, withOther);
	return new WeighedTreasure(item, minCount, maxCount, weight);
}
