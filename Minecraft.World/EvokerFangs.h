#pragma once
#include "Mob.h"
#include "Creature.h"

// neoLegacy - the snapping jaws an Evoker raises from the ground. It is built as a harmless, invulnerable Mob so it spawns,
// syncs and renders through the ordinary mob path (no new packet types needed): after a warm-up it snaps shut for 6 damage
// on everything standing in it (except illagers), then disappears.
// Creature is an empty marker, but the entity tracker and TrackedEntity::getAddEntityPacket only send Creatures to clients - without it
// the server builds no spawn packet for this entity and the game crashes the moment it is added.
class EvokerFangs : public Mob, public Creature
{
public:
	eINSTANCEOF GetType() { return eTYPE_EVOKER_FANGS; }
	static Entity *create(Level *level) { return new EvokerFangs(level); }

private:
	int warmupDelayTicks;
	bool sentSpikeEvent;
	int lifeTicks;
	bool clientSideAttackStarted;
	weak_ptr<LivingEntity> owner;

public:
	EvokerFangs(Level *level);
	EvokerFangs(Level *level, double x, double y, double z, float yRot, int warmup, shared_ptr<LivingEntity> owner);

	virtual void tick();
	virtual void handleEntityEvent(byte id);
	virtual bool hurt(DamageSource *source, float dmg);
	virtual bool isPushable() { return false; }
	virtual bool isPickable() { return false; }
	virtual bool removeWhenFarAway() { return false; }

	// 0 while the fangs are still hidden, 0..1 once they have started to bite (drives the model)
	float getAnimationProgress(float a);

protected:
	virtual void registerAttributes();
	virtual bool useNewAi() { return true; }

private:
	void dealDamageTo(shared_ptr<LivingEntity> target);
};
