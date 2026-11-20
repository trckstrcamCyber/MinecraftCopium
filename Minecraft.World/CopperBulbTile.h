#pragma once
#include "MetalTile.h"

class IconRegister;

// neoLegacy - Copper Age (Phase 3). The copper bulb (Java CopperBulbBlock).
//
// Aux layout (4 bits — data is a 4-bit nibble, aux 0-15):
//   bits 0-1  oxidation 0=unaffected, 1=exposed, 2=weathered, 3=oxidized
//   bit 2     waxed (+4); waxed reuses the unwaxed texture
//   bit 3     POWERED (+8) — remembers the last redstone input so a rising
//             edge can be detected (Java: POWERED is a separate boolean).
// LIT is NOT a data bit: it is carried by the tile id itself (2310 unlit /
// 2329 lit). A 5th bit can never survive the 4-bit nibble store — the old
// POWERED bit 4 spilled into the neighbouring block's data and made
// checkAndFlip recurse forever via onPlace.
//
// Behaviour (Java checkAndFlip, copper-code.txt CopperBulbBlock): on place
// and on neighbor change, if the redstone input differs from POWERED, and
// the block is not currently POWERED (rising edge), toggle LIT and play the
// toggle sound; then store the new input in POWERED. Analog output is 15
// while LIT, 0 otherwise.
//
// Light (engine limitation, documented): Level's light cache reads the
// per-tile-id Tile::lightEmission[] array, so one tile id cannot emit a
// different light level per state. The bulb is registered with
// setLightEmission(0) and getBrightness()/getLightColor() are overridden so
// a lit bulb RENDERS at its per-oxidation brightness (15/14/13/12). True
// per-state light propagation needs a second "lit bulb" tile id (the
// furnace_lit pattern) — a Phase 4 item.
class CopperBulbTile : public MetalTile
{
public:
	static const int OXIDATION_MASK = 3;
	static const int WAXED_BIT = 4;
	static const int POWERED_BIT = 8;	// was 16 — data is a 4-bit nibble; bit 4 does not survive storage
	// LIT is carried by the tile id itself (2310 unlit / 2329 lit), NOT by a data bit.
	// (Old LIT_BIT = 8 is deleted; it is the same bit position, now meaning POWERED.)

	// Lit brightness by oxidation state (Java values; the light level the
	// bulb WOULD emit — used for rendering until the two-id fix lands).
	static const int LIGHT_LEVELS[4];

private:
	// icons[state][lit] — 4 oxidation states x off/lit.
	Icon *bulbIcons[4][2];

	void checkAndFlip(Level *level, int x, int y, int z);

public:
	CopperBulbTile(int id);

	virtual Icon *getTexture(int face, int data) override;
	virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face) override;
	virtual void registerIcons(IconRegister *iconRegister) override;

	virtual void onPlace(Level *level, int x, int y, int z) override;
	virtual void neighborChanged(Level *level, int x, int y, int z, int type) override;

	virtual float getBrightness(LevelSource *level, int x, int y, int z) override;
	virtual int getLightColor(LevelSource *level, int x, int y, int z, int tileId = -1) override;

	virtual bool hasAnalogOutputSignal() override;
	virtual int getAnalogOutputSignal(Level *level, int x, int y, int z, int dir) override;

protected:
	virtual int getSpawnResourcesAuxValue(int data) override;
	// neoLegacy - Copper Age (Phase 4). A lit bulb (tile 2329) drops the
	// unlit bulb item, not a (nonexistent) lit-bulb item.
	virtual int getResource(int data, Random *random, int playerBonusLevel) override;
};
