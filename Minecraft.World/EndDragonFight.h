#pragma once

class Level;
class EnderCrystal;
class EnderDragon;

class EndDragonFight
{
public:
	static const int GATEWAY_X = 65;
	static const int GATEWAY_Y = 75;
	static const int GATEWAY_Z = 0;

private:
	enum RespawnStage
	{
		eRespawn_None,
		eRespawn_Start,
		eRespawn_PreparingPillars,
		eRespawn_SummoningPillars,
		eRespawn_SummoningDragon,
	};

	Level *level;
	RespawnStage respawnStage;
	int respawnTime;
	vector<shared_ptr<EnderCrystal> > respawnCrystals;

public:
	EndDragonFight(Level *level);

	static EndDragonFight *get(Level *level);

	void tick();
	void onDragonKilled();
	void onCrystalDestroyed(shared_ptr<EnderCrystal> crystal);
	void tryRespawn();

	static void spawnExitPortal(Level *level, bool active, bool placeEgg);

private:
	int getPortalY();
	void findPortalCrystals(vector<shared_ptr<EnderCrystal> > &result);
	void setRespawnStage(RespawnStage stage);
	void tickRespawn();
	void rebuildSpike(int index);
	void resetSpikeCrystals();
	void pointCrystalsAt(int x, int y, int z);
	void growl();
	void createNewDragon();
};
