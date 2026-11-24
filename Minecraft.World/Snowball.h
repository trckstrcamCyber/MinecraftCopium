#pragma once
using namespace std;

#include "Throwable.h"

class HitResult;

class ItemInstance;

class Snowball : public Throwable
{
public:
	eINSTANCEOF GetType() { return eTYPE_SNOWBALL; }
	static Entity *create(Level *level) { return new Snowball(level); }

private:
	void _init();
	static const int DATA_ICE_ID = 8;	// neoLegacy - synched: this "snowball" is an Ice Ball
	static const int DATA_HATCHET_ID = 9;	// neoLegacy - synched: this "snowball" is a Marauder's thrown hatchet
	static const int DATA_HATCHET_BOUNCED_ID = 10;	// neoLegacy - synched: the server's copy bounced off a mob

public:
	Snowball(Level *level);
	Snowball(Level *level, shared_ptr<LivingEntity> mob);
	Snowball(Level *level, double x, double y, double z);

	// neoLegacy - Ice Ball: thrown like a snowball, but damage scales with its speed at impact (1.395 x blocks per tick, rounded up)
	// and it loses speed fast, so it hits hard up close and barely stings from far away.
	void setIce(bool ice);
	bool isIce();

	// neoLegacy - a thrown Platinum Infused Hatchet (a Marauder's or a player's): drawn as the hatchet, 8 damage, bounces off what it hits and drops where it lands
	void setHatchet(bool hatchet);
	bool isHatchet();
	void setHatchetStack(shared_ptr<ItemInstance> stack);	// the real item to drop for pickup (server only; none for a Marauder's)
	virtual void tick();

protected:
	virtual void onHit(HitResult *res);

private:
	shared_ptr<ItemInstance> hatchetStack;
	bool hatchetBounced;
	bool hatchetLanded;	// client-side only: the visual hatchet has stopped at a block; see onHatchetHit
	int hatchetLandedTicks;	// client-side only: ticks spent frozen; self-removes after 100 if no server removal arrives
	int hatchetBouncedTicks;	// client-side only: ticks since the bounce; self-removes after 100 if it never lands
	void onHatchetHit(HitResult *res);
};