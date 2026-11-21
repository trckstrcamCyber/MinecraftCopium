#include "stdafx.h"
#include "net.minecraft.h"
#include "CopperButtonTile.h"
#include "IconRegister.h"

CopperButtonTile::CopperButtonTile(int id) : ButtonTile(id, false)
{
}

Icon *CopperButtonTile::getTexture(int face, int data)
{
	// The button's own icon, registered from iconName L"copper_block"
	// (atlas row 32 col 0) by registerIcons below. StoneButtonTile
	// hard-codes the stone texture here; returning the tile's icon is
	// what makes this button copper.
	return icon;
}

void CopperButtonTile::registerIcons(IconRegister *iconRegister)
{
	// ButtonTile::registerIcons is a no-op, so without this the tile's
	// icon stays null: getTexture() returns null, the in-world renderer
	// falls back to the whole-atlas missing icon, and the dropped-item
	// renderer dereferences the null icon (crash +0x2a0b74).
	icon = iconRegister->registerIcon(getIconName()); // L"copper_block", atlas (32,0)
}
