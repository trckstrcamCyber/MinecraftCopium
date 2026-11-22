#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.item.h"
#include "SoundTypes.h"
#include "SynchedEntityData.h"
#include "Snowball.h"



void Snowball::_init()
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	getEntityData()->define(DATA_ICE_ID, static_cast<byte>(0));	// neoLegacy
	getEntityData()->define(DATA_HATCHET_ID, static_cast<byte>(0));	// neoLegacy
	hatchetBounced = false;
	hatchetStack = nullptr;
}

void Snowball::setHatchetStack(shared_ptr<ItemInstance> stack)
{
	hatchetStack = stack;
}

// Like a Java trident: damage on the first thing it hits, then it bounces off (nearly all its speed gone, so it drops) and cannot hit anything else.
// On a block it stops; the real hatchet, if it carries one, is left there for the thrower to pick up.
void Snowball::onHatchetHit(HitResult *res)
{
	if (level->isClientSide) return;

	if (res->entity != nullptr)
	{
		if (hatchetBounced) return;
		DamageSource *damageSource = DamageSource::thrown(shared_from_this(), getOwner());
		res->entity->hurt(damageSource, 8);
		delete damageSource;
		hatchetBounced = true;
		xd *= -0.01;
		yd *= -0.1;
		zd *= -0.01;
		playSound(eSoundType_RANDOM_BOW_HIT, 1.0f, 1.0f);
		return;
	}

	if (hatchetStack != nullptr)
	{
		spawnAtLocation(hatchetStack, 0.1f);
		hatchetStack = nullptr;
	}
	remove();
}

void Snowball::setHatchet(bool hatchet)
{
	getEntityData()->set(DATA_HATCHET_ID, static_cast<byte>(hatchet ? 1 : 0));
}

bool Snowball::isHatchet()
{
	return getEntityData()->getByte(DATA_HATCHET_ID) != 0;
}

void Snowball::setIce(bool ice)
{
	getEntityData()->set(DATA_ICE_ID, static_cast<byte>(ice ? 1 : 0));
}

bool Snowball::isIce()
{
	return getEntityData()->getByte(DATA_ICE_ID) != 0;
}

void Snowball::tick()
{
	Throwable::tick();
	if (isIce())
	{
		// sharp deceleration: an extra 10% of its speed is lost every tick on top of the normal air drag
		xd *= 0.9;
		yd *= 0.9;
		zd *= 0.9;
	}
}

Snowball::Snowball(Level *level) : Throwable(level)
{
	_init();
}

Snowball::Snowball(Level *level, shared_ptr<LivingEntity> mob) : Throwable(level,mob)
{
	_init();
}

Snowball::Snowball(Level *level, double x, double y, double z) : Throwable(level,x,y,z)
{
	_init();
}

void Snowball::onHit(HitResult *res)
{
	if (isHatchet())
	{
		onHatchetHit(res);
		return;
	}

	if (res->entity != nullptr)
	{
		int damage = 0;
		if ( res->entity->instanceof(eTYPE_BLAZE) )
		{
			damage = 3;
		}
		if (isIce())
		{
			// neoLegacy - Ice Ball: 1.395 x speed (blocks per tick), rounded up
			double speed = sqrt(xd * xd + yd * yd + zd * zd);
			damage = static_cast<int>(ceil(1.395 * speed));
		}

		DamageSource *damageSource = DamageSource::thrown(shared_from_this(), getOwner());
		res->entity->hurt(damageSource, damage);
		delete damageSource;
	}
	for (int i = 0; i < 8; i++)
		level->addParticle(eParticleType_snowballpoof, x, y, z, 0, 0, 0);
	if (!level->isClientSide)
	{
		remove();
	}
}