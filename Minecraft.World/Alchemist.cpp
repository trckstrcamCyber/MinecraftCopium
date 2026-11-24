#include "stdafx.h"
#include "RaidGoals.h"
#include "Random.h"
#include "PathNavigation.h"
#include "AttributeInstance.h"
#include "ItemInstance.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.alchemy.h"
#include "net.minecraft.world.effect.h"
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
#include "Alchemist.h"

static const int POTION_INTERVAL = 160;	// ticks between potions (Java: potionCooldown)

Alchemist::Alchemist(Level *level) : Illager(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());
	xpReward = Enemy::XP_REWARD_LARGE;

	potionCooldown = POTION_INTERVAL;
	potionReady = false;
	potionValue = PotionBrewing::POTION_ID_SPLASH_POISON;

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(4, new RangedAttackGoal(this, this, 0.85, 30, 15.0f));
	goalSelector.addGoal(5, new RaidMoveGoal(this, this, 1.0));
	goalSelector.addGoal(6, new PatrolGoal(this, 0.5));
	goalSelector.addGoal(8, new RandomStrollGoal(this, 0.6));
	goalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f));
	goalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Villager), 0, false));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(VillagerGolem), 0, false));
}

void Alchemist::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.38f);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(18);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(24);
}

MobGroupData *Alchemist::finalizeMobSpawn(MobGroupData *groupData, int extraData)
{
	groupData = Monster::finalizeMobSpawn(groupData);
	setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(Item::bow));
	return groupData;
}

// When the timer runs out the bow is swapped for a random potion (visible in its hand); the next ranged attack throws it
void Alchemist::tick()
{
	Illager::tick();
	if (level->isClientSide || !isAlive()) return;

	if (!potionReady && --potionCooldown <= 0)
	{
		static const int potions[3] = { PotionBrewing::POTION_ID_SPLASH_POISON, PotionBrewing::POTION_ID_SPLASH_SLOWNESS, PotionBrewing::POTION_ID_SPLASH_WEAKNESS };
		potionValue = potions[random->nextInt(3)];
		potionReady = true;
		setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(Item::potion, 1, potionValue));
	}
}

Illager::ArmPose Alchemist::getArmPose()
{
	if (isAggressive())
	{
		shared_ptr<ItemInstance> held = getCarriedItem();
		if (held != nullptr && held->id == Item::bow_Id) return BOW_AND_ARROW;
		return ATTACKING;	// holding a potion: one arm up, ready to throw
	}
	return CROSSED;
}

void Alchemist::performRangedAttack(shared_ptr<LivingEntity> target, float power)
{
	shared_ptr<LivingEntity> self = dynamic_pointer_cast<LivingEntity>(shared_from_this());

	if (potionReady)
	{
		// a lobbed potion, aimed like the Witch's
		shared_ptr<ThrownPotion> potion = std::make_shared<ThrownPotion>(level, self, potionValue);
		potion->xRot += 20;	// neoLegacy - was written as -= -20; same value, and shoot() recomputes xRot from the velocity below
		double xd = (target->x + target->xd) - x;
		double yd = (target->y + target->getHeadHeight() - 1.1f) - y;
		double zd = (target->z + target->zd) - z;
		float dist = Mth::sqrt(xd * xd + zd * zd);
		potion->shoot(xd, yd + dist * 0.2f, zd, 0.75f, 8);
		level->addEntity(potion);
		playSound(eSoundType_ENTITY_WITCH_THROW, 1.0f, 0.8f + random->nextFloat() * 0.4f);

		potionReady = false;
		potionCooldown = POTION_INTERVAL;
		setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(Item::bow));
		return;
	}

	// otherwise the same shot as a Skeleton's
	shared_ptr<Arrow> arrow = std::make_shared<Arrow>(level, self, target, 1.60f, 14 - (level->difficulty * 4));
	arrow->setBaseDamage(power * 2.0f + (random->nextGaussian() * 0.25f + (level->difficulty * 0.11f)));
	playSound(eSoundType_RANDOM_BOW, 1.0f, 1 / (getRandom()->nextFloat() * 0.4f + 0.8f));
	level->addEntity(arrow);
}

// the mod's loot table: 0-2 gunpowder, 0-2 glass bottles and, when a player kills it, 0-1 emeralds, all helped by Looting
void Alchemist::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Monster::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);

	int gunpowder = random->nextInt(3);
	int bottles = random->nextInt(3);
	if (playerBonusLevel > 0) { gunpowder += random->nextInt(playerBonusLevel + 1); bottles += random->nextInt(playerBonusLevel + 1); }
	for (int i = 0; i < gunpowder; i++) spawnAtLocation(Item::gunpowder_Id, 1);
	for (int i = 0; i < bottles; i++) spawnAtLocation(Item::glass_bottle_Id, 1);

	if (wasKilledByPlayer)
	{
		int count = random->nextInt(2);
		if (playerBonusLevel > 0) count += random->nextInt(playerBonusLevel + 1);
		for (int i = 0; i < count; i++) spawnAtLocation(Item::emerald_Id, 1);
	}
}

// the mod has no sounds of its own for it; it borrows the Illusioner's
int Alchemist::getAmbientSound() { return eSoundType_MOB_ILLUSIONER_IDLE; }
int Alchemist::getHurtSound() { return eSoundType_MOB_ILLUSIONER_HURT; }
int Alchemist::getDeathSound() { return eSoundType_MOB_ILLUSIONER_DEATH; }

