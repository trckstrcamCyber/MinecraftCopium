#pragma once
#include "SpellcasterIllager.h"

// neoLegacy - Invoker, from the Illager Invasion mod: the illager boss. 250 health, hovers (it falls slowly), never despawns, and its hit blinds for 5 seconds (it hits for 8).
// It keeps away from players and has five spells:
//  - a shockwave (everything within 6 blocks that is not on its side takes 11 magic damage and is flung away), every 15 seconds;
//  - a blink (to a random spot nearby) when a player or iron golem comes within 6 blocks, every 9 seconds;
//  - summon Surrendered: four at a time while fewer than three are near;
//  - fangs at everything non-monster within 18 blocks (a ring of five around each);
//  - the Evoker's fangs: a ring around it up close, three lines of fangs at range.
// A projectile hit sometimes (50%) puts up a shield that makes it immune to projectiles until a hit in melee breaks it (1 in 3).
// Drops 3-5 emeralds (+Looting) and a Primal Essence when a player kills it. Joins raids from wave 6 and wave 8, one at a time.
// Not here yet: the boss health bar and the glowing layers the mod draws (gold eyes, shield armor).
class Invoker : public SpellcasterIllager
{
public:
	eINSTANCEOF GetType() { return eTYPE_INVOKER; }
	static Entity *create(Level *level) { return new Invoker(level); }

	Invoker(Level *level);

	// the spell goals reach these
	int areaDamageCooldown;
	int teleportCooldown;
	int fangAoeCooldown;
	bool isAoeCasting;

	virtual int getCastingSoundId();
	virtual bool doHurtTarget(shared_ptr<Entity> target);
	virtual bool hurt(DamageSource *source, float dmg);
	virtual bool isAlliedTo(shared_ptr<LivingEntity> other);
	virtual bool removeWhenFarAway();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

private:
	bool shielded;	// server only

protected:
	virtual void registerAttributes();
	virtual void newServerAiStep();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
};
