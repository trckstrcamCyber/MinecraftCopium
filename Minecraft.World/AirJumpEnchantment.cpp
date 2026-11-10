#include "stdafx.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.item.h"
#include "AirJumpEnchantment.h"
#include "ArmorItem.h"
#include "HtmlString.h"

const float AirJumpEnchantment::BASE_JUMP_POWER = 0.42f;
const float AirJumpEnchantment::POWER_PER_LEVEL = 0.08f;

AirJumpEnchantment::AirJumpEnchantment(int id, int frequency)
    : Enchantment(id, frequency, EnchantmentCategory::armor_feet)
{
}

int AirJumpEnchantment::getMinCost(int level)
{
    return 15 + (level - 1) * 15;
}

int AirJumpEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 25;
}

int AirJumpEnchantment::getMaxLevel()
{
    return AIR_JUMP_MAX_LEVEL;
}

bool AirJumpEnchantment::canEnchant(shared_ptr<ItemInstance> item)
{
    return dynamic_cast<ArmorItem *>(item->getItem()) != nullptr &&
           item->getItem()->getBaseItemType() == Item::eBaseItemType_boots;
}

HtmlString AirJumpEnchantment::getFullname(int level)
{
    wchar_t formatted[64];
    swprintf(formatted, 64, L"Air Jump%ls", getLevelSuffix(level).c_str());
    return HtmlString(formatted, eHTMLColor_f);
}

int AirJumpEnchantment::getMaxAirJumps(int level)
{
    if (level < 1) return 0;
    return level;
}

float AirJumpEnchantment::getJumpPower(int level)
{
    if (level < 1) return BASE_JUMP_POWER;

    // neoLegacy FIX - jump HEIGHT scales with velocity squared, so the old
    // flat "+POWER_PER_LEVEL per level" growth (0.50 / 0.58 / 0.66) looked
    // mild level-to-level but compounded into a much bigger height gain by
    // level 3 -- and level 3 also grants 3 stacked air jumps (see
    // getMaxAirJumps below), each using that same top-level power, so the
    // total reachable height multiplied on top of that again. Level 1 keeps
    // its existing power (0.50) unchanged; each level after that adds only
    // half of the previous level's increment instead of a full one, so
    // per-jump power -- and therefore height -- tapers off instead of
    // climbing in a straight line. The main benefit of higher levels is
    // still the extra jump charges, not a dramatically higher single jump.
    float power = BASE_JUMP_POWER;
    float increment = POWER_PER_LEVEL;
    for (int i = 1; i <= level; i++)
    {
        power += increment;
        increment *= 0.5f;
    }
    return power;
}
