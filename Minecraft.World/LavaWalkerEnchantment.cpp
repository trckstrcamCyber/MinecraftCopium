#include "stdafx.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.item.h"
#include "LavaWalkerEnchantment.h"
#include "ArmorItem.h"
#include "HtmlString.h"

LavaWalkerEnchantment::LavaWalkerEnchantment(int id, int frequency)
    : Enchantment(id, frequency, EnchantmentCategory::armor_feet)
{
}

int LavaWalkerEnchantment::getMinCost(int level)
{
    return 20 + (level - 1) * 10;
}

int LavaWalkerEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 25;
}

int LavaWalkerEnchantment::getMaxLevel()
{
    return 2;
}

bool LavaWalkerEnchantment::canEnchant(shared_ptr<ItemInstance> item)
{
    return dynamic_cast<ArmorItem *>(item->getItem()) != nullptr &&
           item->getItem()->getBaseItemType() == Item::eBaseItemType_boots;
}

HtmlString LavaWalkerEnchantment::getFullname(int level)
{
    wchar_t formatted[64];
    swprintf(formatted, 64, L"Lava Walker%ls", getLevelSuffix(level).c_str());
    return HtmlString(formatted, eHTMLColor_f);
}

bool LavaWalkerEnchantment::isImmune(int level)
{
    return level > 0;
}
