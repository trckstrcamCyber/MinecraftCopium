#include "stdafx.h"
#include "ItemInstance.h"
#include "DamageSource.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "MinecartDiamond.h"

static const double DIAMOND_MAX_SPEED = 0.8;
static const double RAM_MIN_SPEED = 0.45;		// slower than this it just bumps things like a normal cart
static const float  RAM_MAX_DAMAGE = 12.0f;

MinecartDiamond::MinecartDiamond(Level *level) : MinecartRideable(level)
{
}

MinecartDiamond::MinecartDiamond(Level *level, double x, double y, double z) : MinecartRideable(level, x, y, z)
{
}

int MinecartDiamond::getType()
{
	return TYPE_DIAMOND;
}

double MinecartDiamond::getMaxSpeed()
{
	return DIAMOND_MAX_SPEED;
}

void MinecartDiamond::applyNaturalSlowdown()
{
	// Much less drag than a plain cart (0.997 ridden / 0.96 empty).
	if (rider.lock() != nullptr)
	{
		xd *= 0.9995;
		zd *= 0.9995;
	}
	else
	{
		xd *= 0.985;
		zd *= 0.985;
	}
	yd *= 0;
}

bool MinecartDiamond::hurt(DamageSource *source, float damage)
{
	// Sturdy: half damage, so it takes twice the punishment to break.
	return MinecartRideable::hurt(source, damage * 0.5f);
}

void MinecartDiamond::destroy(DamageSource *source)
{
	remove();
	shared_ptr<ItemInstance> item = std::make_shared<ItemInstance>(Item::diamond_minecart, 1);
	spawnAtLocation(item, 0);
}

void MinecartDiamond::push(shared_ptr<Entity> e)
{
	if (!level->isClientSide && e != rider.lock() && e->instanceof(eTYPE_LIVINGENTITY))
	{
		double speed = sqrt(xd * xd + zd * zd);
		shared_ptr<LivingEntity> living = dynamic_pointer_cast<LivingEntity>(e);
		if (speed > RAM_MIN_SPEED && living != nullptr && living->isAlive())
		{
			// Damage scales from ~1 at ram speed to the cap at full speed.
			float dmg = static_cast<float>((speed - RAM_MIN_SPEED) / (DIAMOND_MAX_SPEED - RAM_MIN_SPEED) * RAM_MAX_DAMAGE) + 2.0f;
			if (dmg > RAM_MAX_DAMAGE) dmg = RAM_MAX_DAMAGE;

			shared_ptr<LivingEntity> driver = dynamic_pointer_cast<LivingEntity>(rider.lock());
			DamageSource *src = (driver != nullptr) ? DamageSource::mobAttack(driver) : DamageSource::cactus;
			if (living->hurt(src, dmg))
			{
				// Throw it clear of the track, sideways to the cart's motion.
				double len = speed > 0.0001 ? speed : 1.0;
				living->xd += (-zd / len) * 0.6 * ((e->x - x) * (-zd) + (e->z - z) * xd >= 0 ? 1 : -1) + xd * 0.3;
				living->zd += ( xd / len) * 0.6 * ((e->x - x) * (-zd) + (e->z - z) * xd >= 0 ? 1 : -1) + zd * 0.3;
				living->yd = 0.35;
				living->hurtMarked = true;
			}
			return;		// rammed it: skip the normal "shove the cart" reaction so the cart keeps its speed
		}
	}
	MinecartRideable::push(e);
}
