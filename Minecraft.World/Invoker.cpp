#include "stdafx.h"
#include "RaidGoals.h"
#include "Random.h"
#include "AABB.h"
#include "AttributeInstance.h"
#include "ItemInstance.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
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
#include "EvokerFangs.h"
#include "MobEffectInstance.h"
#include "SoundTypes.h"
#include "IllagerSpellGoals.h"
#include "Surrendered.h"
#include "Sorcerer.h"
#include "Invoker.h"

namespace
{
	// fangs, planted on the ground below the given height (the Evoker's own rule: walk down until there is solid ground to stand them on)
	void conjureFang(Invoker *invoker, double px, double pz, double minY, double maxY, float yRot, int warmup)
	{
		Level *level = invoker->level;
		int bx = Mth::floor(px), bz = Mth::floor(pz), by = Mth::floor(maxY);
		bool found = false;
		do
		{
			if (level->isSolidBlockingTile(bx, by - 1, bz) && !level->isSolidBlockingTile(bx, by, bz)) { found = true; break; }
			by--;
		} while (by >= Mth::floor(minY) - 1);
		if (found) level->addEntity(std::make_shared<EvokerFangs>(level, px, by, pz, yRot, warmup, dynamic_pointer_cast<LivingEntity>(invoker->shared_from_this())));
	}

	// whoever stands next to it that is not on its side: players that are not in creative, and iron golems
	bool threatWithin(Invoker *invoker, double radius)
	{
		Level *level = invoker->level;
		for (size_t i = 0; i < level->players.size(); i++)
		{
			shared_ptr<Player> p = level->players[i];
			if (p != nullptr && !p->removed && p->isAlive() && !p->abilities.instabuild && invoker->distanceToSqr(p) <= radius * radius) return true;
		}
		AABB *box = invoker->bb->grow(radius, radius, radius);
		vector<shared_ptr<Entity> > *golems = level->getEntitiesOfClass(typeid(VillagerGolem), box);
		bool found = golems != nullptr && !golems->empty();
		delete golems;
		return found;
	}

	// Java: AreaDamageGoal - the shockwave
	class ShockwaveGoal : public SpellcasterUseSpellGoal
	{
		Invoker *invoker;
	public:
		ShockwaveGoal(Invoker *invoker) : SpellcasterUseSpellGoal(invoker) { this->invoker = invoker; }

		// neoLegacy - the base gate (live target, not already casting, casting-interval elapsed) used to be bypassed,
		// so getCastingInterval() was dead code; this goal could also start while another spell was casting.
		virtual bool canUse() { return SpellcasterUseSpellGoal::canUse() && invoker->areaDamageCooldown < 0; }
		virtual void start() { SpellcasterUseSpellGoal::start(); }
		virtual void stop() { SpellcasterUseSpellGoal::stop(); }

	protected:
		virtual void performSpellCasting()
		{
			invoker->areaDamageCooldown = 300;
			Level *level = invoker->level;
			shared_ptr<Entity> self = invoker->shared_from_this();
			AABB *box = invoker->bb->grow(6.0, 6.0, 6.0);
			vector<shared_ptr<Entity> > *found = level->getEntitiesOfClass(typeid(LivingEntity), box);
			if (found != nullptr)
			{
				for (size_t i = 0; i < found->size(); i++)
				{
					shared_ptr<LivingEntity> e = dynamic_pointer_cast<LivingEntity>(found->at(i));
					if (e == nullptr || !e->isAlive()) continue;
					if (dynamic_cast<Illager *>(e.get()) != nullptr || e->GetType() == eTYPE_SURRENDERED || e->GetType() == eTYPE_RAVAGER || e->GetType() == eTYPE_VEX) continue;
					if (e->instanceof(eTYPE_PLAYER) && dynamic_pointer_cast<Player>(e)->abilities.instabuild) continue;

					double dx = e->x - invoker->x, dz = e->z - invoker->z;
					double f = dx * dx + dz * dz;
					if (f < 0.001) f = 0.001;
					double px = dx / f * 6.0, pz = dz / f * 6.0;
					double len = sqrt(px * px + pz * pz);
					if (len > 3.0) { px = px / len * 3.0; pz = pz / len * 3.0; }	// the throw is capped, a victim at point-blank range is not sent into orbit
					e->xd += px;
					e->yd = 0.65;
					e->zd += pz;
					e->hurtMarked = true;

					DamageSource *ds = DamageSource::indirectMagic(self, self);
					e->hurt(ds, 11.0f);
					delete ds;
				}
				delete found;
			}
		}
		virtual int getCastWarmupTime() { return 50; }
		virtual int getCastingTime() { return 50; }
		virtual int getCastingInterval() { return 300; }	// neoLegacy - was 400 (dead code); now the live gate, matching areaDamageCooldown
		virtual int getSpellPrepareSound() { return eSoundType_MOB_INVOKER_BIG_CAST; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_BLINDNESS; }
	};

	// Java: CastTeleportGoal
	class BlinkGoal : public SpellcasterUseSpellGoal
	{
		Invoker *invoker;
	public:
		BlinkGoal(Invoker *invoker) : SpellcasterUseSpellGoal(invoker) { this->invoker = invoker; }

		virtual bool canUse()
		{
			// neoLegacy - see ShockwaveGoal: the base casting-interval gate used to be bypassed.
			if (!SpellcasterUseSpellGoal::canUse()) return false;
			return invoker->teleportCooldown < 0 && threatWithin(invoker, 6.0);
		}
		virtual void start() { SpellcasterUseSpellGoal::start(); invoker->teleportCooldown = 180; }

	protected:
		virtual void performSpellCasting() { Sorcerer::randomTeleport(invoker, eSoundType_MOB_INVOKER_TELEPORT_CAST); }
		virtual int getCastWarmupTime() { return 30; }
		virtual int getCastingTime() { return 30; }
		virtual int getCastingInterval() { return 180; }	// neoLegacy - was 400 (dead code); now the live gate, matching teleportCooldown
		virtual int getSpellPrepareSound() { return eSoundType_MOB_INVOKER_TELEPORT_CAST; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_BLINDNESS; }
	};

	// Java: SummonVexGoal - four Surrendered, while fewer than three are near
	class SummonSurrenderedGoal : public SpellcasterUseSpellGoal
	{
		Invoker *invoker;
	public:
		SummonSurrenderedGoal(Invoker *invoker) : SpellcasterUseSpellGoal(invoker) { this->invoker = invoker; }

		virtual bool canUse()
		{
			if (!SpellcasterUseSpellGoal::canUse()) return false;
			AABB *box = invoker->bb->grow(20.0, 20.0, 20.0);
			vector<shared_ptr<Entity> > *found = invoker->level->getEntitiesOfClass(typeid(Surrendered), box);
			int count = (found != nullptr) ? static_cast<int>(found->size()) : 0;
			delete found;
			return count < 3;
		}

	protected:
		virtual void performSpellCasting()
		{
			Level *level = invoker->level;
			Random *r = invoker->getRandom();
			for (int i = 0; i < 4; i++)
			{
				int bx = Mth::floor(invoker->x) - 2 + r->nextInt(5);
				int by = Mth::floor(invoker->y) + 1;
				int bz = Mth::floor(invoker->z) - 2 + r->nextInt(5);
				shared_ptr<Surrendered> s = std::make_shared<Surrendered>(level);
				s->moveTo(bx + 0.5, by, bz + 0.5, 0.0f, 0.0f);
				s->finalizeMobSpawn(nullptr);
				s->setOwner(dynamic_pointer_cast<Mob>(invoker->shared_from_this()));
				s->setBoundOrigin(bx, by, bz);
				s->setLifeTicks(20 * (30 + r->nextInt(90)));
				s->setSummoned();
				level->addEntity(s);
			}
		}
		virtual int getCastWarmupTime() { return 100; }
		virtual int getCastingTime() { return 100; }
		virtual int getCastingInterval() { return 340; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_INVOKER_SUMMON_CAST; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_SUMMON_VEX; }
	};

	// Java: ConjureAoeFangsGoal - a ring of five fangs around every non-monster within 18 blocks
	class AoeFangsGoal : public SpellcasterUseSpellGoal
	{
		Invoker *invoker;

		vector<shared_ptr<LivingEntity> > getTargets()
		{
			vector<shared_ptr<LivingEntity> > targets;
			AABB *box = invoker->bb->grow(18.0, 18.0, 18.0);
			vector<shared_ptr<Entity> > *found = invoker->level->getEntitiesOfClass(typeid(LivingEntity), box);
			if (found == nullptr) return targets;
			for (size_t i = 0; i < found->size(); i++)
			{
				shared_ptr<LivingEntity> e = dynamic_pointer_cast<LivingEntity>(found->at(i));
				if (e != nullptr && e->isAlive() && !e->instanceof(eTYPE_MONSTER)) targets.push_back(e);
			}
			delete found;
			return targets;
		}

	public:
		AoeFangsGoal(Invoker *invoker) : SpellcasterUseSpellGoal(invoker) { this->invoker = invoker; }

		virtual bool canUse()
		{
			// neoLegacy - see ShockwaveGoal: the base casting-interval gate used to be bypassed.
			if (!SpellcasterUseSpellGoal::canUse() || invoker->fangAoeCooldown >= 0) return false;
			return !getTargets().empty();
		}

	protected:
		virtual void performSpellCasting()
		{
			vector<shared_ptr<LivingEntity> > targets = getTargets();
			for (size_t t = 0; t < targets.size(); t++)
			{
				shared_ptr<LivingEntity> e = targets[t];
				double minY = min(e->y, invoker->y), maxY = max(e->y, invoker->y) + 1.0;
				float base = static_cast<float>(atan2(e->z - invoker->z, e->x - invoker->x));
				for (int i = 0; i < 5; i++)
				{
					float a = base + i * PI * 0.4f;
					conjureFang(invoker, e->x + Mth::cos(a) * 1.5, e->z + Mth::sin(a) * 1.5, minY, maxY, a, 4);
				}
			}
			invoker->fangAoeCooldown = 100;
		}
		virtual int getCastWarmupTime() { return 40; }
		virtual int getCastingTime() { return 40; }
		virtual int getCastingInterval() { return 100; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_INVOKER_FANGS_CAST; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_FANGS; }
	};

	// Java: ConjureFangsGoal - the Evoker's attack: a bigger set of rings up close, three lines at range
	class FangsGoal : public SpellcasterUseSpellGoal
	{
		Invoker *invoker;
	public:
		FangsGoal(Invoker *invoker) : SpellcasterUseSpellGoal(invoker) { this->invoker = invoker; }

	protected:
		virtual void performSpellCasting()
		{
			shared_ptr<LivingEntity> target = invoker->getTarget();
			if (target == nullptr) return;
			double minY = min(target->y, invoker->y), maxY = max(target->y, invoker->y) + 1.0;
			float f = static_cast<float>(atan2(target->z - invoker->z, target->x - invoker->x));
			if (invoker->distanceToSqr(target) < 9.0)
			{
				for (int i = 0; i < 5; i++)
				{
					float a = f + i * PI * 0.4f;
					conjureFang(invoker, invoker->x + Mth::cos(a) * 1.5, invoker->z + Mth::sin(a) * 1.5, minY, maxY, a, 0);
				}
				for (int i = 0; i < 8; i++)
				{
					float a = f + i * PI * 2.0f / 8.0f + 1.2566371f;
					conjureFang(invoker, invoker->x + Mth::cos(a) * 2.5, invoker->z + Mth::sin(a) * 2.5, minY, maxY, a, 3);
					conjureFang(invoker, invoker->x + Mth::cos(a) * 3.5, invoker->z + Mth::sin(a) * 3.5, minY, maxY, a, 3);
				}
			}
			else
			{
				for (int i = 0; i < 16; i++)
				{
					double d = 1.25 * (i + 1);
					conjureFang(invoker, invoker->x + Mth::cos(f) * d, invoker->z + Mth::sin(f) * d, minY, maxY, f, i);
					// neoLegacy - a side line sits at f +/- 0.4: the same offset in sin and cos, or the line skews (was 0.3 in sin)
					conjureFang(invoker, invoker->x + Mth::cos(f + 0.4f) * d, invoker->z + Mth::sin(f + 0.4f) * d, minY, maxY, f, i);
					conjureFang(invoker, invoker->x + Mth::cos(f - 0.4f) * d, invoker->z + Mth::sin(f - 0.4f) * d, minY, maxY, f, i);
				}
			}
		}
		virtual int getCastWarmupTime() { return 40; }
		virtual int getCastingTime() { return 40; }
		virtual int getCastingInterval() { return 100; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_INVOKER_FANGS_CAST; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_FANGS; }
	};
}

Invoker::Invoker(Level *level) : SpellcasterIllager(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());
	xpReward = 50;
	areaDamageCooldown = 0;
	teleportCooldown = 0;
	fangAoeCooldown = 0;
	shielded = false;

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new SpellcasterCastingSpellGoal(this));
	goalSelector.addGoal(3, new AvoidPlayerGoal(this, typeid(Player), 8.0f, 0.6, 1.0));
	goalSelector.addGoal(4, new BlinkGoal(this));
	goalSelector.addGoal(5, new ShockwaveGoal(this));
	goalSelector.addGoal(5, new SummonSurrenderedGoal(this));
	goalSelector.addGoal(5, new AoeFangsGoal(this));
	goalSelector.addGoal(6, new FangsGoal(this));
	goalSelector.addGoal(7, new RaidMoveGoal(this, this, 1.0));
	goalSelector.addGoal(8, new RandomStrollGoal(this, 0.6));
	goalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f));
	goalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Villager), 0, false));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(VillagerGolem), 0, false));
}

void Invoker::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.36f);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(24);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(250);
	getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(8);
	getAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE)->setBaseValue(0.3);
}

void Invoker::newServerAiStep()
{
	SpellcasterIllager::newServerAiStep();
	teleportCooldown--;
	areaDamageCooldown--;
	fangAoeCooldown--;

	// it hovers: a fall is slowed to a drift
	if (!onGround && yd < 0) yd *= 0.6;
}

bool Invoker::doHurtTarget(shared_ptr<Entity> target)
{
	if (!Monster::doHurtTarget(target)) return false;
	if (target != nullptr && target->instanceof(eTYPE_LIVINGENTITY))
	{
		dynamic_pointer_cast<LivingEntity>(target)->addEffect(new MobEffectInstance(MobEffect::blindness->id, 100, 0));
	}
	return true;
}

// The shield (Java: hurtServer / isInvulnerableTo): a projectile has a 50% chance to put it up, and while it is up projectiles do nothing;
// any other hit has a 1 in 3 chance of breaking it.
bool Invoker::hurt(DamageSource *source, float dmg)
{
	if (!level->isClientSide)
	{
		if (source->isProjectile())
		{
			if (shielded) return false;
			if (random->nextInt(2) == 0)
			{
				playSound(eSoundType_MOB_INVOKER_SHIELD_CREATE, 1.0f, 0.8f + random->nextFloat() * 0.4f);
				shielded = true;
				return false;
			}
		}
		else if (shielded && random->nextInt(3) == 0)
		{
			playSound(eSoundType_MOB_INVOKER_SHIELD_BREAK, 1.0f, 0.8f + random->nextFloat() * 0.4f);
			shielded = false;
		}
	}
	return SpellcasterIllager::hurt(source, dmg);
}

// its Surrendered count as allies too
bool Invoker::isAlliedTo(shared_ptr<LivingEntity> other)
{
	if (other == nullptr) return false;
	if (other->GetType() == eTYPE_SURRENDERED)
	{
		// neoLegacy - the cast used to be dereferenced unchecked: a Surrendered-typed entity that is not a Surrendered
		// would crash here with a null virtual call. (Hardening; the type mismatch itself is unproven.)
		shared_ptr<Surrendered> surrendered = dynamic_pointer_cast<Surrendered>(other);
		if (surrendered == nullptr) return false;
		shared_ptr<Mob> owner = surrendered->getOwner();
		if (owner != nullptr) return owner.get() == this || isAlliedTo(owner);
	}
	return SpellcasterIllager::isAlliedTo(other);
}

bool Invoker::removeWhenFarAway()
{
	return false;
}

// the mod's loot table: 3-5 emeralds (+0-2 with Looting) and a Primal Essence, only when a player kills it
void Invoker::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Monster::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);
	if (!wasKilledByPlayer) return;

	int emeralds = 3 + random->nextInt(3);
	if (playerBonusLevel > 0) emeralds += random->nextInt(playerBonusLevel * 2 + 1);
	for (int i = 0; i < emeralds; i++) spawnAtLocation(Item::emerald_Id, 1);
	spawnAtLocation(Item::primal_essence_Id, 1);
}

int Invoker::getCastingSoundId() { return eSoundType_MOB_INVOKER_CAST; }
int Invoker::getAmbientSound() { return eSoundType_MOB_INVOKER_IDLE; }
int Invoker::getHurtSound() { return eSoundType_MOB_INVOKER_HURT; }
int Invoker::getDeathSound() { return eSoundType_MOB_INVOKER_DEATH; }
