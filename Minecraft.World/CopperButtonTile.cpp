#include "stdafx.h"
#include "net.minecraft.h"
#include "CopperButtonTile.h"

CopperButtonTile::CopperButtonTile(int id) : ButtonTile(id, false)
{
}

Icon *CopperButtonTile::getTexture(int face, int data)
{
	// The button's own icon, registered from iconName L"copper_block"
	// (atlas row 32 col 0) by Tile::registerIcons. StoneButtonTile
	// hard-codes the stone texture here; returning the tile's icon is
	// what makes this button copper. (ButtonTile::registerIcons is a
	// no-op, so no atlas-table change is needed.)
	return icon;
}
