#include "stdafx.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.item.h"
#include "MiningPlusEnchantment.h"
#include "PickaxeItem.h"
#include "HtmlString.h"

MiningPlusEnchantment::MiningPlusEnchantment(int id, int frequency)
    : Enchantment(id, frequency, EnchantmentCategory::digger)
{
}

int MiningPlusEnchantment::getMinCost(int level)
{
    return 18 + (level - 1) * 12;
}

int MiningPlusEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 25;
}

int MiningPlusEnchantment::getMaxLevel()
{
    // Level controls durability/selection behavior, while every level remains
    // a 3x3 break as requested.
    return 3;
}

bool MiningPlusEnchantment::canEnchant(shared_ptr<ItemInstance> item)
{
    return dynamic_cast<PickaxeItem *>(item->getItem()) != nullptr;
}

HtmlString MiningPlusEnchantment::getFullname(int level)
{
    wchar_t formatted[64];
    swprintf(formatted, 64, L"Mining+%ls", getLevelSuffix(level).c_str());
    return HtmlString(formatted, eHTMLColor_f);
}

int MiningPlusEnchantment::getRadius(int level)
{
    return level > 0 ? 1 : 0;
}
