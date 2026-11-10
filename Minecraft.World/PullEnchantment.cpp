#include "stdafx.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.h"
#include "PullEnchantment.h"
#include "WeaponItem.h"
#include "HtmlString.h"
#include "Random.h"

PullEnchantment::PullEnchantment(int id, int frequency)
    : Enchantment(id, frequency, EnchantmentCategory::weapon)
{
}

int PullEnchantment::getMinCost(int level)
{
    return 20 + (level - 1) * 10;
}

int PullEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 35;
}

int PullEnchantment::getMaxLevel()
{
    return 1;
}

bool PullEnchantment::canEnchant(shared_ptr<ItemInstance> item)
{
    return dynamic_cast<WeaponItem *>(item->getItem()) != nullptr &&
           item->getItem()->getBaseItemType() == Item::eBaseItemType_sword;
}

bool PullEnchantment::isTreasureEnchantment() const
{
    return true;
}

HtmlString PullEnchantment::getFullname(int level)
{
    wchar_t formatted[64];
    swprintf(formatted, 64, L"Pull%ls", getLevelSuffix(level).c_str());
    return HtmlString(formatted, eHTMLColor_f);
}

bool PullEnchantment::shouldDrop(Random *random)
{
    return random != nullptr && random->nextInt(DROP_CHANCE) == 0;
}
