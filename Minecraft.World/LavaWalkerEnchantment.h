#pragma once

#include "Enchantment.h"

// neoLegacy Added - boots enchantment that turns lava directly below a player
// into magma while protecting the wearer from lava damage.
class LavaWalkerEnchantment : public Enchantment
{
public:
    LavaWalkerEnchantment(int id, int frequency);

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool canEnchant(shared_ptr<ItemInstance> item);
    virtual HtmlString getFullname(int level);

    static bool isImmune(int level);
};
