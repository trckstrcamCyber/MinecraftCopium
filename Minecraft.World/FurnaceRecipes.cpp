#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "Tile.h"
#include "FurnaceRecipes.h"
#include "SmoothStoneBrickTile.h"
#include "BlackStoneTile.h"

FurnaceRecipes *FurnaceRecipes::instance = nullptr;

void FurnaceRecipes::staticCtor()
{
	FurnaceRecipes::instance = new FurnaceRecipes();
}

FurnaceRecipes *FurnaceRecipes::getInstance()
{
	return instance;
}

FurnaceRecipes::FurnaceRecipes()
{
	addFurnaceRecipy(Tile::iron_ore_Id, new ItemInstance(Item::iron_ingot), .7f);
	addFurnaceRecipy(Tile::copper_ore_Id, new ItemInstance(Item::copper_ingot), .7f);
	// neoLegacy - Netherite tier (2026-11-26): ancient debris smelts to netherite scrap (Java XP 2.0).
	addFurnaceRecipy(Tile::ancient_debris_Id, new ItemInstance(Item::netherite_scrap), 2.0f);
	addFurnaceRecipy(new ItemInstance(Item::raw_copper, 1, 0), new ItemInstance(Item::copper_ingot), .7f);

	// neoLegacy - Copper Age (Phase 2): copper gear recycles to a copper nugget (0.1 XP, as in jar1201's copper_nugget_from_smelting.json, which also lists the horse armor); raw rose gold smelts to a rose gold ingot (1.0 XP, as in rosegold's rose_gold_ingot_from_smelting.json).
	addFurnaceRecipy(Item::copper_sword_Id, new ItemInstance(Item::copper_nugget), .1f);
	addFurnaceRecipy(Item::copper_pickaxe_Id, new ItemInstance(Item::copper_nugget), .1f);
	addFurnaceRecipy(Item::copper_axe_Id, new ItemInstance(Item::copper_nugget), .1f);
	addFurnaceRecipy(Item::copper_shovel_Id, new ItemInstance(Item::copper_nugget), .1f);
	addFurnaceRecipy(Item::copper_hoe_Id, new ItemInstance(Item::copper_nugget), .1f);
	addFurnaceRecipy(Item::copper_helmet_Id, new ItemInstance(Item::copper_nugget), .1f);
	addFurnaceRecipy(Item::copper_chestplate_Id, new ItemInstance(Item::copper_nugget), .1f);
	addFurnaceRecipy(Item::copper_leggings_Id, new ItemInstance(Item::copper_nugget), .1f);
	addFurnaceRecipy(Item::copper_boots_Id, new ItemInstance(Item::copper_nugget), .1f);
	addFurnaceRecipy(Item::copper_horse_armor_Id, new ItemInstance(Item::copper_nugget), .1f);

	// neoLegacy - Iron additions (2026-11-22): iron gear recycles to an
	// iron nugget (0.1 XP), mirroring the copper gear above. Java 26.3
	// data/minecraft/recipe/iron_nugget_from_smelting.json lists exactly
	// these (plus iron_spear and iron_nautilus_armor, which do not exist
	// in this port); chainmail smelting to iron nuggets is Java-correct
	// and has no copper equivalent to conflict with.
	addFurnaceRecipy(Item::iron_sword_Id, new ItemInstance(Item::iron_nugget), .1f);
	addFurnaceRecipy(Item::iron_pickaxe_Id, new ItemInstance(Item::iron_nugget), .1f);
	addFurnaceRecipy(Item::iron_axe_Id, new ItemInstance(Item::iron_nugget), .1f);
	addFurnaceRecipy(Item::iron_shovel_Id, new ItemInstance(Item::iron_nugget), .1f);
	addFurnaceRecipy(Item::iron_hoe_Id, new ItemInstance(Item::iron_nugget), .1f);
	addFurnaceRecipy(Item::iron_helmet_Id, new ItemInstance(Item::iron_nugget), .1f);
	addFurnaceRecipy(Item::iron_chestplate_Id, new ItemInstance(Item::iron_nugget), .1f);
	addFurnaceRecipy(Item::iron_leggings_Id, new ItemInstance(Item::iron_nugget), .1f);
	addFurnaceRecipy(Item::iron_boots_Id, new ItemInstance(Item::iron_nugget), .1f);
	addFurnaceRecipy(Item::iron_horse_armor_Id, new ItemInstance(Item::iron_nugget), .1f);
	addFurnaceRecipy(Item::chainmail_helmet_Id, new ItemInstance(Item::iron_nugget), .1f);
	addFurnaceRecipy(Item::chainmail_chestplate_Id, new ItemInstance(Item::iron_nugget), .1f);
	addFurnaceRecipy(Item::chainmail_leggings_Id, new ItemInstance(Item::iron_nugget), .1f);
	addFurnaceRecipy(Item::chainmail_boots_Id, new ItemInstance(Item::iron_nugget), .1f);
	addFurnaceRecipy(new ItemInstance(Item::raw_rose_gold, 1, 0), new ItemInstance(Item::rose_gold_ingot), 1);
	addFurnaceRecipy(Tile::gold_ore_Id, new ItemInstance(Item::gold_ingot), 1);
	addFurnaceRecipy(Tile::diamond_ore_Id, new ItemInstance(Item::diamond), 1);
	addFurnaceRecipy(Tile::sand_Id, new ItemInstance(Tile::glass), .1f);
	addFurnaceRecipy(Item::chorus_fruit_Id, new ItemInstance(Item::chorus_fruit_popped), .1f);
	addFurnaceRecipy(Item::porkchop_Id, new ItemInstance(Item::cooked_porkchop), .35f);
	addFurnaceRecipy(Item::beef_Id, new ItemInstance(Item::cooked_beef), .35f);
	addFurnaceRecipy(Item::rabbit_Id, new ItemInstance(Item::cooked_rabbit), .35f);
	addFurnaceRecipy(Item::mutton_Id, new ItemInstance(Item::cooked_mutton), .35f);
	addFurnaceRecipy(Item::chicken_Id, new ItemInstance(Item::cooked_chicken), .35f);
	addFurnaceRecipy(Item::fish_Id, new ItemInstance(Item::cooked_fish), .35f);
	addFurnaceRecipy(new ItemInstance(Item::raw_fish, 1, 1), new ItemInstance(Item::cooked_fish, 1, 1), .35f); // salmon

	addFurnaceRecipy(Tile::cobblestone_Id, new ItemInstance(Tile::stone, 1, 0), .1f);
	addFurnaceRecipy(Tile::stonebrick_Id, new ItemInstance(Tile::stoneBrick, 1 , SmoothStoneBrickTile::TYPE_CRACKED), .1f);
	addFurnaceRecipy(Item::clay_Id, new ItemInstance(Item::brick), .3f);
	addFurnaceRecipy(Tile::clay_Id, new ItemInstance(Tile::clayHardened), .35f);
	addFurnaceRecipy(Tile::cactus_Id, new ItemInstance(Item::dye, 1, DyePowderItem::GREEN), .2f);
    addFurnaceRecipy(Item::rotten_flesh_Id, new ItemInstance(Item::leather), 1);
	addFurnaceRecipy(Tile::log_Id, new ItemInstance(Item::coal, 1, CoalItem::CHAR_COAL), .15f);
	addFurnaceRecipy(Tile::emerald_ore_Id, new ItemInstance(Item::emerald), 1);
	addFurnaceRecipy(Item::potato_Id, new ItemInstance(Item::baked_potato), .35f);
	addFurnaceRecipy(Tile::netherrack_Id, new ItemInstance(Item::netherbrick), .1f);
	addFurnaceRecipy(new ItemInstance(Tile::sponge, 1, 1), new ItemInstance(Tile::sponge, 1, 0), .15f);

    addFurnaceRecipy(
        new ItemInstance(Tile::blackstone, 1, BlackStoneTile::POLISHED_BLACKSTONE_BRICKS),
        new ItemInstance(Tile::blackstone, 1, BlackStoneTile::CRACKED_POLISHED_BLACKSTONE_BRICKS),
        .1f);

	// special silk touch related recipes:
	addFurnaceRecipy(Tile::coal_ore_Id, new ItemInstance(Item::coal), .1f);
	addFurnaceRecipy(Tile::redstone_ore_Id, new ItemInstance(Item::redstone), .7f);
	addFurnaceRecipy(Tile::lapis_ore_Id, new ItemInstance(Item::dye, 1, DyePowderItem::BLUE), .2f);
	addFurnaceRecipy(Tile::quartz_ore_Id, new ItemInstance(Item::nether_quartz), .2f);

	addFurnaceRecipy(Tile::log2_Id, new ItemInstance(Item::coal, 1, CoalItem::CHAR_COAL), .15f);

	// neoLegacy - stripped logs smelt into charcoal like the plain logs
	addFurnaceRecipy(Tile::stripped_log_Id, new ItemInstance(Item::coal, 1, CoalItem::CHAR_COAL), .15f);
	addFurnaceRecipy(Tile::stripped_log2_Id, new ItemInstance(Item::coal, 1, CoalItem::CHAR_COAL), .15f);
}

void FurnaceRecipes::addFurnaceRecipy(int itemId, ItemInstance *result, float value)
{
	//recipies->put(itemId, result);
	recipies[itemId]=result;
	recipeValue[result->id] = value;
}

void FurnaceRecipes::addFurnaceRecipy(ItemInstance* input, ItemInstance* result, float value)
{
	int key = input->id | (input->getAuxValue() << 12);
	recipies[key] = result;
	recipeValue[result->id] = value;
}

bool FurnaceRecipes::isFurnaceItem(int itemId, int data)
{
	int key = itemId | (data << 12);
	if (recipies.find(key) != recipies.end()) return true;
	return recipies.find(itemId) != recipies.end();
}

ItemInstance* FurnaceRecipes::getResult(int itemId, int data)
{
	int key = itemId | (data << 12);
	auto it = recipies.find(key);
	if (it != recipies.end()) return it->second;
	it = recipies.find(itemId);
	if (it != recipies.end()) return it->second;
	return nullptr;
}

unordered_map<int, ItemInstance *> *FurnaceRecipes::getRecipies()
{
	return &recipies;
}

float FurnaceRecipes::getRecipeValue(int itemId)
{
    auto it = recipeValue.find(itemId);
    if (it != recipeValue.end())
	{
		return it->second;
	}
	return 0.0f;
}