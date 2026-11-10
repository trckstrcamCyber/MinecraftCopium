#pragma once

#include "Enchantment.h"

// neoLegacy Added - helmet enchantment that supplies night vision.
class BrightVisionEnchantment : public Enchantment
{
public:
    BrightVisionEnchantment(int id, int frequency);

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool canEnchant(shared_ptr<ItemInstance> item);
    virtual HtmlString getFullname(int level);
};
