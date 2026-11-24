#pragma once
#include "TileEntity.h"

class Entity;
class LevelChunk;

class TheEndGatewayTileEntity : public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_THEENDGATEWAYTILEENTITY; }
	static TileEntity *create() { return new TheEndGatewayTileEntity(); }

	static const int SPAWN_TIME = 200;
	static const int COOLDOWN_TIME = 40;
	static const int ATTENTION_INTERVAL = 2400;
	static const int EVENT_COOLDOWN = 1;

private:
	int64_t age;
	int teleportCooldown;
	bool hasExit;
	int exitX, exitY, exitZ;
	bool exactTeleport;

public:
	TheEndGatewayTileEntity();

	virtual void save(CompoundTag *tag);
	virtual void load(CompoundTag *tag);
	virtual void tick();
	virtual double getViewDistance();
	virtual shared_ptr<Packet> getUpdatePacket();
	virtual bool triggerEvent(int b0, int b1);
	virtual shared_ptr<TileEntity> clone();

	bool isSpawning();
	bool isCoolingDown();
	float getSpawnPercent(float a);
	float getCooldownPercent(float a);
	int getParticleAmount();
	bool shouldRenderFace(int face);

	void triggerCooldown();
	void teleportEntity(shared_ptr<Entity> entity);
	void setExitPosition(int x, int y, int z);

private:
	void findExitPortal();
	void createExitPortal(int x, int y, int z);
	bool findValidSpawnInChunk(LevelChunk *chunk, int &outX, int &outY, int &outZ);
	static void findTallestBlock(Level *level, int x, int y, int z, int radius, bool allowBedrock, int &outX, int &outY, int &outZ);
};
