#include "SoftStepEnchantment.h"
#include "HtmlString.h"            // REQUIRED
#include "ItemInstance.h"



SoftStepEnchantment::SoftStepEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::armor_feet)
{
}

int SoftStepEnchantment::getMinCost(int level)
{
    return 10 + (level - 1) * 10;
}

int SoftStepEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 15;
}

int SoftStepEnchantment::getMaxLevel()
{
    return 1;
}

HtmlString SoftStepEnchantment::getFullname(int level)
{
    wchar_t formatted[64];
    swprintf(formatted, 64, L"Soft Step%ls", getLevelSuffix(level).c_str());
    return HtmlString(formatted, eHTMLColor_f);
}
