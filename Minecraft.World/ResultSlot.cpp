#include "stdafx.h"
#include "Container.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "ResultSlot.h"

ResultSlot::ResultSlot(Player *player, shared_ptr<Container> craftSlots, shared_ptr<Container> container, int id, int x, int y) : Slot( container, id, x, y )
{
	this->player = player;
	this->craftSlots = craftSlots;
	removeCount = 0;
}

bool ResultSlot::mayPlace(shared_ptr<ItemInstance> item)
{
	return false;
}

shared_ptr<ItemInstance> ResultSlot::remove(int c)
{
	if (hasItem())
	{
		removeCount += min(c, getItem()->count);
	}
	return Slot::remove(c);
}

void ResultSlot::onQuickCraft(shared_ptr<ItemInstance> picked, int count)
{
	removeCount += count;
	checkTakeAchievements(picked);
}


// neoLegacy - Achievements Batch 1 (2026-11-26): 63 Wax On. True when the
// crafted result is a waxed copper-family block/item: the 15 waxing families
// (copper_block, cut_copper, chiseled_copper, copper_grate, copper_golem_statue,
// copper_bulb, copper_door, copper_trapdoor, copper_lantern, copper_chain,
// lightning_rod, copper_bars, copper_chest, cut_copper_slab, cut_copper_stairs)
// are all Complex-Table shapeless recipes whose result is the same id at
// aux = oxidation stage | 4 (waxed bit). Unwaxed results have aux 0-3.
static bool isWaxedCopperResult(shared_ptr<ItemInstance> carried)
{
	if ((carried->getAuxValue() & 4) == 0) return false;
	switch (carried->id)
	{
	case Tile::copper_block_Id:
	case Tile::cut_copper_Id:
	case Tile::chiseled_copper_Id:
	case Tile::copper_grate_Id:
	case Tile::copper_golem_statue_Id:
	case Tile::copper_bulb_Id:
	case Tile::copper_trapdoor_Id:
	case Tile::copper_lantern_Id:
	case Tile::copper_chain_Id:
	case Tile::lightning_rod_Id:
	case Tile::copper_bars_Id:
	case Tile::copper_chest_Id:
	case Tile::cut_copper_slab_Id:
	case Tile::cut_copper_stairs_Id:
	case Item::copper_door_Id:
		return true;
	default:
		return false;
	}
}

void ResultSlot::checkTakeAchievements(shared_ptr<ItemInstance> carried)
{
	carried->onCraftedBy(player->level, dynamic_pointer_cast<Player>( player->shared_from_this() ), removeCount);
	removeCount = 0;

	if (carried->id == Tile::crafting_table_Id)												player->awardStat(GenericStats::buildWorkbench(),		GenericStats::param_buildWorkbench());
	else if (carried->id == Item::wooden_pickaxe_Id)										player->awardStat(GenericStats::buildPickaxe(),			GenericStats::param_buildPickaxe());
	else if (carried->id == Tile::furnace_Id)											player->awardStat(GenericStats::buildFurnace(),			GenericStats::param_buildFurnace());
	else if (carried->getItem()->getBaseItemType() == Item::eBaseItemType_hoe)			player->awardStat(GenericStats::buildHoe(),				GenericStats::param_buildHoe());
	else if (carried->id == Item::bread_Id)												player->awardStat(GenericStats::makeBread(),			GenericStats::param_makeBread());
	else if (carried->id == Item::cake_Id)												player->awardStat(GenericStats::bakeCake(),				GenericStats::param_bakeCake());
	else if (carried->id == Item::stone_pickaxe_Id)										player->awardStat(GenericStats::buildBetterPickaxe(),	GenericStats::param_buildBetterPickaxe());
	// The description is technically wrong but that's accurate
	else if (carried->getItem()->getBaseItemType() == Item::eBaseItemType_sword)		player->awardStat(GenericStats::buildSword(),			GenericStats::param_buildSword());
	//else if (carried->id == Tile::enchanting_table_Id)									player->awardStat(GenericStats::enchantments(),			GenericStats::param_achievement(eAward_));
	else if (carried->id == Tile::bookshelf_Id)											player->awardStat(GenericStats::bookcase(),				GenericStats::param_bookcase());

	// 4J : WESTY : Added new acheivements. 
	else if (carried->id == Tile::dispenser_Id) player->awardStat(GenericStats::dispenseWithThis(), GenericStats::param_dispenseWithThis());

	// neoLegacy - Achievements Batch 1 (2026-11-26): 53 Beaconator (craft a beacon),
	// 57 Rose Gold (Complex Table craft; this slot serves both the normal and
	// Complex crafting menus), 63 Wax On (waxed copper result, see helper above).
	else if (carried->id == Tile::beacon_Id) player->awardStat(GenericStats::beaconator(), GenericStats::param_beaconator());
	else if (carried->id == Item::rose_gold_ingot_Id) player->awardStat(GenericStats::roseGold(), GenericStats::param_roseGold());
	else if (isWaxedCopperResult(carried)) player->awardStat(GenericStats::waxOn(), GenericStats::param_waxOn());
}

void ResultSlot::onTake(shared_ptr<Player> player, shared_ptr<ItemInstance> carried)
{
	checkTakeAchievements(carried);

	for (unsigned int i = 0; i < craftSlots->getContainerSize(); i++)
	{
		shared_ptr<ItemInstance> item = craftSlots->getItem(i);
		if (item != nullptr)
		{
			craftSlots->removeItem(i, 1);

			if (item->getItem()->hasCraftingRemainingItem())
			{
				shared_ptr<ItemInstance> craftResult = std::make_shared<ItemInstance>(item->getItem()->getCraftingRemainingItem());

				/*
				* Try to place this in the player's inventory (See we.java for new method)
				*/
				if (item->getItem()->shouldMoveCraftingResultToInventory(item) && player->inventory->add(craftResult))
				{
					continue;
				}

				// If this slot is now empty, place it there (current behavior)
				if (craftSlots->getItem(i) == nullptr)
				{
					craftSlots->setItem(i, craftResult);
				}
				else
				{
					// Finally, if nothing else, just drop the item
					player->drop(craftResult);
				}
			}

		}
	}
}

bool ResultSlot::mayCombine(shared_ptr<ItemInstance> second)
{
	return false;
}
