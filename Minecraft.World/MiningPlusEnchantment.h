#pragma once

#include "Enchantment.h"

// neoLegacy Added - pickaxe-only 3x3 mining.
// The actual multi-block operation is performed by ServerPlayerGameMode
// because World must not depend on the Server target.
class MiningPlusEnchantment : public Enchantment
{
public:
    MiningPlusEnchantment(int id, int frequency);

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool canEnchant(shared_ptr<ItemInstance> item);
    virtual HtmlString getFullname(int level);

    // Number of extra blocks around the original block.
    static int getRadius(int level);
};
