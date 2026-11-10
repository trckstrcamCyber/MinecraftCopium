#include "stdafx.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.item.h"
#include "BrightVisionEnchantment.h"
#include "ArmorItem.h"
#include "HtmlString.h"

BrightVisionEnchantment::BrightVisionEnchantment(int id, int frequency)
    : Enchantment(id, frequency, EnchantmentCategory::armor_head)
{
}

int BrightVisionEnchantment::getMinCost(int level)
{
    return 15 + (level - 1) * 10;
}

int BrightVisionEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 20;
}

int BrightVisionEnchantment::getMaxLevel()
{
    return 1;
}

bool BrightVisionEnchantment::canEnchant(shared_ptr<ItemInstance> item)
{
    return dynamic_cast<ArmorItem *>(item->getItem()) != nullptr &&
           item->getItem()->getBaseItemType() == Item::eBaseItemType_helmet;
}

HtmlString BrightVisionEnchantment::getFullname(int level)
{
    wchar_t formatted[64];
    swprintf(formatted, 64, L"Bright Vision%ls", getLevelSuffix(level).c_str());
    return HtmlString(formatted, eHTMLColor_f);
}
