#pragma once
#include "Enchantment.h"

class FirearmEnchantment : public Enchantment
{
public:
    enum Kind { AutoFire, ExtendedMag, RapidReload, PrecisionBarrel, HollowPoint, ReinforcedParts };
    FirearmEnchantment(int id, int frequency, Kind kind, int descriptionId);
    virtual int getMaxLevel() override;
    virtual bool isTreasureEnchantment() const override;
    virtual bool isCompatibleWith(Enchantment *other) const override;
private:
    Kind kind;
};
