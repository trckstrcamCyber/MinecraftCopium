#pragma once

class CompoundTag;
class DataInputStream;
class DataOutputStream;

// neoLegacy Arcade Machines - the progress that is kept for a player in a world.
//
//  * Cookie Miner: cookie count, owned upgrades, cookies per second, click value and the golden apple timers
//  * Flappy Craft / Mine Snake: the high score only
//
// One copy lives on every Player (saved inside the player's data in the world save, so it is per player AND per world).
// The client's ArcadeGameSession plays with its own copy; the two are kept in sync through CustomPayloadPacket::ARCADE_SAVE_PACKET
// (client -> server) and CustomPayloadPacket::ARCADE_SYNC_PACKET (server -> client).
struct ArcadeSaveData
{
	static const int UPGRADE_COUNT = 4;		// keep in step with ArcadeGameSession's shop

	double cookies;
	int upgrades[UPGRADE_COUNT];
	double cookiesPerSecond;				// informational: recalculated from the upgrades on load
	int clickValue;							// informational: cookies per click when no golden boost is running
	int goldenCooldownTicks;
	int goldenBoostTicks;
	long long flappyHighScore;
	long long snakeHighScore;

	ArcadeSaveData();
	void reset();
	void sanitize();						// clamp everything into a sane range (never trust a packet or a hand-edited save)

	void save(CompoundTag *tag) const;		// writes a "CopiumArcade" child compound
	void load(CompoundTag *tag);			// reads it back (missing = defaults)

	void write(DataOutputStream *dos) const;
	void read(DataInputStream *dis);
};
