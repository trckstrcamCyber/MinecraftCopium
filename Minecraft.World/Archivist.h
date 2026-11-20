#pragma once
#include "SpellcasterIllager.h"

// neoLegacy - Archivist, from the Illager Invasion mod: a book-keeping illager that never fights hand to hand. 24 health, keeps away from players and has two spells:
//  - a shockwave (the mod calls it "levitate", but it has always been a push): everything alive that is not a monster within 6 blocks (creative players aside)
//    takes 4 magic damage and is flung away from it, every eleven seconds;
//  - enchanting an ally: a random illager within 16 blocks that holds a weapon has it enchanted - Power III for a bow, Sharpness III for a sword or axe,
//    Piercing IV or Multishot for a crossbow.
// Drops paper, books, and emeralds when a player kills it. Joins raids from wave 2 on.
class Archivist : public SpellcasterIllager
{
public:
	eINSTANCEOF GetType() { return eTYPE_ARCHIVIST; }
	static Entity *create(Level *level) { return new Archivist(level); }

	Archivist(Level *level);

	virtual int getCastingSoundId();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

	// true if the illager holds something EnchantAlly can improve
	static bool eligibleForEnchant(shared_ptr<LivingEntity> entity);
	static void enchantHeldGear(shared_ptr<LivingEntity> entity);

protected:
	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
};
