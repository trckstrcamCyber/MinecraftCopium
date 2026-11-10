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
#include "net.minecraft.world.entity.h"
#include "SoundTypes.h"
#include "Vindicator.h"

Vindicator::Vindicator(Level *level) : Illager(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());
	xpReward = Enemy::XP_REWARD_LARGE;

	getNavigation()->setCanOpenDoors(true);

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new BreakDoorGoal(this));
	goalSelector.addGoal(2, new MeleeAttackGoal(this, eTYPE_PLAYER, 1.0, false));
	goalSelector.addGoal(3, new MeleeAttackGoal(this, eTYPE_VILLAGER, 1.0, true));
	goalSelector.addGoal(3, new MeleeAttackGoal(this, eTYPE_VILLAGERGOLEM, 1.0, true));
	goalSelector.addGoal(4, new MoveTowardsRestrictionGoal(this, 1.0));
	goalSelector.addGoal(8, new RandomStrollGoal(this, 0.6));
	goalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f));
	goalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f));
	goalSelector.addGoal(10, new RandomLookAroundGoal(this));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Villager), 0, false));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(VillagerGolem), 0, false));
}

void Vindicator::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.35f);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(12);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(24);
	getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(5);
}

MobGroupData *Vindicator::finalizeMobSpawn(MobGroupData *groupData, int extraData)
{
	groupData = Monster::finalizeMobSpawn(groupData);
	setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(Item::iron_axe));
	return groupData;
}

// Java: emeralds, 0-1 (more with Looting), only when a player did the killing
void Vindicator::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Monster::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);
	if (wasKilledByPlayer)
	{
		int count = random->nextInt(2);
		if (playerBonusLevel > 0) count += random->nextInt(playerBonusLevel + 1);
		for (int i = 0; i < count; i++) spawnAtLocation(Item::emerald_Id, 1);
	}
}

int Vindicator::getAmbientSound() { return eSoundType_MOB_VINDICATOR_IDLE; }
int Vindicator::getHurtSound() { return eSoundType_MOB_VINDICATOR_HURT; }
int Vindicator::getDeathSound() { return eSoundType_MOB_VINDICATOR_DEATH; }
