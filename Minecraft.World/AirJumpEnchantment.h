#pragma once

#include "Enchantment.h"

// neoLegacy Added - boots enchantment that permits additional mid-air jumps.
// Configuration:
//   AIR_JUMP_MAX_LEVEL controls the maximum enchantment level.
//   BASE_JUMP_POWER is the normal jump velocity.
//   POWER_PER_LEVEL is added for each enchantment level.
class AirJumpEnchantment : public Enchantment
{
public:
    static const int AIR_JUMP_MAX_LEVEL = 3;
    static const float BASE_JUMP_POWER;
    static const float POWER_PER_LEVEL;

    AirJumpEnchantment(int id, int frequency);

    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool canEnchant(shared_ptr<ItemInstance> item);
    virtual HtmlString getFullname(int level);

    static int getMaxAirJumps(int level);
    static float getJumpPower(int level);
};
