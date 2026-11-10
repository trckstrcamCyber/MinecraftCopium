#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.ambient.h"
#include "net.minecraft.world.entity.global.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.npc.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.item.trading.h"
#include "net.minecraft.world.h"
#include "net.minecraft.h"
#include "SharedConstants.h"
#include "Enchantment.h"
#include "EnchantedBookItem.h"
#include "WeighedTreasure.h"
#include "Mth.h"
#include <cmath>

#include "LuckyBlockTile.h"
#include "IconRegister.h"
#include "SoundTypes.h"

LuckyBlockTile::LuckyBlockTile(int id) : Tile(id, Material::stone)
{
	icon = nullptr;
}

Icon *LuckyBlockTile::getTexture(int face, int data)
{
	return icon;
}

void LuckyBlockTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"lucky_block");
}

int LuckyBlockTile::getResourceCount(Random *random)
{
	return 0;
}

void LuckyBlockTile::playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data)
{
	bool silkTouched = isSilkTouchable() && EnchantmentHelper::hasSilkTouch(player);

	// Handles stats, food exhaustion, and (if applicable) the Silk Touch
	// drop. getResourceCount() above is 0, so the non-Silk-Touch branch
	// in the base implementation won't drop anything on its own.
	Tile::playerDestroy(level, player, x, y, z, data);

	if (!silkTouched && !level->isClientSide)
	{
		triggerLuckyOutcome(level, x, y, z, player);
	}
}

void LuckyBlockTile::triggerLuckyOutcome(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	// Weighted outcome table, out of 100 - roughly: common everyday stuff
	// is the most likely result, real danger/reward outcomes are uncommon,
	// and the named "boss" zombies / Godly Wooden Sword are true rare
	// finds. Nothing here should feel like a free win - Lucky/God Gear
	// and the Godly Sword were deliberately toned down from earlier
	// passes so a lucky roll stays fun rather than game-breaking.
	//    0-3   (4) good loot (1-in-5 of which is a plain iron sword)
	//    4-5   (2) food loot
	//    6-7   (2) junk loot
	//    8-9   (2) friendly mob squad
	//   10-11  (2) hostile ambush (sometimes "Lucky <Mob>")
	//   12-13  (2) creeper trap
	//   14-15  (2) TNT trap
	//   16-17  (2) TNT rain
	//   18-19  (2) good potion effect
	//   20-21  (2) bad potion effect
	//   22-23  (2) XP burst
	//   24-25  (2) Lucky Gear - enchanted golden gear, roughly
	//               table/anvil-legal power level
	//   26-27  (2) God Gear - diamond gear stacking protections you
	//               couldn't legally combine, but at modest levels
	//   28-29  (2) Lucky Trader
	//   30     (1) Bob
	//   31     (1) Tom
	//   32-33  (2) Anvil trap
	//   34-35  (2) Lava trap
	//   36-37  (2) Witch + bat swarm
	//   38-39  (2) Lightning strikes and charges a creeper
	//   40-41  (2) Night horde
	//   42-43  (2) Huge Slime
	//   44-45  (2) Huge Magma Cube
	//   46-47  (2) 2-3 random iron armor pieces
	//   48-49  (2) A single Sponge
	//   50-51  (2) 2 Ender Chests
	//   52-53  (2) Mr. Rainbows
	//   54-55  (2) Rainbow Wool
	//   56-57  (2) Prison trap
	//   58-59  (2) Ignite
	//   60-61  (2) Chicken shower
	//   62-63  (2) Random type of wood
	//   64-65  (2) Chest loot
	//   66-67  (2) Random enchanted book
	//   68-69  (2) Lucky Platform
	//   70-71  (2) A random music disc
	//   72-74  (3) Weed Wacker - stick w/ Knockback 10 (silly on purpose)
	//   75-76  (2) MLG water bucket
	//   77     (1) Godly Wooden Sword - EXTREMELY rare, on par with Bob/Tom
	//   78-79  (2) The Fishing Rod of Legends
	//   80-81  (2) Basic supplies - coal, wheat, leather, flint and
	//               steel, milk bucket, or a saddle
	//   82-83  (2) Concrete - a stack of one random color
	//   84-86  (3) Small heal - Instant Health + brief Saturation
	//   87-89  (3) Instant Harm - ouch
	//   90-91  (2) Wither for a few seconds
	//   92-94  (3) Mining Fatigue for a while
	//   95-96  (2) Lose 1-3 XP levels
	//   97-99  (3) Hunger
	int roll = level->random->nextInt(100);

	if (roll < 4)
		outcomeGoodLoot(level, x, y, z);
	else if (roll < 6)
		outcomeFoodLoot(level, x, y, z);
	else if (roll < 8)
		outcomeJunkLoot(level, x, y, z);
	else if (roll < 10)
		outcomeFriendlyMobs(level, x, y, z);
	else if (roll < 12)
		outcomeHostileAmbush(level, x, y, z);
	else if (roll < 14)
		outcomeCreeperTrap(level, x, y, z);
	else if (roll < 16)
		outcomeTntTrap(level, x, y, z, player);
	else if (roll < 18)
		outcomeTntRain(level, x, y, z, player);
	else if (roll < 20)
		outcomeGoodEffect(level, x, y, z, player);
	else if (roll < 22)
		outcomeBadEffect(level, x, y, z, player);
	else if (roll < 24)
		outcomeExperienceBurst(level, x, y, z);
	else if (roll < 26)
		outcomeLuckyGear(level, x, y, z);
	else if (roll < 28)
		outcomeGodGear(level, x, y, z);
	else if (roll < 30)
		outcomeLuckyTrader(level, x, y, z);
	else if (roll < 31)
		outcomeBob(level, x, y, z);
	else if (roll < 32)
		outcomeTom(level, x, y, z);
	else if (roll < 34)
		outcomeAnvilTrap(level, x, y, z);
	else if (roll < 36)
		outcomeLavaTrap(level, x, y, z);
	else if (roll < 38)
		outcomeWitchSwarm(level, x, y, z);
	else if (roll < 40)
		outcomeChargedCreeper(level, x, y, z);
	else if (roll < 42)
		outcomeNightHorde(level, x, y, z, player);
	else if (roll < 44)
		outcomeHugeSlime(level, x, y, z);
	else if (roll < 46)
		outcomeHugeMagmaCube(level, x, y, z);
	else if (roll < 48)
		outcomeIronArmorDrop(level, x, y, z);
	else if (roll < 50)
		outcomeSponge(level, x, y, z);
	else if (roll < 52)
		outcomeEnderChests(level, x, y, z);
	else if (roll < 54)
		outcomeMrRainbows(level, x, y, z);
	else if (roll < 56)
		outcomeRainbowWool(level, x, y, z);
	else if (roll < 58)
		outcomePrisonTrap(level, x, y, z, player);
	else if (roll < 60)
		outcomeIgnite(level, x, y, z, player);
	else if (roll < 62)
		outcomeChickenShower(level, x, y, z);
	else if (roll < 64)
		outcomeRandomWood(level, x, y, z);
	else if (roll < 66)
		outcomeChestLoot(level, x, y, z);
	else if (roll < 68)
		outcomeRandomEnchantBook(level, x, y, z);
	else if (roll < 70)
		outcomeLuckyPlatform(level, x, y, z, player);
	else if (roll < 72)
		outcomeMusicDisc(level, x, y, z);
	else if (roll < 75)
		outcomeWeedWacker(level, x, y, z);
	else if (roll < 77)
		outcomeMlgWaterBucket(level, x, y, z, player);
	else if (roll < 78)
		outcomeGodSword(level, x, y, z);
	else if (roll < 80)
		outcomeFishingRodOfLegends(level, x, y, z);
	else if (roll < 82)
		outcomeBasicSupplies(level, x, y, z);
	else if (roll < 84)
		outcomeConcrete(level, x, y, z);
	else if (roll < 87)
		outcomeSmallHeal(level, x, y, z, player);
	else if (roll < 90)
		outcomeInstantHarm(level, x, y, z, player);
	else if (roll < 92)
		outcomeWither(level, x, y, z, player);
	else if (roll < 95)
		outcomeMiningFatigue(level, x, y, z, player);
	else if (roll < 97)
		outcomeXpDrain(level, x, y, z, player);
	else
		outcomeHungerEffect(level, x, y, z, player);
}

void LuckyBlockTile::dropItem(Level *level, int x, int y, int z, int itemId, int count, int aux)
{
	popResource(level, x, y, z, std::make_shared<ItemInstance>(itemId, count, aux));
}

shared_ptr<ItemInstance> LuckyBlockTile::makeEnchantedGear(Level *level, Item *item, std::vector<std::pair<const Enchantment*, int>> enchants)
{
	shared_ptr<ItemInstance> instance = std::make_shared<ItemInstance>(item, 1);
	// 30% chance to apply enchantments; 70% chance to return a plain item
	if (level->random->nextInt(100) < 44)
	{
		for (auto &e : enchants)
		{
			instance->enchant(e.first, e.second);
		}
	}
	return instance;
}

std::vector<std::pair<const Enchantment*, int>> LuckyBlockTile::godArmorEnchants()
{
	// Deliberately stacks all four protection types at once, which the
	// anvil/enchant table would never let you combine legitimately -
	// that's the point of "God Gear" - but at moderate levels rather
	// than maxed out, so it stays a fun rarity rather than a free win.
	return {
		{ Enchantment::allDamageProtection, 5 },
		{ Enchantment::fireProtection, 5 },
		{ Enchantment::explosionProtection, 5 },
		{ Enchantment::projectileProtection, 5 },
		{ Enchantment::thorns, 3 },
		{ Enchantment::digDurability, 4 },
		{ Enchantment::mending, 1 },
	};
}

void LuckyBlockTile::placeWarningSign(Level *level, int x, int y, int z, const std::wstring &line1)
{
	// Placed at the Lucky Block's own position - by the time we get here
	// the block has already been removed from the world, so this spot is
	// free.
	level->setTileAndData(x, y, z, Tile::standing_sign_Id, 0, Tile::UPDATE_ALL);
	shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
	shared_ptr<SignTileEntity> sign = dynamic_pointer_cast<SignTileEntity>(te);
	if (sign != nullptr)
	{
		wstring text = line1;
		sign->SetMessage(0, text);
	}
}

void LuckyBlockTile::spawnFallingBlock(Level *level, int x, int y, int z, int tileId, int data)
{
	// FallingTile only keeps falling if a real tile matching its id is
	// already sitting at its spawn position (it self-destructs on its very
	// first tick otherwise) - so we have to place the block first, exactly
	// like sand/gravel physics does internally, rather than conjuring the
	// entity in mid-air.
	level->setTileAndData(x, y, z, tileId, data, Tile::UPDATE_ALL);
	shared_ptr<FallingTile> falling = std::make_shared<FallingTile>(level, x + 0.5, y + 0.5, z + 0.5, tileId, data);
	level->addEntity(falling);
}

WeighedTreasureArray LuckyBlockTile::chestTreasureTable()
{
	// Built once and reused - flavor mixes a bit of "bonus chest" and a
	// bit of "village blacksmith" without literally copying either table.
	static WeighedTreasureArray table;
	static bool initialized = false;
	if (!initialized)
	{
		table = WeighedTreasureArray(12);
		table[0] = new WeighedTreasure(Item::diamond_Id, 0, 1, 3, 4);
		table[1] = new WeighedTreasure(Item::iron_ingot_Id, 0, 2, 5, 10);
		table[2] = new WeighedTreasure(Item::gold_ingot_Id, 0, 1, 4, 8);
		table[3] = new WeighedTreasure(Item::emerald_Id, 0, 1, 3, 6);
		table[4] = new WeighedTreasure(Item::bread_Id, 0, 2, 5, 12);
		table[5] = new WeighedTreasure(Item::apple_Id, 0, 2, 5, 12);
		table[6] = new WeighedTreasure(Item::iron_sword_Id, 0, 1, 1, 5);
		table[7] = new WeighedTreasure(Item::iron_pickaxe_Id, 0, 1, 1, 5);
		table[8] = new WeighedTreasure(Tile::obsidian_Id, 0, 2, 5, 5);
		table[9] = new WeighedTreasure(Tile::sapling_Id, 0, 1, 3, 6);
		table[10] = new WeighedTreasure(Item::saddle_Id, 0, 1, 1, 3);
		table[11] = new WeighedTreasure(Item::ender_pearl_Id, 0, 1, 2, 5);
		initialized = true;
	}
	return table;
}

void LuckyBlockTile::outcomeGoodLoot(Level *level, int x, int y, int z)
{
	if (level->random->nextInt(5) == 0)
	{
		// A plain, reliable find - no enchant needed.
		popResource(level, x, y, z, std::make_shared<ItemInstance>(Item::iron_sword, 1));
		return;
	}

	static const int pool[] = { Item::diamond_Id, Item::emerald_Id, Item::gold_ingot_Id, Item::iron_ingot_Id };
	int itemId = pool[level->random->nextInt(4)];
	int count = 1 + level->random->nextInt(3);
	dropItem(level, x, y, z, itemId, count);
}

void LuckyBlockTile::outcomeFoodLoot(Level *level, int x, int y, int z)
{
	if (level->random->nextInt(6) == 0)
	{
		dropItem(level, x, y, z, Item::golden_apple_Id, 1);
	}
	else
	{
		int count = 1 + level->random->nextInt(3);
		dropItem(level, x, y, z, Item::bread_Id, count);
		dropItem(level, x, y, z, Item::cooked_beef_Id, count);
	}
}

void LuckyBlockTile::outcomeJunkLoot(Level *level, int x, int y, int z)
{
	static const int pool[] = { Item::rotten_flesh_Id, Item::string_Id, Item::bone_Id, Item::gunpowder_Id, Item::stick_Id };
	int itemId = pool[level->random->nextInt(5)];
	int count = 1 + level->random->nextInt(4);
	dropItem(level, x, y, z, itemId, count);
}

void LuckyBlockTile::outcomeFriendlyMobs(Level *level, int x, int y, int z)
{
	int count = 1 + level->random->nextInt(3);
	for (int i = 0; i < count; i++)
	{
		double ox = x + 0.5 + (level->random->nextFloat() - 0.5f) * 1.5;
		double oz = z + 0.5 + (level->random->nextFloat() - 0.5f) * 1.5;

		shared_ptr<Animal> mob;
		switch (level->random->nextInt(4))
		{
		case 0: mob = std::make_shared<Chicken>(level); break;
		case 1: mob = std::make_shared<Cow>(level); break;
		case 2: mob = std::make_shared<Pig>(level); break;
		default: mob = std::make_shared<Sheep>(level); break;
		}
		mob->moveTo(ox, y + 1, oz, level->random->nextFloat() * 360.0f, 0);
		level->addEntity(mob);
	}
}

void LuckyBlockTile::outcomeHostileAmbush(Level *level, int x, int y, int z)
{
	int count = 1 + level->random->nextInt(3);
	for (int i = 0; i < count; i++)
	{
		double ox = x + 0.5 + (level->random->nextFloat() - 0.5f) * 2.0;
		double oz = z + 0.5 + (level->random->nextFloat() - 0.5f) * 2.0;

		shared_ptr<Mob> mob;
		wstring baseName;
		switch (level->random->nextInt(3))
		{
		case 0: mob = std::make_shared<Zombie>(level); baseName = L"Zombie"; break;
		case 1: mob = std::make_shared<Skeleton>(level); baseName = L"Skeleton"; break;
		default: mob = std::make_shared<Spider>(level); baseName = L"Spider"; break;
		}
		mob->moveTo(ox, y + 1, oz, level->random->nextFloat() * 360.0f, 0);

		// Flavor: no mechanical benefit, just a fun surprise sometimes.
		if (level->random->nextInt(5) == 0)
		{
			mob->setCustomName(L"Lucky " + baseName);
			mob->setCustomNameVisible(true);
		}

		level->addEntity(mob);
	}
}

void LuckyBlockTile::outcomeCreeperTrap(Level *level, int x, int y, int z)
{
	shared_ptr<Creeper> creeper = std::make_shared<Creeper>(level);
	creeper->moveTo(x + 0.5, y + 1, z + 0.5, level->random->nextFloat() * 360.0f, 0);
	level->addEntity(creeper);
}

void LuckyBlockTile::outcomeTntTrap(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	if (!level->newPrimedTntAllowed() || !app.GetGameHostOption(eGameHostOption_TNT))
	{
		// TNT disabled on this world - fall back to something less
		// disappointing than nothing happening.
		outcomeJunkLoot(level, x, y, z);
		return;
	}

	shared_ptr<PrimedTnt> tnt = std::make_shared<PrimedTnt>(level, x + 0.5, y + 0.5, z + 0.5, player);
	tnt->life = 40; // short fuse - this is meant to be a trap
	level->addEntity(tnt);
	level->playEntitySound(tnt, eSoundType_RANDOM_FUSE, 1, 1.0f);
}

void LuckyBlockTile::outcomeTntRain(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	if (!level->newPrimedTntAllowed() || !app.GetGameHostOption(eGameHostOption_TNT))
	{
		outcomeJunkLoot(level, x, y, z);
		return;
	}

	int count = 4 + level->random->nextInt(4); // 4-7 falling TNT
	for (int i = 0; i < count; i++)
	{
		double ox = x + 0.5 + (level->random->nextFloat() - 0.5f) * 5.0;
		double oz = z + 0.5 + (level->random->nextFloat() - 0.5f) * 5.0;
		double oy = y + 6 + level->random->nextInt(5);

		shared_ptr<PrimedTnt> tnt = std::make_shared<PrimedTnt>(level, ox, oy, oz, player);
		tnt->life = 50 + level->random->nextInt(30);
		level->addEntity(tnt);

		if (i == 0)
		{
			level->playEntitySound(tnt, eSoundType_RANDOM_FUSE, 1, 1.0f);
		}
	}
}

void LuckyBlockTile::outcomeGoodEffect(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	static const MobEffect *pool[] = { MobEffect::regeneration, MobEffect::movementSpeed, MobEffect::digSpeed, MobEffect::damageBoost };
	const MobEffect *effect = pool[level->random->nextInt(4)];
	player->addEffect(new MobEffectInstance(effect->id, 30 * SharedConstants::TICKS_PER_SECOND, 0));
}

void LuckyBlockTile::outcomeBadEffect(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	static const MobEffect *pool[] = { MobEffect::poison, MobEffect::movementSlowdown, MobEffect::weakness, MobEffect::confusion };
	const MobEffect *effect = pool[level->random->nextInt(4)];
	player->addEffect(new MobEffectInstance(effect->id, 15 * SharedConstants::TICKS_PER_SECOND, 0));
}

void LuckyBlockTile::outcomeExperienceBurst(Level *level, int x, int y, int z)
{
	popExperience(level, x, y, z, 15 + level->random->nextInt(30));
}

void LuckyBlockTile::outcomeLuckyGear(Level *level, int x, int y, int z)
{
	// Roughly "as good as you could legitimately get from an enchant
	// table/anvil" - a nice find, not a game-breaker.
	shared_ptr<ItemInstance> gear;
	switch (level->random->nextInt(8))
	{
	case 0:
		gear = makeEnchantedGear(level, Item::golden_sword, {
			{ Enchantment::damageBonus, 5 }, { Enchantment::fireAspect, 2 }, { Enchantment::mending, 1 } });
		break;
	case 1:
		gear = makeEnchantedGear(level, Item::golden_pickaxe, {
			{ Enchantment::diggingBonus, 5 }, { Enchantment::resourceBonus, 3 }, { Enchantment::mending, 1 } });
		break;
	case 2:
		gear = makeEnchantedGear(level, Item::golden_axe, {
			{ Enchantment::diggingBonus, 5 }, { Enchantment::damageBonus, 3 }, { Enchantment::mending, 1 } });
		break;
	case 3:
		gear = makeEnchantedGear(level, Item::golden_helmet, {
			{ Enchantment::allDamageProtection, 4 }, { Enchantment::thorns, 2 }, { Enchantment::mending, 1 } });
		break;
	case 4:
		gear = makeEnchantedGear(level, Item::golden_chestplate, {
			{ Enchantment::allDamageProtection, 4 }, { Enchantment::thorns, 2 }, { Enchantment::mending, 1 } });
		break;
	case 5:
		gear = makeEnchantedGear(level, Item::golden_leggings, {
			{ Enchantment::allDamageProtection, 4 }, { Enchantment::mending, 1 } });
		break;
	case 6:
		gear = makeEnchantedGear(level, Item::golden_boots, {
			{ Enchantment::allDamageProtection, 4 }, { Enchantment::frostWalker, 2 }, { Enchantment::mending, 1 } });
		break;
	default:
		gear = makeEnchantedGear(level, Item::golden_boots, {
			{ Enchantment::allDamageProtection, 4 }, { Enchantment::waterWalker, 1 }, { Enchantment::mending, 1 } });
		break;
	}
	popResource(level, x, y, z, gear);
}

void LuckyBlockTile::outcomeGodGear(Level *level, int x, int y, int z)
{
	shared_ptr<ItemInstance> gear;
	switch (level->random->nextInt(7))
	{
	case 0:
		gear = makeEnchantedGear(level, Item::diamond_sword, {
			{ Enchantment::damageBonus, 6 }, { Enchantment::fireAspect, 2 }, { Enchantment::knockback, 2 },
			{ Enchantment::digDurability, 4 }, { Enchantment::mending, 1 } });
		break;
	case 1:
		gear = makeEnchantedGear(level, Item::diamond_pickaxe, {
			{ Enchantment::diggingBonus, 6 }, { Enchantment::resourceBonus, 4 },
			{ Enchantment::digDurability, 4 }, { Enchantment::mending, 1 } });
		break;
	case 2:
		gear = makeEnchantedGear(level, Item::diamond_axe, {
			{ Enchantment::diggingBonus, 6 }, { Enchantment::damageBonus, 4 },
			{ Enchantment::digDurability, 4 }, { Enchantment::mending, 1 } });
		break;
	case 3:
		gear = makeEnchantedGear(level, Item::diamond_helmet, godArmorEnchants());
		break;
	case 4:
		gear = makeEnchantedGear(level, Item::diamond_chestplate, godArmorEnchants());
		break;
	case 5:
		gear = makeEnchantedGear(level, Item::diamond_leggings, godArmorEnchants());
		break;
	default:
		gear = makeEnchantedGear(level, Item::diamond_boots, godArmorEnchants());
		break;
	}
	popResource(level, x, y, z, gear);
}

void LuckyBlockTile::outcomeLuckyTrader(Level *level, int x, int y, int z)
{
	shared_ptr<Villager> trader = std::make_shared<Villager>(level);
	trader->moveTo(x + 0.5, y + 1, z + 0.5, level->random->nextFloat() * 360.0f, 0);
	trader->setCustomName(L"Lucky Trader");
	trader->setCustomNameVisible(true);
	trader->setPersistenceRequired();

	// Generous trades, but each is capped at a handful of uses (maxUses),
	// so he naturally stops being useful if you lean on him too hard -
	// there's no restock, so once every offer is exhausted he's done.
	MerchantRecipeList *offers = new MerchantRecipeList();
	offers->push_back(new MerchantRecipe(
		std::make_shared<ItemInstance>(Item::emerald, 4), nullptr,
		std::make_shared<ItemInstance>(Item::diamond, 1), 0, 3));
	offers->push_back(new MerchantRecipe(
		std::make_shared<ItemInstance>(Item::iron_ingot, 8), nullptr,
		std::make_shared<ItemInstance>(Item::emerald, 3), 0, 3));
	offers->push_back(new MerchantRecipe(
		std::make_shared<ItemInstance>(Item::emerald, 1), nullptr,
		std::make_shared<ItemInstance>(Item::golden_apple, 1), 0, 3));
	offers->push_back(new MerchantRecipe(
		std::make_shared<ItemInstance>(Item::emerald, 3), nullptr,
		std::make_shared<ItemInstance>(Item::ender_pearl, 4), 0, 3));
	trader->overrideOffers(offers);

	level->addEntity(trader);
}

void LuckyBlockTile::outcomeBob(Level *level, int x, int y, int z)
{
	shared_ptr<Zombie> bob = std::make_shared<Zombie>(level);
	bob->moveTo(x + 0.5, y + 1, z + 0.5, level->random->nextFloat() * 360.0f, 0);
	bob->setCustomName(L"Bob");
	bob->setCustomNameVisible(true);
	bob->setPersistenceRequired();

	bob->setEquippedSlot(0, makeEnchantedGear(level, Item::diamond_sword, {
		{ Enchantment::damageBonus, 5 }, { Enchantment::fireAspect, 2 } }));
	bob->setEquippedSlot(LivingEntity::SLOT_HELM, std::make_shared<ItemInstance>(Item::diamond_helmet, 1));
	bob->setEquippedSlot(LivingEntity::SLOT_CHEST, std::make_shared<ItemInstance>(Item::diamond_chestplate, 1));
	bob->setEquippedSlot(2, std::make_shared<ItemInstance>(Item::diamond_leggings, 1));
	bob->setEquippedSlot(1, std::make_shared<ItemInstance>(Item::diamond_boots, 1));

	// Modest drop chance per slot - beating Bob is a nice bonus, not a
	// guaranteed full diamond set every time.
	for (int slot = 0; slot <= 4; slot++)
	{
		bob->setDropChance(slot, 0.15f);
	}

	level->addEntity(bob);
}

void LuckyBlockTile::outcomeTom(Level *level, int x, int y, int z)
{
	shared_ptr<Zombie> tom = std::make_shared<Zombie>(level);
	tom->moveTo(x + 0.5, y + 1, z + 0.5, level->random->nextFloat() * 360.0f, 0);
	tom->setCustomName(L"Tom");
	tom->setCustomNameVisible(true);
	tom->setPersistenceRequired();

	tom->setEquippedSlot(0, makeEnchantedGear(level, Item::golden_sword, {
		{ Enchantment::damageBonus, 5 }, { Enchantment::knockback, 2 } }));
	tom->setEquippedSlot(LivingEntity::SLOT_HELM, makeEnchantedGear(level, Item::golden_helmet, {
		{ Enchantment::allDamageProtection, 4 } }));
	tom->setEquippedSlot(LivingEntity::SLOT_CHEST, makeEnchantedGear(level, Item::golden_chestplate, {
		{ Enchantment::allDamageProtection, 4 } }));
	tom->setEquippedSlot(2, makeEnchantedGear(level, Item::golden_leggings, {
		{ Enchantment::allDamageProtection, 4 } }));
	tom->setEquippedSlot(1, makeEnchantedGear(level, Item::golden_boots, {
		{ Enchantment::allDamageProtection, 4 } }));

	for (int slot = 0; slot <= 4; slot++)
	{
		tom->setDropChance(slot, 0.15f);
	}

	level->addEntity(tom);
}

void LuckyBlockTile::outcomeAnvilTrap(Level *level, int x, int y, int z)
{
	placeWarningSign(level, x, y, z, L"Look Up");

	// Directly overhead, staggered heights so the second lands just after
	// the first rather than both competing for the same falling slot.
	spawnFallingBlock(level, x, y + 5, z, Tile::anvil_Id, 0);
	spawnFallingBlock(level, x, y + 7, z, Tile::anvil_Id, 0);
}

void LuckyBlockTile::outcomeLavaTrap(Level *level, int x, int y, int z)
{
	placeWarningSign(level, x, y, z, L"Look Up");
	// flowing_lava (not the static "lava" variant) is what actually spreads
	// and flows downward, same as a bucket pour.
	level->setTileAndData(x, y + 4, z, Tile::flowing_lava_Id, 0, Tile::UPDATE_ALL);
}

void LuckyBlockTile::outcomeWitchSwarm(Level *level, int x, int y, int z)
{
	shared_ptr<Witch> witch = std::make_shared<Witch>(level);
	witch->moveTo(x + 0.5, y + 1, z + 0.5, level->random->nextFloat() * 360.0f, 0);
	level->addEntity(witch);

	int batCount = 4 + level->random->nextInt(4);
	for (int i = 0; i < batCount; i++)
	{
		double ox = x + 0.5 + (level->random->nextFloat() - 0.5f) * 3.0;
		double oz = z + 0.5 + (level->random->nextFloat() - 0.5f) * 3.0;
		shared_ptr<Bat> bat = std::make_shared<Bat>(level);
		bat->moveTo(ox, y + 1 + level->random->nextInt(3), oz, level->random->nextFloat() * 360.0f, 0);
		level->addEntity(bat);
	}
}

void LuckyBlockTile::outcomeChargedCreeper(Level *level, int x, int y, int z)
{
	shared_ptr<LightningBolt> bolt = std::make_shared<LightningBolt>(level, x + 0.5, y + 1.0, z + 0.5);
	level->addGlobalEntity(bolt);

	shared_ptr<Creeper> creeper = std::make_shared<Creeper>(level);
	creeper->moveTo(x + 0.5, y + 1, z + 0.5, level->random->nextFloat() * 360.0f, 0);
	creeper->thunderHit(bolt.get());
	level->addEntity(creeper);
}

void LuckyBlockTile::outcomeNightHorde(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	level->setDayTime(13000);

	player->inventory->add(std::make_shared<ItemInstance>(Item::wooden_sword, 1));
	player->addEffect(new MobEffectInstance(MobEffect::blindness->id, 10 * SharedConstants::TICKS_PER_SECOND, 0));
	player->addEffect(new MobEffectInstance(MobEffect::movementSlowdown->id, 10 * SharedConstants::TICKS_PER_SECOND, 1));

	int count = 6 + level->random->nextInt(5);
	for (int i = 0; i < count; i++)
	{
		double angle = level->random->nextFloat() * 6.2831853f;
		double dist = 4.0 + level->random->nextFloat() * 4.0;
		double ox = x + 0.5 + cos(angle) * dist;
		double oz = z + 0.5 + sin(angle) * dist;

		shared_ptr<Mob> mob;
		switch (level->random->nextInt(3))
		{
		case 0: mob = std::make_shared<Zombie>(level); break;
		case 1: mob = std::make_shared<Skeleton>(level); break;
		default: mob = std::make_shared<Spider>(level); break;
		}
		mob->moveTo(ox, y + 1, oz, level->random->nextFloat() * 360.0f, 0);
		level->addEntity(mob);
	}
}

void LuckyBlockTile::outcomeHugeSlime(Level *level, int x, int y, int z)
{
	shared_ptr<Slime> slime = std::make_shared<Slime>(level);
	slime->setSize(4); // biggest classic slime size
	slime->moveTo(x + 0.5, y + 1, z + 0.5, level->random->nextFloat() * 360.0f, 0);
	level->addEntity(slime);
}

void LuckyBlockTile::outcomeHugeMagmaCube(Level *level, int x, int y, int z)
{
	shared_ptr<LavaSlime> magmaCube = std::make_shared<LavaSlime>(level);
	magmaCube->setSize(4);
	magmaCube->moveTo(x + 0.5, y + 1, z + 0.5, level->random->nextFloat() * 360.0f, 0);
	level->addEntity(magmaCube);
}

void LuckyBlockTile::outcomeIronArmorDrop(Level *level, int x, int y, int z)
{
	Item *armorPool[] = { Item::iron_helmet, Item::iron_chestplate, Item::iron_leggings, Item::iron_boots };
	int pieces = 2 + level->random->nextInt(2); // 2-3 pieces
	bool used[4] = { false, false, false, false };
	int given = 0;
	while (given < pieces)
	{
		int idx = level->random->nextInt(4);
		if (used[idx]) continue;
		used[idx] = true;
		popResource(level, x, y, z, std::make_shared<ItemInstance>(armorPool[idx], 1));
		given++;
	}
}

void LuckyBlockTile::outcomeSponge(Level *level, int x, int y, int z)
{
	popResource(level, x, y, z, std::make_shared<ItemInstance>(Tile::sponge, 1));
}

void LuckyBlockTile::outcomeEnderChests(Level *level, int x, int y, int z)
{
	popResource(level, x, y, z, std::make_shared<ItemInstance>(Tile::enderChest, 2));
}

void LuckyBlockTile::outcomeMrRainbows(Level *level, int x, int y, int z)
{
	// Homage to the original Lucky Block mod's "Mr Rainbows" easter egg.
	// We don't have a true rainbow-cycling model/shader available here,
	// so this is a normal Chicken with a colorful custom name instead.
	shared_ptr<Chicken> mrRainbows = std::make_shared<Chicken>(level);
	mrRainbows->moveTo(x + 0.5, y + 1, z + 0.5, level->random->nextFloat() * 360.0f, 0);
	mrRainbows->setCustomName(L"Mr. Rainbows");
	mrRainbows->setCustomNameVisible(true);
	mrRainbows->setPersistenceRequired();
	level->addEntity(mrRainbows);
}

void LuckyBlockTile::outcomeRainbowWool(Level *level, int x, int y, int z)
{
	// True per-frame rainbow-cycling isn't practical for a static item, so
	// this drops wool in a random dye color each time as a nod to the idea.
	int color = level->random->nextInt(16);
	int count = 1 + level->random->nextInt(3);
	popResource(level, x, y, z, std::make_shared<ItemInstance>(Tile::wool, count, color));
}

void LuckyBlockTile::outcomePrisonTrap(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	// A 3x3x3 obsidian box around wherever the player is currently
	// standing (not necessarily the block's own position), with one glass
	// window, the bottom two layers of the player's own column filled
	// with water, and the top sealed solid.
	int px = Mth::floor(player->x);
	int py = Mth::floor(player->y);
	int pz = Mth::floor(player->z);

	for (int dy = 0; dy < 3; dy++)
	{
		int wy = py + dy;
		for (int dx = -1; dx <= 1; dx++)
		{
			for (int dz = -1; dz <= 1; dz++)
			{
				int wx = px + dx;
				int wz = pz + dz;
				bool isCenter = (dx == 0 && dz == 0);

				if (isCenter)
				{
					if (dy == 2)
					{
						// Seal the top solid - the only ways out are down
						// through the natural terrain below, or by
						// breaking the glass window.
						level->setTileAndData(wx, wy, wz, Tile::obsidian->id, 0, Tile::UPDATE_ALL);
					}
					else
					{
						// Static (non-flowing) water - we want it to stay
						// put and actually encase the player, not drain
						// away.
						level->setTileAndData(wx, wy, wz, Tile::water_Id, 0, Tile::UPDATE_ALL);
					}
					continue;
				}

				bool isWindow = (dy == 1 && dx == 0 && dz == -1);
				int shellTileId = isWindow ? Tile::glass->id : Tile::obsidian->id;
				level->setTileAndData(wx, wy, wz, shellTileId, 0, Tile::UPDATE_ALL);
			}
		}
	}
}

void LuckyBlockTile::outcomeIgnite(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	player->setOnFire(5 + level->random->nextInt(4));
}

void LuckyBlockTile::outcomeChickenShower(Level *level, int x, int y, int z)
{
	int count = 5 + level->random->nextInt(4);
	for (int i = 0; i < count; i++)
	{
		double ox = x + 0.5 + (level->random->nextFloat() - 0.5f) * 4.0;
		double oz = z + 0.5 + (level->random->nextFloat() - 0.5f) * 4.0;
		double oy = y + 4 + level->random->nextInt(4);

		shared_ptr<Chicken> chick = std::make_shared<Chicken>(level);
		chick->moveTo(ox, oy, oz, level->random->nextFloat() * 360.0f, 0);
		level->addEntity(chick);
	}
}

void LuckyBlockTile::outcomeRandomWood(Level *level, int x, int y, int z)
{
	// Oak/birch/spruce/jungle logs share Tile::treeTrunk; acacia/dark oak
	// split off into Tile::log2 in this engine, same as vanilla did.
	int type = level->random->nextInt(6);
	int count = 2 + level->random->nextInt(3); // 2-4 logs
	Tile *logTile;
	int data;
	switch (type)
	{
	case 0: logTile = Tile::treeTrunk; data = 0; break; // oak
	case 1: logTile = Tile::treeTrunk; data = TreeTile::BIRCH_TRUNK; break;
	case 2: logTile = Tile::treeTrunk; data = TreeTile::SPRUCE_TRUNK; break;
	case 3: logTile = Tile::treeTrunk; data = TreeTile::JUNGLE_TRUNK; break;
	case 4: logTile = Tile::log2; data = TreeTile2::ACACIA_TRUNK; break;
	default: logTile = Tile::log2; data = TreeTile2::DARK_TRUNK; break;
	}
	popResource(level, x, y, z, std::make_shared<ItemInstance>(logTile, count, data));
}

void LuckyBlockTile::outcomeChestLoot(Level *level, int x, int y, int z)
{
	level->setTileAndData(x, y, z, Tile::chest_Id, 0, Tile::UPDATE_ALL);
	shared_ptr<ChestTileEntity> chest = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(x, y, z));
	if (chest != nullptr)
	{
		WeighedTreasure::addChestItems(level->random, chestTreasureTable(), chest, 4 + level->random->nextInt(4));
	}
}

void LuckyBlockTile::outcomeRandomEnchantBook(Level *level, int x, int y, int z)
{
	// Reuses the same "random enchanted book" generator vanilla loot
	// tables use - one random enchantment at a random valid level.
	shared_ptr<ItemInstance> book = Item::enchanted_book->createForRandomLoot(level->random);
	popResource(level, x, y, z, book);
}

void LuckyBlockTile::outcomeLuckyPlatform(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	int px = Mth::floor(player->x);
	int py = Mth::floor(player->y);
	int pz = Mth::floor(player->z);

	// 3x3 sandstone floor beneath wherever the player currently is.
	for (int dx = -1; dx <= 1; dx++)
	{
		for (int dz = -1; dz <= 1; dz++)
		{
			level->setTileAndData(px + dx, py - 1, pz + dz, Tile::sandStone->id, 0, Tile::UPDATE_ALL);
		}
	}

	// A fresh Lucky Block planted at the middle of each of the 4 sides.
	level->setTileAndData(px, py, pz - 1, Tile::lucky_block->id, 0, Tile::UPDATE_ALL);
	level->setTileAndData(px, py, pz + 1, Tile::lucky_block->id, 0, Tile::UPDATE_ALL);
	level->setTileAndData(px - 1, py, pz, Tile::lucky_block->id, 0, Tile::UPDATE_ALL);
	level->setTileAndData(px + 1, py, pz, Tile::lucky_block->id, 0, Tile::UPDATE_ALL);

	// Make sure the player actually ends up standing on the platform,
	// rather than possibly clipping into whatever we just placed.
	player->moveTo(px + 0.5, py, pz + 0.5, player->yRot, player->xRot);
}

void LuckyBlockTile::outcomeMusicDisc(Level *level, int x, int y, int z)
{
	static Item *pool[] = {
		Item::record_01, Item::record_02, Item::record_03, Item::record_04,
		Item::record_05, Item::record_06, Item::record_07, Item::record_08,
		Item::record_09, Item::record_10, Item::record_11, Item::record_12,
	};
	Item *disc = pool[level->random->nextInt(12)];
	popResource(level, x, y, z, std::make_shared<ItemInstance>(disc, 1));
}

void LuckyBlockTile::outcomeWeedWacker(Level *level, int x, int y, int z)
{
	// A plain stick doesn't normally pass isEnchantable(), but
	// ItemInstance::enchant() writes the NBT directly with no such check,
	// so this works exactly the same way God Gear's illegal stacking does.
	shared_ptr<ItemInstance> weedWacker = std::make_shared<ItemInstance>(Item::stick, 1);
	weedWacker->enchant(Enchantment::knockback, 10);
	weedWacker->setHoverName(L"Weed Wacker");
	popResource(level, x, y, z, weedWacker);
}

void LuckyBlockTile::outcomeMlgWaterBucket(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
    // Give the player a water bucket. If the inventory is full, toss it at their feet instead of
    // silently losing it (the launch would otherwise leave them with nothing to clutch with).
    shared_ptr<ItemInstance> bucket = std::make_shared<ItemInstance>(Item::water_bucket, 1);
    if (!player->inventory->add(bucket))
    {
        player->drop(bucket);
    }

    player->xd = 0.0;
    player->zd = 0.0;

    player->yd = 5.0;

    // neoLegacy FIX - the "MLG does nothing sometimes" bug. A player's movement is driven by the
    // client, so changing xd/yd on the server only takes effect if the client is told about it.
    // That happens through a motion packet, which TrackedEntity only sends while hurtMarked is set.
    // This outcome never set it, so the launch only worked on the occasional tick when something else
    // (another hit, knockback, ...) happened to flag the player; otherwise the server changed a
    // velocity the client never received and nothing visibly happened. Flag it explicitly.
    player->hurtMarked = true;

    player->sendMessage(L"MLG");
}

void LuckyBlockTile::outcomeGodSword(Level *level, int x, int y, int z)
{
	// Extremely rare (see the weight table), so the numbers themselves
	// don't need to be as absurd as before - the rarity is what makes it
	// special. Still clearly a cut above anything obtainable normally.
	shared_ptr<ItemInstance> sword = std::make_shared<ItemInstance>(Item::wooden_sword, 1);
	sword->enchant(Enchantment::damageBonus, 7);
	sword->enchant(Enchantment::fireAspect, 5);
	sword->enchant(Enchantment::knockback, 5);
	sword->enchant(Enchantment::digDurability, 7);
    sword->enchant(Enchantment::lifesteal, 3);


	sword->setHoverName(L"Godly Wooden Sword");
	popResource(level, x, y, z, sword);
}

void LuckyBlockTile::outcomeFishingRodOfLegends(Level *level, int x, int y, int z)
{
	shared_ptr<ItemInstance> rod = makeEnchantedGear(level, Item::fishing_rod, {
		{ Enchantment::lure, 5 }, { Enchantment::luckOfTheSea, 5 }, { Enchantment::mending, 1 } });
	rod->setHoverName(L"The Fishing Rod of Legends");
	popResource(level, x, y, z, rod);
}

void LuckyBlockTile::outcomeConcrete(Level *level, int x, int y, int z)
{
	// A stack of concrete in one random color (Tile::concrete stores the color 0-15 as its data
	// value, which is also the item aux value).
	int color = level->random->nextInt(16);
	int count = 8 + level->random->nextInt(9);	// 8-16 blocks
	popResource(level, x, y, z, std::make_shared<ItemInstance>(Tile::concrete, count, color));
}

void LuckyBlockTile::outcomeBasicSupplies(Level *level, int x, int y, int z)
{
	// Plain, everyday useful stuff - not exciting, just handy.
	switch (level->random->nextInt(6))
	{
	case 0:
		dropItem(level, x, y, z, Item::coal->id, 3 + level->random->nextInt(4));
		break;
	case 1:
		dropItem(level, x, y, z, Item::wheat->id, 3 + level->random->nextInt(4));
		break;
	case 2:
		dropItem(level, x, y, z, Item::leather->id, 1 + level->random->nextInt(3));
		break;
	case 3:
		popResource(level, x, y, z, std::make_shared<ItemInstance>(Item::flint_and_steel, 1));
		break;
	case 4:
		popResource(level, x, y, z, std::make_shared<ItemInstance>(Item::milk_bucket, 1));
		break;
	default:
		popResource(level, x, y, z, std::make_shared<ItemInstance>(Item::saddle, 1));
		break;
	}
}

void LuckyBlockTile::outcomeSmallHeal(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	player->addEffect(new MobEffectInstance(MobEffect::heal->id, 1, 0));
	player->addEffect(new MobEffectInstance(MobEffect::saturation->id, 5 * SharedConstants::TICKS_PER_SECOND, 0));
}

void LuckyBlockTile::outcomeInstantHarm(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	player->addEffect(new MobEffectInstance(MobEffect::harm->id, 1, 0));
}

void LuckyBlockTile::outcomeWither(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	// Kept short - Wither is a real damage-over-time effect, this is meant
	// to sting, not be a death sentence.
	player->addEffect(new MobEffectInstance(MobEffect::wither->id, 5 * SharedConstants::TICKS_PER_SECOND, 0));
}

void LuckyBlockTile::outcomeMiningFatigue(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	player->addEffect(new MobEffectInstance(MobEffect::digSlowdown->id, 20 * SharedConstants::TICKS_PER_SECOND, 0));
}

void LuckyBlockTile::outcomeXpDrain(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	int loss = 1 + level->random->nextInt(3);
	if (loss > player->experienceLevel)
	{
		loss = player->experienceLevel;
	}
	if (loss > 0)
	{
		player->giveExperienceLevels(-loss);
	}
}

void LuckyBlockTile::outcomeHungerEffect(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	player->addEffect(new MobEffectInstance(MobEffect::hunger->id, 15 * SharedConstants::TICKS_PER_SECOND, 0));
}