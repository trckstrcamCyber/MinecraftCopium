#include "stdafx.h"
#include "RaidGoals.h"
#include "Random.h"
#include "AABB.h"
#include "AttributeInstance.h"
#include "ItemInstance.h"
#include "MobType.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.damagesource.h"
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
#include "SkullBolt.h"
#include "Necromancer.h"

namespace
{
	// Conjure skull (Java: ConjureSkullGoal): a Skull Bolt from above its head at the target, every 7 seconds or so
	class ConjureSkullGoal : public SpellcasterUseSpellGoal
	{
		Necromancer *necromancer;
	public:
		ConjureSkullGoal(Necromancer *necromancer) : SpellcasterUseSpellGoal(necromancer) { this->necromancer = necromancer; }

	protected:
		virtual void performSpellCasting()
		{
			shared_ptr<LivingEntity> target = necromancer->getTarget();
			if (target == nullptr) return;

			double sx = necromancer->x, sy = necromancer->y + 2.5, sz = necromancer->z;
			double xd = target->x - sx;
			double yd = (target->y + target->bbHeight * 0.5) - sy;
			double zd = target->z - sz;

			shared_ptr<SkullBolt> skull = std::make_shared<SkullBolt>(necromancer->level, dynamic_pointer_cast<LivingEntity>(necromancer->shared_from_this()), xd, yd, zd);
			skull->x = sx;
			skull->y = sy;
			skull->z = sz;
			necromancer->level->addEntity(skull);
		}
		virtual int getCastWarmupTime() { return 40; }
		virtual int getCastingTime() { return 60; }
		virtual int getCastingInterval() { return 140; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_EVOKER_PREPARE_ATTACK; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_BLINDNESS; }
	};

	// Summon undead (Java: SummonUndeadGoal): two zombies (four at night) and a skeleton, at most four times, only while few zombies are near
	class SummonUndeadGoal : public SpellcasterUseSpellGoal
	{
		Necromancer *necromancer;
		int spellCount;

		void summon(shared_ptr<Mob> mob)
		{
			Random *r = necromancer->getRandom();
			double sx = necromancer->x + (-2 + r->nextInt(5));
			double sz = necromancer->z + (-2 + r->nextInt(5));
			mob->moveTo(sx + 0.5, necromancer->y + 1.0, sz + 0.5, 0.0f, 0.0f);
			mob->finalizeMobSpawn(nullptr);
			necromancer->level->addEntity(mob);
		}

	public:
		SummonUndeadGoal(Necromancer *necromancer) : SpellcasterUseSpellGoal(necromancer) { this->necromancer = necromancer; spellCount = 0; }

		virtual bool canUse()
		{
			if (!SpellcasterUseSpellGoal::canUse()) return false;
			if (spellCount >= 4) return false;
			AABB *box = necromancer->bb->grow(16.0, 16.0, 16.0);
			vector<shared_ptr<Entity> > *zombies = necromancer->level->getEntitiesOfClass(typeid(Zombie), box);
			int nearby = (zombies != nullptr) ? static_cast<int>(zombies->size()) : 0;
			delete zombies;
			return necromancer->getRandom()->nextInt(5) + 1 > nearby;
		}

	protected:
		virtual void performSpellCasting()
		{
			Level *level = necromancer->level;
			int zombies = level->isDay() ? 2 : 4;
			for (int i = 0; i < zombies; i++) summon(std::make_shared<Zombie>(level));
			summon(std::make_shared<Skeleton>(level));
			spellCount++;
		}
		virtual int getCastWarmupTime() { return 20; }
		virtual int getCastingTime() { return 100; }
		virtual int getCastingInterval() { return 340; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_NECROMANCER_SUMMON; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_FANGS; }
	};
}

Necromancer::Necromancer(Level *level) : SpellcasterIllager(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());
	xpReward = Enemy::XP_REWARD_LARGE;
	shielded = false;

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new SpellcasterCastingSpellGoal(this));
	goalSelector.addGoal(3, new SummonUndeadGoal(this));
	goalSelector.addGoal(4, new ConjureSkullGoal(this));
	goalSelector.addGoal(5, new AvoidPlayerGoal(this, typeid(Player), 8.0f, 0.6, 1.0));
	goalSelector.addGoal(7, new RaidMoveGoal(this, this, 1.0));
	goalSelector.addGoal(7, new PatrolGoal(this, 0.5));
	goalSelector.addGoal(8, new RandomStrollGoal(this, 0.6));
	goalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f));
	goalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Villager), 0, false));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(VillagerGolem), 0, false));
}

void Necromancer::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.38f);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(18);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(32);
}

// The undead link. Undead within 10 blocks are re-buffed every few ticks (their effects last 10), stop burning, and take the Necromancer's target;
// once a second the shield is switched on or off by whether any are near; and every ten ticks every undead within 30 blocks is told its target.
void Necromancer::newServerAiStep()
{
	SpellcasterIllager::newServerAiStep();

	int64_t now = level->getGameTime();
	shared_ptr<LivingEntity> target = getTarget();

	if (now % 5 == 0)	// neoLegacy - was '|| now % 20 == 0', implied since 20 is a multiple of 5
	{
		AABB *box = bb->grow(10.0, 10.0, 10.0);
		vector<shared_ptr<Entity> > *found = level->getEntitiesOfClass(typeid(Monster), box);
		int undead = 0;
		if (found != nullptr)
		{
			for (size_t i = 0; i < found->size(); i++)
			{
				shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(found->at(i));
				if (mob == nullptr || !mob->isAlive() || mob->getMobType() != UNDEAD) continue;
				undead++;
				if (now % 5 == 0)
				{
					mob->addEffect(new MobEffectInstance(MobEffect::damageBoost->id, 10, 0));
					mob->addEffect(new MobEffectInstance(MobEffect::movementSpeed->id, 10, 0));
					mob->clearFire();
					if (target != nullptr) mob->setTarget(target);
				}
			}
			delete found;
		}
		if (now % 20 == 0) shielded = undead > 0;
	}

	if (target != nullptr && now % 10 == 0)
	{
		AABB *wide = bb->grow(30.0, 30.0, 30.0);
		vector<shared_ptr<Entity> > *found = level->getEntitiesOfClass(typeid(Monster), wide);
		if (found != nullptr)
		{
			for (size_t i = 0; i < found->size(); i++)
			{
				shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(found->at(i));
				if (mob != nullptr && mob->isAlive() && mob->getMobType() == UNDEAD) mob->setTarget(target);
			}
			delete found;
		}
	}
}

// shielded: half damage
bool Necromancer::hurt(DamageSource *source, float dmg)
{
	return SpellcasterIllager::hurt(source, shielded ? dmg / 2.0f : dmg);
}

// the mod's loot table: 0-1 emeralds, and a 2.5% chance each (3.5% with Looting I, +1% per further level) of a skeleton skull and of a zombie head, all only when a player kills it
void Necromancer::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Monster::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);
	if (!wasKilledByPlayer) return;

	int count = random->nextInt(2);
	if (playerBonusLevel > 0) count += random->nextInt(playerBonusLevel + 1);
	for (int i = 0; i < count; i++) spawnAtLocation(Item::emerald_Id, 1);

	int perMille = 25 + (playerBonusLevel > 0 ? 10 + (playerBonusLevel - 1) * 10 : 0);
	if (random->nextInt(1000) < perMille) spawnAtLocation(std::make_shared<ItemInstance>(Item::skull_Id, 1, 0), 0.0f);	// skeleton skull
	if (random->nextInt(1000) < perMille) spawnAtLocation(std::make_shared<ItemInstance>(Item::skull_Id, 1, 2), 0.0f);	// zombie head
}

int Necromancer::getCastingSoundId() { return eSoundType_MOB_EVOKER_CAST; }
int Necromancer::getAmbientSound() { return eSoundType_MOB_ILLUSIONER_IDLE; }
int Necromancer::getHurtSound() { return eSoundType_MOB_ILLUSIONER_HURT; }
int Necromancer::getDeathSound() { return eSoundType_MOB_ILLUSIONER_DEATH; }

