#pragma once
#include "CraftingContainer.h"

// neoLegacy - banner patterns. The short codes are Java's (they are what goes into the banner's NBT), the layouts are Java's pattern
// recipes: # is a dye, a blank is an empty slot, and the banner goes in a blank slot. A few patterns are made with an ingredient item
// instead of a layout (vines, a creeper head, a brick block, a wither skull, a flower, an enchanted golden apple) plus one dye.
// The picture for each pattern is mob/banner_layers/<texture>.png; BannerRenderer keeps its own list of them in the same order.

#define BANNER_PATTERN_COUNT 38

struct BannerPatternDef
{
	const wchar_t *code;
	const wchar_t *texture;
	const wchar_t *rows[3];		// crafting layout, or nullptr when made with an ingredient
	int itemId;					// the ingredient, 0 for a layout pattern
	int itemAux;				// -1 = any
};

class BannerPatterns
{
public:
	static const int MAX_CRAFTED_LAYERS = 6;		// a banner can be crafted up to this many patterns...
	static const int MAX_LAYERS = 16;				// ...but more can be on one (the Ominous Banner has 8)

	static const BannerPatternDef *get(int index);
	static int indexOf(const wstring &code);		// -1 when unknown

	// The pattern the grid would add to its banner, or -1; dyeColor gets the dye's colour. Needs the 3x3 grid.
	static int match(shared_ptr<CraftingContainer> grid, int *dyeColor);
};
