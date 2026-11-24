#include "stdafx.h"
#include "CopperThinTile.h"
#include "IconRegister.h"
#include "net.minecraft.world.level.h"

const unsigned int CopperThinTile::COPPER_CHAIN_NAMES[8] = {
	IDS_TILE_COPPER_CHAIN,
	IDS_TILE_EXPOSED_COPPER_CHAIN,
	IDS_TILE_WEATHERED_COPPER_CHAIN,
	IDS_TILE_OXIDIZED_COPPER_CHAIN,
	IDS_TILE_WAXED_COPPER_CHAIN,
	IDS_TILE_WAXED_EXPOSED_COPPER_CHAIN,
	IDS_TILE_WAXED_WEATHERED_COPPER_CHAIN,
	IDS_TILE_WAXED_OXIDIZED_COPPER_CHAIN
};
const unsigned int CopperThinTile::COPPER_LANTERN_NAMES[8] = {
	IDS_TILE_COPPER_LANTERN,
	IDS_TILE_EXPOSED_COPPER_LANTERN,
	IDS_TILE_WEATHERED_COPPER_LANTERN,
	IDS_TILE_OXIDIZED_COPPER_LANTERN,
	IDS_TILE_WAXED_COPPER_LANTERN,
	IDS_TILE_WAXED_EXPOSED_COPPER_LANTERN,
	IDS_TILE_WAXED_WEATHERED_COPPER_LANTERN,
	IDS_TILE_WAXED_OXIDIZED_COPPER_LANTERN
};
const unsigned int CopperThinTile::LIGHTNING_ROD_NAMES[8] = {
	IDS_TILE_LIGHTNING_ROD,
	IDS_TILE_EXPOSED_LIGHTNING_ROD,
	IDS_TILE_WEATHERED_LIGHTNING_ROD,
	IDS_TILE_OXIDIZED_LIGHTNING_ROD,
	IDS_TILE_WAXED_LIGHTNING_ROD,
	IDS_TILE_WAXED_EXPOSED_LIGHTNING_ROD,
	IDS_TILE_WAXED_WEATHERED_LIGHTNING_ROD,
	IDS_TILE_WAXED_OXIDIZED_LIGHTNING_ROD
};

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

int CopperThinTile::getRenderShape()
{
	// COPPER_MODELS_RENDERER: the lantern (body + cap) and the rod (head +
	// shaft) have Java-style two-box models; the chain has Java's two
	// crossed flat quads (COPPER_CHAIN_JAVA, 2026-11-22 -- see
	// TileRenderer::tesselateCopperChainInWorld). Only the three CopperThinTile
	// ids ever reach this override, so the fallthrough stays SHAPE_BLOCK.
	if (id == Tile::copper_chain_Id) return Tile::SHAPE_COPPER_CHAIN;
	if (id == Tile::copper_lantern_Id) return Tile::SHAPE_COPPER_LANTERN;
	if (id == Tile::lightning_rod_Id) return Tile::SHAPE_LIGHTNING_ROD;
	return Tile::SHAPE_BLOCK;
}

void CopperThinTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	setShape(sx0, sy0, sz0, sx1, sy1, sz1);
}

void CopperThinTile::updateDefaultShape()
{
	setShape(sx0, sy0, sz0, sx1, sy1, sz1);
}

bool CopperThinTile::mayPlace(Level *level, int x, int y, int z)
{
	// The target cell must be empty/replaceable (base rule) and the block
	// must have support: chains hang from above, lanterns hang from above
	// or stand on a solid block below. The lightning rod has no support
	// rule (unchanged behaviour).
	return Tile::mayPlace(level, x, y, z) && isSupported(level, x, y, z);
}

void CopperThinTile::onPlace(Level *level, int x, int y, int z)
{
	// Backstop for placement paths that bypass mayPlace (e.g. /setblock):
	// an unsupported chain/lantern pops immediately instead of floating.
	checkCanSurvive(level, x, y, z);
}

void CopperThinTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	// Support removed above/below: drop the block instead of floating
	// (TorchTile pattern: spawnResources + removeTile).
	checkCanSurvive(level, x, y, z);
}

bool CopperThinTile::isSupported(Level *level, int x, int y, int z)
{
	// The lightning rod has no support rule (unchanged behaviour).
	if (id == Tile::lightning_rod_Id) return true;

	int above = level->getTile(x, y + 1, z);
	bool chainAbove = (above == Tile::copper_chain_Id || above == Tile::iron_chain_Id);
	bool solidAbove = level->isSolidBlockingTileInLoadedChunk(x, y + 1, z, true);
	if (id == Tile::copper_chain_Id || id == Tile::iron_chain_Id)
	{
		// A chain hangs from a solid block or another chain above it.
		return chainAbove || solidAbove;
	}
	// A lantern hangs from a chain/solid block above, or stands on a
	// solid block below.
	bool solidBelow = level->isSolidBlockingTileInLoadedChunk(x, y - 1, z, true);
	return solidBelow || chainAbove || solidAbove;
}

bool CopperThinTile::checkCanSurvive(Level *level, int x, int y, int z)
{
	if (!isSupported(level, x, y, z))
	{
		if (level->getTile(x, y, z) == id)
		{
			spawnResources(level, x, y, z, level->getData(x, y, z), 0);
			level->removeTile(x, y, z);
		}
		return false;
	}
	return true;
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

unsigned int CopperThinTile::getDescriptionId(int iData)
{
	if (iData < 0) iData = 0;
	int aux = iData & 7;
	if (id == Tile::copper_chain_Id) return COPPER_CHAIN_NAMES[aux];
	if (id == Tile::copper_lantern_Id) return COPPER_LANTERN_NAMES[aux];
	if (id == Tile::lightning_rod_Id) return LIGHTNING_ROD_NAMES[aux];
	return Tile::getDescriptionId(iData);
}
