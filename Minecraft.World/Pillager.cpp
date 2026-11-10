#include "stdafx.h"
#include "Random.h"
#include "PathNavigation.h"
#include "AttributeInstance.h"
#include "ItemInstance.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.npc.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.entity.h"
#include "SoundTypes.h"
#include "Mth.h"
#include "Sensing.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "Pillager.h"

namespace
{
	// Java's crossbow routine: close to within 12 blocks, draw the crossbow (1.25 s, charging flag on), then stand loaded and aiming
	// for a moment and fire (loaded flag on), then start over.
	class PillagerCrossbowGoal : public Goal
	{
		Pillager *mob;
		int stage;			// 0 = unloaded, 1 = drawing, 2 = loaded
		int timer;
	public:
		PillagerCrossbowGoal(Pillager *mob)
		{
			this->mob = mob;
			stage = 0;
			timer = 0;
			setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
		}

		virtual bool canUse()
		{
			shared_ptr<LivingEntity> target = mob->getTarget();
			return target != nullptr && target->isAlive();
		}
		virtual bool canContinueToUse() { return canUse(); }

		virtual void start() { stage = 0; timer = 0; }

		virtual void stop()
		{
			mob->setChargingCrossbow(false);
			mob->setCrossbowLoaded(false);
			mob->setEquippedSlot(LivingEntity::SLOT_WEAPON, std::make_shared<ItemInstance>(Item::crossbow));
			stage = 0;
		}

		virtual void tick()
		{
			shared_ptr<LivingEntity> target = mob->getTarget();
			if (target == nullptr) return;

			bool canSee = mob->getSensing()->canSee(target);
			double d2 = mob->distanceToSqr(target);
			mob->getLookControl()->setLookAt(target, 30.0f, 30.0f);

			// walk up to firing range, stand still once there
			if (d2 > 12.0 * 12.0 || !canSee) mob->getNavigation()->moveTo(target, 1.0);
			else mob->getNavigation()->stop();

			if (stage == 0)
			{
				if (canSee && d2 < 14.0 * 14.0)
				{
					stage = 1;
					timer = 0;
					mob->setChargingCrossbow(true);
					mob->playSound(eSoundType_ITEM_CROSSBOW_LOADING_START, 0.5f, 1.0f);
				}
			}
			else if (stage == 1)
			{
				if (++timer >= 25)
				{
					mob->setChargingCrossbow(false);
					mob->setCrossbowLoaded(true);
					mob->setEquippedSlot(LivingEntity::SLOT_WEAPON, std::make_shared<ItemInstance>(Item::crossbow_loaded));
					mob->playSound(eSoundType_ITEM_CROSSBOW_LOADING_END, 1.0f, 1.0f);
					stage = 2;
					timer = 20 + mob->getRandom()->nextInt(20);		// aims for a moment before it fires
				}
			}
			else
			{
				if (--timer <= 0 && canSee)
				{
					mob->performRangedAttack(target, 1.0f);
					mob->setCrossbowLoaded(false);
					mob->setEquippedSlot(LivingEntity::SLOT_WEAPON, std::make_shared<ItemInstance>(Item::crossbow));
					stage = 0;
				}
			}
		}
	};
}

bool Pillager::isChargingCrossbow() { return (getEntityData()->getByte(DATA_FLAGS_ID) & 2) != 0; }
bool Pillager::isCrossbowLoaded() { return (getEntityData()->getByte(DATA_FLAGS_ID) & 4) != 0; }
void Pillager::setChargingCrossbow(bool charging)
{
	byte f = getEntityData()->getByte(DATA_FLAGS_ID);
	getEntityData()->set(DATA_FLAGS_ID, static_cast<byte>(charging ? (f | 2) : (f & ~2)));
}
void Pillager::setCrossbowLoaded(bool loaded)
{
	byte f = getEntityData()->getByte(DATA_FLAGS_ID);
	getEntityData()->set(DATA_FLAGS_ID, static_cast<byte>(loaded ? (f | 4) : (f & ~4)));
}

Pillager::Pillager(Level *level) : Illager(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());
	xpReward = Enemy::XP_REWARD_LARGE;

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(3, new PillagerCrossbowGoal(this));
	goalSelector.addGoal(8, new RandomStrollGoal(this, 0.6));
	goalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 15.0f, 1.0f));
	goalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 15.0f));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Villager), 0, false));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(VillagerGolem), 0, false));
}

void Pillager::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.35f);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(32);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(24);
	getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(5);
}

MobGroupData *Pillager::finalizeMobSpawn(MobGroupData *groupData, int extraData)
{
	groupData = Monster::finalizeMobSpawn(groupData);
	setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(Item::crossbow));
	return groupData;
}

// arms hang loose while it walks about; drawing the string, then holding the loaded crossbow up to aim (Java's poses)
Illager::ArmPose Pillager::getArmPose()
{
	if (isChargingCrossbow()) return CROSSBOW_CHARGE;
	if (isCrossbowLoaded()) return CROSSBOW_HOLD;
	return isAggressive() ? CROSSBOW_HOLD : NEUTRAL;
}

void Pillager::performRangedAttack(shared_ptr<LivingEntity> target, float power)
{
	shared_ptr<Arrow> arrow = std::make_shared<Arrow>(level, dynamic_pointer_cast<LivingEntity>(shared_from_this()), target, 1.60f, 14 - (level->difficulty * 4));
	arrow->setBaseDamage(power * 2.0f + (random->nextGaussian() * 0.25f + (level->difficulty * 0.11f)));
	arrow->setCritArrow(true);
	playSound(eSoundType_ITEM_CROSSBOW_SHOOT, 1.0f, 1.0f / (getRandom()->nextFloat() * 0.4f + 0.8f));
	level->addEntity(arrow);
}

// arrows (0-2, more with Looting) and emeralds (0-1) - the crossbow itself drops through the normal held-item chance
void Pillager::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Monster::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);

	int arrows = random->nextInt(3);
	if (playerBonusLevel > 0) arrows += random->nextInt(playerBonusLevel + 1);
	for (int i = 0; i < arrows; i++) spawnAtLocation(Item::arrow_Id, 1);

	if (wasKilledByPlayer)
	{
		int count = random->nextInt(2);
		if (playerBonusLevel > 0) count += random->nextInt(playerBonusLevel + 1);
		for (int i = 0; i < count; i++) spawnAtLocation(Item::emerald_Id, 1);
	}
}

int Pillager::getAmbientSound() { return eSoundType_MOB_PILLAGER_IDLE; }
int Pillager::getHurtSound() { return eSoundType_MOB_PILLAGER_HURT; }
int Pillager::getDeathSound() { return eSoundType_MOB_PILLAGER_DEATH; }
