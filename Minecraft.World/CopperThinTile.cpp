#include "stdafx.h"
#include "CopperThinTile.h"
#include "IconRegister.h"
#include "net.minecraft.world.level.h"

CopperThinTile::CopperThinTile(int id, const wstring &baseIconName,
	float x0, float y0, float z0, float x1, float y1, float z1)
	: Tile(id, Material::metal, isSolidRender())   // <-- false: solid[] + lightBlock set non-opaque
{
	setLightBlock(0);
	this->baseIconName = baseIconName;
	sx0 = x0; sy0 = y0; sz0 = z0; sx1 = x1; sy1 = y1; sz1 = z1;
	for (int i = 0; i < 4; i++) stateIcons[i] = nullptr;
}

bool CopperThinTile::isSolidRender(bool)
{
	return false;
}

bool CopperThinTile::isCubeShaped()
{
	return false;
}

void CopperThinTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	setShape(sx0, sy0, sz0, sx1, sy1, sz1);
}

void CopperThinTile::updateDefaultShape()
{
	setShape(sx0, sy0, sz0, sx1, sy1, sz1);
}

Icon *CopperThinTile::getTexture(int face, int data)
{
	// Waxed (bit 2) reuses the unwaxed texture for the same oxidation state.
	return stateIcons[data & 3];
}

Icon *CopperThinTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	return stateIcons[level->getData(x, y, z) & 3];
}

void CopperThinTile::registerIcons(IconRegister *iconRegister)
{
	static const wchar_t *prefixes[4] = { L"", L"exposed_", L"weathered_", L"oxidized_" };
	for (int i = 0; i < 4; i++)
	{
		wstring name = wstring(prefixes[i]) + baseIconName;
		stateIcons[i] = iconRegister->registerIcon(name);
	}
	// Keep the base Tile::icon on the unaffected state so anything that reads
	// the icon field directly (particles, pick-block) gets a valid copper icon.
	icon = stateIcons[0];
}

int CopperThinTile::getSpawnResourcesAuxValue(int data)
{
	// Breaking a copper block drops the same oxidation state (waxed included).
	return data & 7;
}
