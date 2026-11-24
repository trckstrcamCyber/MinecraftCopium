#include "stdafx.h"
#include "RaidGoals.h"
#include "Random.h"
#include "AABB.h"
#include "AttributeInstance.h"
#include "ItemInstance.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
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
#include "net.minecraft.world.entity.h"
#include "SoundTypes.h"
#include "IllagerSpellGoals.h"
#include "FlyingMagma.h"
#include "Firecaller.h"

namespace
{
	// Conjure a Flying Magma (Java: ConjureSkullGoal). Needs a target, and nobody (player or iron golem) within 5 blocks; every ~8 seconds.
	class ConjureMagmaGoal : public SpellcasterUseSpellGoal
	{
		Firecaller *firecaller;

		bool somebodyClose()
		{
			Level *level = firecaller->level;
			for (size_t i = 0; i < level->players.size(); i++)
			{
				shared_ptr<Player> p = level->players[i];
				if (p != nullptr && !p->removed && p->isAlive() && firecaller->distanceToSqr(p) <= 25.0) return true;
			}
			AABB *box = firecaller->bb->grow(5.0, 5.0, 5.0);
			vector<shared_ptr<Entity> > *golems = level->getEntitiesOfClass(typeid(VillagerGolem), box);
			bool found = golems != nullptr && !golems->empty();
			delete golems;
			return found;
		}

	public:
		ConjureMagmaGoal(Firecaller *firecaller) : SpellcasterUseSpellGoal(firecaller) { this->firecaller = firecaller; }

		virtual bool canUse()
		{
			// neoLegacy - the base gate (live target, not already casting, casting-interval elapsed) used to be
			// bypassed, so getCastingInterval() was dead code and the real cadence was the cooldown field alone.
			if (!SpellcasterUseSpellGoal::canUse()) return false;
			if (firecaller->conjureSkullCooldown > 0) return false;
			return !somebodyClose();
		}

	protected:
		virtual void performSpellCasting()
		{
			shared_ptr<LivingEntity> target = firecaller->getTarget();
			if (target == nullptr) return;
			double sx = firecaller->x, sy = firecaller->y + 2.5, sz = firecaller->z;
			double xd = target->x - sx;
			double yd = (target->y + target->bbHeight * 0.5) - sy;
			double zd = target->z - sz;
			shared_ptr<FlyingMagma> magma = std::make_shared<FlyingMagma>(firecaller->level, dynamic_pointer_cast<LivingEntity>(firecaller->shared_from_this()), xd, yd, zd);
			magma->x = sx;
			magma->y = sy;
			magma->z = sz;
			firecaller->level->addEntity(magma);
			firecaller->conjureSkullCooldown = 160;
		}
		virtual int getCastWarmupTime() { return 60; }
		virtual int getCastingTime() { return 60; }
		virtual int getCastingInterval() { return 160; }	// neoLegacy - was 400 (dead code); now the live gate, matching conjureSkullCooldown
		virtual int getSpellPrepareSound() { return eSoundType_MOB_FIRECALLER_CAST; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_WOLOLO; }
	};

	// The fire burst (Java: AreaDamageGoal): everything within 6 blocks that is not on its side is thrown up, hurt, set alight and given a fire at its feet
	class FireBurstGoal : public SpellcasterUseSpellGoal
	{
		Firecaller *firecaller;
	public:
		FireBurstGoal(Firecaller *firecaller) : SpellcasterUseSpellGoal(firecaller) { this->firecaller = firecaller; }

		virtual bool canUse()
		{
			// neoLegacy - see ConjureMagmaGoal: the base casting-interval gate used to be bypassed.
			if (!SpellcasterUseSpellGoal::canUse()) return false;
			return firecaller->areaDamageCooldown <= 0;
		}

	protected:
		virtual void performSpellCasting()
		{
			Level *level = firecaller->level;
			bool grief = level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING);
			shared_ptr<Entity> self = firecaller->shared_from_this();

			AABB *box = firecaller->bb->grow(6.0, 6.0, 6.0);
			vector<shared_ptr<Entity> > *found = level->getEntitiesOfClass(typeid(LivingEntity), box);
			if (found != nullptr)
			{
				for (size_t i = 0; i < found->size(); i++)
				{
					shared_ptr<LivingEntity> e = dynamic_pointer_cast<LivingEntity>(found->at(i));
					if (e == nullptr || !e->isAlive()) continue;
					if (dynamic_cast<Illager *>(e.get()) != nullptr || e->GetType() == eTYPE_SURRENDERED || e->GetType() == eTYPE_RAVAGER || e->GetType() == eTYPE_VEX) continue;
					if (e->instanceof(eTYPE_PLAYER) && dynamic_pointer_cast<Player>(e)->abilities.instabuild) continue;

					e->yd += 1.2;
					e->hurtMarked = true;
					DamageSource *ds = DamageSource::indirectMagic(self, self);
					e->hurt(ds, 6.0f);
					delete ds;
					e->setOnFire(6);

					if (grief)
					{
						int fx = Mth::floor(e->x), fy = Mth::floor(e->y), fz = Mth::floor(e->z);
						if (level->isEmptyTile(fx, fy, fz) && Tile::fire->mayPlace(level, fx, fy, fz)) level->setTileAndUpdate(fx, fy, fz, Tile::fire_Id);
					}
				}
				delete found;
			}
			firecaller->areaDamageCooldown = 300;
		}
		virtual int getCastWarmupTime() { return 50; }
		virtual int getCastingTime() { return 50; }
		virtual int getCastingInterval() { return 300; }	// neoLegacy - was 400 (dead code); now the live gate, matching areaDamageCooldown
		virtual int getSpellPrepareSound() { return eSoundType_MOB_FIRECALLER_CAST; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_WOLOLO; }
	};
}

Firecaller::Firecaller(Level *level) : SpellcasterIllager(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());
	xpReward = 10;
	// neoLegacy - both cooldowns used to start at full (160/300 ticks), so a freshly spawned Firecaller could not use
	// either spell for its first 8-15 seconds; with a player in melee the magma spell is blocked outright
	// (somebodyClose), leaving it unable to attack at all until it died. The Evoker has no initial cooldown.
	conjureSkullCooldown = 0;
	areaDamageCooldown = 0;

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(2, new SpellcasterCastingSpellGoal(this));
	goalSelector.addGoal(3, new FireBurstGoal(this));
	goalSelector.addGoal(4, new ConjureMagmaGoal(this));
	goalSelector.addGoal(5, new AvoidPlayerGoal(this, typeid(Player), 8.0f, 0.6, 1.0));
	goalSelector.addGoal(5, new AvoidPlayerGoal(this, typeid(VillagerGolem), 12.0f, 0.6, 1.0));
	goalSelector.addGoal(8, new RandomStrollGoal(this, 0.6));
	goalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f));
	goalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Villager), 0, false));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(VillagerGolem), 0, false));
}

void Firecaller::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.38f);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(18);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(32);
}

void Firecaller::newServerAiStep()
{
	SpellcasterIllager::newServerAiStep();
	conjureSkullCooldown--;
	areaDamageCooldown--;
}

// resting it leans on its staff (the model draws it in the NEUTRAL pose); casting it throws its arms up
Illager::ArmPose Firecaller::getArmPose()
{
	return isCastingSpell() ? SPELLCASTING : NEUTRAL;
}

// the mod's loot table: 1-3 fire charges (+0-1 with Looting), and 0-1 emeralds (+0-1) when a player kills it
void Firecaller::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Monster::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);

	int charges = 1 + random->nextInt(3);
	if (playerBonusLevel > 0) charges += random->nextInt(playerBonusLevel + 1);
	for (int i = 0; i < charges; i++) spawnAtLocation(Item::fire_charge_Id, 1);

	if (wasKilledByPlayer)
	{
		int count = random->nextInt(2);
		if (playerBonusLevel > 0) count += random->nextInt(playerBonusLevel + 1);
		for (int i = 0; i < count; i++) spawnAtLocation(Item::emerald_Id, 1);
	}
}

int Firecaller::getCastingSoundId() { return eSoundType_MOB_FIRECALLER_CAST; }
int Firecaller::getAmbientSound() { return eSoundType_MOB_FIRECALLER_IDLE; }
int Firecaller::getHurtSound() { return eSoundType_MOB_FIRECALLER_HURT; }
int Firecaller::getDeathSound() { return eSoundType_MOB_FIRECALLER_DEATH; }
