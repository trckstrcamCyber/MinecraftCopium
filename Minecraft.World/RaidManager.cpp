#include "stdafx.h"
#include "Random.h"
#include "Mth.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "net.minecraft.world.entity.npc.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.item.h"
#include "com.mojang.nbt.h"
#include "FireworksItem.h"
#include "FireworksRocketEntity.h"
#include "SoundTypes.h"
#include "AABB.h"
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
// neoLegacy - from the Illager Invasion mod's default wave table: Provokers, and the slot its Illusioner uses (here: the biome's caster)
static const int WAVE_PROVOKERS[8]   = { 0, 1, 1, 0, 2, 1, 3, 2 };
static const int WAVE_CASTERS[8]     = { 0, 0, 0, 1, 0, 1, 0, 1 };
static const int WAVE_SORCERERS[8]   = { 0, 0, 0, 0, 0, 1, 1, 1 };
static const int WAVE_ALCHEMISTS[8]  = { 0, 0, 0, 1, 2, 1, 2, 2 };
static const int WAVE_ARCHIVISTS[8]  = { 0, 1, 0, 1, 2, 1, 2, 3 };
static const int WAVE_NECROMANCERS[8] = { 0, 0, 0, 0, 1, 1, 1, 1 };
static const int WAVE_BASHERS[8]     = { 0, 1, 2, 1, 2, 3, 2, 3 };
static const int WAVE_MARAUDERS[8]   = { 0, 1, 1, 1, 2, 2, 3, 3 };

// chances (percent) that a caster slot is actually filled, and that an Evoker is an Iceologer in the cold
static const int CASTER_CHANCE_COLD = 65;
static const int CASTER_CHANCE_HOT = 70;
static const int CASTER_CHANCE_TEMPERATE = 40;
static const int COLD_EVOKER_SWAP_CHANCE = 30;

static const int HERO_DURATION = 48000;		// 40 minutes, as in Java
static const int VICTORY_PAUSE = 40;			// ticks between the last raider falling and the raid being won (Java: post raid ticks)
static const int END_BAR_TICKS = 600;			// Victory / Defeat stays on the bar for 30 seconds (Java: celebration ticks)

// 1 = snowy or icy (Iceologer country), 2 = hot (Illusioner country), 0 = anything in between
static int biomeKindAt(Level *level, int x, int z)
{
	Biome *b = level->getBiome(x, z);
	if (b == nullptr) return 0;
	if (b->hasSnow()) return 1;
	return b->getTemperature() >= 0.95f ? 2 : 0;
}

// a rocket for the villagers to launch when the raid is won: one coloured burst, short flight (Java: CelebrateVillagersSurvivedRaid)
static shared_ptr<ItemInstance> makeCelebrationFirework(Random *r)
{
	static const int COLORS[8] = { 0xB02E26, 0xF9801D, 0xFED83D, 0x80C71F, 0x3AB3DA, 0x3C44AA, 0x8932B8, 0xF38BAA };
	shared_ptr<ItemInstance> rocket = std::make_shared<ItemInstance>(Item::fireworks);
	CompoundTag *itemTag = new CompoundTag();
	CompoundTag *fireTag = new CompoundTag(FireworksItem::TAG_FIREWORKS);
	ListTag<CompoundTag> *expTags = new ListTag<CompoundTag>(FireworksItem::TAG_EXPLOSIONS);
	CompoundTag *exp = new CompoundTag();
	intArray colors(static_cast<unsigned int>(1));
	colors[0] = COLORS[r->nextInt(8)];
	exp->putIntArray(FireworksItem::TAG_E_COLORS, colors);
	exp->putByte(FireworksItem::TAG_E_TYPE, FireworksItem::TYPE_BURST);
	expTags->add(exp);
	fireTag->put(FireworksItem::TAG_EXPLOSIONS, expTags);
	fireTag->putByte(FireworksItem::TAG_FLIGHT, static_cast<byte>(1 + r->nextInt(2)));
	itemTag->put(FireworksItem::TAG_FIREWORKS, fireTag);
	rocket->setTag(itemTag);
	return rocket;
}

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

	if (level->getGameTime() % 20 == 0) detectBadOmen();	// on Peaceful this only tells the player why nothing happens

	if (level->difficulty == 0)
	{
		raids.clear();
		if (level->getGameTime() % 10 == 0) updateRaidBars();
		return;
	}

	tickPatrols();

	for (size_t i = 0; i < raids.size(); i++) tickRaid(raids[i]);
	for (size_t i = raids.size(); i > 0; i--)
	{
		if (raids[i - 1].finished && raids[i - 1].endTicks <= 0) raids.erase(raids.begin() + (i - 1));
	}

	if (level->getGameTime() % 10 == 0) updateRaidBars();
}

// A raid stops: the bar stays for 30 seconds showing how it ended (state 1 victory, 2 defeat, 0 simply called off).
// Whoever is still alive stops heading for the village; after a defeat they cheer.
void RaidManager::endRaid(Raid &raid, int state)
{
	raid.finished = true;
	raid.endState = state;
	raid.endTicks = (state != 0) ? END_BAR_TICKS : 0;
	raid.winning = false;
	releaseRaiders(raid, state == 2);
}

void RaidManager::releaseRaiders(Raid &raid, bool cheer)
{
	for (size_t i = 0; i < raid.raiders.size(); i++)
	{
		shared_ptr<Mob> m = raid.raiders[i].lock();
		if (m == nullptr || m->removed || !m->isAlive()) continue;
		RaidMember *member = dynamic_cast<RaidMember *>(m.get());
		if (member != nullptr) member->leaveRaid();
		Illager *illager = dynamic_cast<Illager *>(m.get());
		if (illager != nullptr && cheer) illager->setCelebrating(true);
	}
}

// Every player who killed a raider becomes a Hero of the Village when the raid is won (Java: Raid::addHeroOfTheVillage).
void RaidManager::addHero(Raid &raid, int entityId)
{
	for (size_t i = 0; i < raid.heroes.size(); i++) if (raid.heroes[i] == entityId) return;
	raid.heroes.push_back(entityId);
}

// 40 minutes of Hero of the Village; the level is the Bad Omen level - 1 (Java). Cheaper trades and villager gifts come from the effect itself.
void RaidManager::grantHeroes(Raid &raid)
{
	int amplifier = raid.badOmenLevel - 1;
	if (amplifier < 0) amplifier = 0;
	if (amplifier > 4) amplifier = 4;
	for (size_t h = 0; h < raid.heroes.size(); h++)
	{
		for (size_t p = 0; p < level->players.size(); p++)
		{
			shared_ptr<Player> player = level->players[p];
			if (player == nullptr || player->removed || !player->isAlive() || player->entityId != raid.heroes[h]) continue;
			player->addEffect(new MobEffectInstance(MobEffect::heroOfTheVillage->id, HERO_DURATION, amplifier));
		}
	}
}

// After a victory the villagers cheer and, when they stand under the open sky, launch fireworks (Java: CelebrateVillagersSurvivedRaid).
// Checked once a second, so the odds are Java's per-tick odds (1 in 100 cheer, 1 in 200 firework) scaled up.
void RaidManager::celebrateVillagers(Raid &raid)
{
	if (level->getGameTime() % 20 != 0) return;
	double r = raid.radius + 32;
	AABB *box = AABB::newTemp(raid.centerX - r, raid.centerY - 32.0, raid.centerZ - r, raid.centerX + r, raid.centerY + 32.0, raid.centerZ + r);
	vector<shared_ptr<Entity> > *found = level->getEntitiesOfClass(typeid(Villager), box);
	if (found == nullptr) return;
	for (size_t i = 0; i < found->size(); i++)
	{
		shared_ptr<Villager> villager = dynamic_pointer_cast<Villager>(found->at(i));
		if (villager == nullptr || villager->removed || !villager->isAlive()) continue;
		if (level->random->nextInt(5) == 0) villager->playSound(eSoundType_MOB_VILLAGER_YES, 1.0f, 1.0f);
		int vx = Mth::floor(villager->x), vz = Mth::floor(villager->z);
		double eyeY = villager->y + villager->getHeadHeight();
		if (level->random->nextInt(10) == 0 && level->canSeeSky(vx, Mth::floor(eyeY), vz))
		{
			shared_ptr<FireworksRocketEntity> rocket = std::make_shared<FireworksRocketEntity>(level, villager->x, eyeY, villager->z, makeCelebrationFirework(level->random));
			level->addEntity(rocket);
		}
	}
	delete found;
}

// The raid bar: each player near a raid gets the nearest one's progress through the Player's synched data, and the HUD draws it with the boss bar.
// It starts full when a wave appears and empties as its raiders fall.
void RaidManager::updateRaidBars()
{
	for (size_t p = 0; p < level->players.size(); p++)
	{
		shared_ptr<Player> player = level->players[p];
		if (player == nullptr || player->removed) continue;

		const Raid *best = nullptr;
		double bestDist = 128.0 * 128.0;
		for (size_t i = 0; i < raids.size(); i++)
		{
			double dx = player->x - raids[i].centerX, dz = player->z - raids[i].centerZ;
			double d = dx * dx + dz * dz;
			if (d < bestDist) { bestDist = d; best = &raids[i]; }
		}

		int packed = 0;
		if (best != nullptr)
		{
			int percent = 100;
			if (best->endState == 1) percent = 100;
			else if (best->endState == 2) percent = 0;
			else if (best->waveOut && best->waveSize > 0)
			{
				int alive = 0;
				for (size_t r = 0; r < best->raiders.size(); r++)
				{
					shared_ptr<Mob> m = best->raiders[r].lock();
					if (m != nullptr && !m->removed && m->isAlive()) alive++;
				}
				percent = alive * 100 / best->waveSize;
				if (percent > 100) percent = 100;
			}

			int wave = best->wavesSpawned > 0 ? best->wavesSpawned : 1;
			if (wave > 15) wave = 15;
			int total = best->totalWaves > 15 ? 15 : best->totalWaves;
			packed = 1 | (percent << 1) | (wave << 8) | (total << 12) | (best->endState << 16);
		}
		if (player->getEntityData()->getInteger(Player::DATA_RAID_BAR_ID) != packed)
		{
			player->getEntityData()->set(Player::DATA_RAID_BAR_ID, packed);
		}
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
	shared_ptr<Illager> leader;
	for (int i = 0; i < members; i++)
	{
		int mx = x + level->random->nextInt(5) - level->random->nextInt(5);
		int mz = z + level->random->nextInt(5) - level->random->nextInt(5);
		if (!level->hasChunkAt(mx, 64, mz)) continue;
		int my = level->getTopSolidBlock(mx, mz);
		if (!MobSpawner::isSpawnPositionOk(MobCategory::monster, level, mx, my, mz)) continue;

		// the captain is always a Pillager; the others are Pillagers too, apart from now and then a Provoker or the biome's caster
		shared_ptr<Illager> pillager;
		if (leader == nullptr) pillager = std::make_shared<Pillager>(level);
		else
		{
			int roll = level->random->nextInt(100);
			int kind = biomeKindAt(level, mx, mz);
			if (roll < 12) pillager = std::make_shared<Provoker>(level);
			else if (roll < 24 && kind == 1) pillager = std::make_shared<Iceologer>(level);
			else if (roll < 24 && (kind == 2 || level->random->nextInt(2) == 0)) pillager = std::make_shared<Illusioner>(level);
			else pillager = std::make_shared<Pillager>(level);
		}
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

		bool hasOmen = player->hasEffect(MobEffect::badOmen) || player->hasEffect(MobEffect::raidOmen);

		// raids do not happen on Peaceful (with RAID_DEBUG_CHAT on, the player is told once instead of the Bad Omen silently doing nothing)
		if (level->difficulty == 0)
		{
#if RAID_DEBUG_CHAT
			bool warned = false;
			for (size_t i = 0; i < warnedPeaceful.size(); i++) if (warnedPeaceful[i] == player->entityId) warned = true;
			if (inVillage && hasOmen && !warned)
			{
				warnedPeaceful.push_back(player->entityId);
				player->sendMessage(L"Raids do not happen on Peaceful. Raise the difficulty to start one.");
			}
#endif
			continue;
		}

		if (inVillage && player->hasEffect(MobEffect::badOmen))
		{
			int amplifier = player->getEffect(MobEffect::badOmen)->getAmplifier();
			player->removeEffect(MobEffect::badOmen->id);
			player->addEffect(new MobEffectInstance(MobEffect::raidOmen->id, 600, amplifier));
#if RAID_DEBUG_CHAT
			player->sendMessage(L"Raid Omen: a raid begins in 30 seconds.");
#endif
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
		if (raids[i].finished) continue;
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
	raid.waveSize = 0;
	raid.endState = 0;
	raid.endTicks = 0;
	raid.winning = false;
	raid.postRaidTicks = 0;
	raid.biomeKind = biomeKindAt(level, cx, cz);
	raids.push_back(raid);

	tell(raids.back(), L"A raid is coming!");
}

// Progress messages in chat were only ever for testing; they stay out unless RAID_DEBUG_CHAT is set (the raid bar shows it all).
void RaidManager::tell(Raid &raid, const wstring &message)
{
#if RAID_DEBUG_CHAT
	for (size_t i = 0; i < level->players.size(); i++)
	{
		shared_ptr<Player> player = level->players[i];
		if (player == nullptr || player->removed) continue;
		double dx = player->x - raid.centerX, dz = player->z - raid.centerZ;
		if (dx * dx + dz * dz < 128.0 * 128.0) player->sendMessage(message);
	}
#endif
}

int RaidManager::aliveRaiders(Raid &raid)
{
	int alive = 0;
	for (size_t i = raid.raiders.size(); i > 0; i--)
	{
		shared_ptr<Mob> m = raid.raiders[i - 1].lock();
		if (m == nullptr || m->removed || !m->isAlive())
		{
			// killed (not just despawned) by a player: that player is a Hero of the Village if the raid is won (Java: Raider::die)
			if (m != nullptr && m->getHealth() <= 0)
			{
				shared_ptr<LivingEntity> killer = m->getLastHurtByMob();
				if (killer != nullptr && killer->instanceof(eTYPE_PLAYER)) addHero(raid, killer->entityId);
			}
			raid.raiders.erase(raid.raiders.begin() + (i - 1));
		}
		else alive++;
	}
	return alive;
}

void RaidManager::tickRaid(Raid &raid)
{
	if (raid.finished)
	{
		if (raid.endTicks > 0)
		{
			raid.endTicks--;
			if (raid.endState == 1) celebrateVillagers(raid);
		}
		return;
	}

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
	else if (++raid.quietTicks > 2400) { endRaid(raid, 0); return; }
	if (!playerNearby) return;	// the chunks around the village are not necessarily loaded

	int alive = aliveRaiders(raid);

	// the village is gone
	shared_ptr<Village> village = level->villages->getClosestVillage(raid.centerX, raid.centerY, raid.centerZ, 32);
	if (raid.wavesSpawned > 0 && (village == nullptr || village->getPopulationSize() == 0))
	{
		tell(raid, L"The village has fallen.");
		endRaid(raid, 2);
		return;
	}

	if (raid.waveOut)
	{
		if (alive > 0) return;

		raid.waveOut = false;
		if (raid.wavesSpawned >= raid.totalWaves)
		{
			raid.winning = true;		// like Java, a short pause before the raid counts as won
			raid.postRaidTicks = 0;
			return;
		}
		tell(raid, L"Wave cleared.");
		raid.countdown = NEXT_WAVE_DELAY;
		return;
	}

	if (raid.winning)
	{
		if (++raid.postRaidTicks >= VICTORY_PAUSE)
		{
			grantHeroes(raid);
			tell(raid, L"Victory! The raid has been defeated.");
			endRaid(raid, 1);
		}
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

	const int TYPES = 13;	// 0 Pillager, 1 Vindicator, 2 Evoker, 3 Witch, 4 Ravager, 5 Provoker, 6 the biome's caster, 7 Sorcerer, 8 Alchemist, 9 Archivist, 10 Necromancer, 11 Basher, 12 Marauder
	int counts[13] = { WAVE_PILLAGERS[w], WAVE_VINDICATORS[w], WAVE_EVOKERS[w], WAVE_WITCHES[w], WAVE_RAVAGERS[w], WAVE_PROVOKERS[w], WAVE_CASTERS[w], WAVE_SORCERERS[w], WAVE_ALCHEMISTS[w], WAVE_ARCHIVISTS[w], WAVE_NECROMANCERS[w], WAVE_BASHERS[w], WAVE_MARAUDERS[w] };
	int casterChance = (raid.biomeKind == 1) ? CASTER_CHANCE_COLD : (raid.biomeKind == 2 ? CASTER_CHANCE_HOT : CASTER_CHANCE_TEMPERATE);
	// a higher Bad Omen brings more raiders: one more Pillager and one more Vindicator per level above I (up to four levels)
	int extra = raid.badOmenLevel - 1;
	if (extra > 4) extra = 4;
	if (extra > 0 && w >= 2) { counts[0] += extra; counts[1] += extra; }
	bool captainMade = false;

	for (int type = 0; type < TYPES; type++)
	{
		for (int n = 0; n < counts[type]; n++)
		{
			if (type == 6 && level->random->nextInt(100) >= casterChance) continue;	// the caster slot is only sometimes filled
			int x, y, z;
			if (!findRaiderSpawn(raid, &x, &y, &z)) continue;

			shared_ptr<Mob> mob;
			switch (type)
			{
			case 0: mob = std::make_shared<Pillager>(level); break;
			case 1: mob = std::make_shared<Vindicator>(level); break;
			case 2:
				// in the snow some of the Evokers are Iceologers instead
				if (raid.biomeKind == 1 && level->random->nextInt(100) < COLD_EVOKER_SWAP_CHANCE) mob = std::make_shared<Iceologer>(level);
				else mob = std::make_shared<Evoker>(level);
				break;
			case 3: mob = std::make_shared<Witch>(level); break;
			case 4: mob = std::make_shared<Ravager>(level); break;
			case 5: mob = std::make_shared<Provoker>(level); break;
			case 7: mob = std::make_shared<Sorcerer>(level); break;
			case 8: mob = std::make_shared<Alchemist>(level); break;
			case 9: mob = std::make_shared<Archivist>(level); break;
			case 10: mob = std::make_shared<Necromancer>(level); break;
			case 11: mob = std::make_shared<Basher>(level); break;
			case 12: mob = std::make_shared<Marauder>(level); break;
			default:
				if (raid.biomeKind == 1) mob = std::make_shared<Iceologer>(level);
				else mob = std::make_shared<Illusioner>(level);
				break;
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
	raid.waveSize = static_cast<int>(raid.raiders.size());

	wchar_t text[64];
	swprintf(text, 64, L"Raid - wave %d of %d", raid.wavesSpawned, raid.totalWaves);
	tell(raid, text);
}
