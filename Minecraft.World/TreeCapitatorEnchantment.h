#pragma once

#include "Enchantment.h"

// 4J neoLegacy Added
// Axe-only digger enchantment. Chops one log block and chain-breaks every
// neighboring log (26-connected, not just 6-face-adjacent, since tree
// branches sit diagonally) of the same species, up to a level-scaled cap -
// same shape as VeinminerEnchantment, but for trees.
//
// Same build-layering note as VeinminerEnchantment: this class stays
// World-only. The actual chain-chop loop lives in
// ServerPlayerGameMode::destroyBlock() (Minecraft.Server), not here, since
// Minecraft.World cannot depend on Minecraft.Server.
class TreeCapitatorEnchantment : public Enchantment
{
public:
	TreeCapitatorEnchantment(int id, int frequency);

	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getMaxLevel();
	virtual bool canEnchant(shared_ptr<ItemInstance> item);
	virtual HtmlString getFullname(int level);

	static bool isTreeLog(int tileId);

	// Compares just the species bits (low 2 bits) of the data value, not the
	// whole data byte - vanilla wood data packs species in bits 0-1 and axis
	// orientation (which way the log is facing) in bits 2-3, and a tree's
	// trunk vs. its branches legitimately differ in axis while still being
	// "the same tree."
	static bool isSameSpecies(int data, int otherData);

	static int getMaxChainSize(int level);
};
