#pragma once
#include "Goal.h"
#include "SpellcasterIllager.h"

// neoLegacy - the two goals every spellcasting illager shares (Evoker, Iceologer). Modelled on Java Edition's
// SpellcasterIllager.SpellcasterCastingSpellGoal / SpellcasterUseSpellGoal.

// While a spell is being cast the caster stands still and watches its target.
class SpellcasterCastingSpellGoal : public Goal
{
private:
	SpellcasterIllager *mob;
public:
	SpellcasterCastingSpellGoal(SpellcasterIllager *mob);
	virtual bool canUse();
	virtual void start();
	virtual void stop();
	virtual void tick();
};

// Base of the individual spells: waits for its cooldown and a target, announces the spell, waits out the warm-up, then casts.
class SpellcasterUseSpellGoal : public Goal
{
protected:
	SpellcasterIllager *mob;
	int attackWarmupDelay;
	int nextAttackTickCount;

public:
	SpellcasterUseSpellGoal(SpellcasterIllager *mob);

	virtual bool canUse();
	virtual bool canContinueToUse();
	virtual bool canInterrupt();
	virtual void start();
	virtual void tick();

protected:
	virtual void performSpellCasting() = 0;
	virtual int getCastWarmupTime() = 0;
	virtual int getCastingTime() = 0;
	virtual int getCastingInterval() = 0;
	virtual int getSpellPrepareSound() = 0;		// an eSoundType, or -1 for none
	virtual SpellcasterIllager::Spell getSpell() = 0;
};
