#include "stdafx.h"
#include "net.minecraft.world.item.enchantment.h"
#include "ClimbEnchantment.h"
#include "HtmlString.h"      // REQUIRED



ClimbEnchantment::ClimbEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::armor_feet)
{
}

int ClimbEnchantment::getMinCost(int level)
{
    return 10 + (level - 1) * 10;
}

int ClimbEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 15;
}

int ClimbEnchantment::getMaxLevel()
{
    // neoLegacy Added - was 1, now 3 so higher rolls give a bigger step-assist.
    return 3;
}

HtmlString ClimbEnchantment::getFullname(int level)
{
    wchar_t formatted[64];
    swprintf(formatted, 64, L"Climb%ls", getLevelSuffix(level).c_str());
    return HtmlString(formatted, eHTMLColor_f);
}

// neoLegacy Added
float ClimbEnchantment::getFootSizeBonus(int level)
{
    if (level <= 0) return 0.0f;

    // level 1: 1.0  (original flat behaviour - step up a full block)
    // level 2: 1.5  (step up a block-and-a-half)
    // level 3: 2.0  (step up two full blocks without jumping)
    return 0.5f + 0.5f * level;
}
