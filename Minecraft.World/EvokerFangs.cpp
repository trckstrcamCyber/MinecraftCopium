#include "stdafx.h"
#include "Random.h"
#include "AttributeInstance.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "SharedMonsterAttributes.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "SoundTypes.h"
#include "Illager.h"
#include "Vex.h"
#include "EvokerFangs.h"

EvokerFangs::EvokerFangs(Level *level) : Mob(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	setSize(0.5f, 0.8f);
	warmupDelayTicks = 0;
	sentSpikeEvent = false;
	lifeTicks = 22;
	clientSideAttackStarted = false;
}

EvokerFangs::EvokerFangs(Level *level, double x, double y, double z, float yRot, int warmup, shared_ptr<LivingEntity> owner) : Mob(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	setSize(0.5f, 0.8f);
	warmupDelayTicks = warmup;
	sentSpikeEvent = false;
	lifeTicks = 22;
	clientSideAttackStarted = false;
	this->owner = owner;
	this->yRot = yRot * (180.0f / PI);
	setPos(x, y, z);
}

void EvokerFangs::registerAttributes()
{
	Mob::registerAttributes();
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(1);
}

bool EvokerFangs::hurt(DamageSource *source, float dmg)
{
	return false;	// invulnerable
}

void EvokerFangs::tick()
{
	// no AI, no gravity, no movement: only the bite timer matters
	Entity::tick();

	if (level->isClientSide)
	{
		if (clientSideAttackStarted)
		{
			lifeTicks--;
			if (lifeTicks == 14)
			{
				for (int i = 0; i < 12; i++)
				{
					double px = x + (random->nextDouble() * 2.0 - 1.0) * bbWidth * 0.5;
					double py = y + 0.05 + random->nextDouble();
					double pz = z + (random->nextDouble() * 2.0 - 1.0) * bbWidth * 0.5;
					level->addParticle(eParticleType_crit, px, py, pz, (random->nextDouble() * 2.0 - 1.0) * 0.3, 0.3 + random->nextDouble() * 0.3, (random->nextDouble() * 2.0 - 1.0) * 0.3);
				}
			}
		}
	}
	else if (--warmupDelayTicks < 0)
	{
		if (warmupDelayTicks == -8)
		{
			vector<shared_ptr<Entity> > *targets = level->getEntitiesOfClass(typeid(LivingEntity), bb->grow(0.2, 0.0, 0.2));
			if (targets != nullptr)
			{
				for (size_t i = 0; i < targets->size(); i++)
				{
					shared_ptr<LivingEntity> living = dynamic_pointer_cast<LivingEntity>((*targets)[i]);
					if (living != nullptr) dealDamageTo(living);
				}
			}
		}

		if (!sentSpikeEvent)
		{
			level->broadcastEntityEvent(shared_from_this(), 4);
			level->playEntitySound(shared_from_this(), eSoundType_MOB_EVOKER_FANGS, 1.0f, random->nextFloat() * 0.2f + 0.85f);
			sentSpikeEvent = true;
		}

		if (--lifeTicks < 0) remove();
	}
}

void EvokerFangs::handleEntityEvent(byte id)
{
	Mob::handleEntityEvent(id);
	if (id == 4) clientSideAttackStarted = true;
}

void EvokerFangs::dealDamageTo(shared_ptr<LivingEntity> target)
{
	shared_ptr<LivingEntity> caster = owner.lock();
	if (target == nullptr || !target->isAlive() || target.get() == this || target == caster) return;
	if (dynamic_cast<EvokerFangs *>(target.get()) != nullptr) return;
	// illagers and their summons are not hurt by their own team's fangs
	if (dynamic_cast<Illager *>(target.get()) != nullptr || dynamic_cast<Vex *>(target.get()) != nullptr) return;

	if (caster == nullptr) target->hurt(DamageSource::magic, 6.0f);
	else target->hurt(DamageSource::indirectMagic(shared_from_this(), caster), 6.0f);
}

float EvokerFangs::getAnimationProgress(float a)
{
	if (!clientSideAttackStarted) return 0.0f;
	int remaining = lifeTicks - 2;
	return remaining <= 0 ? 1.0f : 1.0f - (remaining - a) / 20.0f;
}
