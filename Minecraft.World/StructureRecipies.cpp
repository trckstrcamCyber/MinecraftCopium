#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "BlackStoneTile.h"
#include "WoodTile.h"
#include "StrippedLogTile.h"
#include "Recipy.h"
#include "Recipes.h"
#include "StructureRecipies.h"

void StructureRecipies::addRecipes(Recipes *r)
{
	r->addShapedRecipy(new ItemInstance(Tile::sandStone), //
		L"ssctg",
		L"##", //
		L"##", //

		L'#', Tile::sand,
		L'S');

    // neoLegacy - Bamboo: 9 Bamboo make a Block of Bamboo
    r->addShapedRecipy(new ItemInstance(Tile::bambooBlock, 1), //
        L"ssscig",
        L"###", //
        L"###", //
        L"###", //

        L'#', Item::bamboo,
        L'S');

    // neoLegacy - Bamboo: a Block of Bamboo (stripped or not) makes 2 Bamboo Planks (planks, data 6)
    r->addShapelessRecipy(new ItemInstance(Tile::wood, 2, WoodTile::BAMBOO_PLANKS), //
        L"tg",
        Tile::bambooBlock,
        L'S');
    r->addShapelessRecipy(new ItemInstance(Tile::wood, 2, WoodTile::BAMBOO_PLANKS), //
        L"zg",
        new ItemInstance(Tile::strippedLog2, 1, StrippedLogTile::BAMBOO - 4),
        L'S');

    // neoLegacy - Stripped logs make 4 planks of their own wood, just like the plain logs (planks data 0-5 = oak..dark oak)
    for (int wood = StrippedLogTile::OAK; wood <= StrippedLogTile::DARK_OAK; ++wood)
    {
        Tile *strippedTile = (wood < 4) ? Tile::strippedLog : Tile::strippedLog2;
        int strippedData = (wood < 4) ? wood : (wood - 4);
        r->addShapedRecipy(new ItemInstance(Tile::wood, 4, wood), //
            L"sczg",
            L"#", //

            L'#', new ItemInstance(strippedTile, 1, strippedData),
            L'S');
    }

    r->addShapedRecipy(new ItemInstance(Tile::calcite, 6),

        L"ssczcig",
        L"GG",
        L"GQ",

        L'G', new ItemInstance(Tile::stone, 1, StoneTile::DIORITE),
        L'Q', Item::nether_quartz,
        L'S');

    r->addShapedRecipy(new ItemInstance(Tile::blackstone, 6),

        L"ssczcig",
        L"GG",
        L"GQ",

        L'G', new ItemInstance(Tile::stone, 1, 0),
        L'Q', Item::coal,
        L'S');

    r->addShapedRecipy(new ItemInstance(Tile::blackstone, 4, BlackStoneTile::GILDED_BLACKSTONE),

        L"ssczcig",
        L"GG",
        L"GQ",

        L'G', new ItemInstance(Tile::blackstone, 1, BlackStoneTile::BLACKSTONE),
        L'Q', Item::gold_nugget,
        L'S');

	r->addShapedRecipy(new ItemInstance(Tile::sandStone, 4, SandStoneTile::TYPE_SMOOTHSIDE), //
		L"ssczg",
		L"##", //
		L"##", //

		L'#', new ItemInstance(Tile::sandStone),
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::sandStone, 1, SandStoneTile::TYPE_HEIROGLYPHS), //
		L"ssczg",
		L"#", //
		L"#", //

		L'#', new ItemInstance(Tile::stoneSlabHalf, 1, StoneSlabTile::SAND_SLAB),
		L'S');


	r->addShapedRecipy(new ItemInstance(Tile::red_sandstone), //
		L"ssczg",
		L"##", //
		L"##", //

		L'#',new ItemInstance(Tile::sand, 1, SandTile::RED_SAND),
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::red_sandstone, 4, RedSandStoneTile::TYPE_SMOOTHSIDE), //
		L"ssczg",
		L"##", //
		L"##", //

		L'#', new ItemInstance(Tile::red_sandstone),
		L'S');


	r->addShapedRecipy(new ItemInstance(Tile::red_sandstone, 1, RedSandStoneTile::TYPE_HEIROGLYPHS), //
		L"ssczg",
		L"#", //
		L"#", //

		L'#', new ItemInstance(Tile::stoneSlab2Half, 1, StoneSlabTile2::RED_SANDSTONE_SLAB),
		L'S');




	r->addShapedRecipy(new ItemInstance(Tile::quartzBlock, 1, QuartzBlockTile::TYPE_CHISELED), //
		L"ssczg",
		L"#", //
		L"#", //

		L'#', new ItemInstance(Tile::stoneSlabHalf, 1, StoneSlabTile::QUARTZ_SLAB),
		L'S');

    r->addShapedRecipy(new ItemInstance(Tile::blackstone, 1, BlackStoneTile::CHISELED_POLISHED_BLACKSTONE),
        L"ssczg",
        L"#",
        L"#",

        L'#', new ItemInstance(Tile::stoneSlab2Half, 1, StoneSlabTile2::BLACKSTONE_SLAB),
        L'S');

	r->addShapedRecipy(new ItemInstance(Tile::quartzBlock, 2, QuartzBlockTile::TYPE_LINES_Y), //
		L"ssczg",
		L"#", //
		L"#", //

		L'#', new ItemInstance(Tile::quartzBlock, 1, QuartzBlockTile::TYPE_DEFAULT),
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::stone_Id, 2, StoneTile::DIORITE), //
		L"ssctcig",
		L"#Q", //
		L"Q#", //

		L'#', Tile::cobblestone, L'Q', Item::nether_quartz,
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::stone_Id, 1, StoneTile::GRANITE), //
		L"sczcig",
		L"#Q", //

		L'#', new ItemInstance(Tile::stone_Id, 1, StoneTile::DIORITE), L'Q', Item::nether_quartz,
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::stone_Id, 2, StoneTile::ANDESITE), //
		L"sczctg",
		L"#-", //

		L'#', new ItemInstance(Tile::stone_Id, 1, StoneTile::DIORITE), L'-', Tile::cobblestone,
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::stone_Id, 4, StoneTile::POLISHED_DIORITE), //
		L"ssczg",
		L"##", //
		L"##", //

		L'#', new ItemInstance(Tile::stone_Id, 1, StoneTile::DIORITE),
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::stone_Id, 4, StoneTile::POLISHED_GRANITE), //
		L"ssczg",
		L"##", //
		L"##", //

		L'#', new ItemInstance(Tile::stone_Id, 1, StoneTile::GRANITE),
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::stone_Id, 4, StoneTile::POLISHED_ANDESITE), //
		L"ssczg",
		L"##", //
		L"##", //

		L'#', new ItemInstance(Tile::stone_Id, 1, StoneTile::ANDESITE),
		L'S');





	// 4J Stu - Changed the order, as the blocks that go with sandstone cause a 3-icon scroll
	// that touches the text "Structures" in the title in 720 fullscreen.
	r->addShapedRecipy(new ItemInstance(Tile::workBench), //
		L"ssctg",
		L"##", //
		L"##", //

		L'#', Tile::wood,
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::furnace), //
		L"sssctg",
		L"###", //
		L"# #", //
		L"###", //

		L'#', Tile::cobblestone,
		L'S');

	r->addShapedRecipy(new ItemInstance(static_cast<Tile *>(Tile::chest)), //
		L"sssctg",
		L"###", //
		L"# #", //
		L"###", //

		L'#', Tile::wood,
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::chest_trap), //
		L"sctctg",
		L"#-", //

		L'#', Tile::chest, L'-', Tile::tripWireSource,
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::enderChest), //
		L"sssctcig",
		L"###", //
		L"#E#", //
		L"###", //

		L'#', Tile::obsidian, L'E', Item::eye_of_ender,
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::stoneBrick, 4), //
		L"ssczg",
		L"##", //
		L"##", //

		L'#', new ItemInstance(Tile::stone, 1, 0),
		L'S');
    r->addShapedRecipy(new ItemInstance(Tile::blackstone, 4, BlackStoneTile::POLISHED_BLACKSTONE_BRICKS),
        L"ssczg",
        L"##",
        L"##",

        L'#', new ItemInstance(Tile::blackstone, 1, BlackStoneTile::POLISHED_BLACKSTONE),
        L'S');

    r->addShapedRecipy(new ItemInstance(Tile::blackstone, 4, BlackStoneTile::POLISHED_BLACKSTONE),
        L"ssczg",
        L"##", //
        L"##", //

        L'#', new ItemInstance(Tile::blackstone, 1, BlackStoneTile::BLACKSTONE),
        L'S');

	r->addShapedRecipy(new ItemInstance(Tile::stoneBrick, 1, SmoothStoneBrickTile::TYPE_DETAIL), //
		L"ssczg",
		L"#", //
		L"#", //

		L'#', new ItemInstance(Tile::stoneSlabHalf, 1, StoneSlabTile::SMOOTHBRICK_SLAB),
		L'S');
	r->addShapedRecipy(new ItemInstance(Tile::stoneBrick, 1, SmoothStoneBrickTile::TYPE_MOSSY), //
		L"sczc zg",
		L"#1",
		L'#', new ItemInstance(Tile::stoneBrick,1),
		L'1', new ItemInstance(Tile::vine, 1),
		L'S');


	r->addShapedRecipy(new ItemInstance(Tile::mossyCobblestone, 1), //
		L"sczc zg",
		L"#1", //

		L'#', new ItemInstance(Tile::cobblestone,1),
		L'1', new ItemInstance(Tile::vine, 1),
		L'S');

	// 4J Stu - Move this into "Recipes" to change the order things are displayed on the crafting menu
	//r->addShapedRecipy(new ItemInstance(Tile::iron_bars, 16), //
	//	L"sscig",
	//	L"###", //
	//	L"###", //

	//	L'#', Item::iron_ingot,
	//	L'S');

	r->addShapedRecipy(new ItemInstance(Tile::glass_pane, 16), //
		L"ssctg",
		L"###", //
		L"###", //

		L'#', Tile::glass,
		L'D');




// Stained Glass block + pane per color
for (int i = 0; i < 16; i++)
{
    r->addShapedRecipy(new ItemInstance(Tile::stained_glass, 8, ColoredTile::getItemAuxValueForTileData(i)),
        L"sssczczg",
        L"###",
        L"#X#",
        L"###",
        L'#', new ItemInstance(Tile::glass),
        L'X', new ItemInstance(Item::dye, 1, i),
        L'D');
    r->addShapedRecipy(new ItemInstance(Tile::stained_glass_pane, 16, ColoredTile::getItemAuxValueForTileData(i)),
        L"ssczg",
        L"###",
        L"###",
        L'#', new ItemInstance(Tile::stained_glass, 1, ColoredTile::getItemAuxValueForTileData(i)),
        L'D');
}

		for (int i = 0; i < 16; i++)
{
    r->addShapedRecipy(new ItemInstance(Tile::stained_glass, 8, ColoredTile::getItemAuxValueForTileData(i)),
        L"sssczczg",
        L"###",
        L"#X#",
        L"###",
        L'#', new ItemInstance(Tile::glass),
        L'X', new ItemInstance(Item::dye, 1, i),
        L'D');
    r->addShapedRecipy(new ItemInstance(Tile::stained_glass_pane, 16, ColoredTile::getItemAuxValueForTileData(i)),
        L"ssczg",
        L"###",
        L"###",
        L'#', new ItemInstance(Tile::stained_glass, 1, ColoredTile::getItemAuxValueForTileData(i)),
        L'D');
}

	r->addShapedRecipy(new ItemInstance(Tile::netherBrick, 1), //
		L"sscig",
		L"NN", //
		L"NN", //

		L'N', Item::netherbrick,
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::redstoneLight, 1), //
		L"ssscictg",
		L" R ", //
		L"RGR", //
		L" R ", //
		L'R', Item::redstone, 'G', Tile::glowstone,
		L'M');

	r->addShapedRecipy(new ItemInstance(Tile::beacon, 1), //
		L"sssctcictg",
		L"GGG", //
		L"GSG", //
		L"OOO", //

		L'G', Tile::glass, L'S', Item::nether_star, L'O', Tile::obsidian,
		L'M');

	// Grindstone: stick, stone slab, stick / plank, -, plank
	// VERIFY: group letter 'M' (Mechanism) chosen to match the Lucky Block
	// recipe's placement a few lines up in this file -- neither Anvil nor
	// Enchanting Table is registered in this file, so there was nothing to
	// check their actual group letter against. If they use something other
	// than 'M', change this one letter to match.
	r->addShapedRecipy(new ItemInstance(Tile::grindstone_Id, 1, 0),

		L"ssciczctg",
		L"TLT",
		L"P P",

		L'T', Item::stick,
		L'L', new ItemInstance(Tile::stone_slab_Id, 1, 0),
		L'P', Tile::wood,
		L'S');

	// =================================================================
	// neoLegacy - Copper Age (Phase 3). Copper + rose-gold blocks.
	// All patterns are the jar1201 / jar1165 (rosegold) recipes, verified
	// in copper-rose-gold/work/jar1201-inventory.md and
	// jar1165-rosegold-inventory.md. Group letter 'S' (Structure) on
	// every recipe; every result id is listed in Recipes::isComplexOnly,
	// so these are Complex-Table-only. Deviations from the Phase 3 brief,
	// each following the jar over the brief, are marked DEVIATION below.
	// =================================================================

	// ---- block bucket (13) ----
	// Copper block: 3x3 copper ingots -> 1 (vanilla).
	r->addShapedRecipy(new ItemInstance(Tile::copperBlock, 1), //
		L"ssscig",
		L"###", //
		L"###", //
		L"###", //

		L'#', Item::copper_ingot,
		L'S');

	// Cut copper: 2x2 copper blocks -> 4 (vanilla).
	r->addShapedRecipy(new ItemInstance(Tile::cutCopper, 4), //
		L"ssctg",
		L"##", //
		L"##", //

		L'#', Tile::copperBlock,
		L'S');

	// Chiseled copper: 2 cut-copper slabs, one column -> 1 (jar1201).
	r->addShapedRecipy(new ItemInstance(Tile::chiseledCopper, 1), //
		L"ssczg",
		L"#", //
		L"#", //

		L'#', new ItemInstance(Tile::cutCopperSlabHalf, 1, 0),
		L'S');

	// Raw copper block: 3x3 raw copper -> 1 (vanilla; the tile is Phase 1,
	// the recipe is new in Phase 3).
	r->addShapedRecipy(new ItemInstance(Tile::rawCopperBlock, 1), //
		L"ssscig",
		L"###", //
		L"###", //
		L"###", //

		L'#', Item::raw_copper,
		L'S');

	// Copper grate: cross of 4 copper blocks -> 4 (jar1201).
	r->addShapedRecipy(new ItemInstance(Tile::copperGrate, 4), //
		L"sssctg",
		L" # ", //
		L"# #", //
		L" # ", //

		L'#', Tile::copperBlock,
		L'S');

	// Copper bulb: 3 copper blocks + blaze rod + redstone -> 4 (jar1201:
	// pattern [ C / CBC / R ]).
	r->addShapedRecipy(new ItemInstance(Tile::copperBulb, 4), //
		L"sssctcicig",
		L" C ", //
		L"CBC", //
		L" R ", //

		L'C', Tile::copperBlock,
		L'B', Item::blaze_rod,
		L'R', Item::redstone,
		L'S');

	// Shelf: 6 stripped logs (two rows of 3, middle row empty) -> 6
	// shelves, per wood x7 (jar1201: [### / / ###]).
	for (int wood = StrippedLogTile::OAK; wood <= StrippedLogTile::BAMBOO; ++wood)
	{
		Tile *strippedTile = (wood < 4) ? Tile::strippedLog : Tile::strippedLog2;
		int strippedData = (wood < 4) ? wood : (wood - 4);
		r->addShapedRecipy(new ItemInstance(Tile::shelf, 6, wood), //
			L"sssczg",
			L"###", //
			L"   ", //
			L"###", //

			L'#', new ItemInstance(strippedTile, 1, strippedData),
			L'S');
	}

	// Rose gold block: 3x3 rose-gold ingots -> 1 (rosegold jar).
	r->addShapedRecipy(new ItemInstance(Tile::roseGoldBlock, 1), //
		L"ssscig",
		L"###", //
		L"###", //
		L"###", //

		L'#', Item::rose_gold_ingot,
		L'S');

	// Raw rose gold block: 3x3 raw rose gold -> 1 (rosegold jar).
	r->addShapedRecipy(new ItemInstance(Tile::rawRoseGoldBlock, 1), //
		L"ssscig",
		L"###", //
		L"###", //
		L"###", //

		L'#', Item::raw_rose_gold,
		L'S');

	// Cut copper stairs: standard stair pattern -> 4 (jar1165).
	r->addShapedRecipy(new ItemInstance(Tile::cutCopperStairs, 4), //
		L"sssctg",
		L"#  ", //
		L"## ", //
		L"###", //

		L'#', Tile::cutCopper,
		L'S');

	// Cut copper slab: 3 cut copper -> 6 (jar1165).
	r->addShapedRecipy(new ItemInstance(Tile::cutCopperSlabHalf, 6), //
		L"sctg",
		L"###", //

		L'#', Tile::cutCopper,
		L'S');

	// ---- door bucket (4) ----
	// Copper door: 6 ingots -> 3 (jar1201).
	r->addShapedRecipy(new ItemInstance(Item::copper_door, 3), //
		L"ssscig",
		L"##", //
		L"##", //
		L"##", //

		L'#', Item::copper_ingot,
		L'S');

	// Rose gold door: 6 ingots -> 3 (rosegold jar).
	r->addShapedRecipy(new ItemInstance(Item::rose_gold_door, 3), //
		L"ssscig",
		L"##", //
		L"##", //
		L"##", //

		L'#', Item::rose_gold_ingot,
		L'S');

	// Copper trapdoor: 4 ingots -> 1 (jar1201).
	r->addShapedRecipy(new ItemInstance(Tile::copperTrapdoor, 1), //
		L"sscig",
		L"##", //
		L"##", //

		L'#', Item::copper_ingot,
		L'S');

	// Rose gold trapdoor: 6 ingots -> 2 (rosegold jar: ["###","###"]).
	// DEVIATION from the brief ("4 ingots -> 2"): the jar uses 6 ingots
	// (3x2) for 2 trapdoors, and the jar is followed here.
	r->addShapedRecipy(new ItemInstance(Tile::roseGoldTrapdoor, 2), //
		L"sscig",
		L"###", //
		L"###", //

		L'#', Item::rose_gold_ingot,
		L'S');

	// ---- torch bucket (2) ----
	// Copper torch: copper nugget / coal-or-charcoal / stick -> 4
	// (jar1201). Two recipes, one per coal kind, matching the existing
	// vanilla torch pair in Recipes.cpp.
	r->addShapedRecipy(new ItemInstance(Tile::copperTorch, 4), //
		L"sssciczcig",
		L"N", //
		L"X", //
		L"#", //

		L'N', Item::copper_nugget,
		L'X', new ItemInstance(Item::coal, 1, CoalItem::CHAR_COAL),
		L'#', Item::stick,
		L'S');

	r->addShapedRecipy(new ItemInstance(Tile::copperTorch, 4), //
		L"sssciczcig",
		L"N", //
		L"X", //
		L"#", //

		L'N', Item::copper_nugget,
		L'X', new ItemInstance(Item::coal, 1, CoalItem::STONE_COAL),
		L'#', Item::stick,
		L'S');

	// Copper lantern: 8 copper nuggets around a copper torch -> 1 (jar1201).
	r->addShapedRecipy(new ItemInstance(Tile::copperLantern, 1), //
		L"ssscictg",
		L"XXX", //
		L"X#X", //
		L"XXX", //

		L'X', Item::copper_nugget,
		L'#', Tile::copperTorch,
		L'S');

	// ---- button bucket (1) ----
	// Copper button: 1 cut copper -> 1 (jar1201).
	// DEVIATION from the brief ("1 ingot -> 1"): the jar makes the button
	// from a cut-copper block, not an ingot, and the jar is followed here.
	r->addShapedRecipy(new ItemInstance(Tile::copperButton, 1), //
		L"sctg",
		L"#", //

		L'#', Tile::cutCopper,
		L'S');

	// ---- rod bucket (2) ----
	// Copper chain: nugget / ingot / nugget, vertical -> 1 (jar1201).
	r->addShapedRecipy(new ItemInstance(Tile::copperChain, 1), //
		L"ssscicig",
		L"N", //
		L"I", //
		L"N", //

		L'N', Item::copper_nugget,
		L'I', Item::copper_ingot,
		L'S');

	// Lightning rod: 3 copper ingots, vertical -> 1 (vanilla).
	// DECORATIVE ONLY (Phase 0 item 6): no strike-attraction code exists.
	r->addShapedRecipy(new ItemInstance(Tile::lightningRod, 1), //
		L"ssscig",
		L"I", //
		L"I", //
		L"I", //

		L'I', Item::copper_ingot,
		L'S');

	// ---- fence bucket (1) ----
	// Copper bars: 6 ingots -> 16 (jar1201).
	r->addShapedRecipy(new ItemInstance(Tile::copperBars, 16), //
		L"sscig",
		L"###", //
		L"###", //

		L'#', Item::copper_ingot,
		L'S');

	// ---- chest bucket (1) ----
	// Copper chest: 8 copper ingots around a chest -> 1 (jar1201).
	r->addShapedRecipy(new ItemInstance(Tile::copperChest, 1), //
		L"ssscictg",
		L"###", //
		L"#X#", //
		L"###", //

		L'#', Item::copper_ingot,
		L'X', Tile::chest,
		L'S');

	// ---- ingot bucket (10) ----
	// Copper block -> 9 copper ingots (vanilla, shapeless). Java: only
	// an UNOXIDIZED block (waxed or not) can be crafted back into
	// ingots — an oxidized block uncrafting into 9 fresh ingots is an
	// exploit. The bare Tile::copperBlock ingredient matches ANY aux,
	// so use an exact-aux ItemInstance ('z') instead, with one recipe
	// per accepted state (aux 0 unoxidized, aux 4 waxed unoxidized).
	r->addShapelessRecipy(new ItemInstance(Item::copper_ingot, 9), //
		L"zg",
		new ItemInstance(Tile::copperBlock, 1, 0),
		L'S');
	r->addShapelessRecipy(new ItemInstance(Item::copper_ingot, 9), //
		L"zg",
		new ItemInstance(Tile::copperBlock, 1, 4),
		L'S');

	// Copper ingot -> 9 copper nuggets (jar1201, shapeless).
	r->addShapelessRecipy(new ItemInstance(Item::copper_nugget, 9), //
		L"ig",
		Item::copper_ingot,
		L'S');

	// 9 copper nuggets -> 1 copper ingot (jar1201).
	r->addShapedRecipy(new ItemInstance(Item::copper_ingot, 1), //
		L"ssscig",
		L"NNN", //
		L"NNN", //
		L"NNN", //

		L'N', Item::copper_nugget,
		L'S');

	// Rose gold block -> 9 rose-gold ingots (rosegold jar, shapeless).
	r->addShapelessRecipy(new ItemInstance(Item::rose_gold_ingot, 9), //
		L"tg",
		Tile::roseGoldBlock,
		L'S');

	// Rose gold ingot -> 9 rose-gold nuggets (rosegold jar, shapeless).
	r->addShapelessRecipy(new ItemInstance(Item::rose_gold_nugget, 9), //
		L"ig",
		Item::rose_gold_ingot,
		L'S');

	// 9 rose-gold nuggets -> 1 rose-gold ingot (rosegold jar).
	r->addShapedRecipy(new ItemInstance(Item::rose_gold_ingot, 1), //
		L"ssscig",
		L"NNN", //
		L"NNN", //
		L"NNN", //

		L'N', Item::rose_gold_nugget,
		L'S');

	// Raw copper block -> 9 raw copper (vanilla, shapeless).
	// (Not in the brief's list; the jar/vanilla reverse recipe, added for
	// symmetry with the block -> 9 ingots recipes above.)
	r->addShapelessRecipy(new ItemInstance(Item::raw_copper, 9), //
		L"tg",
		Tile::rawCopperBlock,
		L'S');

	// Raw rose gold block -> 9 raw rose gold (rosegold jar, shapeless).
	// (Not in the brief's list; the jar's raw_rose_gold_from_block recipe.)
	r->addShapelessRecipy(new ItemInstance(Item::raw_rose_gold, 9), //
		L"tg",
		Tile::rawRoseGoldBlock,
		L'S');

	// Raw rose gold: 1 copper ingot + 1 gold ingot -> 1 (rosegold jar).
	r->addShapelessRecipy(new ItemInstance(Item::raw_rose_gold, 1), //
		L"iig",
		Item::copper_ingot, Item::gold_ingot,
		L'S');

	// Raw rose gold: 2 copper ingots + 2 gold ingots -> 2 (rosegold jar).
	r->addShapelessRecipy(new ItemInstance(Item::raw_rose_gold, 2), //
		L"zzg",
		new ItemInstance(Item::copper_ingot, 2),
		new ItemInstance(Item::gold_ingot, 2),
		L'S');

	// DEVIATION from the brief: the "raw copper + raw gold" shapeless
	// recipes (1+1 -> 1 and 2+2 -> 2) are NOT implemented, because this
	// tree has NO raw-gold item (OreTile::getResource returns the gold
	// ore block itself for gold ore; gold auto-smelts to ingots). Raw
	// rose gold is still craftable from ingots via the two recipes above.
	// Adding a raw-gold item would change gold-ore drops game-wide and
	// is a separate decision (Phase 4/6).

	// =================================================================
	// neoLegacy - Copper Age (Phase 4). Waxing: shapeless, one copper
	// block/item + one honeycomb -> the same block/item waxed (aux | 4).
	// 15 oxidizable families x 4 oxidation states = 60 recipes. The
	// copper button is NOT waxed: the tree's copper button is a plain
	// StoneButtonTile with no oxidation states (Phase 3), so there is
	// nothing to wax; jar1201's 3 button waxings have no meaning here.
	// Rose-gold tiles never oxidize. Torch/shelf do not oxidize.
	// The results are the SAME tile/item ids Phase 3 already listed in
	// Recipes::isComplexOnly (keyed on result->id, not aux), so every
	// waxing recipe is Complex-Table-only automatically, and no new
	// H-slot is added: block +24 (9 -> 33), door +8 (4 -> 12),
	// torch +4 (3 -> 7), rod +8 (2 -> 10), fence +4 (1 -> 5),
	// chest +4 (1 -> 5), halfslab +4 (1 -> 5), stairs +4 (1 -> 5).
	// All V-slot counts stay under 99. The waxed result keeps its
	// oxidation stage (aux = stage | 4); scraping with an axe removes
	// the wax (aux - 4) or de-oxidizes one stage (aux - 1).
	// =================================================================

	// ---- waxing, block bucket (+24) ----
	// Copper block (unaffected) + honeycomb -> waxed unaffected copper block.
	r->addShapelessRecipy(new ItemInstance(Tile::copperBlock, 1, 4), //
		L"zig",
		new ItemInstance(Tile::copperBlock, 1, 0),
		Item::honeycomb,
		L'S');
	// Copper block (exposed) + honeycomb -> waxed exposed copper block.
	r->addShapelessRecipy(new ItemInstance(Tile::copperBlock, 1, 5), //
		L"zig",
		new ItemInstance(Tile::copperBlock, 1, 1),
		Item::honeycomb,
		L'S');
	// Copper block (weathered) + honeycomb -> waxed weathered copper block.
	r->addShapelessRecipy(new ItemInstance(Tile::copperBlock, 1, 6), //
		L"zig",
		new ItemInstance(Tile::copperBlock, 1, 2),
		Item::honeycomb,
		L'S');
	// Copper block (oxidized) + honeycomb -> waxed oxidized copper block.
	r->addShapelessRecipy(new ItemInstance(Tile::copperBlock, 1, 7), //
		L"zig",
		new ItemInstance(Tile::copperBlock, 1, 3),
		Item::honeycomb,
		L'S');
	// Cut copper (unaffected) + honeycomb -> waxed unaffected cut copper.
	r->addShapelessRecipy(new ItemInstance(Tile::cutCopper, 1, 4), //
		L"zig",
		new ItemInstance(Tile::cutCopper, 1, 0),
		Item::honeycomb,
		L'S');
	// Cut copper (exposed) + honeycomb -> waxed exposed cut copper.
	r->addShapelessRecipy(new ItemInstance(Tile::cutCopper, 1, 5), //
		L"zig",
		new ItemInstance(Tile::cutCopper, 1, 1),
		Item::honeycomb,
		L'S');
	// Cut copper (weathered) + honeycomb -> waxed weathered cut copper.
	r->addShapelessRecipy(new ItemInstance(Tile::cutCopper, 1, 6), //
		L"zig",
		new ItemInstance(Tile::cutCopper, 1, 2),
		Item::honeycomb,
		L'S');
	// Cut copper (oxidized) + honeycomb -> waxed oxidized cut copper.
	r->addShapelessRecipy(new ItemInstance(Tile::cutCopper, 1, 7), //
		L"zig",
		new ItemInstance(Tile::cutCopper, 1, 3),
		Item::honeycomb,
		L'S');
	// Chiseled copper (unaffected) + honeycomb -> waxed unaffected chiseled copper.
	r->addShapelessRecipy(new ItemInstance(Tile::chiseledCopper, 1, 4), //
		L"zig",
		new ItemInstance(Tile::chiseledCopper, 1, 0),
		Item::honeycomb,
		L'S');
	// Chiseled copper (exposed) + honeycomb -> waxed exposed chiseled copper.
	r->addShapelessRecipy(new ItemInstance(Tile::chiseledCopper, 1, 5), //
		L"zig",
		new ItemInstance(Tile::chiseledCopper, 1, 1),
		Item::honeycomb,
		L'S');
	// Chiseled copper (weathered) + honeycomb -> waxed weathered chiseled copper.
	r->addShapelessRecipy(new ItemInstance(Tile::chiseledCopper, 1, 6), //
		L"zig",
		new ItemInstance(Tile::chiseledCopper, 1, 2),
		Item::honeycomb,
		L'S');
	// Chiseled copper (oxidized) + honeycomb -> waxed oxidized chiseled copper.
	r->addShapelessRecipy(new ItemInstance(Tile::chiseledCopper, 1, 7), //
		L"zig",
		new ItemInstance(Tile::chiseledCopper, 1, 3),
		Item::honeycomb,
		L'S');
	// Copper grate (unaffected) + honeycomb -> waxed unaffected copper grate.
	r->addShapelessRecipy(new ItemInstance(Tile::copperGrate, 1, 4), //
		L"zig",
		new ItemInstance(Tile::copperGrate, 1, 0),
		Item::honeycomb,
		L'S');
	// Copper grate (exposed) + honeycomb -> waxed exposed copper grate.
	r->addShapelessRecipy(new ItemInstance(Tile::copperGrate, 1, 5), //
		L"zig",
		new ItemInstance(Tile::copperGrate, 1, 1),
		Item::honeycomb,
		L'S');
	// Copper grate (weathered) + honeycomb -> waxed weathered copper grate.
	r->addShapelessRecipy(new ItemInstance(Tile::copperGrate, 1, 6), //
		L"zig",
		new ItemInstance(Tile::copperGrate, 1, 2),
		Item::honeycomb,
		L'S');
	// Copper grate (oxidized) + honeycomb -> waxed oxidized copper grate.
	r->addShapelessRecipy(new ItemInstance(Tile::copperGrate, 1, 7), //
		L"zig",
		new ItemInstance(Tile::copperGrate, 1, 3),
		Item::honeycomb,
		L'S');
	// Copper golem statue (unaffected) + honeycomb -> waxed unaffected copper golem statue.
	r->addShapelessRecipy(new ItemInstance(Tile::copperGolemStatue, 1, 4), //
		L"zig",
		new ItemInstance(Tile::copperGolemStatue, 1, 0),
		Item::honeycomb,
		L'S');
	// Copper golem statue (exposed) + honeycomb -> waxed exposed copper golem statue.
	r->addShapelessRecipy(new ItemInstance(Tile::copperGolemStatue, 1, 5), //
		L"zig",
		new ItemInstance(Tile::copperGolemStatue, 1, 1),
		Item::honeycomb,
		L'S');
	// Copper golem statue (weathered) + honeycomb -> waxed weathered copper golem statue.
	r->addShapelessRecipy(new ItemInstance(Tile::copperGolemStatue, 1, 6), //
		L"zig",
		new ItemInstance(Tile::copperGolemStatue, 1, 2),
		Item::honeycomb,
		L'S');
	// Copper golem statue (oxidized) + honeycomb -> waxed oxidized copper golem statue.
	r->addShapelessRecipy(new ItemInstance(Tile::copperGolemStatue, 1, 7), //
		L"zig",
		new ItemInstance(Tile::copperGolemStatue, 1, 3),
		Item::honeycomb,
		L'S');
	// Copper bulb (unaffected) + honeycomb -> waxed unaffected copper bulb.
	r->addShapelessRecipy(new ItemInstance(Tile::copperBulb, 1, 4), //
		L"zig",
		new ItemInstance(Tile::copperBulb, 1, 0),
		Item::honeycomb,
		L'S');
	// Copper bulb (exposed) + honeycomb -> waxed exposed copper bulb.
	r->addShapelessRecipy(new ItemInstance(Tile::copperBulb, 1, 5), //
		L"zig",
		new ItemInstance(Tile::copperBulb, 1, 1),
		Item::honeycomb,
		L'S');
	// Copper bulb (weathered) + honeycomb -> waxed weathered copper bulb.
	r->addShapelessRecipy(new ItemInstance(Tile::copperBulb, 1, 6), //
		L"zig",
		new ItemInstance(Tile::copperBulb, 1, 2),
		Item::honeycomb,
		L'S');
	// Copper bulb (oxidized) + honeycomb -> waxed oxidized copper bulb.
	r->addShapelessRecipy(new ItemInstance(Tile::copperBulb, 1, 7), //
		L"zig",
		new ItemInstance(Tile::copperBulb, 1, 3),
		Item::honeycomb,
		L'S');

	// ---- waxing, door bucket (+8) ----
	// Copper door (unaffected) + honeycomb -> waxed unaffected copper door.
	r->addShapelessRecipy(new ItemInstance(Item::copper_door, 1, 4), //
		L"zig",
		new ItemInstance(Item::copper_door, 1, 0),
		Item::honeycomb,
		L'S');
	// Copper door (exposed) + honeycomb -> waxed exposed copper door.
	r->addShapelessRecipy(new ItemInstance(Item::copper_door, 1, 5), //
		L"zig",
		new ItemInstance(Item::copper_door, 1, 1),
		Item::honeycomb,
		L'S');
	// Copper door (weathered) + honeycomb -> waxed weathered copper door.
	r->addShapelessRecipy(new ItemInstance(Item::copper_door, 1, 6), //
		L"zig",
		new ItemInstance(Item::copper_door, 1, 2),
		Item::honeycomb,
		L'S');
	// Copper door (oxidized) + honeycomb -> waxed oxidized copper door.
	r->addShapelessRecipy(new ItemInstance(Item::copper_door, 1, 7), //
		L"zig",
		new ItemInstance(Item::copper_door, 1, 3),
		Item::honeycomb,
		L'S');
	// Copper trapdoor (unaffected) + honeycomb -> waxed unaffected copper trapdoor.
	r->addShapelessRecipy(new ItemInstance(Tile::copperTrapdoor, 1, 4), //
		L"zig",
		new ItemInstance(Tile::copperTrapdoor, 1, 0),
		Item::honeycomb,
		L'S');
	// Copper trapdoor (exposed) + honeycomb -> waxed exposed copper trapdoor.
	r->addShapelessRecipy(new ItemInstance(Tile::copperTrapdoor, 1, 5), //
		L"zig",
		new ItemInstance(Tile::copperTrapdoor, 1, 1),
		Item::honeycomb,
		L'S');
	// Copper trapdoor (weathered) + honeycomb -> waxed weathered copper trapdoor.
	r->addShapelessRecipy(new ItemInstance(Tile::copperTrapdoor, 1, 6), //
		L"zig",
		new ItemInstance(Tile::copperTrapdoor, 1, 2),
		Item::honeycomb,
		L'S');
	// Copper trapdoor (oxidized) + honeycomb -> waxed oxidized copper trapdoor.
	r->addShapelessRecipy(new ItemInstance(Tile::copperTrapdoor, 1, 7), //
		L"zig",
		new ItemInstance(Tile::copperTrapdoor, 1, 3),
		Item::honeycomb,
		L'S');

	// ---- waxing, torch bucket (+4) ----
	// Copper lantern (unaffected) + honeycomb -> waxed unaffected copper lantern.
	r->addShapelessRecipy(new ItemInstance(Tile::copperLantern, 1, 4), //
		L"zig",
		new ItemInstance(Tile::copperLantern, 1, 0),
		Item::honeycomb,
		L'S');
	// Copper lantern (exposed) + honeycomb -> waxed exposed copper lantern.
	r->addShapelessRecipy(new ItemInstance(Tile::copperLantern, 1, 5), //
		L"zig",
		new ItemInstance(Tile::copperLantern, 1, 1),
		Item::honeycomb,
		L'S');
	// Copper lantern (weathered) + honeycomb -> waxed weathered copper lantern.
	r->addShapelessRecipy(new ItemInstance(Tile::copperLantern, 1, 6), //
		L"zig",
		new ItemInstance(Tile::copperLantern, 1, 2),
		Item::honeycomb,
		L'S');
	// Copper lantern (oxidized) + honeycomb -> waxed oxidized copper lantern.
	r->addShapelessRecipy(new ItemInstance(Tile::copperLantern, 1, 7), //
		L"zig",
		new ItemInstance(Tile::copperLantern, 1, 3),
		Item::honeycomb,
		L'S');

	// ---- waxing, rod bucket (+8) ----
	// Copper chain (unaffected) + honeycomb -> waxed unaffected copper chain.
	r->addShapelessRecipy(new ItemInstance(Tile::copperChain, 1, 4), //
		L"zig",
		new ItemInstance(Tile::copperChain, 1, 0),
		Item::honeycomb,
		L'S');
	// Copper chain (exposed) + honeycomb -> waxed exposed copper chain.
	r->addShapelessRecipy(new ItemInstance(Tile::copperChain, 1, 5), //
		L"zig",
		new ItemInstance(Tile::copperChain, 1, 1),
		Item::honeycomb,
		L'S');
	// Copper chain (weathered) + honeycomb -> waxed weathered copper chain.
	r->addShapelessRecipy(new ItemInstance(Tile::copperChain, 1, 6), //
		L"zig",
		new ItemInstance(Tile::copperChain, 1, 2),
		Item::honeycomb,
		L'S');
	// Copper chain (oxidized) + honeycomb -> waxed oxidized copper chain.
	r->addShapelessRecipy(new ItemInstance(Tile::copperChain, 1, 7), //
		L"zig",
		new ItemInstance(Tile::copperChain, 1, 3),
		Item::honeycomb,
		L'S');
	// Lightning rod (unaffected) + honeycomb -> waxed unaffected lightning rod.
	r->addShapelessRecipy(new ItemInstance(Tile::lightningRod, 1, 4), //
		L"zig",
		new ItemInstance(Tile::lightningRod, 1, 0),
		Item::honeycomb,
		L'S');
	// Lightning rod (exposed) + honeycomb -> waxed exposed lightning rod.
	r->addShapelessRecipy(new ItemInstance(Tile::lightningRod, 1, 5), //
		L"zig",
		new ItemInstance(Tile::lightningRod, 1, 1),
		Item::honeycomb,
		L'S');
	// Lightning rod (weathered) + honeycomb -> waxed weathered lightning rod.
	r->addShapelessRecipy(new ItemInstance(Tile::lightningRod, 1, 6), //
		L"zig",
		new ItemInstance(Tile::lightningRod, 1, 2),
		Item::honeycomb,
		L'S');
	// Lightning rod (oxidized) + honeycomb -> waxed oxidized lightning rod.
	r->addShapelessRecipy(new ItemInstance(Tile::lightningRod, 1, 7), //
		L"zig",
		new ItemInstance(Tile::lightningRod, 1, 3),
		Item::honeycomb,
		L'S');

	// ---- waxing, fence bucket (+4) ----
	// Copper bars (unaffected) + honeycomb -> waxed unaffected copper bars.
	r->addShapelessRecipy(new ItemInstance(Tile::copperBars, 1, 4), //
		L"zig",
		new ItemInstance(Tile::copperBars, 1, 0),
		Item::honeycomb,
		L'S');
	// Copper bars (exposed) + honeycomb -> waxed exposed copper bars.
	r->addShapelessRecipy(new ItemInstance(Tile::copperBars, 1, 5), //
		L"zig",
		new ItemInstance(Tile::copperBars, 1, 1),
		Item::honeycomb,
		L'S');
	// Copper bars (weathered) + honeycomb -> waxed weathered copper bars.
	r->addShapelessRecipy(new ItemInstance(Tile::copperBars, 1, 6), //
		L"zig",
		new ItemInstance(Tile::copperBars, 1, 2),
		Item::honeycomb,
		L'S');
	// Copper bars (oxidized) + honeycomb -> waxed oxidized copper bars.
	r->addShapelessRecipy(new ItemInstance(Tile::copperBars, 1, 7), //
		L"zig",
		new ItemInstance(Tile::copperBars, 1, 3),
		Item::honeycomb,
		L'S');

	// ---- waxing, chest bucket (+4) ----
	// Copper chest (unaffected) + honeycomb -> waxed unaffected copper chest.
	r->addShapelessRecipy(new ItemInstance(Tile::copperChest, 1, 4), //
		L"zig",
		new ItemInstance(Tile::copperChest, 1, 0),
		Item::honeycomb,
		L'S');
	// Copper chest (exposed) + honeycomb -> waxed exposed copper chest.
	r->addShapelessRecipy(new ItemInstance(Tile::copperChest, 1, 5), //
		L"zig",
		new ItemInstance(Tile::copperChest, 1, 1),
		Item::honeycomb,
		L'S');
	// Copper chest (weathered) + honeycomb -> waxed weathered copper chest.
	r->addShapelessRecipy(new ItemInstance(Tile::copperChest, 1, 6), //
		L"zig",
		new ItemInstance(Tile::copperChest, 1, 2),
		Item::honeycomb,
		L'S');
	// Copper chest (oxidized) + honeycomb -> waxed oxidized copper chest.
	r->addShapelessRecipy(new ItemInstance(Tile::copperChest, 1, 7), //
		L"zig",
		new ItemInstance(Tile::copperChest, 1, 3),
		Item::honeycomb,
		L'S');

	// ---- waxing, halfslab bucket (+4) ----
	// Cut copper slab (unaffected) + honeycomb -> waxed unaffected cut copper slab.
	r->addShapelessRecipy(new ItemInstance(Tile::cutCopperSlabHalf, 1, 4), //
		L"zig",
		new ItemInstance(Tile::cutCopperSlabHalf, 1, 0),
		Item::honeycomb,
		L'S');
	// Cut copper slab (exposed) + honeycomb -> waxed exposed cut copper slab.
	r->addShapelessRecipy(new ItemInstance(Tile::cutCopperSlabHalf, 1, 5), //
		L"zig",
		new ItemInstance(Tile::cutCopperSlabHalf, 1, 1),
		Item::honeycomb,
		L'S');
	// Cut copper slab (weathered) + honeycomb -> waxed weathered cut copper slab.
	r->addShapelessRecipy(new ItemInstance(Tile::cutCopperSlabHalf, 1, 6), //
		L"zig",
		new ItemInstance(Tile::cutCopperSlabHalf, 1, 2),
		Item::honeycomb,
		L'S');
	// Cut copper slab (oxidized) + honeycomb -> waxed oxidized cut copper slab.
	r->addShapelessRecipy(new ItemInstance(Tile::cutCopperSlabHalf, 1, 7), //
		L"zig",
		new ItemInstance(Tile::cutCopperSlabHalf, 1, 3),
		Item::honeycomb,
		L'S');

	// ---- waxing, stairs bucket (+4) ----
	// Cut copper stairs (unaffected) + honeycomb -> waxed unaffected cut copper stairs.
	r->addShapelessRecipy(new ItemInstance(Tile::cutCopperStairs, 1, 4), //
		L"zig",
		new ItemInstance(Tile::cutCopperStairs, 1, 0),
		Item::honeycomb,
		L'S');
	// Cut copper stairs (exposed) + honeycomb -> waxed exposed cut copper stairs.
	r->addShapelessRecipy(new ItemInstance(Tile::cutCopperStairs, 1, 5), //
		L"zig",
		new ItemInstance(Tile::cutCopperStairs, 1, 1),
		Item::honeycomb,
		L'S');
	// Cut copper stairs (weathered) + honeycomb -> waxed weathered cut copper stairs.
	r->addShapelessRecipy(new ItemInstance(Tile::cutCopperStairs, 1, 6), //
		L"zig",
		new ItemInstance(Tile::cutCopperStairs, 1, 2),
		Item::honeycomb,
		L'S');
	// Cut copper stairs (oxidized) + honeycomb -> waxed oxidized cut copper stairs.
	r->addShapelessRecipy(new ItemInstance(Tile::cutCopperStairs, 1, 7), //
		L"zig",
		new ItemInstance(Tile::cutCopperStairs, 1, 3),
		Item::honeycomb,
		L'S');

}