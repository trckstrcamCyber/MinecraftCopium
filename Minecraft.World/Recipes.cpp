#include "stdafx.h"
#include "Container.h"
#include "AbstractContainerMenu.h"
#include "CraftingContainer.h"
#include "CoalItem.h"
#include "Item.h"
#include "ItemInstance.h"
#include "StoneSlabTile2.h"
#include "AmethystTile.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.crafting.h"
#include "BannerPatternRecipe.h"
#include "DyePowderItem.h"
#include "ObserverTile.h"
#include "DrillTile.h"
#include "ProcessorTile.h"
#include "GrindstoneTile.h"
#include "PistonBaseTile.h"
#include "HopperTile.h"
#include "ArcadeMachineTile.h"

Recipes *Recipes::instance = nullptr;
ArmorRecipes *Recipes::pArmorRecipes=nullptr;
ClothDyeRecipes *Recipes::pClothDyeRecipes=nullptr;
FoodRecipies *Recipes::pFoodRecipies=nullptr;
OreRecipies *Recipes::pOreRecipies=nullptr;
StructureRecipies *Recipes::pStructureRecipies=nullptr;
ToolRecipies *Recipes::pToolRecipies=nullptr;
WeaponRecipies *Recipes::pWeaponRecipies=nullptr;
FireworksRecipe *Recipes::pFireworksRecipes=nullptr;
BannerPatternRecipe *Recipes::pBannerPatternRecipe=nullptr;	// neoLegacy

void Recipes::staticCtor()
{
	Recipes::instance = new Recipes();

}

void Recipes::_init()
{
	// 4J Jev: instance = new Recipes();
	recipies = new RecipyList();

	pArmorRecipes = new ArmorRecipes;
	pClothDyeRecipes = new ClothDyeRecipes;
	pFoodRecipies = new FoodRecipies;
	pOreRecipies = new OreRecipies;
	pStructureRecipies = new StructureRecipies;
	pToolRecipies = new ToolRecipies;
	pWeaponRecipies = new WeaponRecipies;

	// 4J Stu - These just don't work with our crafting menu
	//recipies->push_back(new ArmorDyeRecipe());
	//recipies->add(new MapCloningRecipe());
	//recipies->add(new MapExtendingRecipe());
	//recipies->add(new FireworksRecipe());
	pFireworksRecipes = new FireworksRecipe();
	pBannerPatternRecipe = new BannerPatternRecipe();	// neoLegacy - not in the list either: asked directly by getItemFor
}

void Recipes::_compileRecipes()
{
	// neoLegacy - plain banners: 6 wool of one color + 1 stick. Wool aux is
	// standard (0 = white); banner aux is dye order, so banner aux = 15 - wool aux.
	for (int i = 0; i < 16; i++) {
		addShapedRecipy(new ItemInstance(Tile::standing_banner_Id, 1, 15 - i), //
			L"sssczcig",
			L"###", //
			L"###", //
			L" I ", //

			L'#', new ItemInstance(Tile::wool_Id, 1, i),
			L'I', Item::stick,
			L'B');
	}

	addShapedRecipy(new ItemInstance(Tile::wood, 4, 0), //
		L"sczg",
		L"#", //

		L'#', new ItemInstance(Tile::treeTrunk, 1, 0),
		L'S');

	// TU9 - adding coloured wood
	addShapedRecipy(new ItemInstance(Tile::wood, 4, TreeTile::BIRCH_TRUNK), //
		L"sczg",
		L"#", //

		L'#', new ItemInstance(Tile::treeTrunk, 1, TreeTile::BIRCH_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::wood, 4, TreeTile::SPRUCE_TRUNK), //
		L"sczg",
		L"#", //

		L'#', new ItemInstance(Tile::treeTrunk, 1, TreeTile::SPRUCE_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::wood, 4, TreeTile::JUNGLE_TRUNK), //
		L"sczg",
		L"#", //

		L'#', new ItemInstance(Tile::treeTrunk, 1, TreeTile::JUNGLE_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::wood, 4, TreeTile::ACACIA_TRUNK), //
		L"sczg",
		L"#", //

		L'#', new ItemInstance(Tile::log2, 1, TreeTile2::ACACIA_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::wood, 4, TreeTile::DARK_TRUNK), //
		L"sczg",
		L"#", //

		L'#', new ItemInstance(Tile::log2, 1, TreeTile2::DARK_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Item::stick, 4), //
		L"ssctg",
		L"#", //
		L"#", //

		L'#', Tile::wood,
		L'S');

	pToolRecipies->addRecipes(this);
	pFoodRecipies->addRecipes(this);
	pStructureRecipies->addRecipes(this);


	// 4J-PB - changing the order to the way we want to have things in the crafting menu
	// bed
	// neoLegacy - Fancy Beds: three wool of ONE color over three planks gives a bed of that color (one item id, aux = color,
	// wool order white..black). The old recipe accepted any wool and always made the same bed.
	for (int bedColor = 0; bedColor < 16; bedColor++)
	{
		addShapedRecipy(new ItemInstance(Item::bed, 1, bedColor), //
			L"ssczctg",
			L"###", //
			L"XXX", //
			L'#', new ItemInstance(Tile::wool, 1, bedColor), L'X', Tile::wood,
			L'S');
	}

	// neoLegacy - Straw Bed: three hay bales in a row give four straw beds (aux 16 of the bed item; a one-row recipe works in any row,
	// including the middle one)
	addShapedRecipy(new ItemInstance(Item::bed, 4, 16), //
		L"sctg",
		L"###", //
		L'#', Tile::hayBlock,
		L'S');

    addShapedRecipy(new ItemInstance(Tile::complexTable, 1), //
        L"sssctcicig",
        L"BBB", //
        L"D D", //
        L"###", //

        L'#', Tile::wood, L'B', Item::stick, L'D', Item::iron_ingot,
        L'S');

	addShapedRecipy(new ItemInstance(Tile::enchantTable, 1), //
		L"sssctcicig",
		L" B ", //
		L"D#D", //
		L"###", //

		L'#', Tile::obsidian, L'B', Item::book, L'D', Item::diamond,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::anvil, 1), //
		L"sssctcig",
		L"III", //
		L" i ", //
		L"iii", //

		L'I', Tile::ironBlock, L'i', Item::iron_ingot,
		L'S');

	// 4J Stu - Reordered for crafting menu
	addShapedRecipy(new ItemInstance(Tile::ladder, 3), //
		L"ssscig",
		L"# #", //
		L"###", //
		L"# #", //

		L'#', Item::stick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::fence, 3), //
		L"ssciczg",
		L"W#W", //
		L"W#W", //

		L'#', Item::stick,
		L'W', new ItemInstance(Tile::wood, 1, TreeTile::OAK_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::birchFence, 3), //
		L"ssciczg",
		L"W#W", //
		L"W#W", //

		L'#', Item::stick,
		L'W', new ItemInstance(Tile::wood, 1, TreeTile::BIRCH_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::spruceFence, 3), //
		L"ssciczg",
		L"W#W", //
		L"W#W", //

		L'#', Item::stick,
		L'W', new ItemInstance(Tile::wood, 1, TreeTile::SPRUCE_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::jungleFence, 3), //
		L"ssciczg",
		L"W#W", //
		L"W#W", //

		L'#', Item::stick,
		L'W', new ItemInstance(Tile::wood, 1, TreeTile::JUNGLE_TRUNK),
		L'S');

    // neoLegacy - MAC-10 (Complex Table only): expensive late-progression firearm.
    addShapedRecipy(new ItemInstance(Item::mac10, 1),
        L"ssscictcictcicig",
        L"ISI",
        L"GFG",
        L"DRD",
        L'I', Item::iron_ingot,
        L'S', static_cast<Tile *>(Tile::sticky_piston),
        L'G', Item::gold_ingot,
        L'F', static_cast<Tile *>(Tile::ironBlock),
        L'D', Item::diamond,
        L'R', Item::redstone,
        L'M');

    // neoLegacy - 9mm Bullets: TU43-compatible ingredients; no copper dependency.
    addShapelessRecipy(new ItemInstance(Item::bullet_9mm, 6), L"zzzg",
        new ItemInstance(Item::iron_ingot, 1),
        new ItemInstance(Item::gunpowder, 1),
        new ItemInstance(Item::flint, 1),
        L'T');

    // neoLegacy - M16 (Complex Table only): the MAC-10's recipe stepped up - iron BLOCK on top, a redstone BLOCK instead of dust, and the
    // sticky piston (the bolt carrier) moves to the middle.
    addShapedRecipy(new ItemInstance(Item::m16, 1),
        L"ssscictcictcictg",
        L"IFI",
        L"GSG",
        L"DBD",
        L'I', Item::iron_ingot,
        L'F', static_cast<Tile *>(Tile::ironBlock),
        L'G', Item::gold_ingot,
        L'S', static_cast<Tile *>(Tile::sticky_piston),
        L'D', Item::diamond,
        L'B', static_cast<Tile *>(Tile::redstoneBlock),
        L'M');

    // neoLegacy - 5.56 rounds: the 9mm recipe with a second charge of powder.
    addShapelessRecipy(new ItemInstance(Item::bullet_556, 6), L"zzzzg",
        new ItemInstance(Item::iron_ingot, 1),
        new ItemInstance(Item::gunpowder, 1),
        new ItemInstance(Item::gunpowder, 1),
        new ItemInstance(Item::flint, 1),
        L'T');

    // neoLegacy - Flintlock Pistols (Complex Table only, machine category). One shape, three metals: barrel (M M), flint lock (F), redstone trigger work (R), iron-block grip (S).
    //   Copper   - copper ingots
    //   Iron     - iron ingots
    //   Gold     - gold ingots
    {
        const int pistolMetals[3] = { Item::copper_ingot_Id, Item::iron_ingot_Id, Item::gold_ingot_Id };
        Item *pistols[3] = { Item::flintlock_copper, Item::flintlock_iron, Item::flintlock_gold };
        for (int v = 0; v < 3; ++v)
        {
            addShapedRecipy(new ItemInstance(pistols[v], 1),
                L"ssscicicictg",
                L"MMF",
                L" RM",
                L" S ",
                L'M', Item::items[pistolMetals[v]],
                L'F', Item::flint,
                L'R', Item::redstone,
                L'S', static_cast<Tile *>(Tile::ironBlock),
                L'M');
        }
    }

	// neoLegacy - Bamboo wood set (all from Bamboo Planks = planks data 6)
	addShapedRecipy(new ItemInstance(Tile::bambooFence, 3), //
		L"ssciczg",
		L"W#W", //
		L"W#W", //

		L'#', Item::stick,
		L'W', new ItemInstance(Tile::wood, 1, WoodTile::BAMBOO_PLANKS),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::bambooGate, 1), //
		L"ssciczg",
		L"#W#", //
		L"#W#", //

		L'#', Item::stick, L'W', new ItemInstance(Tile::wood, 1, WoodTile::BAMBOO_PLANKS),
		L'S');

	addShapedRecipy(new ItemInstance(Item::bamboo_door, 3), //
		L"sssczg",
		L"##", //
		L"##", //
		L"##", //

		L'#', new ItemInstance(Tile::wood, 1, WoodTile::BAMBOO_PLANKS),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::bambooTrapdoor, 2), //
		L"ssczg",
		L"###", //
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, WoodTile::BAMBOO_PLANKS),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::bambooStairs, 6), //
		L"sssczg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, WoodTile::BAMBOO_PLANKS),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::bambooMosaicStairs, 6), //
		L"sssczg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, WoodTile::BAMBOO_MOSAIC),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::acaciaFence, 3), //
		L"ssciczg",
		L"W#W", //
		L"W#W", //

		L'#', Item::stick,
		L'W', new ItemInstance(Tile::wood, 1, TreeTile::ACACIA_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::darkFence, 3), //
		L"ssciczg",
		L"W#W", //
		L"W#W", //

		L'#', Item::stick,
		L'W', new ItemInstance(Tile::wood, 1, TreeTile::DARK_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::cobbleWall, 6, WallTile::TYPE_NORMAL), //
		L"ssctg",
		L"###", //
		L"###", //

		L'#', Tile::cobblestone,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::cobbleWall, 6, WallTile::TYPE_MOSSY), //
		L"ssctg",
		L"###", //
		L"###", //

		L'#', Tile::mossyCobblestone,
		L'S');

    addShapedRecipy(new ItemInstance(Tile::calciteWall, 6),
    	L"ssctg",
    	L"CCC",
    	L"CCC",

    	L'C', Tile::calcite,
    	L'S');

    addShapedRecipy(new ItemInstance(Tile::blackstoneWall, 6),
        L"ssctg",
        L"CCC",
        L"CCC",

        L'C', Tile::blackstone,
        L'S');

	addShapedRecipy(new ItemInstance(Tile::netherFence, 6), //
		L"ssctg",
		L"###", //
		L"###", //

		L'#', Tile::netherBrick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::iron_bars, 16), //
		L"sscig",
		L"###", //
		L"###", //

		L'#', Item::iron_ingot,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::fenceGate, 1), //
		L"ssciczg",
		L"#W#", //
		L"#W#", //

		L'#', Item::stick, L'W', new ItemInstance(Tile::wood, 1, 0),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::birchGate, 1), //
		L"ssciczg",
		L"#W#", //
		L"#W#", //

		L'#', Item::stick, L'W', new ItemInstance(Tile::wood, 1, TreeTile::BIRCH_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::spruceGate, 1), //
		L"ssciczg",
		L"#W#", //
		L"#W#", //

		L'#', Item::stick, L'W', new ItemInstance(Tile::wood, 1, TreeTile::SPRUCE_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::jungleGate, 1), //
		L"ssciczg",
		L"#W#", //
		L"#W#", //

		L'#', Item::stick, L'W', new ItemInstance(Tile::wood, 1, TreeTile::JUNGLE_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::acaciaGate, 1), //
		L"ssciczg",
		L"#W#", //
		L"#W#", //

		L'#', Item::stick, L'W', new ItemInstance(Tile::wood, 1, TreeTile::ACACIA_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::darkGate, 1), //
		L"ssciczg",
		L"#W#", //
		L"#W#", //

		L'#', Item::stick, L'W', new ItemInstance(Tile::wood, 1, TreeTile::DARK_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Item::wooden_door, 3), //
		L"sssczg",
		L"##", //
		L"##", //
		L"##", //

		L'#', new ItemInstance(Tile::wood, 1, 0),
		L'S');

	addShapedRecipy(new ItemInstance(Item::birch_door, 3), //
		L"sssczg",
		L"##", //
		L"##", //
		L"##", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::BIRCH_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Item::spruce_door, 3), //
		L"sssczg",
		L"##", //
		L"##", //
		L"##", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::SPRUCE_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Item::jungle_door, 3), //
		L"sssczg",
		L"##", //
		L"##", //
		L"##", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::JUNGLE_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Item::acacia_door, 3), //
		L"sssczg",
		L"##", //
		L"##", //
		L"##", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::ACACIA_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Item::dark_oak_door, 3), //
		L"sssczg",
		L"##", //
		L"##", //
		L"##", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::DARK_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Item::iron_door, 3), //
		L"ssscig",
		L"##", //
		L"##", //
		L"##", //

		L'#', Item::iron_ingot,
		L'S');

	// neoLegacy - one trapdoor recipe per plank type (aux = WoodTile data); each gives its own trapdoor
	{
		Tile *trapdoors[6] = { Tile::trapdoor, Tile::spruce_trapdoor, Tile::birch_trapdoor, Tile::jungle_trapdoor, Tile::acacia_trapdoor, Tile::dark_oak_trapdoor };
		for (int woodType = 0; woodType < 6; woodType++)
		{
			addShapedRecipy(new ItemInstance(trapdoors[woodType], 2), //
				L"ssczg",
				L"###", //
				L"###", //

				L'#', new ItemInstance(Tile::wood, 1, woodType),
				L'S');
		}
	}

	addShapedRecipy(new ItemInstance(Tile::iron_trapdoor, 2), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::iron_ingot,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stairs_wood, 6), //
		L"sssczg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, 0),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stairs_stone, 6), //
		L"sssctg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', Tile::cobblestone,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stairs_bricks, 6), //
		L"sssctg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', Tile::redBrick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stone_brick_stairsSmooth, 6), //
		L"sssctg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', Tile::stoneBrick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::nether_brick_stairs, 6), //
		L"sssctg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', Tile::netherBrick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::purpur_block, 4),
		L"sscig",
		L"FF",
		L"FF",

		L'F', Item::chorus_fruit_popped,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::purpur_pillar, 1),
		L"ssctg",
		L"#",
		L"#",

		L'#', Tile::purpurSlabHalf,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::purpur_stairs, 4),
		L"sssctg",
		L"#  ",
		L"## ",
		L"###",

		L'#', Tile::purpur_block,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::purpur_stairs, 4),
		L"sssctg",
		L"#  ",
		L"## ",
		L"###",

		L'#', Tile::purpur_pillar,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::purpurSlabHalf, 6),
		L"sctg",
		L"###",

		L'#', Tile::purpur_block,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::purpurSlabHalf, 6),
		L"sctg",
		L"###",

		L'#', Tile::purpur_pillar,
		L'S');

	addShapedRecipy(new ItemInstance((Tile*)Tile::end_rod, 4),
		L"sscicig",
		L"/",
		L"#",

		L'/', Item::blaze_rod,
		L'#', Item::chorus_fruit_popped,
		L'D');

	addShapedRecipy(new ItemInstance(Tile::stairs_sandstone, 6), //
		L"sssctg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', Tile::sandStone,
		L'S');
	addShapedRecipy(new ItemInstance(Tile::stairs_red_sandstone, 6), //
		L"sssctg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', Tile::red_sandstone,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::woodStairsBirch, 6), //
		L"sssczg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::BIRCH_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::woodStairsSpruce, 6), //
		L"sssczg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::SPRUCE_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::woodStairsJungle, 6), //
		L"sssczg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::JUNGLE_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::woodStairsAcacia, 6), //
		L"sssczg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::ACACIA_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::woodStairsDark, 6), //
		L"sssczg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::DARK_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stairs_quartz, 6), //
		L"sssctg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', Tile::quartzBlock,
		L'S');

    addShapedRecipy(new ItemInstance(Tile::stairs_calcite, 6), //
        L"sssctg",
        L"#  ", //
        L"## ", //
        L"###", //

        L'#', Tile::calcite,
        L'S');

    addShapedRecipy(new ItemInstance(Tile::stairs_blackstone, 6), //
        L"sssctg",
        L"#  ", //
        L"## ", //
        L"###", //

        L'#', Tile::blackstone,
        L'S');


	pArmorRecipes->addRecipes(this);


	pClothDyeRecipes->addRecipes(this);

	addShapedRecipy(new ItemInstance(Tile::snow, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::snowball,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::prismarine, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::prismarine_shard,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::prismarine, 1, PrismarineTile::TYPE_BRICKS), //
		L"ssscig",
		L"###", //
		L"###", //
		L"###",

		L'#', Item::prismarine_shard,
		L'S');


	addShapedRecipy(new ItemInstance(Tile::prismarine, 1, PrismarineTile::TYPE_DARK), //
		L"ssscicig",
		L"###", //
		L"#X#", //
		L"###", //

		L'#', Item::prismarine_shard, L'X', new ItemInstance(Item::dye, 1, DyePowderItem::BLACK),
		L'S');



	addShapedRecipy(new ItemInstance(Tile::topSnow, 6), //
		L"sctg",
		L"###", //

		L'#', Tile::snow,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::clay, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::clay,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::redBrick, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::brick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::wool, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::string,
		L'D');

	addShapedRecipy(new ItemInstance(Tile::tnt, 1), //
		L"ssscictg",
		L"X#X", //
		L"#X#", //
		L"X#X", //

		L'X', Item::gunpowder,//
		L'#', Tile::sand,
		L'T');



	addShapedRecipy(new ItemInstance(Tile::stoneSlabHalf, 6, StoneSlabTile::SAND_SLAB), //
		L"sctg",
		L"###", //

		L'#', Tile::sandStone,
		L'S');
	addShapedRecipy(new ItemInstance(Tile::stoneSlab2Half, 6, StoneSlabTile2::RED_SANDSTONE_SLAB), //
		L"sctg",
		L"###", //

		L'#', Tile::red_sandstone,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stoneSlabHalf, 6, StoneSlabTile::STONE_SLAB), //
		L"sczg",
		L"###", //

		L'#', new ItemInstance(Tile::stone, 1, 0),
		L'S');
	addShapedRecipy(new ItemInstance(Tile::stoneSlabHalf, 6, StoneSlabTile::COBBLESTONE_SLAB), //
		L"sctg",
		L"###", //

		L'#', Tile::cobblestone,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stoneSlabHalf, 6, StoneSlabTile::BRICK_SLAB), //
		L"sctg",
		L"###", //

		L'#', Tile::redBrick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stoneSlabHalf, 6, StoneSlabTile::SMOOTHBRICK_SLAB), //
		L"sctg",
		L"###", //

		L'#', Tile::stoneBrick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stoneSlabHalf, 6, StoneSlabTile::NETHERBRICK_SLAB), //
		L"sctg",
		L"###", //

		L'#', Tile::netherBrick,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::stoneSlabHalf, 6, StoneSlabTile::QUARTZ_SLAB), //
		L"sctg",
		L"###", //

		L'#', Tile::quartzBlock,
		L'S');

    addShapedRecipy(new ItemInstance(Tile::stoneSlab2Half, 6, StoneSlabTile2::CALCITE_SLAB),
        L"sctg",
        L"###", //

        L'#', Tile::calcite,
        L'S');
    //(new ItemInstance(Tile::stoneSlab2Half, 6, StoneSlabTile2::CALCITE_SLAB),
    addShapedRecipy(new ItemInstance(Tile::stoneSlab2Half, 6, StoneSlabTile2::BLACKSTONE_SLAB),
        L"sctg",
        L"###", //

        L'#', Tile::blackstone,
        L'S');

	addShapedRecipy(new ItemInstance(Tile::woodSlabHalf, 6, 0), //
		L"sczg",
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, 0),
		L'S');
	// TU9 - adding wood slabs

	addShapedRecipy(new ItemInstance(Tile::woodSlabHalf, 6, TreeTile::BIRCH_TRUNK), //
		L"sczg",
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::BIRCH_TRUNK),
		L'S');


	addShapedRecipy(new ItemInstance(Tile::woodSlabHalf, 6, TreeTile::SPRUCE_TRUNK), //
		L"sczg",
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::SPRUCE_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::woodSlabHalf, 6, TreeTile::JUNGLE_TRUNK), //
		L"sczg",
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::JUNGLE_TRUNK),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::woodSlabHalf, 6, TreeTile::ACACIA_TRUNK), //
		L"sczg",
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::ACACIA_TRUNK),
		L'S');

	// neoLegacy - Bamboo slabs (data 6) and Bamboo Mosaic slabs (data 7)
	addShapedRecipy(new ItemInstance(Tile::woodSlabHalf, 6, 6), //
		L"sczg",
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, 6),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::woodSlabHalf, 6, 7), //
		L"sczg",
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, 7),
		L'S');

	// two Bamboo Slabs stacked make a Bamboo Mosaic block (like Java)
	addShapedRecipy(new ItemInstance(Tile::wood, 1, 7), //
		L"ssczg",
		L"#", //
		L"#", //

		L'#', new ItemInstance(Tile::woodSlabHalf, 1, 6),
		L'S');

	addShapedRecipy(new ItemInstance(Tile::woodSlabHalf, 6, TreeTile::DARK_TRUNK), //
		L"sczg",
		L"###", //

		L'#', new ItemInstance(Tile::wood, 1, TreeTile::DARK_TRUNK),
		L'S');



	addShapedRecipy(new ItemInstance(Item::cake, 1), //
		L"ssscicicicig",
		L"AAA", //
		L"BEB", //
		L"CCC", //

		L'A', Item::milk_bucket,//
		L'B', Item::sugar,//
		L'C', Item::wheat, L'E', Item::egg,
		L'F');

	addShapedRecipy(new ItemInstance(Item::sugar, 1), //
		L"scig",
		L"#", //

		L'#', Item::reeds,
		L'F');

	addShapedRecipy(new ItemInstance(Tile::rail, 16), //
		L"ssscicig",
		L"X X", //
		L"X#X", //
		L"X X", //

		L'X', Item::iron_ingot,//
		L'#', Item::stick,
		L'V');

	addShapedRecipy(new ItemInstance(Tile::goldenRail, 6), //
		L"ssscicicig",
		L"X X", //
		L"X#X", //
		L"XRX", //

		L'X', Item::gold_ingot,//
		L'R', Item::redstone,//
		L'#', Item::stick,
		L'V');

	addShapedRecipy(new ItemInstance(Tile::activatorRail, 6), //
		L"ssscictcig",
		L"XSX", //
		L"X#X", //
		L"XSX", //

		L'X', Item::iron_ingot,//
		L'#', Tile::redstoneTorch_on,//
		L'S', Item::stick,
		L'V');

	addShapedRecipy(new ItemInstance(Tile::detectorRail, 6), //
		L"ssscicictg",
		L"X X", //
		L"X#X", //
		L"XRX", //

		L'X', Item::iron_ingot,//
		L'R', Item::redstone,//
		L'#', Tile::pressurePlate_stone,
		L'V');

	addShapedRecipy(new ItemInstance(Item::minecart, 1), //
		L"sscig",
		L"# #", //
		L"###", //

		L'#', Item::iron_ingot,
		L'V');

	// neoLegacy - Complex Table: the Mace. (In Java it is only found in trial vaults, which this build does not have, so it needs a
	// recipe to be obtainable in Survival: a diamond-block head on a blaze-rod handle, braced by two iron blocks.)
	//   I D I      I = iron block, D = diamond block, R = blaze rod
	//   . R .
	//   . R .
	addShapedRecipy(new ItemInstance(Item::mace, 1), //
		L"sssctctcig",
		L"IDI", //
		L" R ", //
		L" R ", //

		L'I', Tile::ironBlock,
		L'D', Tile::diamondBlock,
		L'R', Item::blaze_rod,
		L'T');

	// neoLegacy - Complex Table: Diamond Minecart = the normal minecart recipe, but with diamonds.
	//   # . #      # = diamond
	//   # # #
	addShapedRecipy(new ItemInstance(Item::diamond_minecart, 1), //
		L"sscig",
		L"# #", //
		L"###", //

		L'#', Item::diamond,
		L'V');

	addShapedRecipy(new ItemInstance(Item::chest_minecart, 1), //
		L"ssctcig",
		L"A", //
		L"B", //

		L'A', Tile::chest, L'B', Item::minecart,
		L'V');

	addShapedRecipy(new ItemInstance(Item::furnace_minecart, 1), //
		L"ssctcig",
		L"A", //
		L"B", //

		L'A', Tile::furnace, L'B', Item::minecart,
		L'V');

	addShapedRecipy(new ItemInstance(Item::tnt_minecart, 1), //
		L"ssctcig",
		L"A", //
		L"B", //

		L'A', Tile::tnt, L'B', Item::minecart,
		L'V');

	addShapedRecipy(new ItemInstance(Item::hopper_minecart, 1), //
		L"ssctcig",
		L"A", //
		L"B", //

		L'A', Tile::hopper, L'B', Item::minecart,
		L'V');

	// neoLegacy - one boat recipe per plank type; the result's aux value is the wood type (6 = bamboo: the Bamboo Raft)
	for (int woodType = 0; woodType < 7; woodType++)
	{
		addShapedRecipy(new ItemInstance(Item::boat, 1, woodType), //
			L"ssczg",
			L"# #", //
			L"###", //

			L'#', new ItemInstance(Tile::wood, 1, woodType),
			L'V');
	}

	addShapedRecipy(new ItemInstance((Item *)Item::fishing_rod, 1), //
		L"ssscicig",
		L"  #", //
		L" #X", //
		L"# X", //

		L'#', Item::stick, L'X', Item::string,
		L'T');

	addShapedRecipy(new ItemInstance(Item::carrot_on_a_stick, 1), //
		L"sscicig",
		L"# ", //
		L" X", //

		L'#', Item::fishing_rod, L'X', Item::carrots,
		L'T')->keepTag();

	addShapedRecipy(new ItemInstance(Item::flint_and_steel, 1), //
		L"sscicig",
		L"A ", //
		L" B", //

		L'A', Item::iron_ingot, L'B', Item::flint,
		L'T');

	addShapedRecipy(new ItemInstance(Item::bread, 1), //
		L"scig",
		L"###", //

		L'#', Item::wheat,
		L'F');

	// Moved bow and arrow in from weapons to avoid stacking on the group name display
	addShapedRecipy(new ItemInstance((Item*)Item::bow, 1), //
		L"ssscicig",
		L" #X", //
		L"# X", //
		L" #X", //

		L'X', Item::string,//
		L'#', Item::stick,
		L'T');

	// neoLegacy - Crossbow (Java recipe): sticks, an iron ingot, two string and a tripwire hook
	addShapedRecipy(new ItemInstance(Item::crossbow, 1), //
		L"ssscicicicig",
		L"#I#", //
		L"XHX", //
		L" # ", //

		L'#', Item::stick,//
		L'I', Item::iron_ingot,//
		L'X', Item::string,//
		L'H', Tile::tripWireSource,
		L'T');

	addShapedRecipy(new ItemInstance(Item::arrow, 4), //
		L"ssscicicig",
		L"X", //
		L"#", //
		L"Y", //

		L'Y', Item::feather,//
		L'X', Item::flint,//
		L'#', Item::stick,
		L'T');

	addShapedRecipy(new ItemInstance(Item::spectral_arrow, 2),
		L"ssscicig",
		L" # ",
		L"#X#",
		L" # ",

		L'#', Item::glowstone_dust,
		L'X', Item::arrow,
		L'T');

	{
		static const int tippedArrowPotions[] =
		{
			MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_REGENERATION), MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_REGENERATION), MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_REGENERATION),
			MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_SPEED), MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_SPEED), MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_SPEED),
			MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_FIRE_RESISTANCE), MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_FIRE_RESISTANCE),
			MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_POISON), MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_POISON), MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_POISON),
			MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_INSTANTHEALTH), MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_INSTANTHEALTH),
			MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_NIGHTVISION), MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_NIGHTVISION),
			MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_INVISIBILITY), MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_INVISIBILITY),
			MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_WEAKNESS), MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_WEAKNESS),
			MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_STRENGTH), MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_STRENGTH), MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_STRENGTH),
			MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_SLOWNESS), MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_SLOWNESS),
			MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_INSTANTDAMAGE), MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_INSTANTDAMAGE),
			MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_WATERBREATHING), MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_WATERBREATHING),
			MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_JUMPBOOST), MACRO_MAKEPOTION_AUXVAL(0, MASK_LEVEL2, MASK_JUMPBOOST), MACRO_MAKEPOTION_AUXVAL(0, MASK_EXTENDED, MASK_JUMPBOOST),
			MACRO_MAKEPOTION_AUXVAL(0, 0, MASK_LUCK),
		};
		for (int i = 0; i < sizeof(tippedArrowPotions) / sizeof(tippedArrowPotions[0]); i++)
		{
			addShapedRecipy(new ItemInstance(Item::tipped_arrow_Id, 8, tippedArrowPotions[i]),
				L"sssciczg",
				L"###",
				L"#X#",
				L"###",

				L'#', Item::arrow,
				L'X', new ItemInstance(Item::lingering_potion_Id, 1, tippedArrowPotions[i]),
				L'T');
		}
	}

	pWeaponRecipies->addRecipes(this);

	addShapedRecipy(new ItemInstance(Item::bucket, 1), //
		L"sscig",
		L"# #", //
		L" # ", //

		L'#', Item::iron_ingot,
		L'T');

	addShapedRecipy(new ItemInstance(Item::bowl, 4), //
		L"ssctg",
		L"# #", //
		L" # ", //

		L'#', Tile::wood,
		L'T');

	addShapedRecipy(new ItemInstance(Item::glassBottle, 3), //
		L"ssctg",
		L"# #", //
		L" # ", //

		L'#', Tile::glass,
		L'T');



	// torch made of charcoal - moved to be the default due to the tutorial using it
	addShapedRecipy(new ItemInstance(Tile::torch, 4), //
		L"ssczcig",
		L"X", //
		L"#", //

		L'X', new ItemInstance(Item::coal, 1, CoalItem::CHAR_COAL),//
		L'#', Item::stick,
		L'T');

	addShapedRecipy(new ItemInstance(Tile::torch, 4), //
		L"ssczcig",
		L"X", //
		L"#", //
		L'X', new ItemInstance(Item::coal, 1, CoalItem::STONE_COAL),//
		L'#', Item::stick,
		L'T');

	addShapedRecipy(new ItemInstance(Tile::glowstone, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::glowstone_dust,
		L'T');

	addShapedRecipy(new ItemInstance(Tile::quartzBlock, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::nether_quartz,
		L'S');

	addShapedRecipy(new ItemInstance(Tile::lever, 1), //
		L"ssctcig",
		L"X", //
		L"#", //

		L'#', Tile::cobblestone, L'X', Item::stick,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::tripWireSource, 2), //
		L"sssctcicig",
		L"I", //
		L"S", //
		L"#", //

		L'#', Tile::wood, L'S', Item::stick, L'I', Item::iron_ingot,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::redstoneTorch_on, 1), //
		L"sscicig",
		L"X", //
		L"#", //

		L'#', Item::stick, L'X', Item::redstone,
		L'M');

	addShapedRecipy(new ItemInstance(Item::repeater, 1), //
		L"ssctciczg",
		L"#X#", //
		L"III", //

		L'#', Tile::redstoneTorch_on, L'X', Item::redstone, L'I', new ItemInstance(Tile::stone, 1, 0),
		L'M');

	addShapedRecipy(new ItemInstance(Item::comparator, 1), //
		L"sssctciczg",
		L" # ", //
		L"#X#", //
		L"III", //

		L'#', Tile::redstoneTorch_on, L'X', Item::nether_quartz, L'I', new ItemInstance(Tile::stone, 1, 0),
		L'M');

	addShapedRecipy(new ItemInstance(Tile::daylightDetector),
		L"sssctcictg",
		L"GGG",
		L"QQQ",
		L"WWW",

		L'G', Tile::glass, L'Q', Item::nether_quartz, L'W', Tile::woodSlabHalf,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::hopper),
		L"ssscictg",
		L"I I", //
		L"ICI", //
		L" I ", //

		L'I', Item::iron_ingot, L'C', Tile::chest,
		L'M');

	addShapedRecipy(new ItemInstance(Item::clock, 1), //
		L"ssscicig",
		L" # ", //
		L"#X#", //
		L" # ", //
		L'#', Item::gold_ingot, L'X', Item::redstone,
		L'T');

	addShapelessRecipy(new ItemInstance(Item::eye_of_ender, 1), //
		L"iig",
		Item::ender_pearl, Item::blaze_powder,
		L'T');

	addShapelessRecipy(new ItemInstance(Item::fireball, 3), //
		L"iiig",
		Item::gunpowder, Item::blaze_powder,Item::coal,
		L'T');

	addShapelessRecipy(new ItemInstance(Item::fireball, 3), //
		L"iizg",
		Item::gunpowder, Item::blaze_powder,new ItemInstance(Item::coal, 1, CoalItem::CHAR_COAL),
		L'T');

	addShapedRecipy(new ItemInstance(Item::lead, 2), //
		L"ssscicig",
		L"~~ ", //
		L"~O ", //
		L"  ~", //

		L'~', Item::string, L'O', Item::slime_ball,
		L'T');


	addShapedRecipy(new ItemInstance(Item::compass, 1), //
		L"ssscicig",
		L" # ", //
		L"#X#", //
		L" # ", //

		L'#', Item::iron_ingot, L'X', Item::redstone,
		L'T');

	addShapedRecipy(new ItemInstance((Item*)Item::empty_map, 1), //
		L"ssscicig",
		L"###", //
		L"#X#", //
		L"###", //

		L'#', Item::paper, L'X', Item::compass,
		L'T');

	addShapedRecipy(new ItemInstance(Tile::button, 1), //
		L"sczg",
		L"#", //

		L'#', new ItemInstance(Tile::stone, 1, 0),
		L'M');

	addShapedRecipy(new ItemInstance(Tile::button_wood, 1), //
		L"sctg",
		L"#", //

		L'#', Tile::wood,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::pressurePlate_wood, 1), //
		L"sctg",
		L"##", //
		L'#', Tile::wood,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::pressurePlate_stone, 1), //
		L"sczg",
		L"##", //
		L'#', new ItemInstance(Tile::stone, 1, 0),
		L'M');

	addShapedRecipy(new ItemInstance(Tile::heavy_weighted_pressure_plate, 1), //
		L"scig",
		L"##", //

		L'#', Item::iron_ingot,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::light_weighted_pressure_plate, 1), //
		L"scig",
		L"##", //

		L'#', Item::gold_ingot,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::dispenser, 1), //
		L"sssctcicig",
		L"###", //
		L"#X#", //
		L"#R#", //
		L'#', Tile::cobblestone, L'X', Item::bow, L'R', Item::redstone,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::dropper, 1), //
		L"sssctcig",
		L"###", //
		L"# #", //
		L"#R#", //

		L'#', Tile::cobblestone, L'R', Item::redstone,
		L'M');

    addShapedRecipy(new ItemInstance(Tile::observer, 1), //
        L"sssctcicig",
        L"###", //
        L"RRX", //
        L"###", //
        L'#', Tile::cobblestone, L'X', Item::nether_quartz, L'R', Item::redstone,
        L'M');

    // Lucky Block
    addShapedRecipy(new ItemInstance(Tile::lucky_block, 1),

        L"ssscictg",
        L"GGG",
        L"GDG",
        L"GGG",

        L'G', Item::gold_ingot,
        L'D', Tile::dispenser,
        L'M');

	addShapedRecipy(new ItemInstance(Item::cauldron, 1), //
		L"ssscig",
		L"# #", //
		L"# #", //
		L"###", //

		L'#', Item::iron_ingot,
		L'T');

	addShapedRecipy(new ItemInstance(Item::brewing_stand, 1), //
		L"ssctcig",
		L" B ", //
		L"###", //

		L'#', Tile::cobblestone, L'B', Item::blaze_rod,
		L'S');


	addShapedRecipy(new ItemInstance(Tile::litPumpkin, 1), //
		L"ssctctg",
		L"A", //
		L"B", //

		L'A', Tile::pumpkin, L'B', Tile::torch,
		L'T');

	addShapedRecipy(new ItemInstance(Item::flower_pot, 1), //
		L"sscig",
		L"# #", //
		L" # ", //

		L'#', Item::brick,
		L'D');

	addShapedRecipy(new ItemInstance(Tile::jukebox, 1), //
		L"sssctcig",
		L"###", //
		L"#X#", //
		L"###", //

		L'#', Tile::wood, L'X', Item::diamond,
		'D');

	addShapedRecipy(new ItemInstance(Item::leather, 1),
		L"sscig",
		L"##",
		L"##",

		L'#', Item::rabbit_hide,
		L'D');

	addShapedRecipy(new ItemInstance(Item::armor_stand, 1),
		L"ssscictg",
		L"SSS",
		L" S ",
		L"SXS",
		L'S', Item::stick,
		L'X', Tile::stoneSlabHalf,
		L'D');



	addShapedRecipy(new ItemInstance(Item::paper, 3), //
		L"scig",
		L"###", //

		L'#', Item::reeds,
		L'D');

	addShapelessRecipy(new ItemInstance(Item::book, 1),
		L"iiiig",
		Item::paper,
		Item::paper,
		Item::paper,
		Item::leather,
		L'D');

	addShapelessRecipy(new ItemInstance(Item::writable_book, 1),
		L"iiig",
		Item::book,
		Item::feather,
		ItemInstance(Item::dye, 1, DyePowderItem::BLACK),
		L'D');

	// neoLegacy - Enchantment Encyclopedia: a Book and a Lapis Lazuli (shapeless). Complex-Table-only via isComplexOnly()
	// (keyed on the result item), so it is not craftable at a normal Workbench.
	addShapelessRecipy(new ItemInstance(Item::enchantment_encyclopedia, 1),
		L"izg",
		Item::book,
		new ItemInstance(Item::dye, 1, DyePowderItem::BLUE),
		L'D');

	//addShapelessRecipy(new ItemInstance(Item.writable_book, 1), //
	//            Item.book, new ItemInstance(Item.dye, 1, DyePowderItem.BLACK), Item.feather);

	addShapedRecipy(new ItemInstance(Tile::noteblock, 1), //
		L"sssctcig",
		L"###", //
		L"#X#", //
		L"###", //

		L'#', Tile::wood, L'X', Item::redstone,
		L'M');

	addShapedRecipy(new ItemInstance(Tile::bookshelf, 1), //
		L"sssctcig",
		L"###", //
		L"XXX", //
		L"###", //

		L'#', Tile::wood, L'X', Item::book,
		L'D');

	addShapedRecipy(new ItemInstance(Item::painting, 1), //
		L"ssscictg",
		L"###", //
		L"#X#", //
		L"###", //

		L'#', Item::stick, L'X', Tile::wool,
		L'D');


	addShapedRecipy(new ItemInstance(Item::frame, 1), //
		L"ssscicig",
		L"###", //
		L"#X#", //
		L"###", //

		L'#', Item::stick, L'X', Item::leather,
		L'D');

	pOreRecipies->addRecipes(this);

	// neoLegacy - Jelly Block / Jelly Ball. Registered right after the ore/storage-block recipes so they sit next to the Slime Block and
	// Slime Ball recipes (the last pair in that list) in the same crafting-menu group.
	// 4 Jelly Balls in a 2x2 square make a Jelly Block (like Java's honey block); a Jelly Block breaks back into 4 Jelly Balls.
	addShapedRecipy(new ItemInstance(Tile::jellyBlock, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::jelly_ball,
		L'D');

	addShapedRecipy(new ItemInstance(Item::jelly_ball, 4), //
		L"sctg",
		L"#", //

		L'#', Tile::jellyBlock,
		L'D');

	// neoLegacy - Platinum Sheet (Illager Invasion): four Platinum Chunks, shapeless
	addShapelessRecipy(new ItemInstance(Item::platinum_sheet, 1), //
		L"iiiig",
		Item::platinum_chunk, Item::platinum_chunk, Item::platinum_chunk, Item::platinum_chunk,
		L'T');

	// neoLegacy - Hallowed Gem (Illager Invasion, mod recipe hallowed_gem.json):
	// 4 Amethyst Shard + 2 Unusual Dust + 2 Illusionary Dust + 1 Diamond, shaped 3x3.
	// The mod's category is "misc"; there is no Misc group here, so 'T' (Tool) like
	// the Platinum Sheet above. Group is provisional - see FIX-NOTES.
	addShapedRecipy(new ItemInstance(Item::hallowed_gem, 1), //
		L"ssscicicicig",
		L"#B#", //
		L"RDR", //
		L"#B#", //

		L'#', Item::amethyst_shard,
		L'B', Item::unusual_dust,
		L'R', Item::illusionary_dust,
		L'D', Item::diamond,
		L'T');

	// neoLegacy - Horn of Sight (Illager Invasion, mod recipe horn_of_sight.json):
	// 1 Hallowed Gem + 3 Gold Ingot + 1 Goat Horn, shaped. Mod category "equipment".
	addShapedRecipy(new ItemInstance(Item::horn_of_sight, 1), //
		L"ssscicicig",
		L" G ", //
		L"#H#", //
		L" # ", //

		L'G', Item::hallowed_gem,
		L'#', Item::gold_ingot,
		L'H', Item::goat_horn,
		L'T');

	// neoLegacy - Complex Table: an Ice block makes 4 Ice Balls, and 4 Ice Balls make an Ice block again. Both are Complex-Table-only
	// via isComplexOnly() (keyed on the results: nothing else in the game produces Ice or Ice Balls).
	addShapedRecipy(new ItemInstance(Item::ice_ball, 4), //
		L"sctg",
		L"#", //

		L'#', Tile::ice,
		L'D');

	addShapedRecipy(new ItemInstance(Tile::ice, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::ice_ball,
		L'D');

	addShapedRecipy(new ItemInstance(Item::gold_ingot), //
		L"ssscig",
		L"###", //
		L"###", //
		L"###", //

		L'#', Item::gold_nugget,
		L'D');

	addShapedRecipy(new ItemInstance(Item::gold_nugget, 9), //
		L"scig",
		L"#", //
		L'#', Item::gold_ingot,
		L'D');

	// neoLegacy - Iron ingot <-> iron nugget (2026-11-23): moved here from
	// StructureRecipies.cpp so both sit in the Decoration tab of the recipes
	// next to the gold ingot <-> gold nugget pair above (group 'D'), mirroring
	// gold's tags exactly. Complex-Table-only (2026-11-26): the 9 -> 1
	// result (iron_ingot_Id) is shared with vanilla recipes, so it is gated
	// on result + ingredient in Recipes::isComplexOnly, red-dye pattern;
	// the 1 -> 9 is gated on the iron_nugget_Id result alone. The 1 -> 9 was
	// shapeless in StructureRecipies.cpp; gold's 1 -> 9 is shaped 1-slot
	// (L"scig"), which is functionally identical for a single ingredient.
	addShapedRecipy(new ItemInstance(Item::iron_ingot), //
		L"ssscig",
		L"###", //
		L"###", //
		L"###", //

		L'#', Item::iron_nugget,
		L'D');

	addShapedRecipy(new ItemInstance(Item::iron_nugget, 9), //
		L"scig",
		L"#", //
		L'#', Item::iron_ingot,
		L'D');

	// neoLegacy - Netherite tier (2026-11-26): ingot is shapeless, 4 scrap +
	// 4 gold ingots in the Crafting Table (Java recipe). Nugget exchange
	// copies gold exactly. Nugget exchange is Complex-Table-only; only the
	// shapeless scrap+gold ingot recipe stays workbench.
	addShapelessRecipy(new ItemInstance(Item::netherite_ingot, 1), //
		L"iiiiiiiig",
		Item::netherite_scrap, Item::netherite_scrap, Item::netherite_scrap, Item::netherite_scrap,
		Item::gold_ingot, Item::gold_ingot, Item::gold_ingot, Item::gold_ingot,
		L'D');

	addShapedRecipy(new ItemInstance(Item::netherite_ingot), //
		L"ssscig",
		L"###", //
		L"###", //
		L"###", //

		L'#', Item::netherite_nugget,
		L'D');

	addShapedRecipy(new ItemInstance(Item::netherite_nugget, 9), //
		L"scig",
		L"#", //
		L'#', Item::netherite_ingot,
		L'D');

	// 4J-PB - moving into decorations to make the structures list smaller
	// neoLegacy - one sign recipe per plank type (aux = WoodTile data, 6 = bamboo); every one gives THE sign item with that wood as its aux value
	{
		for (int woodType = 0; woodType < 7; woodType++)
		{
			addShapedRecipy(new ItemInstance(Item::sign, 3, woodType), //
				L"sssczcig",
				L"###", //
				L"###", //
				L" X ", //

				L'#', new ItemInstance(Tile::wood, 1, woodType), L'X', Item::stick,
				L'D');
		}
	}

	// 4J - TODO - put these new 1.7.3 items in required place within recipes
	addShapedRecipy(new ItemInstance(static_cast<Tile*>(Tile::pistonBase), 1), //
		L"sssctcicictg",
		L"TTT", //
		L"#X#", //
		L"#R#", //

        L'#', Tile::cobblestone, L'X', Item::iron_ingot, L'R', Item::redstone, L'T', Tile::wood,
		L'M');

    addShapedRecipy(new ItemInstance(static_cast<Tile *>(Tile::sticky_piston), 1), //
		L"sscictg",
		L"S", //
		L"P", //

        L'S', Item::slime_ball, L'P', Tile::pistonBase,
		L'M');


	// 4J Stu - Added some dummy firework recipes to allow us to navigate forward to the fireworks scene
	addShapedRecipy(new ItemInstance(Item::fireworks, 1), //
		L"sscicig",
		L" P ", //
		L" G ", //

		L'P', Item::paper, L'G', Item::gunpowder,
		L'D');

	addShapedRecipy(new ItemInstance(Item::firework_charge,1), //
		L"sscicig",
		L" D ", //
		L" G ", //

		L'D', Item::dye, L'G', Item::gunpowder,
		L'D');

	addShapedRecipy(new ItemInstance(Item::firework_charge,1), //
		L"sscicig",
		L" D ", //
		L" C ", //

		L'D', Item::dye, L'C', Item::firework_charge,
		L'D');

	// neoLegacy - Complex Table: "4 sand + 4 gravel + 1 dye -> 8 concrete powder" for all
	// 16 colors. Index 0-15 is BOTH the dye's aux value and Tile::concrete_powder's own
	// color data (DyePowderItem::BLACK..WHITE == concrete powder color 0..15).
	// These live in the shared list on purpose (recipe-book UI + CraftItemPacket use list
	// indexes) and are kept exclusive to the Complex Table via isComplexOnly().
	for (int color = 0; color < 16; color++)
	{
		vector<ItemInstance *> *ingredients = new vector<ItemInstance *>();
		for (int n = 0; n < 4; n++) ingredients->push_back(new ItemInstance(Tile::sand, 1));
		for (int n = 0; n < 4; n++) ingredients->push_back(new ItemInstance(Tile::gravel, 1));
		ingredients->push_back(new ItemInstance(Item::dye, 1, color));

		recipies->push_back(new ShapelessRecipy(new ItemInstance(Tile::concrete_powder, 8, color), ingredients, Recipy::eGroupType_Structure));
	}

	// neoLegacy - Complex Table: concrete stairs and slabs, all 16 colors. Same yields as the
	// vanilla stone recipes: 6 concrete -> 4 stairs, 3 concrete -> 6 slabs. The ingredient is
	// the matching COLOR of concrete block (Tile::concrete data 0-15) and the result carries
	// that same 0-15 color as its aux value (stairs/slabs are single-ID, color in the item aux
	// / tile entity). Complex-Table-only via isComplexOnly(), same as the powder recipes.
	// ShapedRecipy takes ownership of the ingredient array (delete[] in its destructor), so it
	// is heap-allocated here with nullptr for the empty grid cells.
	for (int color = 0; color < 16; color++)
	{
		// Stairs: staircase pattern in a 3x3 grid
		//   #..
		//   ##.
		//   ###
		ItemInstance **stairIngredients = new ItemInstance *[9];
		for (int i = 0; i < 9; i++) stairIngredients[i] = nullptr;
		stairIngredients[0] = new ItemInstance(Tile::concrete, 1, color);
		stairIngredients[3] = new ItemInstance(Tile::concrete, 1, color);
		stairIngredients[4] = new ItemInstance(Tile::concrete, 1, color);
		stairIngredients[6] = new ItemInstance(Tile::concrete, 1, color);
		stairIngredients[7] = new ItemInstance(Tile::concrete, 1, color);
		stairIngredients[8] = new ItemInstance(Tile::concrete, 1, color);
		recipies->push_back(new ShapedRecipy(3, 3, stairIngredients, new ItemInstance(Tile::concrete_stairs, 4, color), Recipy::eGroupType_Structure));

		// Slabs: three across
		ItemInstance **slabIngredients = new ItemInstance *[3];
		for (int i = 0; i < 3; i++) slabIngredients[i] = new ItemInstance(Tile::concrete, 1, color);
		recipies->push_back(new ShapedRecipy(3, 1, slabIngredients, new ItemInstance(Tile::concrete_slab_half, 6, color), Recipy::eGroupType_Structure));
	}

	// neoLegacy - Complex Table: 1 redstone -> 4 red dye. Complex-Table-only (see isComplexOnly,
	// which matches on the redstone ingredient so the normal rose -> red dye recipe is untouched).
	{
		vector<ItemInstance *> *redIngredients = new vector<ItemInstance *>();
		redIngredients->push_back(new ItemInstance(Item::redstone, 1, 0));
		recipies->push_back(new ShapelessRecipy(new ItemInstance(Item::dye, 4, DyePowderItem::RED), redIngredients, Recipy::eGroupType_Decoration));
	}

	// neoLegacy - Complex Table: Zoro swords. The Cursed Hilt is the core of every sword; the
	// catalyst decides which one you get. The Diamond Sword may be any durability (ANY_AUX_VALUE).
	//   Charcoal + Blaze Rod + Flint          -> Cursed Hilt
	//   Cursed Hilt + Diamond Sword + Lapis   -> Emma
	//   Cursed Hilt + Diamond Sword + Quartz  -> Wado Ichimonji
	//   Cursed Hilt + Diamond Sword + Redstone-> Sandai Kitetsu
	//   Cursed Hilt + Diamond Sword + Coal    -> Shusui
	// All are Complex-Table-only via isComplexOnly() (keyed on the result item).
	{
		addShapelessRecipy(new ItemInstance(Item::cursed_hilt, 1), //
			L"zizg",
			new ItemInstance(Item::coal, 1, CoalItem::CHAR_COAL), Item::blaze_rod, new ItemInstance(Item::flint, 1, 0),
			L'T');

		struct ZoroForge { Item *result; ItemInstance *catalyst; };
		ZoroForge forges[4] =
		{
			{ Item::enma,			new ItemInstance(Item::dye, 1, DyePowderItem::BLUE) },	// lapis lazuli
			{ Item::wado_ichimonji,	new ItemInstance(Item::nether_quartz, 1, 0) },
			{ Item::sandai_kitetsu,	new ItemInstance(Item::redstone, 1, 0) },
			{ Item::shusui,			new ItemInstance(Item::coal, 1, CoalItem::STONE_COAL) },
		};
		for (int i = 0; i < 4; i++)
		{
			ItemInstance *anySword = new ItemInstance(Item::diamond_sword->id, 1, Recipes::ANY_AUX_VALUE);
			ItemInstance *hilt = new ItemInstance(Item::cursed_hilt, 1, 0);
			addShapelessRecipy(new ItemInstance(forges[i].result, 1), //
				L"zzzg",
				hilt, anySword, forges[i].catalyst,
				L'T');
		}
	}

	// neoLegacy - Complex Table: Drill.
	//   I I I      I = iron ingot
	//   P R Q      P = iron pickaxe (any durability), R = redstone, Q = nether quartz
	//   C C C      C = cobblestone
	// Complex-Table-only via isComplexOnly() (keyed on the result being the drill). Shown in the
	// Mechanisms group like the Observer. The 'i'/'t' ingredient types build ANY_AUX_VALUE
	// ingredients, so a used (damaged) iron pickaxe works.
	addShapedRecipy(new ItemInstance(Tile::drill, 1), //
		L"ssscicicicictg",
		L"III", //
		L"PRQ", //
		L"CCC", //

		L'I', Item::iron_ingot, L'P', Item::iron_pickaxe, L'R', Item::redstone, L'Q', Item::nether_quartz, L'C', Tile::cobblestone,
		L'M');

	// neoLegacy - Complex Table: Crusher.
	//   I P I      I = iron ingot, P = piston (the crushing head)
	//   I G I      G = grindstone
	//   C H C      C = cobblestone, H = hopper (the feed)
	// A piston drives a grindstone fed by a hopper -- which is exactly what the block does.
	addShapedRecipy(new ItemInstance(Tile::crusher, 1), //
		L"ssscictctctctg",
		L"IPI", //
		L"IGI", //
		L"CHC", //

		L'I', Item::iron_ingot, L'P', static_cast<Tile *>(Tile::pistonBase), L'G', static_cast<Tile *>(Tile::grindstone), L'C', Tile::cobblestone, L'H', static_cast<Tile *>(Tile::hopper),
		L'M');

	// neoLegacy - Complex Table: Refiner. A Crusher upgraded with the things it produces.
	//   G G G      G = gold ingot
	//   R X L      R = redstone, X = Crusher, L = lapis lazuli
	//   I I I      I = iron ingot
	addShapedRecipy(new ItemInstance(Tile::refiner, 1), //
		L"ssscicictczcig",
		L"GGG", //
		L"RXL", //
		L"III", //

		L'G', Item::gold_ingot, L'R', Item::redstone, L'X', static_cast<Tile *>(Tile::crusher), L'L', new ItemInstance(Item::dye, 1, DyePowderItem::BLUE), L'I', Item::iron_ingot,
		L'M');

    // Copium Arcade machines are listed under Decoration, separate from Mechanisms.
    //   O = Obsidian, G = Glass, D = Diamond, R = Redstone Dust.
    addShapedRecipy(new ItemInstance(Tile::cookieMiner, 1), //
        L"sssctcicictcig",
        L"OCO", //
        L"ORO", //
        L"GDG", //
        L'O', Tile::obsidian, L'C', Item::cookie, L'R', Item::redstone, L'G', Tile::glass, L'D', Item::diamond,
        L'D');

    addShapedRecipy(new ItemInstance(Tile::flappyCraft, 1), //
        L"sssctcicictcig",
        L"OCO", //
        L"ORO", //
        L"GDG", //
        L'O', Tile::obsidian, L'C', Item::raw_chicken, L'R', Item::redstone, L'G', Tile::glass, L'D', Item::diamond,
        L'D');

    addShapedRecipy(new ItemInstance(Tile::mineSnake, 1), //
        L"sssctcicictcig",
        L"OCO", //
        L"ORO", //
        L"GDG", //
        L'O', Tile::obsidian, L'C', Item::minecart, L'R', Item::redstone, L'G', Tile::glass, L'D', Item::diamond,
        L'D');


    // neoLegacy - Amethyst Geodes (2026-11-26): Complex-Table-only recipes.
    // 2x2 Amethyst Blocks -> 4 Amethyst Bricks.
    addShapedRecipy(new ItemInstance(Tile::amethystBricks, 4), L"ssczg",
        L"##", L"##", L'#', new ItemInstance(Tile::amethystStone, 1, AmethystTile::AMETHYST_BLOCK), L'D');

    // 3x3 shards -> Amethyst Crystal Block; reverse recipe returns nine shards.
    addShapedRecipy(new ItemInstance(Tile::amethystCrystalBlock, 1), L"ssscig",
        L"###", L"###", L"###", L'#', Item::amethyst_shard, L'D');
    addShapedRecipy(new ItemInstance(Item::amethyst_shard, 9), L"sctg",
        L"#", L'#', Tile::amethystCrystalBlock, L'D');

    // Six bricks -> four stairs (both mirrored orientations).
    addShapedRecipy(new ItemInstance(Tile::amethystBrickStairs, 4), L"sssctg",
        L"B  ", L"BB ", L"BBB", L'B', Tile::amethystBricks, L'D');
    addShapedRecipy(new ItemInstance(Tile::amethystBrickStairs, 4), L"sssctg",
        L"  B", L" BB", L"BBB", L'B', Tile::amethystBricks, L'D');

    // Three bricks -> six slabs (existing StoneSlabTile2 metadata variant 3).
    addShapedRecipy(new ItemInstance(Tile::stone_slab2_Id, 6, StoneSlabTile2::AMETHYST_BRICKS_SLAB), L"sctg",
        L"BBB", L'B', Tile::amethystBricks, L'D');

    // Six bricks -> six walls.
    addShapedRecipy(new ItemInstance(Tile::amethystBrickWall, 6), L"ssctg",
        L"BBB", L"BBB", L'B', Tile::amethystBricks, L'D');

    // Four shards around glowstone -> one Amethyst Crystal Lamp.
    addShapedRecipy(new ItemInstance(Tile::amethystCrystalLamp, 1), L"ssscictg",
        L" S ", L"SGS", L" S ", L'S', Item::amethyst_shard, L'G', Tile::glowstone, L'D');

    // Spyglass (Java-faithful): one Amethyst Shard above two Copper Ingots.
    addShapedRecipy(new ItemInstance(Item::spyglass, 1), L"ssscicig",
        L" S ", L" I ", L" I ", L'S', Item::amethyst_shard, L'I', Item::copper_ingot, L'T');
	buildRecipeIngredientsArray();
}

void Recipes::_wipeRecipes()
{
	int iCount = recipies->size();

	for (int i = 0; i < iCount; i++) {
		Recipy::INGREDIENTS_REQUIRED& req = m_pRecipeIngredientsRequired[i];

		delete[] req.iIngIDA;
		delete[] req.iIngValA;
		delete[] req.iIngAuxValA;
		delete[] req.uiGridA;
	}

	recipies->clear();

	delete[] m_pRecipeIngredientsRequired;
	m_pRecipeIngredientsRequired = nullptr;
}

Recipes::Recipes()
{
	_init();
	_compileRecipes();
}

// 4J-PB - this function has been substantially changed due to the differences with a va_list of classes in C++ and Java
ShapedRecipy *Recipes::addShapedRecipy(ItemInstance *result, ...)
{
	wstring map = L"";
	int p = 0;
	int width = 0;
	int height = 0;
	int group = ShapedRecipy::eGroupType_Decoration;
	va_list vl;
	wchar_t *wchTypes;
	wchar_t *pwchString;
	wstring wString;
	wstring *wStringA;
	ItemInstance *pItemInstance;
	Tile *pTile;
	Item *pItem;
	wchar_t wchFrom;
	int iCount;
	ItemInstance **ids = nullptr;

	myMap *mappings = new unordered_map<wchar_t, ItemInstance *>();

	va_start(vl,result);
	// 4J-PB - second argument is a list of the types
	// s - string
	// w - string array
	// a - char *
	// c - char
	// z - ItemInstance *
	// i - Item *
	// t - Tile *
	// g - group [wt] - which group does the item created by the recipe belong in. Set a default until all recipes have a group

	wchTypes = va_arg(vl,wchar_t *);

	for(int i = 0; wchTypes[i] != L'\0'; ++i )
	{
		if(wchTypes[i+1]==L'\0' && wchTypes[i]!=L'g')
		{
			app.DebugPrintf("Missing group type\n");
		}

		switch(wchTypes[i])
		{
		case L'a':
			pwchString=va_arg(vl,wchar_t *);
			wString=pwchString;
			height++;
			width = static_cast<int>(wString.length());
			map += wString;
			break;
		case L's':
			pwchString=va_arg(vl,wchar_t *);
			wString=pwchString;
			height++;
			width = static_cast<int>(wString.length());
			map += wString;
			break;
		case L'w':
			wStringA=va_arg(vl,wstring *);
			iCount=0;
			do
			{
				wString=wStringA[iCount++];
				if(!wString.empty())
				{
					height++;
					width = static_cast<int>(wString.length());
					map += wString;
				}
			}
			while(!wString.empty());

			break;
		case L'c':
		    wchFrom=va_arg(vl,wchar_t);
		    break;
		case L'z':
			pItemInstance=va_arg(vl,ItemInstance *);
			mappings->insert(myMap::value_type(wchFrom,pItemInstance));
			break;
		case L'i':
			pItem=va_arg(vl,Item *);
			pItemInstance= new ItemInstance(pItem,1,ANY_AUX_VALUE);
			mappings->insert(myMap::value_type(wchFrom,pItemInstance));
			break;
		case L't':
			pTile=va_arg(vl,Tile *);
			pItemInstance= new ItemInstance(pTile,1,ANY_AUX_VALUE);
			mappings->insert(myMap::value_type(wchFrom,pItemInstance));
			break;
		case L'g':
			wchFrom=static_cast<wchar_t>(va_arg(vl,int));
			switch(wchFrom)
			{
// 			case L'W':
// 				group=ShapedRecipy::eGroupType_Weapon;
// 				break;
			case L'T':
				group=ShapedRecipy::eGroupType_Tool;
				break;
			case L'A':
				group=ShapedRecipy::eGroupType_Armour;
				break;
			case L'S':
				group=ShapedRecipy::eGroupType_Structure;
				break;
			case L'V':
				group=ShapedRecipy::eGroupType_Transport;
				break;
			case L'M':
				group=ShapedRecipy::eGroupType_Mechanism;
				break;
			case L'F':
				group=ShapedRecipy::eGroupType_Food;
				break;
			case L'B':
				group=ShapedRecipy::eGroupType_Banner_CustomLogic;
				break;
			case L'D':
			default:
				group=ShapedRecipy::eGroupType_Decoration;
				break;

			}
			break;

		}


		ids = new ItemInstance *[width * height];

		for (int j = 0; j < width * height; j++)
		{
			wchar_t ch = map[j];
			myMap::iterator it=mappings->find(ch);
			if (it != mappings->end())
			{
				ids[j] =it->second;
			}
			else
			{
				ids[j] = nullptr;
			}
		}
	}

	va_end(vl);

	ShapedRecipy *recipe = new ShapedRecipy(width, height, ids, result, group);
	recipies->push_back(recipe);
	return recipe;
}

void Recipes::addShapelessRecipy(ItemInstance *result,... )
{
	va_list vl;
	wchar_t *szTypes;
	wstring String;
	ItemInstance *pItemInstance;
	Tile *pTile;
	Item *pItem;
	Recipy::_eGroupType group = Recipy::eGroupType_Decoration;
	wchar_t wchFrom;
	vector <ItemInstance *> *ingredients = new vector<ItemInstance *>();

	va_start(vl,result);
	// 4J-PB - second argument is a list of the types
	// z - ItemInstance *
	// i - Item *
	// t - Tile *
	szTypes = va_arg(vl,wchar_t *);

	for(int i = 0; szTypes[i] != L'\0'; ++i )
	{
		switch(szTypes[i])
		{
		case L'z':
			pItemInstance=va_arg(vl,ItemInstance *);
			// 4J-PB - original code copies the item instance, copy the pointer isnt the same...
			// TODO
			ingredients->push_back(pItemInstance->copy_not_shared());
			break;
		case L'i':
			pItem=va_arg(vl,Item *);
			pItemInstance= new ItemInstance(pItem);
			ingredients->push_back(pItemInstance);
			break;
		case L't':
			pTile=va_arg(vl,Tile *);
			ingredients->push_back(new ItemInstance(pTile));
			break;
		case L'g':
			wchFrom=va_arg(vl,wchar_t);
			switch(wchFrom)
			{

			case L'T':
				group=Recipy::eGroupType_Tool;
				break;
			case L'A':
				group=Recipy::eGroupType_Armour;
				break;
			case L'S':
				group=Recipy::eGroupType_Structure;
				break;
			case L'V':
				group=Recipy::eGroupType_Transport;
				break;
			case L'M':
				group=Recipy::eGroupType_Mechanism;
				break;
			case L'F':
				group=Recipy::eGroupType_Food;
				break;
			case L'B':
				group=Recipy::eGroupType_Banner_CustomLogic;
				break;
			case L'D':
			default:
				group=Recipy::eGroupType_Decoration;
				break;

			}
			break;		}
	}

	recipies->push_back(new ShapelessRecipy(result, ingredients, group));
}

shared_ptr<ItemInstance> Recipes::getItemFor(shared_ptr<CraftingContainer> craftSlots, Level *level, Recipy *recipesClass /*= nullptr*/)
{
	int count = 0;
	shared_ptr<ItemInstance> first = nullptr;
	shared_ptr<ItemInstance> second = nullptr;
	for (int i = 0; i < craftSlots->getContainerSize(); i++)
	{
		shared_ptr<ItemInstance> item = craftSlots->getItem(i);
		if (item != nullptr)
		{
			if (count == 0) first = item;
			if (count == 1) second = item;
			count++;
		}
	}

	if (count == 2 && first->id == second->id && first->count == 1 && second->count == 1 && Item::items[first->id]->canBeDepleted())
	{
		Item *item = Item::items[first->id];
		int remaining1 = item->getMaxDamage() - first->getDamageValue();
		int remaining2 = item->getMaxDamage() - second->getDamageValue();
		int remaining = (remaining1 + remaining2) + item->getMaxDamage() * 5 / 100;
		int resultDamage = item->getMaxDamage() - remaining;
		if (resultDamage < 0) resultDamage = 0;
		return std::make_shared<ItemInstance>(first->id, 1, resultDamage);
	}

	if(recipesClass != nullptr)
	{
		if (recipesClass->matches(craftSlots, level)) return recipesClass->assemble(craftSlots);
	}
	else
	{
		for (auto& r : *recipies)
		{
			if (isComplexOnly(r)) continue;	// neoLegacy - Complex Table recipes only match at a Complex Table
			if (r->matches(craftSlots, level)) return r->assemble(craftSlots);
		}
		// neoLegacy - a banner and dyes in a pattern add a pattern to the banner (free-form 3x3 grids)
		if (pBannerPatternRecipe != nullptr && pBannerPatternRecipe->matches(craftSlots, level)) return pBannerPatternRecipe->assemble(craftSlots);
	}
	return nullptr;
}

// neoLegacy - Complex Table-only recipes include late-game machinery, selected custom items, the MAC-10, and 9mm ammunition.
bool Recipes::isComplexOnly(Recipy *r)
{
	if (r == nullptr) return false;
	const ItemInstance *result = r->getResultItem();
	if (result == nullptr) return false;
	if (result->id == Tile::concrete_powder_Id
		|| result->id == Tile::concrete_stairs_Id
		|| result->id == Tile::concrete_slab_half_Id
		|| result->id == Tile::drill_Id
		|| result->id == Tile::crusher_Id
		|| result->id == Tile::refiner_Id
        || result->id == Tile::cookie_miner_Id
        || result->id == Tile::flappy_craft_Id
        || result->id == Tile::mine_snake_Id
		|| result->id == Item::diamond_minecart_Id
		|| result->id == Item::cursed_hilt_Id		// neoLegacy - Zoro sword forging
		|| result->id == Item::wado_ichimonji_Id
		|| result->id == Item::sandai_kitetsu_Id
		|| result->id == Item::enma_Id
		|| result->id == Item::shusui_Id
		|| result->id == Item::mac10_Id
		|| result->id == Item::bullet_9mm_Id
		|| result->id == Item::m16_Id			// neoLegacy - M16, 5.56 rounds, Flintlock Pistols
		|| result->id == Item::bullet_556_Id
		|| result->id == Item::flintlock_copper_Id
		|| result->id == Item::flintlock_iron_Id
		|| result->id == Item::flintlock_gold_Id
		|| result->id == Item::mace_Id			// neoLegacy - Mace
		|| result->id == Item::ice_ball_Id		// neoLegacy - Ice Ball <-> Ice
		|| result->id == Tile::ice_Id
		// neoLegacy - Amethyst Geodes (2026-11-26): Complex-Table-only like the copper recipes.
		|| result->id == Tile::amethyst_bricks_Id
		|| result->id == Tile::amethyst_brick_stairs_Id
		|| result->id == Tile::amethyst_brick_wall_Id
		|| result->id == Tile::amethyst_crystal_block_Id
		|| result->id == Tile::amethyst_crystal_lamp_Id
		|| result->id == Item::amethyst_shard_Id
		|| result->id == Item::spyglass_Id
		|| result->id == Item::enchantment_encyclopedia_Id	// neoLegacy - Enchantment Encyclopedia
		// neoLegacy - Illager Invasion recipes: Complex-Table-only, like the
		// copper recipes below (registered in the shared list, gated here).
		|| result->id == Item::platinum_sheet_Id
		|| result->id == Item::hallowed_gem_Id
		|| result->id == Item::horn_of_sight_Id
		// neoLegacy - Copper Age (Phase 3). Every Phase 3 recipe result is
		// Complex-Table-only. Tiles (2305 raw copper block + 2306-2319 +
		// 2321-2327; the golem statue 2320 had no recipe in Phase 3, and
		// the full cut-copper slab 2328 is placed by stacking, not crafted).
		// neoLegacy - Copper Age (Phase 4). The golem statue 2320 now HAS
		// recipes (the 4 waxing recipes), so it is added here to keep them
		// Complex-Table-only like every other copper recipe.
		|| result->id == Tile::copper_golem_statue_Id
		|| result->id == Tile::raw_copper_block_Id
		|| result->id == Item::raw_copper_Id	// neoLegacy - Copper Age (Phase 3). Raw copper (reverse recipe: raw copper block -> 9 raw copper)
		|| result->id == Tile::copper_block_Id
		|| result->id == Tile::cut_copper_Id
		|| result->id == Tile::chiseled_copper_Id
		|| result->id == Tile::copper_grate_Id
		|| result->id == Tile::copper_bulb_Id
		|| result->id == Tile::copper_chest_Id
		|| result->id == Tile::copper_door_Id
		|| result->id == Tile::copper_trapdoor_Id
		|| result->id == Tile::copper_chain_Id
		|| result->id == Tile::copper_bars_Id
		|| result->id == Tile::copper_lantern_Id
		|| result->id == Tile::iron_chain_Id	// neoLegacy - Iron additions (2026-11-22): Complex-Table-only like the copper chain
		|| result->id == Tile::iron_lantern_Id	// neoLegacy - Iron additions (2026-11-22): Complex-Table-only like the copper lantern
		|| result->id == Tile::copper_torch_Id
		|| result->id == Tile::copper_button_Id
		|| result->id == Tile::lightning_rod_Id
		// neoLegacy - Copper Age: shelves are Complex-Table-only
		// (Cameron 2026-11-23). The 6-stripped-logs recipes live in
		// StructureRecipies.cpp; the normal table skips Complex-only
		// recipes, so this gate must return true for shelf results.
		|| result->id == Tile::shelf_Id
		|| result->id == Tile::cut_copper_slab_Id
		|| result->id == Tile::cut_copper_stairs_Id
		|| result->id == Tile::rose_gold_block_Id
		|| result->id == Tile::raw_rose_gold_block_Id
		|| result->id == Tile::rose_gold_door_Id
		|| result->id == Tile::rose_gold_trapdoor_Id
		// Copper Age (Phase 3) items: ingot/nugget recycling, the raw
		// rose-gold shapeless recipes, the table-driven tools + armour,
		// and the two door items.
		|| result->id == Item::copper_ingot_Id
		|| result->id == Item::copper_nugget_Id
		|| result->id == Item::copper_sword_Id
		|| result->id == Item::copper_pickaxe_Id
		|| result->id == Item::copper_axe_Id
		|| result->id == Item::copper_shovel_Id
		|| result->id == Item::copper_hoe_Id
		|| result->id == Item::copper_helmet_Id
		|| result->id == Item::copper_chestplate_Id
		|| result->id == Item::copper_leggings_Id
		|| result->id == Item::copper_boots_Id
		|| result->id == Item::raw_rose_gold_Id
		|| result->id == Item::rose_gold_ingot_Id
		|| result->id == Item::rose_gold_nugget_Id
		|| result->id == Item::rose_gold_sword_Id
		|| result->id == Item::rose_gold_pickaxe_Id
		|| result->id == Item::rose_gold_axe_Id
		|| result->id == Item::rose_gold_shovel_Id
		|| result->id == Item::rose_gold_hoe_Id
		|| result->id == Item::rose_gold_helmet_Id
		|| result->id == Item::rose_gold_chestplate_Id
		|| result->id == Item::rose_gold_leggings_Id
		|| result->id == Item::rose_gold_boots_Id
		|| result->id == Item::copper_door_Id
		|| result->id == Item::rose_gold_door_Id
		|| result->id == Item::rose_gilded_beetroot_Id	// neoLegacy - Copper Age (Phase 6)
		|| result->id == Item::rose_gilded_mushroom_Id
		|| result->id == Item::rose_gilded_cookie_Id
		|| result->id == Item::rose_full_jelly_Id
		|| result->id == Item::rose_half_jelly_Id
		|| result->id == Item::rose_mushroom_omelette_Id
		|| result->id == Item::rose_mushroom_partial_Id
		|| result->id == Item::rose_mushroom_crumbs_Id
		|| result->id == Item::rosey_beetroot_stew_Id)
	{
		return true;
	}

	// neoLegacy - Nugget exchange (2026-11-26): the 1 -> 9 recipes are gated
	// on their result ids, which no other recipe produces. The 9 -> 1 recipes
	// cannot be gated on result alone (iron_ingot_Id is shared with vanilla
	// recipes; netherite_ingot_Id is also the result of the shapeless
	// scrap+gold ingot recipe at line 1708, which must stay workbench), so
	// they are gated on result + ingredient, red-dye pattern.
	if (result->id == Item::iron_nugget_Id) return true;
	if (result->id == Item::netherite_nugget_Id) return true;
	if (result->id == Item::iron_ingot_Id)
	{
		ShapedRecipy *shaped = dynamic_cast<ShapedRecipy *>(r);
		return shaped != nullptr && shaped->hasIngredient(Item::iron_nugget_Id);
	}
	if (result->id == Item::netherite_ingot_Id)
	{
		ShapedRecipy *shaped = dynamic_cast<ShapedRecipy *>(r);
		return shaped != nullptr && shaped->hasIngredient(Item::netherite_nugget_Id);
	}
	// Red dye has other (normal workbench) recipes, e.g. rose -> 2 red dye, so identify the
	// Complex Table one by its redstone ingredient rather than by its result alone.
	if (result->id == Item::dye_Id && result->getAuxValue() == DyePowderItem::RED)
	{
		ShapelessRecipy *sr = dynamic_cast<ShapelessRecipy *>(r);
		return sr != nullptr && sr->hasIngredient(Item::redstone_Id);
	}
	// neoLegacy - Amethyst Geodes (2026-11-26): the amethyst brick slab is a
	// StoneSlabTile2 metadata variant, so gate it on the aux value — not on
	// stone_slab2_Id alone, which would gate every slab variant.
	if (result->id == Tile::stone_slab2_Id && result->getAuxValue() == StoneSlabTile2::AMETHYST_BRICKS_SLAB) return true;
	return false;
}

// neoLegacy - Complex Table: grid matching that considers ONLY the Complex-Table-only recipes.
shared_ptr<ItemInstance> Recipes::getComplexItemFor(shared_ptr<CraftingContainer> craftSlots, Level *level)
{
	for (auto& r : *recipies)
	{
		if (!isComplexOnly(r)) continue;
		if (r->matches(craftSlots, level)) return r->assemble(craftSlots);
	}
	return nullptr;
}

vector <Recipy *> *Recipes::getRecipies()
{
	return recipies;
}

// 4J-PB - added to deal with Xb0x 'crafting'
shared_ptr<ItemInstance> Recipes::getItemForRecipe(Recipy *r)
{
	return r->assemble(nullptr);
}

// 4J-PB - build the required ingredients for recipes
void Recipes::buildRecipeIngredientsArray(void)
{
	//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
	 
	int iRecipeC=static_cast<int>(recipies->size());

	m_pRecipeIngredientsRequired = new Recipy::INGREDIENTS_REQUIRED [iRecipeC];

	int iCount=0;
	for (auto& recipe : *recipies)
	{
		recipe->reqs(&m_pRecipeIngredientsRequired[iCount++]);
	}

	//printf("Total recipes in buildRecipeIngredientsArray - %d",iCount);
}

Recipy::INGREDIENTS_REQUIRED *Recipes::getRecipeIngredientsArray(void)
{
	return m_pRecipeIngredientsRequired;
}

void Recipes::loadFromLocal()
{
	this->_wipeRecipes();
	this->_compileRecipes();
}

void Recipes::loadFromPacket(byteArray packetData)
{
	ByteArrayInputStream bais(packetData);
	DataInputStream input(&bais);

	// Chunked registry transfer (see createUpdatePacket): the first chunk
	// wipes the local registry, continuation chunks append to it, and the
	// ingredients index is rebuilt only once, on the last chunk.
	bool isContinuation = input.readByte() != 0;
	bool isLast = input.readByte() != 0;
	if (!isContinuation)
	{
		this->_wipeRecipes();
	}
	{
		int iCount = input.readInt();
		for (int i = 0; i < iCount; i++) {
			int recipeType = input.readByte();

			if (recipeType == 1) {
				recipies->push_back(ShapelessRecipy::readFromStream(&input));
			} else if (recipeType == 2) {
				recipies->push_back(ShapedRecipy::readFromStream(&input));
			}
		}
	}
	if (isLast)
	{
		this->buildRecipeIngredientsArray();
	}
}

// The recipe registry no longer fits in a single custom-payload packet:
// payloads are capped at Short::MAX_VALUE (32767) bytes and the
// CustomPayloadPacket constructor DEBUG_BREAKs past it, which killed the
// game at login (crash on start, Nov 26). Split the registry into chunks,
// each well under the cap. Chunk data layout:
//   byte isContinuation (0 = first chunk, 1 = continuation)
//   byte isLast         (0 = more chunks follow, 1 = last chunk)
//   int  recipeCount    (recipes carried by this chunk)
//   ...recipe bytes...
// The client wipes its registry on the first chunk, appends on continuation
// chunks, and rebuilds the ingredients index on the last chunk
// (see loadFromPacket).
static const int RECIPE_REGISTRY_CHUNK_MAX_BYTES = 30000;

std::vector<std::shared_ptr<CustomPayloadPacket>> Recipes::createUpdatePacket()
{
	// Serialize every recipe once so we know each one's exact size.
	// (byteArray does not free its data, so each one is delete[]'d below.)
	std::vector<byteArray> recipeBytes;
	recipeBytes.reserve(recipies->size());
	for (int i = 0; i < (int)recipies->size(); i++)
	{
		ByteArrayOutputStream recipeBaos;
		DataOutputStream recipeDos(&recipeBaos);
		(*recipies)[i]->writeToStream(&recipeDos);
		recipeBytes.push_back(recipeBaos.toByteArray());
	}

	std::vector<std::shared_ptr<CustomPayloadPacket>> packets;
	size_t next = 0;
	bool isFirstChunk = true;
	while (next < recipeBytes.size())
	{
		ByteArrayOutputStream chunkBaos;
		DataOutputStream chunkDos(&chunkBaos);
		int chunkCount = 0;
		// Fill one chunk without exceeding the payload cap. Always take at
		// least one recipe per chunk so we always make progress.
		while (next < recipeBytes.size() &&
		       (chunkCount == 0 ||
		        chunkBaos.size() + recipeBytes[next].length <= RECIPE_REGISTRY_CHUNK_MAX_BYTES))
		{
			chunkDos.write(recipeBytes[next]);
			chunkCount++;
			next++;
		}

		bool isLast = (next >= recipeBytes.size());
		byteArray chunkData = chunkBaos.toByteArray();
		ByteArrayOutputStream packetBaos;
		DataOutputStream packetDos(&packetBaos);
		packetDos.writeByte(isFirstChunk ? 0 : 1);
		packetDos.writeByte(isLast ? 1 : 0);
		packetDos.writeInt(chunkCount);
		packetDos.write(chunkData);
		delete[] chunkData.data;

		// The packet takes over this buffer (same as the old single-packet
		// code, which also passed baos.toByteArray() straight in).
		byteArray packetData = packetBaos.toByteArray();
		packets.push_back(std::make_shared<CustomPayloadPacket>(
			CustomPayloadPacket::UPDATE_RECIPE_REGISTRY, packetData));
		isFirstChunk = false;
	}

	for (size_t i = 0; i < recipeBytes.size(); i++)
		delete[] recipeBytes[i].data;

	if (packets.empty())
	{
		// No recipes at all: still send one (empty) chunk so the client
		// wipes its registry, matching the old single-packet behaviour.
		ByteArrayOutputStream packetBaos;
		DataOutputStream packetDos(&packetBaos);
		packetDos.writeByte(0);
		packetDos.writeByte(1);
		packetDos.writeInt(0);
		byteArray packetData = packetBaos.toByteArray();
		packets.push_back(std::make_shared<CustomPayloadPacket>(
			CustomPayloadPacket::UPDATE_RECIPE_REGISTRY, packetData));
	}
	return packets;
}
