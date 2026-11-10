#pragma once

#include "Enchantment.h"

// 4J neoLegacy Added
// Pickaxe-only digger enchantment. Breaking one ore block chain-breaks every
// directly-adjacent (6-connected) block sharing the same tile id + data value,
// up to a level-scaled cap.
//
// IMPORTANT (build-layering note): this class stays World-only on purpose.
// The actual chain-break loop lives in ServerPlayerGameMode::destroyBlock()
// (Minecraft.Server), not here - Minecraft.World cannot include or reference
// ServerPlayerGameMode, since Minecraft.Server depends on Minecraft.World and
// not the other way around. An earlier version of this file tried to call
// back into ServerPlayerGameMode directly and failed to compile for exactly
// this reason ('ServerPlayerGameMode.h' file not found while building the
// Minecraft.World target). Keep this class limited to data/rules that belong
// to World; put orchestration that needs to call destroyBlock() in Server.
class VeinminerEnchantment : public Enchantment
{
public:
	VeinminerEnchantment(int id, int frequency);

	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool canEnchant(shared_ptr<ItemInstance> item);
	virtual HtmlString getFullname(int level);

	static bool isVeinmineable(int tileId);
	static int getMaxChainSize(int level);
};
