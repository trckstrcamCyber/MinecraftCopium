#include "stdafx.h"
#include "CopperBulbTile.h"
#include "IconRegister.h"
#include "net.minecraft.world.level.h"
#include "SoundTypes.h"

const int CopperBulbTile::LIGHT_LEVELS[4] = { 15, 14, 13, 12 };

CopperBulbTile::CopperBulbTile(int id) : MetalTile(id)
{
	for (int i = 0; i < 4; i++)
	{
		bulbIcons[i][0] = nullptr;
		bulbIcons[i][1] = nullptr;
	}
}

Icon *CopperBulbTile::getTexture(int face, int data)
{
	int state = data & OXIDATION_MASK;
	int lit = (data & LIT_BIT) != 0 ? 1 : 0;
	return bulbIcons[state][lit];
}

Icon *CopperBulbTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	return getTexture(face, level->getData(x, y, z));
}

void CopperBulbTile::registerIcons(IconRegister *iconRegister)
{
	static const wchar_t *prefixes[4] = { L"", L"exposed_", L"weathered_", L"oxidized_" };
	for (int i = 0; i < 4; i++)
	{
		wstring off = wstring(prefixes[i]) + L"copper_bulb";
		wstring on = wstring(prefixes[i]) + L"copper_bulb_lit";
		bulbIcons[i][0] = iconRegister->registerIcon(off);
		bulbIcons[i][1] = iconRegister->registerIcon(on);
	}
	icon = bulbIcons[0][0];
}

void CopperBulbTile::onPlace(Level *level, int x, int y, int z)
{
	Tile::onPlace(level, x, y, z);
	checkAndFlip(level, x, y, z);
}

void CopperBulbTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	Tile::neighborChanged(level, x, y, z, type);
	checkAndFlip(level, x, y, z);
}

void CopperBulbTile::checkAndFlip(Level *level, int x, int y, int z)
{
	int data = level->getData(x, y, z);
	bool powered = (data & POWERED_BIT) != 0;
	bool signal = level->hasNeighborSignal(x, y, z);

	if (signal != powered)
	{
		if (!powered)
		{
			// Rising edge: toggle LIT and play the toggle sound.
			data ^= LIT_BIT;
			level->playSound(x + 0.5, y + 0.5, z + 0.5,
				eSoundType_COPPER_BULB_TOGGLE, 0.5f, 1.0f);
		}
		data = signal ? (data | POWERED_BIT) : (data & ~POWERED_BIT);
		level->setData(x, y, z, data, Tile::UPDATE_CLIENTS);
		// The analog output may have changed — let comparators re-read.
		level->updateNeighborsAt(x, y, z, id);
	}
}

float CopperBulbTile::getBrightness(LevelSource *level, int x, int y, int z)
{
	int data = level->getData(x, y, z);
	int emission = ((data & LIT_BIT) != 0) ? LIGHT_LEVELS[data & OXIDATION_MASK] : 0;
	return level->getBrightness(x, y, z, emission);
}

int CopperBulbTile::getLightColor(LevelSource *level, int x, int y, int z, int tileId)
{
	int data = level->getData(x, y, z);
	int emission = ((data & LIT_BIT) != 0) ? LIGHT_LEVELS[data & OXIDATION_MASK] : 0;
	return level->getLightColor(x, y, z, emission, -1);
}

bool CopperBulbTile::hasAnalogOutputSignal()
{
	return true;
}

int CopperBulbTile::getAnalogOutputSignal(Level *level, int x, int y, int z, int dir)
{
	int data = level->getData(x, y, z);
	return ((data & LIT_BIT) != 0) ? 15 : 0;
}

int CopperBulbTile::getSpawnResourcesAuxValue(int data)
{
	// Breaking a bulb drops it unlit and unpowered (oxidation + waxed kept).
	return data & 7;
}
