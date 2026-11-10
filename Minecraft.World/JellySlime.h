#pragma once
using namespace std;

#include "Slime.h"

class Player;
class Level;

// ---------------------------------------------------------------------------------------------------------------------------
// neoLegacy - Jelly Slime. A rare pink slime in the style of Terraria's Pinky.
//
//   * Spawns by day on the surface of warm biomes: Plains (and Sunflower Plains), Savanna, Desert and Mesa.
//   * A lot tougher and meaner than a normal slime: 40 health and a 5 point bite, but it NEVER splits into smaller slimes.
//   * Wild ones are hostile (and vanish on Peaceful, like any slime). Feed one SUGAR (1 in 3 chance per sugar) to tame it: a tame
//     Jelly Slime is docile and never attacks anything. It is more cat than dog - it follows its owner around but does not fight
//     mobs for them - and it never despawns.
//   * Drops 1-3 Jelly Balls when it dies (looting adds more). Jelly Balls are food, and 4 of them make a Jelly Block.
// ---------------------------------------------------------------------------------------------------------------------------
class JellySlime : public Slime
{
public:
	eINSTANCEOF GetType() { return eTYPE_JELLYSLIME; }
	static Entity *create(Level *level) { return new JellySlime(level); }

	static const int DATA_OWNER_UUID = 17;			// synched string: UUID of the owner, empty while wild
	static const int JELLY_SIZE = 2;				// slime "size": a 1.2 block cube, Pinky-sized
	static constexpr float JELLY_MAX_HEALTH = 40.0f;	// a size-2 slime has 4
	static const int JELLY_ATTACK_DAMAGE = 5;		// a big (size-4) slime hits for 4
	static constexpr double FOLLOW_MIN_DIST = 4.0;	// a tame one starts following when its owner is farther than this
	static constexpr double FOLLOW_MAX_DIST = 24.0;	// ...and gives up beyond this

private:
	bool sizeInitialised;

public:
	JellySlime(Level *level);

	using Slime::setSize;
	virtual void setSize(int size);

	// ---- taming ----
	bool isTame();
	wstring getOwnerUUID();
	void setOwnerUUID(const wstring &uuid);
	void tame(const wstring &ownerUUID);

	virtual bool mobInteract(shared_ptr<Player> player);
	virtual void handleEntityEvent(byte id);
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

	virtual void tick();
	virtual void remove();				// no splitting
	virtual bool canSpawn();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

protected:
	virtual ePARTICLE_TYPE getParticleName();		// pink jelly blobs instead of slime-green ones
	virtual void serverAiStep();
	virtual bool isDealsDamage();
	virtual int getAttackDamage();

private:
	void spawnTamingParticles(bool success);
};
