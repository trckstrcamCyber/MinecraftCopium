#pragma once
#include "Tile.h"
#include "ArrayWithLength.h"
#include <vector>
#include <utility>
#include <string>

class Random;
class Enchantment;
class TileEntity;
class WeighedTreasure;

// Lucky Block: a full-cube tile that, when mined normally (no Silk Touch),
// triggers one random outcome from a weighted table - good loot, junk,
// mob spawns, traps, potion effects, XP, rare enchanted gear, a friendly
// (but limited) trader, a couple of named "boss" zombies, and a handful
// of classic Lucky Block mod set-pieces.
//
// Silk Touch bypasses all of this and just gives the player the block back
// (this falls out of the normal Tile::playerDestroy / isSilkTouchable path,
// same as any other cube tile).
class LuckyBlockTile : public Tile
{
private:
	Icon *icon;

public:
	LuckyBlockTile(int id);

	virtual Icon *getTexture(int face, int data) override;
	virtual void registerIcons(IconRegister *iconRegister) override;

	// We handle all drops ourselves via the outcome table, so suppress the
	// tile's normal "drop myself" behavior.
	virtual int getResourceCount(Random *random) override;

	virtual void playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data) override;

private:
	void triggerLuckyOutcome(Level *level, int x, int y, int z, shared_ptr<Player> player);

	// Outcomes
	void outcomeGoodLoot(Level *level, int x, int y, int z);
	void outcomeFoodLoot(Level *level, int x, int y, int z);
	void outcomeJunkLoot(Level *level, int x, int y, int z);
	void outcomeFriendlyMobs(Level *level, int x, int y, int z);
	void outcomeHostileAmbush(Level *level, int x, int y, int z);
	void outcomeCreeperTrap(Level *level, int x, int y, int z);
	void outcomeTntTrap(Level *level, int x, int y, int z, shared_ptr<Player> player);
	void outcomeTntRain(Level *level, int x, int y, int z, shared_ptr<Player> player);
	void outcomeGoodEffect(Level *level, int x, int y, int z, shared_ptr<Player> player);
	void outcomeBadEffect(Level *level, int x, int y, int z, shared_ptr<Player> player);
	void outcomeExperienceBurst(Level *level, int x, int y, int z);
	void outcomeLuckyGear(Level *level, int x, int y, int z);
	void outcomeGodGear(Level *level, int x, int y, int z);
	void outcomeLuckyTrader(Level *level, int x, int y, int z);
	void outcomeBob(Level *level, int x, int y, int z);
	void outcomeTom(Level *level, int x, int y, int z);
	void outcomeAnvilTrap(Level *level, int x, int y, int z);
	void outcomeLavaTrap(Level *level, int x, int y, int z);
	void outcomeWitchSwarm(Level *level, int x, int y, int z);
	void outcomeChargedCreeper(Level *level, int x, int y, int z);
	void outcomeNightHorde(Level *level, int x, int y, int z, shared_ptr<Player> player);
	void outcomeHugeSlime(Level *level, int x, int y, int z);
	void outcomeHugeMagmaCube(Level *level, int x, int y, int z);
	void outcomeIronArmorDrop(Level *level, int x, int y, int z);
	void outcomeSponge(Level *level, int x, int y, int z);
	void outcomeEnderChests(Level *level, int x, int y, int z);
	void outcomeMrRainbows(Level *level, int x, int y, int z);
	void outcomeRainbowWool(Level *level, int x, int y, int z);
	void outcomePrisonTrap(Level *level, int x, int y, int z, shared_ptr<Player> player);
	void outcomeIgnite(Level *level, int x, int y, int z, shared_ptr<Player> player);
	void outcomeChickenShower(Level *level, int x, int y, int z);
	void outcomeRandomWood(Level *level, int x, int y, int z);
	void outcomeChestLoot(Level *level, int x, int y, int z);
	void outcomeRandomEnchantBook(Level *level, int x, int y, int z);
	void outcomeLuckyPlatform(Level *level, int x, int y, int z, shared_ptr<Player> player);
	void outcomeMusicDisc(Level *level, int x, int y, int z);
	void outcomeWeedWacker(Level *level, int x, int y, int z);
	void outcomeMlgWaterBucket(Level *level, int x, int y, int z, shared_ptr<Player> player);
	void outcomeGodSword(Level *level, int x, int y, int z);
	void outcomeFishingRodOfLegends(Level *level, int x, int y, int z);
	void outcomeBasicSupplies(Level *level, int x, int y, int z);
	void outcomeConcrete(Level *level, int x, int y, int z);
	void outcomeSmallHeal(Level *level, int x, int y, int z, shared_ptr<Player> player);
	void outcomeInstantHarm(Level *level, int x, int y, int z, shared_ptr<Player> player);
	void outcomeWither(Level *level, int x, int y, int z, shared_ptr<Player> player);
	void outcomeMiningFatigue(Level *level, int x, int y, int z, shared_ptr<Player> player);
	void outcomeXpDrain(Level *level, int x, int y, int z, shared_ptr<Player> player);
	void outcomeHungerEffect(Level *level, int x, int y, int z, shared_ptr<Player> player);

	// Helpers
	void dropItem(Level *level, int x, int y, int z, int itemId, int count, int aux = 0);
	shared_ptr<ItemInstance> makeEnchantedGear(Level *level, Item *item, std::vector<std::pair<const Enchantment*, int>> enchants);
	std::vector<std::pair<const Enchantment*, int>> godArmorEnchants();
	void placeWarningSign(Level *level, int x, int y, int z, const std::wstring &line1);
	void spawnFallingBlock(Level *level, int x, int y, int z, int tileId, int data);
	WeighedTreasureArray chestTreasureTable();
};