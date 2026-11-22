#pragma once
#include "SpellcasterIllager.h"

// neoLegacy - Sorcerer, from the Illager Invasion mod: a fragile-looking illager that never fights hand to hand. 32 health, keeps away from players
// and casts two spells at its target: it conjures flames (3 magic damage and a fire at the target's feet when mob griefing is on) every seven
// seconds, and when a player or iron golem comes within 8 blocks it teleports away (every eleven seconds). Drops books, and emeralds when a
// player kills it. Joins raids from wave 6 on.
class Sorcerer : public SpellcasterIllager
{
public:
	eINSTANCEOF GetType() { return eTYPE_SORCERER; }
	static Entity *create(Level *level) { return new Sorcerer(level); }

	Sorcerer(Level *level);

	virtual int getCastingSoundId();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

	// the teleport spell: somewhere within about 12 blocks, on solid ground, out of the water. false if nothing suitable was found.
	bool tryRandomTeleport();
	static bool randomTeleport(Mob *mob);	// the same blink for any mob (the Invoker uses it)

protected:
	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
};
