#include "stdafx.h"
#include "MobEffectInstance.h"
#include "MobEffect.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "Frostbite.h"
#include "ItemInstance.h"
#include "SynchedEntityData.h"
#include "Material.h"
#include "SharedConstants.h"
#include "SoundTypes.h"
#include "Random.h"
#include "Mth.h"
#include "Sensing.h"
#include "PathNavigation.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.projectile.h"

namespace
{
	// A Frostbite holding Ice Balls stops at a distance, faces its target and pelts it, then closes in for melee once it is out of them
	// (or the target comes within 3 blocks).
	class FrostbiteThrowIceGoal : public Goal
	{
		Frostbite *mob;
		int cooldown;

		bool inRange()
		{
			shared_ptr<LivingEntity> target = mob->getTarget();
			if (target == nullptr || !target->isAlive() || !mob->hasIceBalls()) return false;
			double d2 = mob->distanceToSqr(target);
			return d2 >= 9.0 && d2 <= 100.0 && mob->getSensing()->canSee(target);
		}

	public:
		FrostbiteThrowIceGoal(Frostbite *mob)
		{
			this->mob = mob;
			cooldown = 0;
			setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
		}

		virtual bool canUse() { return inRange(); }
		virtual bool canContinueToUse() { return inRange(); }
		virtual void start() { cooldown = 10; }

		virtual void tick()
		{
			shared_ptr<LivingEntity> target = mob->getTarget();
			if (target == nullptr) return;
			mob->getLookControl()->setLookAt(target, 30.0f, 30.0f);
			mob->getNavigation()->stop();
			if (--cooldown <= 0)
			{
				mob->throwIceBall(target);
				cooldown = 30 + mob->getRandom()->nextInt(20);
			}
		}
	};
}

Frostbite::Frostbite(Level *level) : Zombie(level)
{
	// Zombie's constructor already registered attributes, goals and targets: a Frostbite has the same stats and AI as a zombie.
	underWaterTicks = 0;

	// Zombie's constructor called Zombie::defineSynchedData (a base constructor can't reach this class's version), so define our own flag here
	getEntityData()->define(DATA_AGGRESSIVE_ID, static_cast<byte>(0));

	goalSelector.addGoal(1, new FrostbiteThrowIceGoal(this));
}

// one spawn in three carries 4-12 Ice Balls
MobGroupData *Frostbite::finalizeMobSpawn(MobGroupData *groupData, int extraData)
{
	groupData = Zombie::finalizeMobSpawn(groupData, extraData);
	if (random->nextInt(3) == 0)
	{
		setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(Item::ice_ball, 4 + random->nextInt(9), 0));
		dropChances[SLOT_WEAPON] = 0;
	}
	return groupData;
}

bool Frostbite::hasIceBalls()
{
	shared_ptr<ItemInstance> held = getCarried(SLOT_WEAPON);
	return held != nullptr && held->id == Item::ice_ball_Id && held->count > 0;
}

void Frostbite::throwIceBall(shared_ptr<LivingEntity> target)
{
	if (!hasIceBalls() || target == nullptr || level->isClientSide) return;

	// same throw as the Snow Golem's snowball, but it is an Ice Ball (speed-scaled damage, sharp deceleration)
	shared_ptr<Snowball> ball = std::make_shared<Snowball>(level, dynamic_pointer_cast<LivingEntity>(shared_from_this()));
	ball->setIce(true);
	double dx = target->x - x;
	double dy = (target->y + target->getHeadHeight() - 1.1f) - ball->y;
	double dz = target->z - z;
	float lift = Mth::sqrt(dx * dx + dz * dz) * 0.2f;
	ball->shoot(dx, dy + lift, dz, 1.6f, 6);

	playSound(eSoundType_RANDOM_BOW, 1.0f, 1.0f / (getRandom()->nextFloat() * 0.4f + 0.8f));
	level->addEntity(ball);

	// use one up
	shared_ptr<ItemInstance> held = getCarried(SLOT_WEAPON);
	held->count--;
	if (held->count <= 0) setEquippedSlot(SLOT_WEAPON, nullptr);
}

bool Frostbite::isAggressive()
{
	return getEntityData()->getByte(DATA_AGGRESSIVE_ID) != 0;
}

void Frostbite::meltIntoZombie()
{
	// Left submerged in water for 15 seconds the ice melts away and a plain zombie is left behind.
	shared_ptr<Zombie> zombie = std::make_shared<Zombie>(level);
	zombie->copyPosition(shared_from_this());
	zombie->setBaby(isBaby());
	for (int slot = 0; slot < 5; slot++)
	{
		shared_ptr<ItemInstance> item = getCarried(slot);
		if (item != nullptr) zombie->setEquippedSlot(slot, item->copy());
	}
	zombie->setHealth(getHealth() / getMaxHealth() * zombie->getMaxHealth());
	level->addEntity(zombie);
	playSound(eSoundType_MOB_FROSTBITE_CONVERT, 1.0f, 1.0f);
	remove();
}

void Frostbite::tick()
{
	if (!level->isClientSide)
	{
		// keep the clients informed whether it is hunting something (they can't see the target)
		byte aggressive = (getTarget() != nullptr) ? 1 : 0;
		if (getEntityData()->getByte(DATA_AGGRESSIVE_ID) != aggressive) getEntityData()->set(DATA_AGGRESSIVE_ID, aggressive);

		if (isUnderLiquid(Material::water))
		{
			if (++underWaterTicks >= MELT_TICKS) { meltIntoZombie(); return; }
		}
		else
		{
			underWaterTicks = 0;
		}
	}
	Zombie::tick();
}

bool Frostbite::doHurtTarget(shared_ptr<Entity> target)
{
	bool result = Zombie::doHurtTarget(target);

	if (result)
	{
		shared_ptr<LivingEntity> living = dynamic_pointer_cast<LivingEntity>(target);
		if (living != nullptr)
		{
			// 3 seconds of Freezing per hit; hits that land while it is still active keep it going, so it builds up
			living->addEffect(new MobEffectInstance(MobEffect::freezing->id, 3 * SharedConstants::TICKS_PER_SECOND, 0));
		}
	}
	return result;
}

// the usual zombie loot, plus 0-2 Ice Balls (looting adds more)
void Frostbite::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Zombie::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);

	int count = random->nextInt(3);
	if (playerBonusLevel > 0) count += random->nextInt(playerBonusLevel + 1);
	for (int i = 0; i < count; i++) spawnAtLocation(Item::ice_ball_Id, 1);
}

int Frostbite::getAmbientSound() { return eSoundType_MOB_FROSTBITE_IDLE; }
int Frostbite::getHurtSound() { return eSoundType_MOB_FROSTBITE_HURT; }
int Frostbite::getDeathSound() { return eSoundType_MOB_FROSTBITE_DEATH; }

void Frostbite::playStepSound(int xt, int yt, int zt, int t)
{
	playSound(eSoundType_MOB_FROSTBITE_STEP, 0.15f, 1.0f);
}
