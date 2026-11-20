#pragma once

#include "Golem.h"

class CompoundTag;
class DamageSource;
class ItemInstance;
class Level;
class Player;

// neoLegacy - Copper Age (Phase 5). The copper golem, ported from Java
// 1.21.9 (copper-code.txt: CopperGolem :64, CopperGolemState :1488,
// CopperGolemAi :627). A passive golem: 12 HP, walks at 0.2, opens doors,
// panics at 1.5x when hurt, and idly spins its head every 200-240 ticks
// (spin sound at +10 ticks). It oxidizes through four states over time;
// once OXIDIZED it has a 0.58% chance per tick, while standing in air, to
// turn into a copper golem statue (Tile 2320), keeping its custom name.
// Drops 1-3 copper ingots (+looting) on death.
//
// Transport (Phase 5b) is NOT implemented: no mob/AI code in this tree ever
// touches a TileEntity inventory, so there is no precedent to build the
// chest-search / take-stack / deposit goals on. The CopperGolemState enum
// below (GETTING/DROPPING states) is defined now and stays unreached until
// 5b lands; the golem's live state is always IDLE. See copper-phase5-REPORT.md.
enum CopperGolemState
{
	COPPER_GOLEM_STATE_IDLE = 0,
	COPPER_GOLEM_STATE_GETTING_ITEM = 1,
	COPPER_GOLEM_STATE_GETTING_NO_ITEM = 2,
	COPPER_GOLEM_STATE_DROPPING_ITEM = 3,
	COPPER_GOLEM_STATE_DROPPING_NO_ITEM = 4
};

class CopperGolem : public Golem
{
	// CopperGolemAi registers the goals (goalSelector is protected on
	// Mob), and drives the transport state machine from tick().
	friend class CopperGolemAi;

public:
	eINSTANCEOF GetType() { return eTYPE_COPPER_GOLEM; }
	static Entity *create(Level *level) { return new CopperGolem(level); }

	// Oxidation stages (Java CopperGolemOxidationLevels). UNAFFECTED and
	// EXPOSED share the regular sound set; WEATHERED and OXIDIZED have
	// their own sets. Public: the renderer switches on getWeatherState().
	static const int WEATHER_STATE_UNAFFECTED = 0;
	static const int WEATHER_STATE_EXPOSED = 1;
	static const int WEATHER_STATE_WEATHERED = 2;
	static const int WEATHER_STATE_OXIDIZED = 3;

protected:
	// Synched-data id for the weather/wax byte (Sheep uses 16 for its wool
	// byte, VillagerGolem uses 16 for its flags byte — the id is
	// per-entity-type, so 16 is the established slot for a mob's private
	// byte). Bits 0-1 weather state, bit 2 waxed.
	static const int DATA_WEATHER_ID = 16;

	// Weathering timing, in ticks (Java CopperGolem WEATHERING_TICK_FROM /
	// WEATHERING_TICK_TO). The next oxidation stage is scheduled at a random
	// absolute game time in [now + FROM, now + TO]; absolute game time comes
	// from Level::getGameTime(), as in Java.
	static const int64_t WEATHERING_TICK_FROM = 504000;
	static const int64_t WEATHERING_TICK_TO = 552000;

	// Head-spin timing (Java CopperGolemAi SPIN_ANIMATION_MIN/MAX_COOLDOWN
	// 200/240 ticks, SPIN_SOUND_TIME_INTERVAL_OFFSET 10). This tree uses a
	// fixed 220-tick period (the midpoint) driven off tickCount; the model
	// derives the head angle from the same tickCount, so client and server
	// agree without synched data.
	static const int SPIN_PERIOD_TICKS = 220;
	static const int SPIN_DURATION_TICKS = 70; // ~3.5 s idle animation
	static const int SPIN_SOUND_OFFSET_TICKS = 10;

private:
	int64_t nextWeatheringTick;
	CopperGolemState state;
	// Block item carried on the antenna (Java EQUIPMENT_SLOT_ANTENNA, the
	// SADDLE slot). Starts empty; nothing in the extracted Java text ever
	// fills it, so shearing it is a no-op until a fill path is settled.
	// It is NOT a new Item ID: any block item may sit here.
	shared_ptr<ItemInstance> antennaItem;

public:
	CopperGolem(Level *level);

	// Oxidation / wax state.
	virtual int getWeatherState();
	virtual void setWeatherState(int state);
	virtual bool isWaxed();
	virtual void setWaxed(bool waxed);

	// Transport state machine (Phase 5b: always IDLE until 5b lands).
	virtual CopperGolemState getState();
	virtual void setState(CopperGolemState state);

	// Antenna (shear target).
	virtual shared_ptr<ItemInstance> getAntennaItem();
	virtual void setAntennaItem(shared_ptr<ItemInstance> item);
	virtual bool readyForShearing();
	virtual void shear();

	// Head-spin phase for the model: 0..1 while spinning, -1 while idle.
	// Derived from tickCount so no extra synched data is needed.
	virtual float getSpinPhase();

	// Per-oxidation-state sounds (regular set covers UNAFFECTED + EXPOSED).
	virtual int getSpinSound();
	virtual int getHurtSoundForState();
	virtual int getDeathSoundForState();
	virtual int getStepSoundForState();

	// Build-spawning: clear the pumpkin at (x, y, z) and the copper block
	// below it, then spawn a golem. The trigger itself is UNWIRED (this
	// tree has no carved pumpkin; PumpkinTile.cpp is untouched) — call this
	// once Cameron confirms the pattern (plain pumpkin on a copper block).
	// Follows the PumpkinTile::onPlace iron-golem branch.
	static bool spawnFromBuild(Level *level, int x, int y, int z);

	// Turn this golem into a statue in place (oxidized + in air + luck).
	virtual void turnToStatue();

protected:
	virtual void defineSynchedData();
	virtual void registerAttributes();
	virtual void updateWeathering();

public:
	virtual bool useNewAi();
	virtual void tick();
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);
	virtual bool mobInteract(shared_ptr<Player> player);
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

protected:
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
	virtual void playStepSound(int xt, int yt, int zt, int t);
};
