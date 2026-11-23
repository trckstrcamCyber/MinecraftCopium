#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "Mth.h"
#include "SoundTypes.h"
#include "LostCandleItem.h"

LostCandleItem::LostCandleItem(int id) : Item(id)
{
	maxStackSize = 1;
}

// the ores it can find, and what to call them
static const wchar_t *oreName(int tile)
{
	switch (tile)
	{
	case Tile::coal_ore_Id: return L"Coal Ore";
	case Tile::copper_ore_Id: return L"Copper Ore";	// neoLegacy - Copper Age: the branch's own ore, same as the rest
	case Tile::iron_ore_Id: return L"Iron Ore";
	case Tile::gold_ore_Id: return L"Gold Ore";
	case Tile::lapis_ore_Id: return L"Lapis Lazuli Ore";
	case Tile::diamond_ore_Id: return L"Diamond Ore";
	case Tile::redstone_ore_Id:
	case Tile::lit_redstone_ore_Id: return L"Redstone Ore";
	case Tile::emerald_ore_Id: return L"Emerald Ore";
	case Tile::quartz_ore_Id: return L"Nether Quartz Ore";
	default: return nullptr;
	}
}

// Java scans the blocks within a manhattan distance of 8 of the player and reports the first ore it meets (the nearest, as its scan goes outwards)
bool LostCandleItem::useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	if (bTestUseOnOnly) return true;
	if (level->isClientSide) return true;

	int px = Mth::floor(player->x), py = Mth::floor(player->y), pz = Mth::floor(player->z);
	for (int dist = 0; dist <= RANGE; dist++)
	{
		for (int dx = -dist; dx <= dist; dx++)
		{
			for (int dy = -dist + abs(dx); dy <= dist - abs(dx); dy++)
			{
				int rest = dist - abs(dx) - abs(dy);
				for (int s = 0; s < 2; s++)
				{
					int dz = (s == 0) ? rest : -rest;
					if (s == 1 && rest == 0) break;
					int t = level->getTile(px + dx, py + dy, pz + dz);
					const wchar_t *name = oreName(t);
					if (name == nullptr) continue;

					level->playSound(player->x, player->y, player->z, eSoundType_RANDOM_ORB, 1.0f, 1.2f);
					player->sendMessage(wstring(L"Ore nearby: ") + name);
					return true;
				}
			}
		}
	}

	level->playSound(player->x, player->y, player->z, eSoundType_RANDOM_FIZZ, 0.6f, 1.0f);	// nothing near
	return true;
}
