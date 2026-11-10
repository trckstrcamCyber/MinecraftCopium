#include "stdafx.h"
#include "Random.h"
#include "PathNavigation.h"
#include "AttributeInstance.h"
#include "ItemInstance.h"
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
#include "Mth.h"
#include "IllagerSpellGoals.h"
#include "Illusioner.h"

namespace
{
	// mirror image: vanish for a minute (the copies are drawn by the client)
	class IllusionerMirrorSpellGoal : public SpellcasterUseSpellGoal
	{
		Illusioner *illusioner;
	public:
		IllusionerMirrorSpellGoal(Illusioner *illusioner) : SpellcasterUseSpellGoal(illusioner) { this->illusioner = illusioner; }

		virtual bool canUse()
		{
			return SpellcasterUseSpellGoal::canUse() && !illusioner->hasEffect(MobEffect::invisibility);
		}

	protected:
		virtual void performSpellCasting()
		{
			illusioner->addEffect(new MobEffectInstance(MobEffect::invisibility->id, 1200));
		}
		virtual int getCastWarmupTime() { return 20; }
		virtual int getCastingTime() { return 20; }
		virtual int getCastingInterval() { return 340; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_ILLUSIONER_PREPARE_MIRROR; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_DISAPPEAR; }
	};

	// blindness: once per target
	class IllusionerBlindnessSpellGoal : public SpellcasterUseSpellGoal
	{
		Illusioner *illusioner;
		int lastTargetId;
	public:
		IllusionerBlindnessSpellGoal(Illusioner *illusioner) : SpellcasterUseSpellGoal(illusioner)
		{
			this->illusioner = illusioner;
			lastTargetId = -1;
		}

		virtual bool canUse()
		{
			if (!SpellcasterUseSpellGoal::canUse()) return false;
			shared_ptr<LivingEntity> target = illusioner->getTarget();
			return target != nullptr && target->entityId != lastTargetId;
		}

		virtual void start()
		{
			SpellcasterUseSpellGoal::start();
			shared_ptr<LivingEntity> target = illusioner->getTarget();
			if (target != nullptr) lastTargetId = target->entityId;
		}

	protected:
		virtual void performSpellCasting()
		{
			shared_ptr<LivingEntity> target = illusioner->getTarget();
			if (target != nullptr) target->addEffect(new MobEffectInstance(MobEffect::blindness->id, 400));
		}
		virtual int getCastWarmupTime() { return 20; }
		virtual int getCastingTime() { return 40; }
		virtual int getCastingInterval() { return 180; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_ILLUSIONER_PREPARE_BLIND; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_BLINDNESS; }
	};
}

Illusioner::Illusioner(Level *level) : SpellcasterIllager(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());
	xpReward = Enemy::XP_REWARD_LARGE;

	clientSideIllusionTicks = 0;
	for (int s = 0; s < 2; s++)
		for (int i = 0; i < 4; i++)
			illusionOffsets[s][i][0] = illusionOffsets[s][i][1] = illusionOffsets[s][i][2] = 0.0;

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new SpellcasterCastingSpellGoal(this));
	goalSelector.addGoal(4, new IllusionerMirrorSpellGoal(this));
	goalSelector.addGoal(5, new IllusionerBlindnessSpellGoal(this));
	goalSelector.addGoal(6, new RangedAttackGoal(this, this, 0.5, 20, 15.0f));
	goalSelector.addGoal(8, new RandomStrollGoal(this, 0.6));
	goalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f));
	goalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Villager), 0, false));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(VillagerGolem), 0, false));
}

void Illusioner::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5f);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(18);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(32);
}

MobGroupData *Illusioner::finalizeMobSpawn(MobGroupData *groupData, int extraData)
{
	groupData = Monster::finalizeMobSpawn(groupData);
	setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(Item::bow));
	return groupData;
}

Illager::ArmPose Illusioner::getArmPose()
{
	if (isCastingSpell()) return SPELLCASTING;
	if (isAggressive()) return BOW_AND_ARROW;
	return CROSSED;
}

// same shot as a Skeleton's
void Illusioner::performRangedAttack(shared_ptr<LivingEntity> target, float power)
{
	shared_ptr<Arrow> arrow = std::make_shared<Arrow>(level, dynamic_pointer_cast<LivingEntity>(shared_from_this()), target, 1.60f, 14 - (level->difficulty * 4));
	arrow->setBaseDamage(power * 2.0f + (random->nextGaussian() * 0.25f + (level->difficulty * 0.11f)));
	playSound(eSoundType_RANDOM_BOW, 1.0f, 1 / (getRandom()->nextFloat() * 0.4f + 0.8f));
	level->addEntity(arrow);
}

// client side: while invisible, four images flicker to new spots around the real position whenever it is hurt (and once a minute)
void Illusioner::aiStep()
{
	SpellcasterIllager::aiStep();

	if (level->isClientSide && isInvisible())
	{
		clientSideIllusionTicks--;
		if (clientSideIllusionTicks < 0) clientSideIllusionTicks = 0;

		if (hurtTime == 1 || tickCount % 1200 == 0)
		{
			clientSideIllusionTicks = 3;
			for (int i = 0; i < 4; i++)
			{
				for (int k = 0; k < 3; k++) illusionOffsets[0][i][k] = illusionOffsets[1][i][k];
				illusionOffsets[1][i][0] = (-6.0f + random->nextInt(13)) * 0.5;
				illusionOffsets[1][i][1] = max(0, random->nextInt(6) - 4);
				illusionOffsets[1][i][2] = (-6.0f + random->nextInt(13)) * 0.5;
			}
			for (int i = 0; i < 16; i++)
			{
				level->addParticle(eParticleType_explode, x + random->nextGaussian() * 0.3, y + random->nextFloat() * 1.3, z + random->nextGaussian() * 0.3, 0.0, 0.0, 0.0);
			}
			level->playLocalSound(x, y, z, eSoundType_MOB_ILLUSIONER_MIRROR_MOVE, 1.0f, 1.0f, false);
		}
		else if (hurtTime == hurtDuration - 1)
		{
			clientSideIllusionTicks = 3;
			for (int i = 0; i < 4; i++)
			{
				for (int k = 0; k < 3; k++)
				{
					illusionOffsets[0][i][k] = illusionOffsets[1][i][k];
					illusionOffsets[1][i][k] = 0.0;
				}
			}
		}
	}
}

void Illusioner::getRenderLocations(float a, double out[4][3])
{
	if (clientSideIllusionTicks <= 0)
	{
		for (int i = 0; i < 4; i++) for (int k = 0; k < 3; k++) out[i][k] = illusionOffsets[1][i][k];
		return;
	}
	double t = (clientSideIllusionTicks - a) / 3.0;
	t = pow(t, 0.25);
	for (int i = 0; i < 4; i++)
		for (int k = 0; k < 3; k++)
			out[i][k] = illusionOffsets[1][i][k] * (1.0 - t) + illusionOffsets[0][i][k] * t;
}

int Illusioner::getCastingSoundId() { return eSoundType_MOB_EVOKER_CAST; }	// VERIFY: no illusioner "cast spell" sound was supplied, the evoker's stands in
int Illusioner::getAmbientSound() { return eSoundType_MOB_ILLUSIONER_IDLE; }
int Illusioner::getHurtSound() { return eSoundType_MOB_ILLUSIONER_HURT; }
int Illusioner::getDeathSound() { return eSoundType_MOB_ILLUSIONER_DEATH; }
