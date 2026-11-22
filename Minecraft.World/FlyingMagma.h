#pragma once
#include "LargeFireball.h"

// neoLegacy - the Firecaller's Flying Magma (Illager Invasion mod): a fireball that does 12 magic damage to what it hits and then explodes with fire (power 1, blocks only with
// mob griefing on). A LargeFireball that deliberately keeps its entity type, so it needs no entity id, tracker or renderer: it flies and looks like a Ghast's fireball.
class FlyingMagma : public LargeFireball
{
public:
	FlyingMagma(Level *level, shared_ptr<LivingEntity> owner, double xa, double ya, double za);

	virtual void onHit(HitResult *res);
};
