#pragma once

class Level;
class Mob;
class Player;
class Village;

// Set to 1 to get raid progress messages in chat again while testing. In normal play the raid bar tells the player everything.
#define RAID_DEBUG_CHAT 0

// neoLegacy - patrols and raids, one manager per level (ticked from ServerLevel next to the village siege).
//
// PATROLS: every 12000-13200 ticks, by day, from the fifth day on, there is a one in five chance (for a random player) of a patrol spawning
// 24-48 blocks from that player and away from villages: a Pillager captain wearing the Ominous Banner and a few more Pillagers who follow it.
// Killing the captain gives Bad Omen (Illager::die).
//
// RAIDS (as in modern Minecraft): Bad Omen comes from an Ominous Bottle (dropped by raid captains). A player with Bad Omen who walks into a village
// has it turned into Raid Omen, a 30 second countdown; when the countdown ends while the player is in a village, a raid starts there. Drinking milk
// (or dying) cancels the countdown. After a short wait, waves of raiders
// (Pillagers, Vindicators, Evokers, Witches, Ravagers: Java's wave table) spawn outside the village and head for its centre, attacking
// anything on the way. The next wave comes when the last one is dead; the raid has 3 / 5 / 7 waves on Easy / Normal / Hard (one more when
// the Bad Omen was level II or higher). It is lost if the village is wiped out. Not saved: a raid does not survive leaving the world.
//
// BIOMES: the raid's casters depend on where the village is. In snowy and icy biomes some of the Evokers are Iceologers and the waves that have a
// caster slot (3, 5, 7 and the bonus wave) usually send an Iceologer; everywhere else that slot is an Illusioner (always in hot biomes, sometimes in
// temperate ones). Provokers (they buff the other illagers) join the waves from the first one on, as in the Illager Invasion mod.
// Patrols get the same variety: a follower is now and then a Provoker or the biome's caster.
//
// VICTORY: two seconds after the last raider of the last wave falls, the raid is won. Every player who killed a raider during it becomes a Hero of
// the Village (40 minutes, level = Bad Omen level - 1, as in Java), the bar shows Victory for 30 seconds, and the villagers set off fireworks.
// DEFEAT: when the village falls, the surviving illagers cheer.
class RaidManager
{
private:
	struct Raid
	{
		int centerX, centerY, centerZ, radius;
		int badOmenLevel;
		int totalWaves;
		int wavesSpawned;
		int countdown;			// ticks until the next wave, when none is out
		int quietTicks;			// how long no player has been near
		bool waveOut;
		bool finished;
		int waveSize;			// how many raiders the wave in progress started with (the bar is what is left of them)
		int endState;			// 0 under way, 1 victory, 2 defeat: shown on the bar for 30 seconds after it ends (Java)
		int endTicks;
		bool winning;			// the last wave is dead; waiting out the short pause before the raid counts as won
		int postRaidTicks;
		int biomeKind;			// 0 temperate, 1 snowy / icy, 2 hot - decides which casters join the raid
		vector<int> heroes;		// entity ids of the players who killed a raider
		vector<weak_ptr<Mob> > raiders;
	};

	// a player's Raid Omen, as seen one second ago: when it vanishes with almost no time left it ran out (and the raid starts), otherwise it was cancelled
	struct OmenWatch
	{
		int entityId;
		int amplifier;
		int duration;
	};

	Level *level;
	int patrolCountdown;
	vector<Raid> raids;
	vector<OmenWatch> omens;
	vector<int> warnedPeaceful;		// players already told that raids do not happen on Peaceful

	void tickPatrols();
	void spawnPatrol(shared_ptr<Player> player);
	void detectBadOmen();
	void startRaid(int cx, int cy, int cz, int radius, int badOmenLevel);
	void startOrRaiseRaid(shared_ptr<Village> village, int badOmenLevel);
	void tickRaid(Raid &raid);
	void spawnWave(Raid &raid);
	bool findRaiderSpawn(Raid &raid, int *x, int *y, int *z);
	int aliveRaiders(Raid &raid);
	void tell(Raid &raid, const wstring &message);
	void updateRaidBars();
	void endRaid(Raid &raid, int state);
	void addHero(Raid &raid, int entityId);
	void grantHeroes(Raid &raid);
	void releaseRaiders(Raid &raid, bool cheer);
	void celebrateVillagers(Raid &raid);

public:
	RaidManager(Level *level);
	void tick();
};
