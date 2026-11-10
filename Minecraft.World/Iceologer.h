#pragma once
#include "SpellcasterIllager.h"

// neoLegacy - Iceologer (ported from the FrozenFiend mod): an ice illager that conjures an Ice Cube above its target.
// 24 health, keeps its distance from players, spawns in snowy biomes and hills, drops an Ice Wand and emeralds.
class Iceologer : public SpellcasterIllager
{
public:
	eINSTANCEOF GetType() { return eTYPE_ICEOLOGER; }
	static Entity *create(Level *level) { return new Iceologer(level); }

	// cape physics (client visuals, updated every tick like the original mod)
	double prevChasingPosX, prevChasingPosY, prevChasingPosZ;
	double chasingPosX, chasingPosY, chasingPosZ;

	Iceologer(Level *level);

	virtual void tick();
	virtual int getCastingSoundId();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

protected:
	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();

private:
	void updateCape();
};
