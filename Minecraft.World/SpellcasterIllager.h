#pragma once
#include "Illager.h"

// neoLegacy - an illager that casts spells (Evoker, Iceologer). A spell goes through three stages the goals below drive:
//   prepare (sound + the spell id is synched so the client shows the casting pose and particles),
//   warm-up (the caster stands still facing its target),
//   cast    (performSpellCasting on the server).
class SpellcasterIllager : public Illager
{
public:
	enum Spell { SPELL_NONE = 0, SPELL_SUMMON_VEX = 1, SPELL_FANGS = 2, SPELL_WOLOLO = 3, SPELL_ICE = 4, SPELL_DISAPPEAR = 5, SPELL_BLINDNESS = 6 };

protected:
	static const int DATA_SPELL_CASTING_ID = 17;
	int spellCastingTickCount;
	Spell currentSpell;		// server only

public:
	SpellcasterIllager(Level *level);

	virtual bool isCastingSpell();
	virtual Spell getCurrentSpell();
	virtual void setIsCastingSpell(Spell spell);
	virtual int getSpellCastingTime() { return spellCastingTickCount; }
	virtual void setSpellCastingTime(int ticks) { spellCastingTickCount = ticks; }
	virtual int getCastingSoundId() = 0;

	virtual ArmPose getArmPose();
	virtual void tick();

protected:
	virtual void newServerAiStep();
};
