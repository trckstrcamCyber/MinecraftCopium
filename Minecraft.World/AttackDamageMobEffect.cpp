#include "stdafx.h"

#include "AttackDamageMobEffect.h"

AttackDamageMobEffect::AttackDamageMobEffect(int id, bool isHarmful, eMinecraftColour color) : MobEffect(id, isHarmful, color)
{
}

double AttackDamageMobEffect::getAttributeModifierValue(int amplifier, AttributeModifier *original)
{
	if (id == MobEffect::weakness->id)
	{
		return -4.0 * (amplifier + 1);
	}
	else
	{
		return 3.0 * (amplifier + 1);
	}
}