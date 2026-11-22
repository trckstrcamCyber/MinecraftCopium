#include "stdafx.h"
#include "Random.h"
#include "AABB.h"
#include "AttributeInstance.h"
#include "ItemInstance.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.h"
#include "SoundTypes.h"
#include "Inquisitor.h"

Inquisitor::Inquisitor(Level *level) : Basher(level)
{
	// the Basher's constructor registered the attributes; this just sets the Inquisitor's own values
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(80);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.33f);
	getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(10);
	getAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE)->setBaseValue(0.8);
	setHealth(getMaxHealth());
	xpReward = 20;
}

// Java: it breaks leaves, cobwebs and wooden doors it runs into, only with mob griefing on
void Inquisitor::aiStep()
{
	if (horizontalCollision && !level->isClientSide && level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
	{
		int x0 = Mth::floor(bb->x0 - 1.0), x1 = Mth::floor(bb->x1 + 1.0);
		int y0 = Mth::floor(bb->y0 - 1.0), y1 = Mth::floor(bb->y1 + 1.0);
		int z0 = Mth::floor(bb->z0 - 1.0), z1 = Mth::floor(bb->z1 + 1.0);
		for (int x = x0; x <= x1; x++)
			for (int y = y0; y <= y1; y++)
				for (int z = z0; z <= z1; z++)
				{
					int t = level->getTile(x, y, z);
					if (t != Tile::leaves_Id && t != Tile::leaves2_Id && t != Tile::web_Id && t != Tile::wooden_door_Id) continue;
					if (level->destroyTile(x, y, z, true) && t == Tile::wooden_door_Id) playSound(eSoundType_MOB_ZOMBIE_WOOD_BREAK, 1.0f, 1.0f);
				}
	}
	Basher::aiStep();
}

// The roar when the shield breaks (Java: blockedByItem): every non-monster within 8 blocks is thrown back and weakened for 10 seconds
void Inquisitor::onShieldBroken()
{
	playSound(eSoundType_MOB_RAVAGER_ROAR, 1.0f, 1.0f);

	AABB *box = bb->grow(8.0, 8.0, 8.0);
	vector<shared_ptr<Entity> > *found = level->getEntitiesOfClass(typeid(LivingEntity), box);
	if (found == nullptr) return;
	for (size_t i = 0; i < found->size(); i++)
	{
		shared_ptr<LivingEntity> e = dynamic_pointer_cast<LivingEntity>(found->at(i));
		if (e == nullptr || !e->isAlive() || e->instanceof(eTYPE_MONSTER)) continue;
		double dx = e->x - x, dz = e->z - z;
		double f = dx * dx + dz * dz;
		if (f < 0.001) f = 0.001;
		e->xd += dx / f * 0.6;
		e->yd += 0.4;
		e->zd += dz / f * 0.6;
		e->hurtMarked = true;
		e->addEffect(new MobEffectInstance(MobEffect::weakness->id, 200, 0));
	}
	delete found;
}

// the mod's loot table: 0-3 iron ingots, 0-2 leather (+0-2 each with Looting), and when a player kills it 0-1 emeralds and a Platinum Chunk (50%, as for the Pillager)
void Inquisitor::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	Monster::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);

	int iron = random->nextInt(4), leather = random->nextInt(3);
	if (playerBonusLevel > 0) { iron += random->nextInt(playerBonusLevel * 2 + 1); leather += random->nextInt(playerBonusLevel * 2 + 1); }
	for (int i = 0; i < iron; i++) spawnAtLocation(Item::iron_ingot_Id, 1);
	for (int i = 0; i < leather; i++) spawnAtLocation(Item::leather_Id, 1);

	if (wasKilledByPlayer)
	{
		int count = random->nextInt(2);
		if (playerBonusLevel > 0) count += random->nextInt(playerBonusLevel + 1);
		for (int i = 0; i < count; i++) spawnAtLocation(Item::emerald_Id, 1);
		int chunkChance = (playerBonusLevel > 0) ? 5625 + 625 * (playerBonusLevel - 1) : 5000;
		if (random->nextInt(10000) < chunkChance) spawnAtLocation(Item::platinum_chunk_Id, 1);
	}
}

// the mod's own "illager brute" voice
int Inquisitor::getAmbientSound() { return eSoundType_MOB_INQUISITOR_IDLE; }
int Inquisitor::getHurtSound() { return eSoundType_MOB_INQUISITOR_HURT; }
int Inquisitor::getDeathSound() { return eSoundType_MOB_INQUISITOR_DEATH; }
