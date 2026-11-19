#pragma once
#include "Mob.h"
#include "Creature.h"

// neoLegacy - Ice Cube (from the Iceologer mod): a 2x1.5 block of ice conjured above a victim. It hovers over its target for
// a while, then plunges: whatever it lands on takes damage by how far it fell (3 per block) plus Nausea and heavy Slowness.
// Built as a harmless, invulnerable Mob so it spawns / syncs / renders through the normal mob path.
// Creature is an empty marker, but the entity tracker and TrackedEntity::getAddEntityPacket only send Creatures to clients - without it
// the server builds no spawn packet for this entity and the game crashes the moment it is added.
class IceCube : public Mob, public Creature
{
public:
	static const int FLOAT_TICKS = 60;			// default hover time before it drops
	static const int DATA_HOVER_ID = 16;		// synched: the hover time the server gave it, so the client's copy drops at the same moment
	eINSTANCEOF GetType() { return eTYPE_ICE_CUBE; }
	static Entity *create(Level *level) { return new IceCube(level); }

private:
	static const float FALL_DAMAGE_PER_BLOCK;	// damage per block fallen
	int floatTicks;
	int fallTime;
	bool landed;
	bool dropping;			// hover is over, it is now falling
	bool homing;			// the Ice Wand's cubes slide after their target while they hover and fall, so they land on it
	bool hoverSynced;		// (client) has read the hover time from the server
	double dropStartY;		// where the fall began (the damage depends on how far it fell)
	weak_ptr<LivingEntity> caster;
	weak_ptr<LivingEntity> target;

public:
	IceCube(Level *level);
	// hoverTicks: how long it hovers over the target before dropping (the Iceologer's spell: 3 s, the Ice Wand: about 1.2 s)
	IceCube(Level *level, shared_ptr<LivingEntity> caster, shared_ptr<LivingEntity> target, int hoverTicks = FLOAT_TICKS, bool homing = false);

	virtual void tick();
	virtual void travel(float xa, float ya);
	virtual bool hurt(DamageSource *source, float dmg);
	virtual bool isPushable() { return false; }
	virtual bool isPickable() { return false; }
	virtual bool removeWhenFarAway() { return false; }

protected:
	virtual void registerAttributes();
	virtual bool useNewAi() { return true; }
	virtual void causeFallDamage(float distance);

private:
	void damageEntity(shared_ptr<LivingEntity> victim, int blocksFallen);
	void trackTarget();
};
