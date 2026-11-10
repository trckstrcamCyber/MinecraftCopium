#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.h"
#include "SynchedEntityData.h"
#include "Mth.h"
#include "SpellcasterIllager.h"

SpellcasterIllager::SpellcasterIllager(Level *level) : Illager(level)
{
	spellCastingTickCount = 0;
	currentSpell = SPELL_NONE;
	getEntityData()->define(DATA_SPELL_CASTING_ID, static_cast<byte>(0));
}

bool SpellcasterIllager::isCastingSpell()
{
	if (level->isClientSide) return getEntityData()->getByte(DATA_SPELL_CASTING_ID) > 0;
	return currentSpell != SPELL_NONE;
}

SpellcasterIllager::Spell SpellcasterIllager::getCurrentSpell()
{
	if (!level->isClientSide) return currentSpell;
	return static_cast<Spell>(getEntityData()->getByte(DATA_SPELL_CASTING_ID));
}

void SpellcasterIllager::setIsCastingSpell(Spell spell)
{
	currentSpell = spell;
	getEntityData()->set(DATA_SPELL_CASTING_ID, static_cast<byte>(spell));
}

Illager::ArmPose SpellcasterIllager::getArmPose()
{
	// casters keep their arms crossed (or swinging) until they actually cast
	return isCastingSpell() ? SPELLCASTING : CROSSED;
}

void SpellcasterIllager::newServerAiStep()
{
	Illager::newServerAiStep();
	if (spellCastingTickCount > 0) spellCastingTickCount--;
}

void SpellcasterIllager::tick()
{
	Illager::tick();

	// the casting hands throw coloured particles (client side), like Java: 0.6 blocks out to both sides of the body
	if (level->isClientSide && isCastingSpell())
	{
		Spell spell = getCurrentSpell();
		double r = 0.7, g = 0.7, b = 0.8;
		if (spell == SPELL_SUMMON_VEX)	{ r = 0.7; g = 0.7; b = 0.8; }
		else if (spell == SPELL_FANGS)	{ r = 0.4; g = 0.3; b = 0.35; }
		else if (spell == SPELL_WOLOLO)	{ r = 0.7; g = 0.5; b = 0.2; }
		else if (spell == SPELL_ICE)	{ r = 0.55; g = 0.8; b = 0.95; }
		else if (spell == SPELL_DISAPPEAR)	{ r = 0.4; g = 0.6; b = 0.7; }
		else if (spell == SPELL_BLINDNESS)	{ r = 0.6; g = 0.6; b = 0.6; }

		float bodyAngle = yBodyRot * (PI / 180.0f) + Mth::cos(tickCount * 0.6662f) * 0.25f;
		float cs = Mth::cos(bodyAngle);
		float sn = Mth::sin(bodyAngle);
		level->addParticle(eParticleType_mobSpell, x + cs * 0.6, y + 1.8, z + sn * 0.6, r, g, b);
		level->addParticle(eParticleType_mobSpell, x - cs * 0.6, y + 1.8, z - sn * 0.6, r, g, b);
	}
}
