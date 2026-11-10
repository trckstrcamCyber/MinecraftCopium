#pragma once
#include "Monster.h"

// neoLegacy - base class of the illagers (Vindicator, Evoker, Iceologer; Pillager and friends follow).
// Illagers share one thing the client needs: whether the mob is hunting something (it can't see the target, only the
// server can), so the server keeps a synched "aggressive" flag and the model reads it to pick the arm pose.
class Illager : public Monster
{
public:
	enum ArmPose { CROSSED, ATTACKING, SPELLCASTING, CELEBRATING, NEUTRAL, BOW_AND_ARROW, CROSSBOW_HOLD, CROSSBOW_CHARGE };

protected:
	static const int DATA_FLAGS_ID = 16;	// bit 0: aggressive

public:
	Illager(Level *level);

	virtual void tick();
	virtual bool isAggressive();
	virtual ArmPose getArmPose();

	// illagers and their summons never turn on each other
	virtual bool isAlliedTo(shared_ptr<LivingEntity> other);

protected:
	// Without this the mob falls back to the old pre-goal AI: it just walks up and hits, never casts, and getTarget() stays empty
	// (so the aggressive flag / arm pose never changes). Zombie does the same.
	virtual bool useNewAi();
};
