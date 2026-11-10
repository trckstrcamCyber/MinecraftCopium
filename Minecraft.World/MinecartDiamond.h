#pragma once
#include "MinecartRideable.h"

// neoLegacy - Diamond Minecart: a brand-new cart tier.
//   * FAST: tops out at 0.8 blocks/tick on a rail (twice a normal cart's 0.4) and loses much less speed
//     to friction, so a powered-rail line really gets it moving;
//   * a RAM: at speed it slams into mobs it hits (players riding on it are never hurt), dealing damage
//     that grows with its speed and knocking them aside;
//   * STURDY: it takes half damage from hits, so it isn't smashed by a stray arrow or a clumsy swing, and
//     drops itself (a diamond minecart, not a plain one) when finally broken.
// Crafted at the Complex Table from a minecart, powered rail and diamonds.
class MinecartDiamond : public MinecartRideable
{
public:
	eINSTANCEOF GetType() { return eTYPE_MINECART_DIAMOND; }
	static Entity *create(Level *level) { return new MinecartDiamond(level); }

	MinecartDiamond(Level *level);
	MinecartDiamond(Level *level, double x, double y, double z);

	virtual int getType();
	virtual double getMaxSpeed();
	virtual bool hurt(DamageSource *source, float damage);
	virtual void destroy(DamageSource *source);
	virtual void push(shared_ptr<Entity> e);

protected:
	virtual void applyNaturalSlowdown();
};
