#include "stdafx.h"
#include "RaidGoals.h"
#include "Random.h"
#include "PathNavigation.h"
#include "AttributeInstance.h"
#include "ItemInstance.h"
#include "AABB.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
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
#include "IllagerSpellGoals.h"
#include "Provoker.h"

namespace
{
	// The war cry (Java: Provoker.BuffAllyGoal). Needs a target; the 300 tick interval is the mod's cooldown. Everyone within 12 blocks that is an
	// illager gets Strength and Speed for 6 seconds. The orange casting particles (the Evoker's "wololo" colour) are the only visible sign of it.
	class ProvokerWarCryGoal : public SpellcasterUseSpellGoal
	{
		Provoker *provoker;
	public:
		ProvokerWarCryGoal(Provoker *provoker) : SpellcasterUseSpellGoal(provoker) { this->provoker = provoker; }

	protected:
		virtual void performSpellCasting()
		{
			AABB *box = provoker->bb->grow(12.0, 12.0, 12.0);
			vector<shared_ptr<Entity> > *found = provoker->level->getEntitiesOfClass(typeid(Monster), box);
			if (found == nullptr) return;

			for (size_t i = 0; i < found->size(); i++)
			{
				shared_ptr<Entity> e = found->at(i);
				if (dynamic_cast<Illager *>(e.get()) == nullptr) continue;
				shared_ptr<LivingEntity> living = dynamic_pointer_cast<LivingEntity>(e);
				if (living == nullptr || !living->isAlive()) continue;
				living->addEffect(new MobEffectInstance(MobEffect::damageBoost->id, 120, 0));
				living->addEffect(new MobEffectInstance(MobEffect::movementSpeed->id, 120, 0));
			}
			delete found;
		}
		virtual int getCastWarmupTime() { return 40; }
		virtual int getCastingTime() { return 60; }
		virtual int getCastingInterval() { return 300; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_ILLUSIONER_PREPARE_BLIND; }	// the mod uses the Illusioner's blindness sound
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_WOLOLO; }
	};
}

Provoker::Provoker(Level *level) : SpellcasterIllager(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());
	xpReward = Enemy::XP_REWARD_LARGE;

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new SpellcasterCastingSpellGoal(this));
	goalSelector.addGoal(3, new ProvokerWarCryGoal(this));
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

void Provoker::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5f);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(18);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(24);
}

MobGroupData *Provoker::finalizeMobSpawn(MobGroupData *groupData, int extraData)
{
	groupData = Monster::finalizeMobSpawn(groupData);
	setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(Item::bow));
	return groupData;
}

Illager::ArmPose Provoker::getArmPose()
{
	if (isCastingSpell()) return SPELLCASTING;
	if (isAggressive()) return BOW_AND_ARROW;
	return CROSSED;	// the model switches to the cheering pose by itself after a lost raid
}

// the same shot as a Skeleton's
void Provoker::performRangedAttack(shared_ptr<LivingEntity> target, float power)
{
	shared_ptr<Arrow> arrow = std::make_shared<Arrow>(level, dynamic_pointer_cast<LivingEntity>(shared_from_this()), target, 1.60f, 14 - (level->difficulty * 4));
	arrow->setBaseDamage(power * 2.0f + (random->nextGaussian() * 0.25f + (level->difficulty * 0.11f)));
	playSound(eSoundType_RANDOM_BOW, 1.0f, 1 / (getRandom()->nextFloat() * 0.4f + 0.8f));
	level->addEntity(arrow);
}

// the mod's loot table: 0-2 arrows, and 0-1 emeralds when a player did it, both helped by Looting
void Provoker::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
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

int Provoker::getCastingSoundId() { return eSoundType_MOB_EVOKER_CAST; }
int Provoker::getAmbientSound() { return eSoundType_MOB_PROVOKER_IDLE; }
int Provoker::getHurtSound() { return eSoundType_MOB_PROVOKER_HURT; }
int Provoker::getDeathSound() { return eSoundType_MOB_PROVOKER_DEATH; }
