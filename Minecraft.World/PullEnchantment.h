#pragma once

#include "Enchantment.h"

// neoLegacy Added - very rare treasure sword enchantment.
// On a successful player kill, there is a small chance of dropping the
// matching spawn egg. The drop is limited to mobs with registered spawn eggs.
class PullEnchantment : public Enchantment
{
public:
    // 1 in this many kills. Change this value to tune the drop rate.
    static const int DROP_CHANCE = 400;

    PullEnchantment(int id, int frequency);

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool canEnchant(shared_ptr<ItemInstance> item);
    virtual bool isTreasureEnchantment() const;
    virtual HtmlString getFullname(int level);

    static bool shouldDrop(Random *random);
};
