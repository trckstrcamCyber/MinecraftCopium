#include "stdafx.h"
#include "FirearmEnchantment.h"
#include "EnchantmentCategory.h"

FirearmEnchantment::FirearmEnchantment(int id, int frequency, Kind kind, int descriptionId)
    : Enchantment(id, frequency, EnchantmentCategory::firearm), kind(kind)
{
    setDescriptionId(descriptionId);
}

int FirearmEnchantment::getMaxLevel()
{
    switch (kind)
    {
    case AutoFire: return 1;
    case ExtendedMag: return 2;
    case RapidReload: return 2;
    case PrecisionBarrel: return 3;
    case HollowPoint: return 2;
    case ReinforcedParts: return 3;
    }
    return 1;
}

bool FirearmEnchantment::isTreasureEnchantment() const
{
    return kind == AutoFire;
}

bool FirearmEnchantment::isCompatibleWith(Enchantment *other) const
{
    if (!other || other == this) return false;
    int otherId = other->id;
    // Extended Mag conflicts with Rapid Reload; Precision Barrel conflicts with Hollow Point.
    if ((kind == ExtendedMag && otherId == 84) || (kind == RapidReload && otherId == 83)) return false;
    if ((kind == PrecisionBarrel && otherId == 86) || (kind == HollowPoint && otherId == 85)) return false;
    return Enchantment::isCompatibleWith(other);
}
