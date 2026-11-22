#include "stdafx.h"
#include "RaidGoals.h"
#include "Random.h"
#include "PathNavigation.h"
#include "AttributeInstance.h"
#include "ItemInstance.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.damagesource.h"
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
#include "HatchetItem.h"
#include "SoundTypes.h"
#include "Basher.h"

static const int STUN_TICKS = 60;	// three seconds
static const int BLOCKS_BEFORE_BREAK = 4;

Item *Basher::shieldItem() { return Item::iron_door; }

Basher::Basher(Level *level) : Illager(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());
	xpReward = Enemy::XP_REWARD_LARGE;
	stunTicks = 0;
	blockedCount = 0;

	getNavigation()->setCanOpenDoors(true);

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new BreakDoorGoal(this));
	goalSelector.addGoal(2, new MeleeAttackGoal(this, eTYPE_PLAYER, 1.0, false));
	goalSelector.addGoal(3, new MeleeAttackGoal(this, eTYPE_VILLAGER, 1.0, true));
	goalSelector.addGoal(3, new MeleeAttackGoal(this, eTYPE_VILLAGERGOLEM, 1.0, true));
	goalSelector.addGoal(4, new RaidMoveGoal(this, this, 1.0));
	goalSelector.addGoal(4, new PatrolGoal(this, 0.5));
	goalSelector.addGoal(8, new RandomStrollGoal(this, 0.6));
	goalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f));
	goalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f));
	goalSelector.addGoal(10, new RandomLookAroundGoal(this));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Villager), 0, true));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(VillagerGolem), 0, true));
}

void Basher::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.31f);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(12);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(32);
	getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(3);
}

MobGroupData *Basher::finalizeMobSpawn(MobGroupData *groupData, int extraData)
{
	groupData = Monster::finalizeMobSpawn(groupData);
	setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(shieldItem()));
	setDropChance(SLOT_WEAPON, 0.0f);	// the stand-in shield is not something to hand out
	return groupData;
}

bool Basher::hasShield()
{
	shared_ptr<ItemInstance> held = getCarriedItem();
	return held != nullptr && held->id == shieldItem()->id;
}

void Basher::setStunTicks(int ticks)
{
	stunTicks = ticks < 0 ? 0 : ticks;
	byte flags = getEntityData()->getByte(DATA_FLAGS_ID);
	byte wanted = (stunTicks > 0) ? static_cast<byte>(flags | FLAG_STUNNED) : static_cast<byte>(flags & ~FLAG_STUNNED);
	if (flags != wanted) getEntityData()->set(DATA_FLAGS_ID, wanted);
}

void Basher::tick()
{
	Illager::tick();
	if (!level->isClientSide && isAlive() && stunTicks > 0) setStunTicks(stunTicks - 1);
}

// stunned: it cannot move or attack (the game's own "immobile" hook)
bool Basher::isImmobile()
{
	return Illager::isImmobile() || isStunned();
}

// The shield (Java: Basher.hurtServer). Only while it is hunting; blocked hits do nothing at all.
bool Basher::hurt(DamageSource *source, float dmg)
{
	if (!level->isClientSide && isAggressive() && hasShield())
	{
		shared_ptr<Entity> attacker = source->getEntity();
		bool attackedByLiving = attacker != nullptr && attacker->instanceof(eTYPE_LIVINGENTITY);
		bool axe = false;
		if (attackedByLiving)
		{
			shared_ptr<ItemInstance> weapon = dynamic_pointer_cast<LivingEntity>(attacker)->getCarriedItem();
			axe = weapon != nullptr && dynamic_cast<HatchetItem *>(weapon->getItem()) != nullptr;
		}
		bool golem = attacker != nullptr && attacker->instanceof(eTYPE_VILLAGERGOLEM);

		// (as in the mod, only a living attacker can break it - lava or a fall never do)
		if (attackedByLiving && (axe || golem || blockedCount >= BLOCKS_BEFORE_BREAK))
		{
			// the shield splinters: stunned, and back to a stone axe
			playSound(eSoundType_RANDOM_BREAK, 1.0f, 1.0f);
			setStunTicks(STUN_TICKS);
			blockedCount = 0;
			setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(Item::stone_axe));
			return Illager::hurt(source, dmg);
		}

		shared_ptr<Entity> direct = source->getDirectEntity();
		if (direct != nullptr && (direct->instanceof(eTYPE_ARROW) || direct->instanceof(eTYPE_LIVINGENTITY)))
		{
			playSound(eSoundType_RANDOM_BREAK, 0.5f, 1.6f);	// the block: a short clack
			blockedCount++;
			return false;
		}
	}
	return Illager::hurt(source, dmg);
}

// the mod's loot table: 0-1 emeralds when a player kills it, and 3-7 iron nuggets (there are none here: half the time an iron ingot instead, a bit more with Looting)
void Basher::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Monster::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);

	if (random->nextInt(100) < 50 + playerBonusLevel * 10) spawnAtLocation(Item::iron_ingot_Id, 1);

	if (wasKilledByPlayer)
	{
		int count = random->nextInt(2);
		if (playerBonusLevel > 0) count += random->nextInt(playerBonusLevel + 1);
		for (int i = 0; i < count; i++) spawnAtLocation(Item::emerald_Id, 1);
	}
}

int Basher::getAmbientSound() { return eSoundType_MOB_BASHER_IDLE; }
int Basher::getHurtSound() { return eSoundType_MOB_BASHER_HURT; }
int Basher::getDeathSound() { return eSoundType_MOB_BASHER_DEATH; }
