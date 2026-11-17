#pragma once
#include "Item.h"
#include "Enchantment.h"

// neoLegacy - Crossbow. Hold right-click to draw it (1.25 s, uses one arrow), release: it is now LOADED. Right-click again to fire
// (speed 3.15, always critical). The loaded state is a second item id (Item::crossbow_loaded, shown with the arrow icon and kept out of
// the creative menu) that shares the first one's damage and its enchantment tag (see withItem), so no per-stack icon logic is needed.
//
// Enchantments (all defined at the bottom of this file):
//   Quick Charge I-III  the draw takes 0.25 s less per level
//   Multishot           fires 3 arrows in a spread for the price of one (exclusive with Piercing and Torched)
//   Piercing I-IV       an arrow passes through that many extra mobs (exclusive with Multishot)
//   Torched             loads torches instead of arrows; a shot torch sets every mob it hits on fire and is placed where it hits a block
//   Looting, Unbreaking and Mending also fit a crossbow (see isEnchantmentAllowed)
// Ammunition: the first arrow or firework rocket in the inventory (a Torched crossbow takes torches only). A rocket is fired straight at speed 1.6
// and bursts on whatever it hits (three at once with Multishot); it costs 3 durability instead of 1. The loaded-with-a-rocket crossbow is a third
// item id (crossbow_loaded_firework, the firework icon) that keeps the rocket it was loaded with in its tag under "ChargedFirework".
class CrossbowItem : public Item
{
private:
	bool loaded;
	bool firework;
	Icon *pullingIcons[3];
public:
	static const int CHARGE_TICKS = 25;							// the draw time with no Quick Charge (1.25 s)
	static const int QUICK_CHARGE_TICKS_PER_LEVEL = 5;			// 0.25 s per level
	static const int MULTISHOT_SPREAD_DEGREES = 10;

	CrossbowItem(int id, bool loaded, bool firework = false);

	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual void releaseUsing(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, int durationLeft);
	virtual shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance);
	virtual void registerIcons(IconRegister *iconRegister);
	Icon *getDrawnIcon(int amount);	// 0..2: how far the string is drawn back (used by Player::getItemInHandIcon, like the bow)
	virtual UseAnim getUseAnimation(shared_ptr<ItemInstance> itemInstance);
	virtual int getEnchantmentValue();	// > 0 is what lets the enchanting table accept a crossbow (the bow's value, like Java)

	static bool isCrossbow(shared_ptr<ItemInstance> item);
	static bool isEnchantmentAllowed(Enchantment *e);			// Enchantment::canEnchant and the enchanting-table roll both check this
	static int getChargeTicks(shared_ptr<ItemInstance> item);	// 25 ticks, less with Quick Charge (never below 1)
	static int getAmmoId(shared_ptr<ItemInstance> item);		// Item::arrow_Id, or Tile::torch_Id for a Torched crossbow
	static int findAmmoSlot(shared_ptr<Player> player, shared_ptr<ItemInstance> crossbow);	// first inventory slot it can load, or -1

private:
	// the same stack with another item id: keeps the damage, the enchantments and the name
	static shared_ptr<ItemInstance> withItem(shared_ptr<ItemInstance> from, int newId);
	static void shootLoaded(shared_ptr<ItemInstance> weapon, Level *level, shared_ptr<Player> player);
};

// ---------------------------------------------------------------------------------------------------------------------------
// neoLegacy - crossbow enchantments (ids 53-56). Names come from getFullname() like the other custom enchantments.
// ---------------------------------------------------------------------------------------------------------------------------
class QuickChargeEnchantment : public Enchantment
{
public:
	QuickChargeEnchantment(int id, int frequency);
	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual HtmlString getFullname(int level);
};

class MultishotEnchantment : public Enchantment
{
public:
	MultishotEnchantment(int id, int frequency);
	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool isCompatibleWith(Enchantment *other) const;
	virtual HtmlString getFullname(int level);
};

class PiercingEnchantment : public Enchantment
{
public:
	PiercingEnchantment(int id, int frequency);
	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool isCompatibleWith(Enchantment *other) const;
	virtual HtmlString getFullname(int level);
};

class TorchedEnchantment : public Enchantment
{
public:
	TorchedEnchantment(int id, int frequency);
	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool isCompatibleWith(Enchantment *other) const;
	virtual HtmlString getFullname(int level);
};
