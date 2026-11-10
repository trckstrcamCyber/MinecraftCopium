#include "stdafx.h"
#include "Random.h"
#include "AttributeInstance.h"
#include "ItemInstance.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "com.mojang.nbt.h"
#include "SynchedEntityData.h"
#include "SoundTypes.h"
#include "Mth.h"
#include "Vex.h"
#include "Illager.h"

Vex::Vex(Level *level) : Monster(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	getEntityData()->define(DATA_FLAGS_ID, static_cast<byte>(0));

	setSize(0.4f, 0.8f);
	fireImmune = true;
	xpReward = Enemy::XP_REWARD_SMALL;

	hasBound = false;
	boundX = boundY = boundZ = 0;
	hasLimitedLife = false;
	limitedLifeTicks = 0;
	hasWanted = false;
	wantedX = wantedY = wantedZ = 0;

	// Who it hunts: whoever hurts it, and any player it can see (a Vex that has an Evoker also copies the Evoker's target, see newServerAiStep)
	targetSelector.addGoal(1, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));
}

bool Vex::isInWall()
{
	return false;
}

void Vex::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(14);
	getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(4);
}

bool Vex::useNewAi() { return true; }

void Vex::tick()
{
	// the Vex phases through walls: collisions are switched off while it moves
	noPhysics = true;
	Monster::tick();
	noPhysics = false;
}

// flying: no gravity, light air drag
void Vex::travel(float xa, float ya)
{
	move(xd, yd, zd);
	xd *= 0.91f;
	yd *= 0.91f;
	zd *= 0.91f;
}

void Vex::setOwner(shared_ptr<Mob> owner) { this->owner = owner; }
shared_ptr<Mob> Vex::getOwner() { return owner.lock(); }

void Vex::setBoundOrigin(int x, int y, int z)
{
	hasBound = true;
	boundX = x; boundY = y; boundZ = z;
}

void Vex::setLifeTicks(int ticks)
{
	hasLimitedLife = true;
	limitedLifeTicks = ticks;
}

bool Vex::isCharging() { return (getEntityData()->getByte(DATA_FLAGS_ID) & 1) != 0; }

void Vex::setIsCharging(bool charging)
{
	byte flags = getEntityData()->getByte(DATA_FLAGS_ID);
	flags = charging ? (flags | 1) : (flags & ~1);
	getEntityData()->set(DATA_FLAGS_ID, flags);
}

MobGroupData *Vex::finalizeMobSpawn(MobGroupData *groupData, int extraData)
{
	groupData = Monster::finalizeMobSpawn(groupData);
	setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(Item::iron_sword));
	dropChances[SLOT_WEAPON] = 0;
	return groupData;
}

bool Vex::isAlliedTo(shared_ptr<LivingEntity> other)
{
	if (other == nullptr) return false;
	if (other.get() == this) return true;
	if (dynamic_cast<Illager *>(other.get()) != nullptr || dynamic_cast<Vex *>(other.get()) != nullptr) return true;
	return Monster::isAlliedTo(other);
}

void Vex::newServerAiStep()
{
	Monster::newServerAiStep();

	// it withers away once its life runs out
	if (hasLimitedLife && --limitedLifeTicks <= 0)
	{
		limitedLifeTicks = 20;
		hurt(DamageSource::starve, 1.0f);
	}

	// hunts whatever its owner hunts
	shared_ptr<Mob> master = owner.lock();
	if (master != nullptr && master->isAlive() && master->getTarget() != nullptr && getTarget() != master->getTarget())
	{
		setTarget(master->getTarget());
	}

	shared_ptr<LivingEntity> target = getTarget();

	if (isCharging())
	{
		if (target == nullptr || !target->isAlive())
		{
			setIsCharging(false);
			hasWanted = false;
		}
		else if (bb->intersects(target->bb))
		{
			doHurtTarget(target);
			setIsCharging(false);
			hasWanted = false;
		}
		else if (distanceToSqr(target) < 9.0)
		{
			wantedX = target->x; wantedY = target->y + target->bbHeight * 0.5; wantedZ = target->z;
			hasWanted = true;
		}
	}
	else
	{
		// start a charge now and then
		if (target != nullptr && target->isAlive() && !hasWanted && random->nextInt(7) == 0 && distanceToSqr(target) > 4.0)
		{
			wantedX = target->x; wantedY = target->y + target->bbHeight * 0.5; wantedZ = target->z;
			hasWanted = true;
			setIsCharging(true);
			playSound(eSoundType_MOB_VEX_CHARGE, 1.0f, 1.0f);
		}
		else if (!hasWanted && random->nextInt(7) == 0)
		{
			// drift around the point it was summoned at (or wherever it is)
			double cx = hasBound ? boundX + 0.5 : x;
			double cy = hasBound ? boundY + 0.5 : y;
			double cz = hasBound ? boundZ + 0.5 : z;
			wantedX = cx + random->nextInt(15) - 7;
			wantedY = cy + random->nextInt(11) - 5;
			wantedZ = cz + random->nextInt(15) - 7;
			hasWanted = true;
		}
	}

	// steer towards the wanted point
	if (hasWanted)
	{
		double dx = wantedX - x, dy = wantedY - y, dz = wantedZ - z;
		double dist = sqrt(dx * dx + dy * dy + dz * dz);
		if (dist < 0.5)
		{
			hasWanted = false;
		}
		else
		{
			double speed = isCharging() ? 0.55 : 0.25;
			xd += (dx / dist * speed - xd) * 0.3;
			yd += (dy / dist * speed - yd) * 0.3;
			zd += (dz / dist * speed - zd) * 0.3;

			float faceDegrees = static_cast<float>(atan2(zd, xd) * 180 / PI) - 90;
			yRot += Mth::wrapDegrees(faceDegrees - yRot) * 0.4f;
		}
	}
}

int Vex::getAmbientSound() { return eSoundType_MOB_VEX_IDLE; }
int Vex::getHurtSound() { return eSoundType_MOB_VEX_HURT; }
int Vex::getDeathSound() { return eSoundType_MOB_VEX_DEATH; }
bool Vex::makeStepSound() { return false; }
void Vex::causeFallDamage(float distance) {}
void Vex::checkFallDamage(double ya, bool onGround) {}
bool Vex::isIgnoringTileTriggers() { return true; }

void Vex::readAdditionalSaveData(CompoundTag *tag)
{
	Monster::readAdditionalSaveData(tag);
	if (tag->contains(L"BoundX"))
	{
		setBoundOrigin(tag->getInt(L"BoundX"), tag->getInt(L"BoundY"), tag->getInt(L"BoundZ"));
	}
	if (tag->contains(L"LifeTicks")) setLifeTicks(tag->getInt(L"LifeTicks"));
}

void Vex::addAdditonalSaveData(CompoundTag *entityTag)
{
	Monster::addAdditonalSaveData(entityTag);
	if (hasBound)
	{
		entityTag->putInt(L"BoundX", boundX);
		entityTag->putInt(L"BoundY", boundY);
		entityTag->putInt(L"BoundZ", boundZ);
	}
	if (hasLimitedLife) entityTag->putInt(L"LifeTicks", limitedLifeTicks);
}
