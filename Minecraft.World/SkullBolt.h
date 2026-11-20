#pragma once
#include "WitherSkull.h"

// neoLegacy - the Necromancer's skull (Illager Invasion mod). A Wither Skull that never explodes or withers: it hurts for 7 magic damage and slows
// for five seconds, but an undead mob it hits is healed 5 and sped up instead. Deliberately does NOT override GetType(): as far as saving, networking
// and drawing go it is a Wither Skull (so it looks like one, and is one again if the world is reloaded mid-flight).
class SkullBolt : public WitherSkull
{
public:
	SkullBolt(Level *level, shared_ptr<LivingEntity> owner, double xa, double ya, double za);

protected:
	virtual void onHit(HitResult *res);
};
