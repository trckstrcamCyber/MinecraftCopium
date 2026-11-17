#include "stdafx.h"
#include "Random.h"
#include "Mth.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "MobSpawner.h"
#include "MobCategory.h"
#include "MobEffect.h"
#include "MobEffectInstance.h"
#include "net.minecraft.world.item.enchantment.h"
#include "CrossbowItem.h"
#include "RaidMember.h"
#include "RaidManager.h"

// Java's raid wave table: how many of each raider a wave has (index = wave number, 1-based)
static const int WAVE_VINDICATORS[8] = { 0, 0, 2, 0, 1, 4, 2, 5 };
static const int WAVE_EVOKERS[8]     = { 0, 0, 0, 0, 0, 1, 1, 2 };
static const int WAVE_PILLAGERS[8]   = { 0, 4, 3, 3, 4, 4, 4, 2 };
static const int WAVE_WITCHES[8]     = { 0, 0, 0, 0, 3, 0, 0, 1 };
static const int WAVE_RAVAGERS[8]    = { 0, 0, 0, 1, 0, 1, 0, 2 };

static const int PATROL_FIRST_DAY_TICKS = 5 * 24000;
static const int FIRST_WAVE_DELAY = 600;		// 30 seconds
static const int NEXT_WAVE_DELAY = 200;			// 10 seconds after the last raider falls

RaidManager::RaidManager(Level *level)
{
	this->level = level;
	patrolCountdown = 6000;
}

void RaidManager::tick()
{
	if (level->isClientSide || level->dimension->id != 0) return;

	if (level->difficulty == 0)
	{
		raids.clear();
		return;
	}

	tickPatrols();

	if (level->getGameTime() % 20 == 0) detectBadOmen();

	for (size_t i = 0; i < raids.size(); i++) tickRaid(raids[i]);
	for (size_t i = raids.size(); i > 0; i--)
	{
		if (raids[i - 1].finished) raids.erase(raids.begin() + (i - 1));
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------
// Patrols
// ---------------------------------------------------------------------------------------------------------------------------------
void RaidManager::tickPatrols()
{
	if (--patrolCountdown > 0) return;
	patrolCountdown = 12000 + level->random->nextInt(1200);

	if (!level->isDay() || level->getGameTime() < PATROL_FIRST_DAY_TICKS) return;
	if (level->players.empty()) return;
	if (level->random->nextInt(5) != 0) return;

	shared_ptr<Player> player = level->players[level->random->nextInt(static_cast<int>(level->players.size()))];
	if (player == nullptr || player->removed) return;
	spawnPatrol(player);
}

void RaidManager::spawnPatrol(shared_ptr<Player> player)
{
	int x = Mth::floor(player->x) + (24 + level->random->nextInt(24)) * (level->random->nextInt(2) == 0 ? 1 : -1);
	int z = Mth::floor(player->z) + (24 + level->random->nextInt(24)) * (level->random->nextInt(2) == 0 ? 1 : -1);
	if (!level->hasChunkAt(x, 64, z)) return;

	int y = level->getTopSolidBlock(x, z);
	if (level->villages->getClosestVillage(x, y, z, 16) != nullptr) return;	// patrols keep away from villages

	int members = 1 + level->difficulty + level->random->nextInt(3);	// the captain and the rest
	shared_ptr<Pillager> leader;
	for (int i = 0; i < members; i++)
	{
		int mx = x + level->random->nextInt(5) - level->random->nextInt(5);
		int mz = z + level->random->nextInt(5) - level->random->nextInt(5);
		if (!level->hasChunkAt(mx, 64, mz)) continue;
		int my = level->getTopSolidBlock(mx, mz);
		if (!MobSpawner::isSpawnPositionOk(MobCategory::monster, level, mx, my, mz)) continue;

		shared_ptr<Pillager> pillager = std::make_shared<Pillager>(level);
		pillager->moveTo(mx + 0.5, my, mz + 0.5, level->random->nextFloat() * 360, 0);
		pillager->finalizeMobSpawn(nullptr);

		if (leader == nullptr)
		{
			leader = pillager;
			pillager->makeCaptain();
			double angle = level->random->nextDouble() * 2 * PI;
			pillager->startPatrol(nullptr, static_cast<int>(mx + cos(angle) * 160), static_cast<int>(mz + sin(angle) * 160));
		}
		else
		{
			pillager->startPatrol(leader, 0, 0);
		}
		level->addEntity(pillager);
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------
// Raids
// ---------------------------------------------------------------------------------------------------------------------------------

// Bad Omen on a player who walks into a village turns into Raid Omen (30 seconds, same level). When the Raid Omen runs out while the player is in a
// village, the raid starts (or, if one has not begun there yet, grows a level). Milk or death cancels it: that shows as the effect vanishing with a lot
// of time left. Checked once a second.
void RaidManager::detectBadOmen()
{
	for (size_t p = 0; p < level->players.size(); p++)
	{
		shared_ptr<Player> player = level->players[p];
		if (player == nullptr || player->removed) continue;

		int px = Mth::floor(player->x), py = Mth::floor(player->y), pz = Mth::floor(player->z);
		shared_ptr<Village> village = level->villages->getClosestVillage(px, py, pz, 16);
		bool inVillage = village != nullptr && village->isInside(px, py, pz);

		if (inVillage && player->hasEffect(MobEffect::badOmen))
		{
			int amplifier = player->getEffect(MobEffect::badOmen)->getAmplifier();
			player->removeEffect(MobEffect::badOmen->id);
			player->addEffect(new MobEffectInstance(MobEffect::raidOmen->id, 600, amplifier));
		}

		// keep track of the Raid Omen
		int index = -1;
		for (size_t i = 0; i < omens.size(); i++)
		{
			if (omens[i].entityId == player->entityId) { index = static_cast<int>(i); break; }
		}

		MobEffectInstance *omen = player->hasEffect(MobEffect::raidOmen) ? player->getEffect(MobEffect::raidOmen) : nullptr;
		if (omen != nullptr)
		{
			if (index < 0)
			{
				OmenWatch w;
				w.entityId = player->entityId;
				index = static_cast<int>(omens.size());
				omens.push_back(w);
			}
			omens[index].amplifier = omen->getAmplifier();
			omens[index].duration = omen->getDuration();
		}
		else if (index >= 0)
		{
			bool ranOut = omens[index].duration <= 40;
			int amplifier = omens[index].amplifier;
			omens.erase(omens.begin() + index);

			if (ranOut && inVillage) startOrRaiseRaid(village, amplifier + 1);
		}
	}

	// players who have left the level
	for (size_t i = omens.size(); i > 0; i--)
	{
		bool here = false;
		for (size_t p = 0; p < level->players.size() && !here; p++)
		{
			if (level->players[p] != nullptr && level->players[p]->entityId == omens[i - 1].entityId) here = true;
		}
		if (!here) omens.erase(omens.begin() + (i - 1));
	}
}

void RaidManager::startOrRaiseRaid(shared_ptr<Village> village, int badOmenLevel)
{
	Pos *center = village->getCenter();
	for (size_t i = 0; i < raids.size(); i++)
	{
		double dx = raids[i].centerX - center->x, dz = raids[i].centerZ - center->z;
		if (dx * dx + dz * dz < static_cast<double>(raids[i].radius + 32) * (raids[i].radius + 32))
		{
			if (raids[i].wavesSpawned == 0 && raids[i].badOmenLevel < 5)
			{
				raids[i].badOmenLevel += badOmenLevel;
				if (raids[i].badOmenLevel > 5) raids[i].badOmenLevel = 5;
			}
			return;
		}
	}
	startRaid(center->x, center->y, center->z, village->getRadius(), badOmenLevel);
}

void RaidManager::startRaid(int cx, int cy, int cz, int radius, int badOmenLevel)
{
	Raid raid;
	raid.centerX = cx;
	raid.centerY = cy;
	raid.centerZ = cz;
	raid.radius = radius;
	raid.badOmenLevel = badOmenLevel;
	raid.totalWaves = (level->difficulty <= 1) ? 3 : (level->difficulty == 2 ? 5 : 7);
	if (badOmenLevel > 1) raid.totalWaves += 1;		// the bonus wave
	raid.wavesSpawned = 0;
	raid.countdown = FIRST_WAVE_DELAY;
	raid.quietTicks = 0;
	raid.waveOut = false;
	raid.finished = false;
	raids.push_back(raid);

	tell(raids.back(), L"A raid is coming!");
}

void RaidManager::tell(Raid &raid, const wstring &message)
{
	for (size_t i = 0; i < level->players.size(); i++)
	{
		shared_ptr<Player> player = level->players[i];
		if (player == nullptr || player->removed) continue;
		double dx = player->x - raid.centerX, dz = player->z - raid.centerZ;
		if (dx * dx + dz * dz < 128.0 * 128.0) player->sendMessage(message);
	}
}

int RaidManager::aliveRaiders(Raid &raid)
{
	int alive = 0;
	for (size_t i = raid.raiders.size(); i > 0; i--)
	{
		shared_ptr<Mob> m = raid.raiders[i - 1].lock();
		if (m == nullptr || m->removed || !m->isAlive()) raid.raiders.erase(raid.raiders.begin() + (i - 1));
		else alive++;
	}
	return alive;
}

void RaidManager::tickRaid(Raid &raid)
{
	if (raid.finished) return;

	// nobody near for two minutes: the raid is called off
	bool playerNearby = false;
	for (size_t i = 0; i < level->players.size(); i++)
	{
		shared_ptr<Player> player = level->players[i];
		if (player == nullptr || player->removed) continue;
		double dx = player->x - raid.centerX, dz = player->z - raid.centerZ;
		if (dx * dx + dz * dz < 112.0 * 112.0) { playerNearby = true; break; }
	}
	if (playerNearby) raid.quietTicks = 0;
	else if (++raid.quietTicks > 2400) { raid.finished = true; return; }
	if (!playerNearby) return;	// the chunks around the village are not necessarily loaded

	int alive = aliveRaiders(raid);

	// the village is gone
	shared_ptr<Village> village = level->villages->getClosestVillage(raid.centerX, raid.centerY, raid.centerZ, 32);
	if (raid.wavesSpawned > 0 && (village == nullptr || village->getPopulationSize() == 0))
	{
		tell(raid, L"The village has fallen.");
		raid.finished = true;
		return;
	}

	if (raid.waveOut)
	{
		if (alive > 0) return;

		raid.waveOut = false;
		if (raid.wavesSpawned >= raid.totalWaves)
		{
			tell(raid, L"Victory! The raid has been defeated.");
			raid.finished = true;
			return;
		}
		tell(raid, L"Wave cleared.");
		raid.countdown = NEXT_WAVE_DELAY;
		return;
	}

	if (--raid.countdown <= 0) spawnWave(raid);
}

// Somewhere outside the village, 14-32 blocks beyond its edge, on solid ground.
bool RaidManager::findRaiderSpawn(Raid &raid, int *x, int *y, int *z)
{
	for (int attempt = 0; attempt < 24; attempt++)
	{
		double angle = level->random->nextDouble() * 2 * PI;
		double dist = raid.radius + 14 + level->random->nextInt(19);
		if (dist < 40) dist = 40;
		int sx = static_cast<int>(raid.centerX + cos(angle) * dist);
		int sz = static_cast<int>(raid.centerZ + sin(angle) * dist);
		if (!level->hasChunkAt(sx, 64, sz)) continue;

		int sy = level->getTopSolidBlock(sx, sz);
		if (!MobSpawner::isSpawnPositionOk(MobCategory::monster, level, sx, sy, sz)) continue;

		*x = sx; *y = sy; *z = sz;
		return true;
	}
	return false;
}

void RaidManager::spawnWave(Raid &raid)
{
	raid.wavesSpawned++;
	int w = raid.wavesSpawned;
	if (w > 7) w = 7;	// the bonus wave repeats the last row

	int counts[5] = { WAVE_PILLAGERS[w], WAVE_VINDICATORS[w], WAVE_EVOKERS[w], WAVE_WITCHES[w], WAVE_RAVAGERS[w] };
	// a higher Bad Omen brings more raiders: one more Pillager and one more Vindicator per level above I (up to four levels)
	int extra = raid.badOmenLevel - 1;
	if (extra > 4) extra = 4;
	if (extra > 0 && w >= 2) { counts[0] += extra; counts[1] += extra; }
	bool captainMade = false;

	for (int type = 0; type < 5; type++)
	{
		for (int n = 0; n < counts[type]; n++)
		{
			int x, y, z;
			if (!findRaiderSpawn(raid, &x, &y, &z)) continue;

			shared_ptr<Mob> mob;
			switch (type)
			{
			case 0: mob = std::make_shared<Pillager>(level); break;
			case 1: mob = std::make_shared<Vindicator>(level); break;
			case 2: mob = std::make_shared<Evoker>(level); break;
			case 3: mob = std::make_shared<Witch>(level); break;
			default: mob = std::make_shared<Ravager>(level); break;
			}
			mob->moveTo(x + 0.5, y, z + 0.5, level->random->nextFloat() * 360, 0);
			mob->finalizeMobSpawn(nullptr);

			// from level III on the Pillagers carry enchanted crossbows: Piercing, and from level IV Quick Charge as well
			if (type == 0 && raid.badOmenLevel >= 3 && mob->getCarriedItem() != nullptr)
			{
				mob->getCarriedItem()->enchant(Enchantment::piercing, 1);
				if (raid.badOmenLevel >= 4) mob->getCarriedItem()->enchant(Enchantment::quickCharge, 1);
			}

			RaidMember *member = dynamic_cast<RaidMember *>(mob.get());
			if (member != nullptr) member->joinRaid(raid.centerX, raid.centerY, raid.centerZ);

			// one illager of each wave is its captain
			Illager *illager = dynamic_cast<Illager *>(mob.get());
			if (illager != nullptr && !captainMade)
			{
				illager->makeCaptain();
				captainMade = true;
			}

			level->addEntity(mob);
			raid.raiders.push_back(mob);
		}
	}

	raid.waveOut = true;

	wchar_t text[64];
	swprintf(text, 64, L"Raid - wave %d of %d", raid.wavesSpawned, raid.totalWaves);
	tell(raid, text);
}
