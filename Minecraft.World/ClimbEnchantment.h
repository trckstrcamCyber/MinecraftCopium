#pragma once

#include "Enchantment.h"

class ClimbEnchantment : public Enchantment
{
public:
    ClimbEnchantment(int id, int frequency);

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual HtmlString getFullname(int level);

    // neoLegacy Added - turns the flat "step up 1 block" assist into a
    // level-scaled step height. Called from Entity::move() instead of
    // hardcoding footSize = 1.0f.
    static float getFootSizeBonus(int level);
};
