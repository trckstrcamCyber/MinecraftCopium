#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "ItemInstance.h"
#include "BannerItem.h"
#include "BannerPatterns.h"

static const BannerPatternDef PATTERNS[BANNER_PATTERN_COUNT] =
{
	{ L"bl", L"square_bottom_left", { L"   ", L"   ", L"#  " }, 0, 0 },
	{ L"br", L"square_bottom_right", { L"   ", L"   ", L"  #" }, 0, 0 },
	{ L"tl", L"square_top_left", { L"#  ", L"   ", L"   " }, 0, 0 },
	{ L"tr", L"square_top_right", { L"  #", L"   ", L"   " }, 0, 0 },
	{ L"bs", L"stripe_bottom", { L"   ", L"   ", L"###" }, 0, 0 },
	{ L"ts", L"stripe_top", { L"###", L"   ", L"   " }, 0, 0 },
	{ L"ls", L"stripe_left", { L"#  ", L"#  ", L"#  " }, 0, 0 },
	{ L"rs", L"stripe_right", { L"  #", L"  #", L"  #" }, 0, 0 },
	{ L"cs", L"stripe_center", { L" # ", L" # ", L" # " }, 0, 0 },
	{ L"ms", L"stripe_middle", { L"   ", L"###", L"   " }, 0, 0 },
	{ L"drs", L"stripe_downright", { L"#  ", L" # ", L"  #" }, 0, 0 },
	{ L"dls", L"stripe_downleft", { L"  #", L" # ", L"#  " }, 0, 0 },
	{ L"ss", L"small_stripes", { L"# #", L"   ", L"# #" }, 0, 0 },
	{ L"cr", L"cross", { L"# #", L" # ", L"# #" }, 0, 0 },
	{ L"sc", L"straight_cross", { L" # ", L"###", L" # " }, 0, 0 },
	{ L"bt", L"triangle_bottom", { L"   ", L" # ", L"# #" }, 0, 0 },
	{ L"tt", L"triangle_top", { L"# #", L" # ", L"   " }, 0, 0 },
	{ L"bts", L"triangles_bottom", { L"   ", L"# #", L" # " }, 0, 0 },
	{ L"tts", L"triangles_top", { L" # ", L"# #", L"   " }, 0, 0 },
	{ L"ld", L"diagonal_left", { L"## ", L"#  ", L"   " }, 0, 0 },
	{ L"rd", L"diagonal_right", { L"   ", L"  #", L" ##" }, 0, 0 },
	{ L"lud", L"diagonal_up_left", { L"   ", L"#  ", L"## " }, 0, 0 },
	{ L"rud", L"diagonal_up_right", { L" ##", L"  #", L"   " }, 0, 0 },
	{ L"mc", L"circle", { L"   ", L" # ", L"   " }, 0, 0 },
	{ L"mr", L"rhombus", { L" # ", L"# #", L" # " }, 0, 0 },
	{ L"vh", L"half_vertical", { L"## ", L"## ", L"## " }, 0, 0 },
	{ L"hh", L"half_horizontal", { L"###", L"###", L"   " }, 0, 0 },
	{ L"vhr", L"half_vertical_right", { L" ##", L" ##", L" ##" }, 0, 0 },
	{ L"hhb", L"half_horizontal_bottom", { L"   ", L"###", L"###" }, 0, 0 },
	{ L"bo", L"border", { L"###", L"# #", L"###" }, 0, 0 },
	{ L"cbo", L"curly_border", { nullptr, nullptr, nullptr }, Tile::vine_Id, -1 },
	{ L"cre", L"creeper", { nullptr, nullptr, nullptr }, Item::skull_Id, 4 },
	{ L"gra", L"gradient", { L"# #", L" # ", L" # " }, 0, 0 },
	{ L"gru", L"gradient_up", { L" # ", L" # ", L"# #" }, 0, 0 },
	{ L"bri", L"bricks", { nullptr, nullptr, nullptr }, Tile::brick_block_Id, -1 },
	{ L"sku", L"skull", { nullptr, nullptr, nullptr }, Item::skull_Id, 1 },
	{ L"flo", L"flower", { nullptr, nullptr, nullptr }, Tile::red_flower_Id, -1 },
	{ L"moj", L"mojang", { nullptr, nullptr, nullptr }, Item::golden_apple_Id, 1 }
};

const BannerPatternDef *BannerPatterns::get(int index)
{
	if (index < 0 || index >= BANNER_PATTERN_COUNT) return nullptr;
	return &PATTERNS[index];
}

int BannerPatterns::indexOf(const wstring &code)
{
	for (int i = 0; i < BANNER_PATTERN_COUNT; i++)
	{
		if (code == PATTERNS[i].code) return i;
	}
	return -1;
}

// Java's rule: exactly one banner (with room for another pattern); then either the dyes sit exactly on the layout's # slots (all the same
// dye, the banner on a blank slot), or there is one dye and the pattern's ingredient item.
int BannerPatterns::match(shared_ptr<CraftingContainer> grid, int *dyeColor)
{
	if (grid == nullptr || grid->getContainerSize() != 9) return -1;

	int banners = 0;
	for (unsigned int i = 0; i < 9; i++)
	{
		shared_ptr<ItemInstance> s = grid->getItem(i);
		if (s != nullptr && dynamic_cast<BannerItem *>(s->getItem()) != nullptr)
		{
			if (++banners > 1) return -1;
			if (static_cast<int>(BannerItem::getPatterns(s).size()) >= MAX_CRAFTED_LAYERS) return -1;
		}
	}
	if (banners != 1) return -1;

	for (int p = 0; p < BANNER_PATTERN_COUNT; p++)
	{
		const BannerPatternDef &def = PATTERNS[p];
		bool ok = true;
		int color = -1;

		if (def.itemId != 0)
		{
			bool haveItem = false;
			bool haveDye = false;
			for (unsigned int i = 0; i < 9 && ok; i++)
			{
				shared_ptr<ItemInstance> s = grid->getItem(i);
				if (s == nullptr || dynamic_cast<BannerItem *>(s->getItem()) != nullptr) continue;
				if (s->id == Item::dye_Id)
				{
					if (haveDye) ok = false;
					haveDye = true;
					color = s->getAuxValue() & 15;
				}
				else if (!haveItem && s->id == def.itemId && (def.itemAux < 0 || s->getAuxValue() == def.itemAux))
				{
					haveItem = true;
				}
				else
				{
					ok = false;
				}
			}
			if (!haveItem || !haveDye) ok = false;
		}
		else
		{
			for (unsigned int i = 0; i < 9 && ok; i++)
			{
				shared_ptr<ItemInstance> s = grid->getItem(i);
				wchar_t want = def.rows[i / 3][i % 3];
				bool isBanner = s != nullptr && dynamic_cast<BannerItem *>(s->getItem()) != nullptr;
				if (s != nullptr && !isBanner)
				{
					if (s->id != Item::dye_Id || want == L' ') { ok = false; break; }
					int c = s->getAuxValue() & 15;
					if (color != -1 && color != c) { ok = false; break; }
					color = c;
				}
				else if (want != L' ')
				{
					ok = false;
				}
			}
			if (color == -1) ok = false;
		}

		if (ok)
		{
			if (dyeColor != nullptr) *dyeColor = color;
			return p;
		}
	}
	return -1;
}
