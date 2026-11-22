#include "stdafx.h"
#include "RaidGoals.h"
#include "Random.h"
#include "AABB.h"
#include "AttributeInstance.h"
#include "ItemInstance.h"
#include "EnchantmentHelper.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
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
#include "HatchetItem.h"
#include "WeaponItem.h"
#include "Archivist.h"

namespace
{
	// the Java rule, in one place: a bow, a sword, an axe or a crossbow in hand
	bool isEnchantableWeapon(shared_ptr<ItemInstance> item)
	{
		if (item == nullptr) return false;
		if (item->id == Item::bow_Id || item->id == Item::crossbow_Id) return true;
		Item *i = item->getItem();
		return dynamic_cast<WeaponItem *>(i) != nullptr || dynamic_cast<HatchetItem *>(i) != nullptr;
	}

	// Enchant ally (Java: EnchantAllyGoal): the target is a random illager within 16 blocks that holds something to improve
	class EnchantAllyGoal : public SpellcasterUseSpellGoal
	{
		Archivist *archivist;
		weak_ptr<Illager> enchantTarget;
		int lastTargetId;

		shared_ptr<Illager> pickTarget()
		{
			AABB *box = archivist->bb->grow(16.0, 4.0, 16.0);
			vector<shared_ptr<Entity> > *found = archivist->level->getEntitiesOfClass(typeid(Monster), box);
			vector<shared_ptr<Illager> > candidates;
			if (found != nullptr)
			{
				for (size_t i = 0; i < found->size(); i++)
				{
					shared_ptr<Illager> illager = dynamic_pointer_cast<Illager>(found->at(i));
					if (illager == nullptr || !illager->isAlive() || illager->GetType() == eTYPE_ARCHIVIST) continue;
					if (Archivist::eligibleForEnchant(illager)) candidates.push_back(illager);
				}
				delete found;
			}
			if (candidates.empty()) return nullptr;
			return candidates[archivist->getRandom()->nextInt(static_cast<int>(candidates.size()))];
		}

	public:
		EnchantAllyGoal(Archivist *archivist) : SpellcasterUseSpellGoal(archivist) { this->archivist = archivist; lastTargetId = -1; }

		virtual bool canUse()
		{
			if (!SpellcasterUseSpellGoal::canUse()) return false;
			shared_ptr<Illager> t = pickTarget();
			if (t == nullptr || t->entityId == lastTargetId) return false;
			enchantTarget = t;
			return true;
		}
		virtual void stop()
		{
			SpellcasterUseSpellGoal::stop();
			enchantTarget.reset();
		}

	protected:
		virtual void performSpellCasting()
		{
			shared_ptr<Illager> t = enchantTarget.lock();
			if (t == nullptr || !t->isAlive()) return;
			lastTargetId = t->entityId;
			Archivist::enchantHeldGear(t);
		}
		virtual int getCastWarmupTime() { return 50; }
		virtual int getCastingTime() { return 50; }
		virtual int getCastingInterval() { return 400; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_SORCERER_CAST; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_SUMMON_VEX; }
	};

	// The shockwave (Java: LevitateTargetsGoal): needs a target, and something alive that is not a monster within 6 blocks
	class ShockwaveGoal : public SpellcasterUseSpellGoal
	{
		Archivist *archivist;

		vector<shared_ptr<LivingEntity> > getTargets()
		{
			vector<shared_ptr<LivingEntity> > targets;
			AABB *box = archivist->bb->grow(6.0, 6.0, 6.0);
			vector<shared_ptr<Entity> > *found = archivist->level->getEntitiesOfClass(typeid(LivingEntity), box);
			if (found == nullptr) return targets;
			for (size_t i = 0; i < found->size(); i++)
			{
				shared_ptr<LivingEntity> e = dynamic_pointer_cast<LivingEntity>(found->at(i));
				if (e == nullptr || !e->isAlive() || e->instanceof(eTYPE_MONSTER)) continue;
				if (e->instanceof(eTYPE_PLAYER) && dynamic_pointer_cast<Player>(e)->abilities.instabuild) continue;
				targets.push_back(e);
			}
			delete found;
			return targets;
		}

	public:
		ShockwaveGoal(Archivist *archivist) : SpellcasterUseSpellGoal(archivist) { this->archivist = archivist; }

		virtual bool canUse() { return SpellcasterUseSpellGoal::canUse() && !getTargets().empty(); }

	protected:
		virtual void performSpellCasting()
		{
			vector<shared_ptr<LivingEntity> > targets = getTargets();
			shared_ptr<Entity> self = archivist->shared_from_this();
			for (size_t i = 0; i < targets.size(); i++)
			{
				shared_ptr<LivingEntity> e = targets[i];
				// thrown away from the Archivist: stronger the closer they stand, but never absurdly so
				double dx = e->x - archivist->x, dz = e->z - archivist->z;
				double f = dx * dx + dz * dz;
				if (f < 0.001) f = 0.001;
				double px = dx / f * 5.0, pz = dz / f * 5.0;
				double len = sqrt(px * px + pz * pz);
				if (len > 2.5) { px = px / len * 2.5; pz = pz / len * 2.5; }
				e->xd += px;
				e->yd = 0.6;
				e->zd += pz;
				e->hurtMarked = true;

				DamageSource *ds = DamageSource::indirectMagic(self, self);
				e->hurt(ds, 4.0f);
				delete ds;
			}
		}
		virtual int getCastWarmupTime() { return 50; }
		virtual int getCastingTime() { return 50; }
		virtual int getCastingInterval() { return 220; }
		virtual int getSpellPrepareSound() { return eSoundType_MOB_ILLUSIONER_PREPARE_BLIND; }
		virtual SpellcasterIllager::Spell getSpell() { return SpellcasterIllager::SPELL_FANGS; }
	};
}

Archivist::Archivist(Level *level) : SpellcasterIllager(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());
	xpReward = Enemy::XP_REWARD_LARGE;

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new SpellcasterCastingSpellGoal(this));
	goalSelector.addGoal(3, new EnchantAllyGoal(this));
	goalSelector.addGoal(4, new ShockwaveGoal(this));
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

void Archivist::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5f);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(18);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(24);
}

bool Archivist::eligibleForEnchant(shared_ptr<LivingEntity> entity)
{
	return entity != nullptr && isEnchantableWeapon(entity->getCarriedItem());
}

// Java's EnchantToolUtil, with this build's enchantments. It raises the level, never lowers it (and keeps whatever else is on the weapon).
void Archivist::enchantHeldGear(shared_ptr<LivingEntity> entity)
{
	if (entity == nullptr) return;
	shared_ptr<ItemInstance> item = entity->getCarriedItem();
	if (!isEnchantableWeapon(item)) return;

	int id, level;
	if (item->id == Item::bow_Id) { id = Enchantment::arrowBonus->id; level = 3; }
	else if (item->id == Item::crossbow_Id)
	{
		if (entity->getRandom()->nextInt(2) == 0) { id = Enchantment::piercing->id; level = 4; }
		else { id = Enchantment::multishot->id; level = 1; }
	}
	else { id = Enchantment::damageBonus->id; level = 3; }

	unordered_map<int, int> *enchantments = EnchantmentHelper::getEnchantments(item);
	if (enchantments == nullptr) return;
	auto it = enchantments->find(id);
	if (it == enchantments->end() || it->second < level) (*enchantments)[id] = level;
	EnchantmentHelper::setEnchantments(enchantments, item);
	delete enchantments;
}

// the mod's loot table: 0-1 emeralds when a player kills it, 0-1 books, 0-2 paper, all helped by Looting
void Archivist::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Monster::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);

	int books = random->nextInt(2), paper = random->nextInt(3);
	if (playerBonusLevel > 0) { books += random->nextInt(playerBonusLevel + 1); paper += random->nextInt(playerBonusLevel + 1); }
	for (int i = 0; i < books; i++) spawnAtLocation(Item::book_Id, 1);
	for (int i = 0; i < paper; i++) spawnAtLocation(Item::paper_Id, 1);

	if (wasKilledByPlayer)
	{
		int count = random->nextInt(2);
		if (playerBonusLevel > 0) count += random->nextInt(playerBonusLevel + 1);
		for (int i = 0; i < count; i++) spawnAtLocation(Item::emerald_Id, 1);

		// not from the mod: its Lost Candle only turns up in labyrinth chests, and there is no labyrinth here yet - so an Archivist sometimes carries one (5%, +1% per Looting level)
		if (random->nextInt(100) < 5 + playerBonusLevel) spawnAtLocation(Item::lost_candle_Id, 1);
	}
}

int Archivist::getCastingSoundId() { return eSoundType_MOB_SORCERER_COMPLETECAST; }
int Archivist::getAmbientSound() { return eSoundType_MOB_ARCHIVIST_IDLE; }
int Archivist::getHurtSound() { return eSoundType_MOB_ARCHIVIST_HURT; }
int Archivist::getDeathSound() { return eSoundType_MOB_ARCHIVIST_DEATH; }
