#pragma once
#include "SpellcasterIllager.h"

// neoLegacy - Necromancer, from the Illager Invasion mod: a 32 health illager that raises the dead. It never fights by itself, keeps away from players and:
//  - summons undead: two zombies (four at night) and a skeleton next to it, up to four times in its life, while few zombies are near;
//  - conjures skulls (Skull Bolts): 7 magic damage and slowness on whatever they hit - but undead they hit are healed and sped up instead;
//  - links with undead within 10 blocks: they get Strength and Speed, stop burning and share its target (every undead within 30 blocks is told its target),
//    and while any are near it is shielded: it takes half damage.
// Drops emeralds, and now and then a skeleton skull or zombie head, when a player kills it. Joins raids from wave 5 on.
// (The mod draws a glowing armor layer while it is shielded; there is no such layer here yet, so the shield is only felt, not seen.)
class Necromancer : public SpellcasterIllager
{
private:
	bool shielded;	// server only

public:
	eINSTANCEOF GetType() { return eTYPE_NECROMANCER; }
	static Entity *create(Level *level) { return new Necromancer(level); }

	Necromancer(Level *level);

	virtual int getCastingSoundId();
	virtual bool hurt(DamageSource *source, float dmg);
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

protected:
	virtual void registerAttributes();
	virtual void newServerAiStep();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
};
