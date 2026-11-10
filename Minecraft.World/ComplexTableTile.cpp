#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"
#include "net.minecraft.h"
#include "ComplexTableTile.h"

// Single source of truth for the blue hue, shared by the in-world and item tint paths.
static const int TINT = 0x1E3CB4;

ComplexTableTile::ComplexTableTile(int id) : Tile(id, Material::wood)
{
	iconTop = nullptr;
	iconFront = nullptr;
}

Icon *ComplexTableTile::getTexture(int face, int data)
{
	if (face == Facing::UP) return iconTop;
	if (face == Facing::DOWN) return Tile::wood->getTexture(face);
	if (face == Facing::NORTH || face == Facing::WEST) return iconFront;
	return icon;
}

void ComplexTableTile::registerIcons(IconRegister *iconRegister)
{
	// Same icon names as WorkbenchTile -- reuses the existing stitched
	// textures rather than registering new ones; the blue hue comes
	// entirely from the getColor() tint below.
	icon = iconRegister->registerIcon(L"workbench_side");
	iconTop = iconRegister->registerIcon(L"workbench_top");
	iconFront = iconRegister->registerIcon(L"workbench_front");
}

int ComplexTableTile::getColor(LevelSource *level, int x, int y, int z)
{
	// Straight RGB multiply against the reused wood texture. This is a
	// tint, not a repaint -- it darkens/cools the existing texture rather
	// than overlaying a new color, same as every other getColor() tint in
	// this engine (e.g. grass/foliage). Adjust this constant to taste.
	// Deep, saturated blue (R and G pulled well below B for a "rich" look,
	// and all three lower than before for "darker") -- was 0x99AAFF.
	return TINT;
}

int ComplexTableTile::getColor(int auxData)
{
	// Inventory / held / dropped item path -- must match the in-world tint above.
	return TINT;
}

// 4J-PB - Adding a TestUse for tooltip display
bool ComplexTableTile::TestUse()
{
	return true;
}

bool ComplexTableTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/)
{
	if( soundOnly ) return false;
	if (level->isClientSide)
	{
		return true;
	}
	player->startComplexCrafting(x, y, z);
	return true;
}
