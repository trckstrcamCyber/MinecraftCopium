#pragma once
using namespace std;

#include "Throwable.h"

class HitResult;

class Snowball : public Throwable
{
public:
	eINSTANCEOF GetType() { return eTYPE_SNOWBALL; }
	static Entity *create(Level *level) { return new Snowball(level); }

private:
	void _init();
	static const int DATA_ICE_ID = 8;	// neoLegacy - synched: this "snowball" is an Ice Ball

public:
	Snowball(Level *level);
	Snowball(Level *level, shared_ptr<LivingEntity> mob);
	Snowball(Level *level, double x, double y, double z);

	// neoLegacy - Ice Ball: thrown like a snowball, but damage scales with its speed at impact (1.395 x blocks per tick, rounded up)
	// and it loses speed fast, so it hits hard up close and barely stings from far away.
	void setIce(bool ice);
	bool isIce();
	virtual void tick();

protected:
	virtual void onHit(HitResult *res);
};