#pragma once
#include "SpellcasterIllager.h"

class Sheep;

// neoLegacy - Evoker: the robed illager caster. 24 health, keeps its distance and casts three spells:
//   * fangs   - a ring around it when you are close, a line of jaws racing towards you when you are not,
//   * vex     - summons three Vexes (only if few are already around),
//   * wololo  - turns nearby blue sheep red when it has nothing to fight.
// (It does NOT drop a Totem of Undying - that is reserved for a boss to come.)
class Evoker : public SpellcasterIllager
{
public:
	eINSTANCEOF GetType() { return eTYPE_EVOKER; }
	static Entity *create(Level *level) { return new Evoker(level); }

private:
	weak_ptr<Sheep> wololoTarget;

public:
	Evoker(Level *level);

	virtual int getCastingSoundId();
	void setWololoTarget(shared_ptr<Sheep> sheep);
	shared_ptr<Sheep> getWololoTarget();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

protected:
	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
};
