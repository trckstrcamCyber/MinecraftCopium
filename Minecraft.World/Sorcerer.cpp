#include "stdafx.h"
#include "RaidGoals.h"
#include "Random.h"
#include "PathNavigation.h"
#include "AttributeInstance.h"
#include "ItemInstance.h"
#include "AABB.h"
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
#include "Sorcerer.h"

namespace
{
	// Conjure flames (Java: ConjureFlamesGoal): needs a target. 3 magic damage, and a fire where it stands (or in an empty block next to that).
	class ConjureFlamesGoal : public SpellcasterUseSpellGoal
	{
		Sorcerer *sorcerer;

		static bool tryFire(Level *level, int x, int y, int z)
		{
			if (!level->isEmptyTile(x, y, z) || !Tile::fire->mayPlace(level, x, y, z)) return false;
			level->setTileAndUpdate(x, y, z, Tile::fire_Id);
			return true;
		}

	public:
		ConjureFlamesGoal(Sorcerer *sorcerer) : SpellcasterUseSpellGoal(sorcerer) { this->sorcerer = sorcerer; }

	protected:
		virtual void performSpellCasting()
		{
			shared_ptr<LivingEntity> target = sorcerer->getTarget();
			if (target == nullptr) return;

			Level *level = sorcerer->level;
			if (level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
			{
				int x = Mth::floor(target->x), y = Mth::floor(target->y), z = Mth::floor(target->z);
				if (!tryFire(level, x, y, z)
					&& !tryFire(level, x + 1, y, z) && !tryFire(level, x - 1, y, z)
					&& !tryFire(level, x, y, z + 1) && !tryFire(level, x, y, z - 1)
					&& !tryFire(level, x, y + 1, z)) { /* nowhere to burn */ }
			}
			shared_ptr<Entity> self = sorcerer->shared_from_this();
			target->hurt(DamageSource::indirectMagic(self, self), 3.0f);
		}
		virtual int getCastWarmupTime() { return 60; }
		virtual int getCastingTime() { return 60; }
		virtual int getCastingInterval() { return 140; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_SORCERER_CAST; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_WOLOLO; }
	};

	// Teleport (Java: CastTeleportGoal): only when a player or an iron golem is within 8 blocks of it
	class CastTeleportGoal : public SpellcasterUseSpellGoal
	{
		Sorcerer *sorcerer;

		bool threatNearby()
		{
			Level *level = sorcerer->level;
			for (size_t i = 0; i < level->players.size(); i++)
			{
				shared_ptr<Player> p = level->players[i];
				if (p != nullptr && !p->removed && p->isAlive() && sorcerer->distanceToSqr(p) <= 64.0) return true;
			}
			AABB *box = sorcerer->bb->grow(8.0, 8.0, 8.0);
			vector<shared_ptr<Entity> > *golems = level->getEntitiesOfClass(typeid(VillagerGolem), box);
			bool found = golems != nullptr && !golems->empty();
			delete golems;
			return found;
		}

	public:
		CastTeleportGoal(Sorcerer *sorcerer) : SpellcasterUseSpellGoal(sorcerer) { this->sorcerer = sorcerer; }

		virtual bool canUse() { return SpellcasterUseSpellGoal::canUse() && threatNearby(); }

	protected:
		virtual void performSpellCasting() { sorcerer->tryRandomTeleport(); }
		virtual int getCastWarmupTime() { return 30; }
		virtual int getCastingTime() { return 30; }
		virtual int getCastingInterval() { return 220; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_SORCERER_CAST; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_DISAPPEAR; }
	};
}

Sorcerer::Sorcerer(Level *level) : SpellcasterIllager(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());
	xpReward = Enemy::XP_REWARD_LARGE;

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new SpellcasterCastingSpellGoal(this));
	goalSelector.addGoal(3, new ConjureFlamesGoal(this));
	goalSelector.addGoal(4, new CastTeleportGoal(this));
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

void Sorcerer::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5f);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(18);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(32);
}

// Settles onto the ground the way the EnderMan does (it is the engine's own proven way of finding a legal spot)
bool Sorcerer::tryRandomTeleport()
{
	double xo = x, yo = y, zo = z;
	for (int attempt = 0; attempt < 16; attempt++)
	{
		double nx = xo + (random->nextDouble() - 0.5) * 24.0;
		double nz = zo + (random->nextDouble() - 0.5) * 24.0;
		double dx = nx - xo, dz = nz - zo;
		if (dx * dx + dz * dz < 25.0) continue;	// at least 5 blocks away, or it was pointless

		int xt = Mth::floor(nx), zt = Mth::floor(nz);
		int yt = Mth::floor(yo) + random->nextInt(9) - 4;
		if (yt < 2 || !level->hasChunkAt(xt, yt, zt)) continue;

		bool landed = false;
		while (!landed && yt > 0)
		{
			int t = level->getTile(xt, yt - 1, zt);
			if (t == 0 || !(Tile::tiles[t]->material->blocksMotion())) yt--;
			else landed = true;
		}
		if (!landed) continue;

		setPos(nx, yt, nz);
		if (level->getCubes(shared_from_this(), bb)->empty() && !level->containsAnyLiquid(bb))
		{
			level->playSound(xo, yo, zo, eSoundType_MOB_SORCERER_COMPLETECAST, 1.0f, 1.0f);
			playSound(eSoundType_MOB_SORCERER_COMPLETECAST, 1.0f, 1.0f);
			getNavigation()->stop();
			return true;
		}
		setPos(xo, yo, zo);
	}
	return false;
}

// the mod's loot table: 0-2 books and, when a player kills it, 0-1 emeralds, both helped by Looting (its Unusual Dust has no counterpart here)
void Sorcerer::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Monster::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);

	int books = random->nextInt(3);
	if (playerBonusLevel > 0) books += random->nextInt(playerBonusLevel + 1);
	for (int i = 0; i < books; i++) spawnAtLocation(Item::book_Id, 1);

	if (wasKilledByPlayer)
	{
		int count = random->nextInt(2);
		if (playerBonusLevel > 0) count += random->nextInt(playerBonusLevel + 1);
		for (int i = 0; i < count; i++) spawnAtLocation(Item::emerald_Id, 1);

		// Unusual Dust (the mod's): 40%, 60% with Looting I and 20% more per further level
		int dustChance = (playerBonusLevel > 0) ? 60 + 20 * (playerBonusLevel - 1) : 40;
		if (random->nextInt(100) < dustChance) spawnAtLocation(Item::unusual_dust_Id, 1);
	}
}

int Sorcerer::getCastingSoundId() { return eSoundType_MOB_SORCERER_COMPLETECAST; }
int Sorcerer::getAmbientSound() { return eSoundType_MOB_SORCERER_IDLE; }
int Sorcerer::getHurtSound() { return eSoundType_MOB_SORCERER_HURT; }
int Sorcerer::getDeathSound() { return eSoundType_MOB_SORCERER_DEATH; }
