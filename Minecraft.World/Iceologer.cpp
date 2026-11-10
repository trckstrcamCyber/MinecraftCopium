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
#include "IllagerSpellGoals.h"
#include "IceCube.h"
#include "Iceologer.h"

namespace
{
	// the Iceologer's only spell: an Ice Cube conjured above its target
	class IceSpellGoal : public SpellcasterUseSpellGoal
	{
		Iceologer *iceologer;
	public:
		IceSpellGoal(Iceologer *iceologer) : SpellcasterUseSpellGoal(iceologer) { this->iceologer = iceologer; }

	protected:
		virtual void performSpellCasting()
		{
			shared_ptr<LivingEntity> target = iceologer->getTarget();
			if (target != nullptr)
			{
				iceologer->level->addEntity(std::make_shared<IceCube>(iceologer->level, dynamic_pointer_cast<LivingEntity>(iceologer->shared_from_this()), target));
			}
		}
		virtual int getCastWarmupTime() { return 20; }
		virtual int getCastingTime() { return 40; }
		virtual int getCastingInterval() { return 100; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_ICEOLOGER_PREPARE; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_ICE; }
	};
}

Iceologer::Iceologer(Level *level) : SpellcasterIllager(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	setSize(0.6f, 1.95f);
	xpReward = Enemy::XP_REWARD_LARGE;
	prevChasingPosX = prevChasingPosY = prevChasingPosZ = 0;
	chasingPosX = chasingPosY = chasingPosZ = 0;

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new SpellcasterCastingSpellGoal(this));
	goalSelector.addGoal(2, new AvoidPlayerGoal(this, typeid(Player), 8.0f, 0.6, 1.0));
	goalSelector.addGoal(4, new IceSpellGoal(this));
	goalSelector.addGoal(8, new RandomStrollGoal(this, 0.6));
	goalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f));
	goalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Villager), 0, false));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(VillagerGolem), 0, false));
}

void Iceologer::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5f);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(12);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(24);
}

void Iceologer::tick()
{
	SpellcasterIllager::tick();
	updateCape();
}

// the cape trails behind the body: a chasing point that lags the real position (same maths as the original mod)
void Iceologer::updateCape()
{
	prevChasingPosX = chasingPosX;
	prevChasingPosY = chasingPosY;
	prevChasingPosZ = chasingPosZ;
	double dx = x - chasingPosX;
	double dy = y - chasingPosY;
	double dz = z - chasingPosZ;

	if (dx > 10.0 || dx < -10.0) { chasingPosX = x; prevChasingPosX = chasingPosX; }
	if (dz > 10.0 || dz < -10.0) { chasingPosZ = z; prevChasingPosZ = chasingPosZ; }
	if (dy > 10.0 || dy < -10.0) { chasingPosY = y; prevChasingPosY = chasingPosY; }

	chasingPosX += dx * 0.25;
	chasingPosZ += dz * 0.25;
	chasingPosY += dy * 0.25;
}

int Iceologer::getCastingSoundId() { return eSoundType_MOB_ICEOLOGER_CAST; }

// loot table of the mod: an Ice Wand half of the time, and 0-1 emeralds (Looting adds more) when a player killed it
void Iceologer::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Monster::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);

	if (random->nextInt(2) == 0) spawnAtLocation(std::make_shared<ItemInstance>(Item::ice_wand), 0.0f);
	if (wasKilledByPlayer)
	{
		int count = random->nextInt(2);
		if (playerBonusLevel > 0) count += random->nextInt(playerBonusLevel + 1);
		for (int i = 0; i < count; i++) spawnAtLocation(Item::emerald_Id, 1);
	}
}

int Iceologer::getAmbientSound() { return eSoundType_MOB_ICEOLOGER_IDLE; }
int Iceologer::getHurtSound() { return eSoundType_MOB_ICEOLOGER_HURT; }
int Iceologer::getDeathSound() { return eSoundType_MOB_ICEOLOGER_DEATH; }
