#pragma once

class Level;
class Mob;
class Player;
class Village;

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
// Not in: the raid bar (progress is told in chat), Hero of the Village, raiders celebrating.
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

public:
	RaidManager(Level *level);
	void tick();
};
