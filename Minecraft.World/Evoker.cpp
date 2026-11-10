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
#include "net.minecraft.world.phys.h"
#include "SoundTypes.h"
#include "Mth.h"
#include "IllagerSpellGoals.h"
#include "EvokerFangs.h"
#include "Vex.h"
#include "Evoker.h"

namespace
{
	// ---- summon three vexes -------------------------------------------------------------------------------------
	class EvokerSummonSpellGoal : public SpellcasterUseSpellGoal
	{
		Evoker *evoker;
	public:
		EvokerSummonSpellGoal(Evoker *evoker) : SpellcasterUseSpellGoal(evoker) { this->evoker = evoker; }

		virtual bool canUse()
		{
			if (!SpellcasterUseSpellGoal::canUse()) return false;
			// the fewer vexes already around it, the more likely it summons more
			int vexes = 0;
			vector<shared_ptr<Entity> > *nearby = evoker->level->getEntitiesOfClass(typeid(Vex), evoker->bb->grow(16, 16, 16));
			if (nearby != nullptr) vexes = static_cast<int>(nearby->size());
			return evoker->getRandom()->nextInt(8) + 1 > vexes;
		}

	protected:
		virtual void performSpellCasting()
		{
			Level *level = evoker->level;
			shared_ptr<Mob> master = dynamic_pointer_cast<Mob>(evoker->shared_from_this());
			for (int i = 0; i < 3; i++)
			{
				int bx = Mth::floor(evoker->x) - 2 + evoker->getRandom()->nextInt(5);
				int by = Mth::floor(evoker->y) + 1;
				int bz = Mth::floor(evoker->z) - 2 + evoker->getRandom()->nextInt(5);

				shared_ptr<Vex> vex = std::make_shared<Vex>(level);
				vex->moveTo(bx + 0.5, by + 0.05, bz + 0.5, 0.0f, 0.0f);
				vex->finalizeMobSpawn(nullptr);
				vex->setOwner(master);
				vex->setBoundOrigin(bx, by, bz);
				vex->setLifeTicks(SharedConstants::TICKS_PER_SECOND * (30 + evoker->getRandom()->nextInt(90)));
				level->addEntity(vex);
			}
		}
		virtual int getCastWarmupTime() { return 100; }
		virtual int getCastingTime() { return 100; }	// was 340: the evoker stood casting for 17 s and re-summoned the moment it ended, so the fangs never got a turn
		virtual int getCastingInterval() { return 340; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_EVOKER_PREPARE_SUMMON; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_SUMMON_VEX; }
	};

	// ---- fangs ---------------------------------------------------------------------------------------------------
	class EvokerAttackSpellGoal : public SpellcasterUseSpellGoal
	{
		Evoker *evoker;

		void createSpellEntity(double px, double pz, double minY, double maxY, float yRot, int warmup)
		{
			Level *level = evoker->level;
			int bx = Mth::floor(px);
			int bz = Mth::floor(pz);
			int by = Mth::floor(maxY);
			bool found = false;
			do
			{
				// walk down until there is solid ground to stand the fangs on
				if (level->isSolidBlockingTile(bx, by - 1, bz) && !level->isSolidBlockingTile(bx, by, bz))
				{
					found = true;
					break;
				}
				by--;
			} while (by >= Mth::floor(minY) - 1);

			if (found)
			{
				level->addEntity(std::make_shared<EvokerFangs>(level, px, by, pz, yRot, warmup, dynamic_pointer_cast<LivingEntity>(evoker->shared_from_this())));
			}
		}

	public:
		EvokerAttackSpellGoal(Evoker *evoker) : SpellcasterUseSpellGoal(evoker) { this->evoker = evoker; }

	protected:
		virtual void performSpellCasting()
		{
			shared_ptr<LivingEntity> target = evoker->getTarget();
			if (target == nullptr) return;

			double minY = min(target->y, evoker->y);
			double maxY = max(target->y, evoker->y) + 1.0;
			float angle = static_cast<float>(atan2(target->z - evoker->z, target->x - evoker->x));

			if (evoker->distanceToSqr(target) < 9.0)
			{
				// close: two rings of jaws around the evoker
				for (int i = 0; i < 5; i++)
				{
					float a = angle + i * PI * 0.4f;
					createSpellEntity(evoker->x + Mth::cos(a) * 1.5, evoker->z + Mth::sin(a) * 1.5, minY, maxY, a, 0);
				}
				for (int i = 0; i < 8; i++)
				{
					float a = angle + i * PI * 2.0f / 8.0f + 1.2566371f;
					createSpellEntity(evoker->x + Mth::cos(a) * 2.5, evoker->z + Mth::sin(a) * 2.5, minY, maxY, a, 3);
				}
			}
			else
			{
				// far: a line of jaws racing towards the target
				for (int i = 0; i < 16; i++)
				{
					double d = 1.25 * (i + 1);
					createSpellEntity(evoker->x + Mth::cos(angle) * d, evoker->z + Mth::sin(angle) * d, minY, maxY, angle, i);
				}
			}
		}
		virtual int getCastWarmupTime() { return 40; }
		virtual int getCastingTime() { return 40; }
		virtual int getCastingInterval() { return 100; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_EVOKER_PREPARE_ATTACK; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_FANGS; }
	};

	// ---- wololo: blue sheep turn red ---------------------------------------------------------------------------
	class EvokerWololoSpellGoal : public SpellcasterUseSpellGoal
	{
		Evoker *evoker;
	public:
		EvokerWololoSpellGoal(Evoker *evoker) : SpellcasterUseSpellGoal(evoker) { this->evoker = evoker; }

		virtual bool canUse()
		{
			if (evoker->getTarget() != nullptr) return false;
			if (evoker->isCastingSpell()) return false;
			if (evoker->tickCount < nextAttackTickCount) return false;

			vector<shared_ptr<Entity> > *sheep = evoker->level->getEntitiesOfClass(typeid(Sheep), evoker->bb->grow(16, 4, 16));
			if (sheep == nullptr) return false;

			vector<shared_ptr<Sheep> > blue;
			for (size_t i = 0; i < sheep->size(); i++)
			{
				shared_ptr<Sheep> s = dynamic_pointer_cast<Sheep>((*sheep)[i]);
				if (s != nullptr && s->isAlive() && s->getColor() == 11) blue.push_back(s);	// 11 = blue fleece
			}
			if (blue.empty()) return false;

			evoker->setWololoTarget(blue[evoker->getRandom()->nextInt(static_cast<int>(blue.size()))]);
			return true;
		}

		virtual bool canContinueToUse()
		{
			shared_ptr<Sheep> s = evoker->getWololoTarget();
			return s != nullptr && attackWarmupDelay > 0;
		}

		virtual void stop()
		{
			SpellcasterUseSpellGoal::stop();
			evoker->setWololoTarget(nullptr);
		}

	protected:
		virtual void performSpellCasting()
		{
			shared_ptr<Sheep> s = evoker->getWololoTarget();
			if (s != nullptr && s->isAlive()) s->setColor(14);		// 14 = red fleece
		}
		virtual int getCastWarmupTime() { return 40; }
		virtual int getCastingTime() { return 60; }
		virtual int getCastingInterval() { return 140; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_EVOKER_PREPARE_WOLOLO; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_WOLOLO; }
	};
}

Evoker::Evoker(Level *level) : SpellcasterIllager(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());
	xpReward = Enemy::XP_REWARD_LARGE;

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new SpellcasterCastingSpellGoal(this));
	goalSelector.addGoal(2, new AvoidPlayerGoal(this, typeid(Player), 8.0f, 0.6, 1.0));
	goalSelector.addGoal(4, new EvokerSummonSpellGoal(this));
	goalSelector.addGoal(5, new EvokerAttackSpellGoal(this));
	goalSelector.addGoal(6, new EvokerWololoSpellGoal(this));
	goalSelector.addGoal(8, new RandomStrollGoal(this, 0.6));
	goalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f));
	goalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Villager), 0, false));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(VillagerGolem), 0, false));
}

void Evoker::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5f);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(12);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(24);
}

int Evoker::getCastingSoundId() { return eSoundType_MOB_EVOKER_CAST; }
void Evoker::setWololoTarget(shared_ptr<Sheep> sheep) { wololoTarget = sheep; }
shared_ptr<Sheep> Evoker::getWololoTarget() { return wololoTarget.lock(); }

// emeralds like the other illagers - and deliberately no Totem of Undying
void Evoker::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Monster::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);
	if (wasKilledByPlayer)
	{
		// the other illagers drop 0-1; the evoker, the rich one of the family, drops 2-4 (Looting adds up to 3 more)
		int count = 2 + random->nextInt(3);
		if (playerBonusLevel > 0) count += random->nextInt(playerBonusLevel + 1);
		for (int i = 0; i < count; i++) spawnAtLocation(Item::emerald_Id, 1);
	}
}

int Evoker::getAmbientSound() { return eSoundType_MOB_EVOKER_IDLE; }
int Evoker::getHurtSound() { return eSoundType_MOB_EVOKER_HURT; }
int Evoker::getDeathSound() { return eSoundType_MOB_EVOKER_DEATH; }
