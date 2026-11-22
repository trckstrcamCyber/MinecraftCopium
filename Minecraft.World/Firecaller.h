#pragma once
#include "SpellcasterIllager.h"

// neoLegacy - Firecaller, from the Illager Invasion mod: a staff-carrying fire illager. 32 health, keeps away from players and iron golems. Two spells:
//  - Flying Magma: a fireball at its target (12 damage and a small fiery explosion) every ~8 seconds, but only while no player or iron golem is within 5 blocks of it;
//  - Fire burst: every ~15 seconds everything within 6 blocks that is not an illager, Surrendered or Ravager is thrown up, takes 6 magic damage, is set alight for 6 seconds
//    and gets a fire at its feet (fires only with mob griefing on).
// Drops 1-3 Fire Charges, and emeralds when a player kills it. Not part of raids; its own hut (a structure) comes later.
class Firecaller : public SpellcasterIllager
{
public:
	eINSTANCEOF GetType() { return eTYPE_FIRECALLER; }
	static Entity *create(Level *level) { return new Firecaller(level); }

	Firecaller(Level *level);

	// the spell goals reach these
	int conjureSkullCooldown;
	int areaDamageCooldown;

	virtual int getCastingSoundId();
	virtual ArmPose getArmPose();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

protected:
	virtual void registerAttributes();
	virtual void newServerAiStep();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
};
