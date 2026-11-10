#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.level.tile.h"
#include "TreeCapitatorEnchantment.h"
#include "HtmlString.h"

#include "HatchetItem.h"

// CAM - This project uses HatchetItem for axes, not AxeItem.

TreeCapitatorEnchantment::TreeCapitatorEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::digger)
{
    // Name comes from the getFullname() override below instead of a
    // descriptionId - see AdrenalineEnchantment.cpp for why.
}

int TreeCapitatorEnchantment::getMinCost(int level)
{
    return 15 + (level - 1) * 15;
}

int TreeCapitatorEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 30;
}

int TreeCapitatorEnchantment::getMaxLevel()
{
    return 3;
}

bool TreeCapitatorEnchantment::canEnchant(shared_ptr<ItemInstance> item)
{
    // Hatchet only - same caveat as VeinminerEnchantment::canEnchant(): this
    // override is only consulted by the anvil/book/`/enchant` command paths,
    // not by the enchant table's actual roll (see the ServerPlayerGameMode.cpp
    // hook for the authoritative runtime check).
    return dynamic_cast<HatchetItem *>(item->getItem()) != nullptr;
}

HtmlString TreeCapitatorEnchantment::getFullname(int level)
{
    wchar_t formatted[64];
    swprintf(formatted, 64, L"Treecapitator%ls", getLevelSuffix(level).c_str());
    return HtmlString(formatted, eHTMLColor_f);
}

bool TreeCapitatorEnchantment::isTreeLog(int tileId)
{
    return tileId == Tile::log_Id || tileId == Tile::log2_Id;
}

bool TreeCapitatorEnchantment::isSameSpecies(int data, int otherData)
{
    return (data & 0x3) == (otherData & 0x3);
}

int TreeCapitatorEnchantment::getMaxChainSize(int level)
{
    if (level <= 1) return 16;
    if (level == 2) return 32;
    return 64;
}
