#pragma once

#include "Enchantment.h"

class SoftStepEnchantment : public Enchantment
{
public:
    SoftStepEnchantment(int id, int frequency);

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual HtmlString getFullname(int level);
};
