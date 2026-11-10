#pragma once
#include "Goal.h"

class Goat;

// neoLegacy - the goat's "very high jump": now and then an idle adult goat springs about 4-5 blocks
// into the air (with a little forward momentum), like the leaps modern goats make on mountainsides.
class GoatLeapGoal : public Goal
{
private:
	Goat *goat;

public:
	GoatLeapGoal(Goat *goat);

	virtual bool canUse();
	virtual bool canContinueToUse();
	virtual void start();
};
