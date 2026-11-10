#pragma once

class Enchantment;
class Random;
class WeighedTreasure;
class ItemInstance;

// neoLegacy - where curses come from, besides the anvil: structure chests, fishing and villager trades.
// Everything here makes ordinary items/books that simply carry a curse, optionally with another enchantment as well.
class CurseLoot
{
public:
	enum Category { ANY = 0, ARMOR, WEAPON, TOOL };

	// one of Hook of Muddy Waters / Curse of Binding / Curse of Vanishing
	static Enchantment *randomCurse(Random *random);

	// Adds a random curse that suits the item and doesn't clash with what it already has (an enchanted book takes any).
	// Returns false when none fits.
	static bool addRandomCurse(shared_ptr<ItemInstance> item, Random *random);

	// an enchanted book holding one curse, and with withOther also one ordinary enchantment (when they are compatible)
	static shared_ptr<ItemInstance> createCursedBook(Random *random, bool withOther);

	// a piece of armor / a weapon / a tool / a fishing rod carrying a curse, and with withOther some normal enchantments too
	static shared_ptr<ItemInstance> createCursedItem(Random *random, Category category, bool withOther);

	// a chest loot entry: a cursed book (3 in 5) or a cursed item (2 in 5)
	static WeighedTreasure *createTreasure(Random *random, int minCount, int maxCount, int weight);

private:
	static bool isCompatibleWithAll(shared_ptr<ItemInstance> item, Enchantment *curse);
};
