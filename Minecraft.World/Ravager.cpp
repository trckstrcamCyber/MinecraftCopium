#include "stdafx.h"
#include "RaidGoals.h"
#include "Random.h"
#include "Mth.h"
#include "AABB.h"
#include "PathNavigation.h"
#include "AttributeInstance.h"
#include "ItemInstance.h"
#include "GameRules.h"
#include "EntityEvent.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
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
#include "Ravager.h"

Ravager::Ravager(Level *level) : Monster(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());
	xpReward = 20;
	attackTick = 0;
	inRaid = false;
	raidX = raidY = raidZ = 0;
	setSize(1.95f, 2.2f);

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(4, new MeleeAttackGoal(this, eTYPE_PLAYER, 1.0, false));
	goalSelector.addGoal(4, new MeleeAttackGoal(this, eTYPE_VILLAGER, 1.0, false));
	goalSelector.addGoal(4, new MeleeAttackGoal(this, eTYPE_VILLAGERGOLEM, 1.0, false));
	goalSelector.addGoal(4, new RaidMoveGoal(this, this, 1.0));
	goalSelector.addGoal(5, new RandomStrollGoal(this, 0.4));
	goalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Player), 6.0f));
	goalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f));
	goalSelector.addGoal(10, new RandomLookAroundGoal(this));

	targetSelector.addGoal(2, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));
	targetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Villager), 0, false));
	targetSelector.addGoal(4, new NearestAttackableTargetGoal(this, typeid(VillagerGolem), 0, false));
}

void Ravager::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(100);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.3f);
	getAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE)->setBaseValue(0.75);
	getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(12);
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(32);
}

void Ravager::aiStep()
{
	Monster::aiStep();

	if (attackTick > 0) attackTick--;

	if (isAlive())
	{
		// it speeds up while it is hunting something
		double wanted = (getTarget() != nullptr) ? 0.35 : 0.3;
		AttributeInstance *speed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
		double current = speed->getBaseValue();
		speed->setBaseValue(current + (wanted - current) * 0.1);

		// it barrels through leaves in its way; if nothing gave way it hops instead
		if (!level->isClientSide && horizontalCollision && level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
		{
			bool broke = false;
			int x0 = Mth::floor(bb->x0 - 0.2), x1 = Mth::floor(bb->x1 + 0.2);
			int y0 = Mth::floor(bb->y0 - 0.2), y1 = Mth::floor(bb->y1 + 0.2);
			int z0 = Mth::floor(bb->z0 - 0.2), z1 = Mth::floor(bb->z1 + 0.2);
			for (int x = x0; x <= x1; x++)
			{
				for (int y = y0; y <= y1; y++)
				{
					for (int z = z0; z <= z1; z++)
					{
						int t = level->getTile(x, y, z);
						if (t == Tile::leaves_Id || t == Tile::leaves2_Id)
						{
							broke = level->destroyTile(x, y, z, true) || broke;
						}
					}
				}
			}
			if (!broke && onGround) jumpFromGround();
		}
	}
}

// the bite: its head lunges (attackTick drives the model), and the victim is hurled away from it
bool Ravager::doHurtTarget(shared_ptr<Entity> target)
{
	attackTick = 10;
	level->broadcastEntityEvent(shared_from_this(), EntityEvent::START_ATTACKING);
	playSound(eSoundType_MOB_RAVAGER_BITE, 1.0f, 1.0f);

	bool hurt = Monster::doHurtTarget(target);
	if (hurt)
	{
		double dx = target->x - x;
		double dz = target->z - z;
		double len = sqrt(dx * dx + dz * dz);
		if (len < 0.0001) len = 1.0;
		target->xd += dx / len * 0.9;
		target->zd += dz / len * 0.9;
		target->yd += 0.3;
	}
	return hurt;
}

void Ravager::handleEntityEvent(byte id)
{
	if (id == EntityEvent::START_ATTACKING)
	{
		attackTick = 10;
	}
	else Monster::handleEntityEvent(id);
}

// a Ravager always drops one saddle
void Ravager::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Monster::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);
	spawnAtLocation(Item::saddle_Id, 1);

	// the Illager Invasion mod adds a Platinum Chunk: 50%, 56.25% with Looting I and 6.25% more per further level, when a player kills it
	if (wasKilledByPlayer)
	{
		int chunkChance = (playerBonusLevel > 0) ? 5625 + 625 * (playerBonusLevel - 1) : 5000;
		if (random->nextInt(10000) < chunkChance) spawnAtLocation(Item::platinum_chunk_Id, 1);
	}
}

int Ravager::getAmbientSound() { return eSoundType_MOB_RAVAGER_IDLE; }
int Ravager::getHurtSound() { return eSoundType_MOB_RAVAGER_HURT; }
int Ravager::getDeathSound() { return eSoundType_MOB_RAVAGER_DEATH; }

void Ravager::playStepSound(int xt, int yt, int zt, int t)
{
	playSound(eSoundType_MOB_RAVAGER_STEP, 0.15f, 1.0f);
}
