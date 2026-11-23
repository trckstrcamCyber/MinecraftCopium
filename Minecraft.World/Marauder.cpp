#include "stdafx.h"
#include "RaidGoals.h"
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
#include "OminousBottleItem.h"
#include "SoundTypes.h"
#include "Marauder.h"

namespace
{
	// Java: MarauderMeleeAttackGoal - it only closes in for a melee fight when the target is within 4 blocks, and gives up the chase beyond 6
	class MarauderMeleeGoal : public MeleeAttackGoal
	{
		Marauder *marauder;
	public:
		MarauderMeleeGoal(Marauder *marauder, eINSTANCEOF type, double speed, bool track) : MeleeAttackGoal(marauder, type, speed, track) { this->marauder = marauder; }

		virtual bool canUse()
		{
			shared_ptr<LivingEntity> t = marauder->getTarget();
			return MeleeAttackGoal::canUse() && t != nullptr && marauder->distanceToSqr(t) < 16.0;
		}
		virtual bool canContinueToUse()
		{
			shared_ptr<LivingEntity> t = marauder->getTarget();
			return MeleeAttackGoal::canContinueToUse() && t != nullptr && marauder->distanceToSqr(t) < 36.0;
		}
	};
}

Marauder::Marauder(Level *level) : Illager(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());
	xpReward = Enemy::XP_REWARD_LARGE;

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(2, new MarauderMeleeGoal(this, eTYPE_PLAYER, 1.35, false));
	goalSelector.addGoal(2, new MarauderMeleeGoal(this, eTYPE_VILLAGER, 1.35, false));
	goalSelector.addGoal(2, new MarauderMeleeGoal(this, eTYPE_VILLAGERGOLEM, 1.35, false));
	goalSelector.addGoal(3, new RangedAttackGoal(this, this, 1.0, 40, 8.0f));
	goalSelector.addGoal(4, new RaidMoveGoal(this, this, 1.0));
	goalSelector.addGoal(4, new PatrolGoal(this, 0.5));
	goalSelector.addGoal(8, new RandomStrollGoal(this, 0.6));
	goalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f));
	goalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Villager), 0, false));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(VillagerGolem), 0, false));
}

void Marauder::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.3f);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(18);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(24);
	getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(5);
}

MobGroupData *Marauder::finalizeMobSpawn(MobGroupData *groupData, int extraData)
{
	groupData = Monster::finalizeMobSpawn(groupData);
	setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(Item::platinum_hatchet));
	return groupData;
}

// arms hang loose while it walks about; raised to attack when it has a target
// (same pattern as Pillager::getArmPose). NEUTRAL has no pose branch in
// IllagerModel::setupAnim, so the arms stay at their sides on the
// HumanoidModel walk-swing rest pose -- and IllagerRenderer stops hiding the
// held hatchet, so an idle Marauder holds it at its side like the Pillager
// holds its crossbow.
Illager::ArmPose Marauder::getArmPose()
{
	return isAggressive() ? ATTACKING : NEUTRAL;
}

// the throw: a snowball that is drawn and hurts like a hatchet
void Marauder::performRangedAttack(shared_ptr<LivingEntity> target, float power)
{
	shared_ptr<Snowball> hatchet = std::make_shared<Snowball>(level, dynamic_pointer_cast<LivingEntity>(shared_from_this()));
	hatchet->setHatchet(true);
	double xd = target->x - x;
	double yd = (target->y + target->bbHeight / 3.0) - hatchet->y;
	double zd = target->z - z;
	float dist = Mth::sqrt(xd * xd + zd * zd);
	hatchet->shoot(xd, yd + dist * 0.2f, zd, 1.6f, static_cast<float>(14 - level->difficulty * 4));
	level->addEntity(hatchet);
	playSound(eSoundType_RANDOM_BOW, 1.0f, 1.0f / (getRandom()->nextFloat() * 0.4f + 0.8f));
}

// the mod's loot table: always an Ominous Bottle of a random level (I-V), and 0-1 emeralds when a player kills it
void Marauder::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Monster::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);

	spawnAtLocation(OminousBottleItem::create(1 + random->nextInt(5)), 0.0f);

	if (wasKilledByPlayer)
	{
		int count = random->nextInt(2);
		if (playerBonusLevel > 0) count += random->nextInt(playerBonusLevel + 1);
		for (int i = 0; i < count; i++) spawnAtLocation(Item::emerald_Id, 1);
	}
}

// the mod gives it the Pillager's voice
int Marauder::getAmbientSound() { return eSoundType_MOB_PILLAGER_IDLE; }
int Marauder::getHurtSound() { return eSoundType_MOB_PILLAGER_HURT; }
int Marauder::getDeathSound() { return eSoundType_MOB_PILLAGER_DEATH; }
