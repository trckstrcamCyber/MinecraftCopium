#include "stdafx.h"
#include "net.minecraft.world.inventory.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "GrindstoneMenu.h"
#include "GrindstoneContainer.h"
#include "GrindstoneResultSlot.h"

GrindstoneMenu::GrindstoneMenu(shared_ptr<Inventory> inventory, Level *level, int xt, int yt, int zt, shared_ptr<Player> player)
{
	resultSlots = std::make_shared<ResultContainer>();
	repairSlots = std::make_shared<GrindstoneContainer>(this, IDS_GRINDSTONE, false, 2);

	pendingAction = ACTION_NONE;
	pendingEnchantId = -1;
	pendingEnchantLevel = 0;
	xpReward = 0;

	this->level = level;
	x = xt;
	y = yt;
	z = zt;
	this->player = player;

	addSlot(new Slot(repairSlots, INPUT_SLOT, 27, 43 + 4));
	addSlot(new Slot(repairSlots, ADDITIONAL_SLOT, 76, 43 + 4));
	addSlot(new GrindstoneResultSlot(this, xt, yt, zt, resultSlots, RESULT_SLOT, 134, 43 + 4));

	for (int y = 0; y < 3; y++)
	{
		for (int x = 0; x < 9; x++)
		{
			addSlot(new Slot(inventory, x + y * 9 + 9, 8 + x * 18, 84 + y * 18));
		}
	}
	for (int x = 0; x < 9; x++)
	{
		addSlot(new Slot(inventory, x, 8 + x * 18, 142));
	}
}

void GrindstoneMenu::slotsChanged(shared_ptr<Container> container)
{
	AbstractContainerMenu::slotsChanged();

	if (container == repairSlots) createResult();
}

// neoLegacy - Netherite tier (2026-11-26): map a diamond tool/armor item
// id to its netherite upgrade, or nullptr if the item cannot be upgraded.
static Item* getNetheriteUpgradeItem(int diamondId)
{
	if (diamondId == Item::diamond_sword_Id) return Item::netherite_sword;
	if (diamondId == Item::diamond_shovel_Id) return Item::netherite_shovel;
	if (diamondId == Item::diamond_pickaxe_Id) return Item::netherite_pickaxe;
	if (diamondId == Item::diamond_axe_Id) return Item::netherite_axe;
	if (diamondId == Item::diamond_hoe_Id) return Item::netherite_hoe;
	if (diamondId == Item::diamond_helmet_Id) return Item::netherite_helmet;
	if (diamondId == Item::diamond_chestplate_Id) return Item::netherite_chestplate;
	if (diamondId == Item::diamond_leggings_Id) return Item::netherite_leggings;
	if (diamondId == Item::diamond_boots_Id) return Item::netherite_boots;
	return nullptr;
}

void GrindstoneMenu::createResult()
{
	shared_ptr<ItemInstance> itemA = repairSlots->getItem(INPUT_SLOT);
	shared_ptr<ItemInstance> itemB = repairSlots->getItem(ADDITIONAL_SLOT);

	pendingAction = ACTION_NONE;
	pendingEnchantId = -1;
	pendingEnchantLevel = 0;
	xpReward = 0;

	if (itemA == nullptr)
	{
		resultSlots->setItem(0, nullptr);
		broadcastChanges();
		return;
	}

	unordered_map<int, int> *enchantmentsA = EnchantmentHelper::getEnchantments(itemA);

	// Curses (once this codebase has any -- see Enchantment::isCurse) never
	// come off in a grindstone. Everything else is "strippable": either
	// dropped entirely, or moved onto a book.
	unordered_map<int, int> strippable;
	unordered_map<int, int> curses;
	for (const auto &it : *enchantmentsA)
	{
		Enchantment *ench = Enchantment::enchantments[it.first];
		if (ench != nullptr && ench->isCurse())
			curses[it.first] = it.second;
		else
			strippable[it.first] = it.second;
	}

	shared_ptr<ItemInstance> result = nullptr;

	bool sameType = (itemB != nullptr) && (itemB->id == itemA->id);
	bool bIsPlainBook = (itemB != nullptr) && (itemB->id == Item::book->id);
	const bool aIsEnchantedBook = (itemA->id == Item::enchanted_book_Id);

	// neoLegacy - Netherite tier (2026-11-26): diamond tool/armor +
	// netherite ingot -> netherite tool/armor. Checked first: an enchanted
	// diamond item would otherwise match the plain-strip fallback below.
	// Enchantments and damage carry over (Java smithing-table behaviour);
	// the ingot is consumed when the result is taken. Free (xpReward 0).
	Item* netheriteItem = nullptr;
	if (itemB != nullptr && itemB->id == Item::netherite_ingot_Id)
	{
		netheriteItem = getNetheriteUpgradeItem(itemA->id);
	}
	if (netheriteItem != nullptr)
	{
		result = std::make_shared<ItemInstance>(netheriteItem, 1, 0);
		result->setAuxValue(itemA->getDamageValue());
		unordered_map<int, int> keepEnch(*enchantmentsA);
		EnchantmentHelper::setEnchantments(&keepEnch, result);
		pendingAction = ACTION_NETHERITE_UPGRADE;
		xpReward = 0;
	}
	else if (sameType && itemA->isDamageableItem() && itemA->getMaxDamage() > 0)
	{
		// ---- Repair-combine: sum remaining durability + a 5% bonus. ----
		// This clears enchantments same as a plain strip (curses excepted),
		// matching real grindstone behaviour.
		result = itemA->copy();

		int remainingA = result->getMaxDamage() - result->getDamageValue();
		int remainingB = itemB->getMaxDamage() - itemB->getDamageValue();
		int bonus = result->getMaxDamage() * 5 / 100;
		int totalRemaining = remainingA + remainingB + bonus;
		if (totalRemaining > result->getMaxDamage()) totalRemaining = result->getMaxDamage();

		int newDamage = result->getMaxDamage() - totalRemaining;
		if (newDamage < 0) newDamage = 0;
		result->setAuxValue(newDamage);

		unordered_map<int, int> keep(curses);
		EnchantmentHelper::setEnchantments(&keep, result);

		for (const auto &it : strippable) xpReward += 1 + it.second;

		pendingAction = ACTION_COMBINE;
	}
	else if (bIsPlainBook && !aIsEnchantedBook && !strippable.empty())
	{
		// ---- Book absorb: move ONE enchantment onto the book. ----
		// Costs the player levels rather than refunding them, matching
		// "removes the enchantment from the item costing levels".
		auto first = strippable.begin();
		pendingEnchantId = first->first;
		pendingEnchantLevel = first->second;

		result = Item::enchanted_book->createForEnchantment(
			new EnchantmentInstance(Enchantment::enchantments[pendingEnchantId], pendingEnchantLevel));

		pendingAction = ACTION_BOOK_ABSORB;
		xpReward = -(1 + pendingEnchantLevel);
	}
	else if (!strippable.empty())
	{
		// ---- Plain strip: drop everything except curses. ----
		if (aIsEnchantedBook)
		{
			// EnchantmentHelper::setEnchantments() ADDS to a book's stored
			// enchantments instead of replacing them, so it cannot strip one.
			// A stripped enchanted book is simply a plain book (or, if it
			// carries curses, an enchanted book holding only those).
			if (curses.empty())
			{
				result = std::make_shared<ItemInstance>(Item::book, 1, 0);
			}
			else
			{
				result = std::make_shared<ItemInstance>(Item::enchanted_book, 1, 0);
				for (const auto &it : curses)
				{
					Item::enchanted_book->addEnchantment(result, new EnchantmentInstance(it.first, it.second));
				}
			}
		}
		else
		{
			result = itemA->copy();

			unordered_map<int, int> keep(curses);
			EnchantmentHelper::setEnchantments(&keep, result);
		}

		for (const auto &it : strippable) xpReward += 1 + it.second;

		pendingAction = ACTION_STRIP;
	}

	delete enchantmentsA;

	resultSlots->setItem(0, result);
	broadcastChanges();
}

void GrindstoneMenu::addSlotListener(ContainerListener *listener)
{
	AbstractContainerMenu::addSlotListener(listener);
}

void GrindstoneMenu::removed(shared_ptr<Player> player)
{
	AbstractContainerMenu::removed(player);
	if (level->isClientSide) return;

	for (int i = 0; i < repairSlots->getContainerSize(); i++)
	{
		shared_ptr<ItemInstance> item = repairSlots->removeItemNoUpdate(i);
		if (item != nullptr)
		{
			player->drop(item);
		}
	}
}

bool GrindstoneMenu::stillValid(shared_ptr<Player> player)
{
	if (level->getTile(x, y, z) != Tile::grindstone_Id) return false;
	if (player->distanceToSqr(x + 0.5, y + 0.5, z + 0.5) > 8 * 8) return false;
	return true;
}

shared_ptr<ItemInstance> GrindstoneMenu::quickMoveStack(shared_ptr<Player> player, int slotIndex)
{
	shared_ptr<ItemInstance> clicked = nullptr;
	Slot *slot = slots.at(slotIndex);
	if (slot != nullptr && slot->hasItem())
	{
		shared_ptr<ItemInstance> stack = slot->getItem();
		clicked = stack->copy();

		if (slotIndex == RESULT_SLOT)
		{
			if (!moveItemStackTo(stack, INV_SLOT_START, USE_ROW_SLOT_END, true))
			{
				return nullptr;
			}
			slot->onQuickCraft(stack, clicked);
		}
		else if (slotIndex == INPUT_SLOT || slotIndex == ADDITIONAL_SLOT)
		{
			if (!moveItemStackTo(stack, INV_SLOT_START, USE_ROW_SLOT_END, false))
			{
				return nullptr;
			}
		}
		else if (slotIndex >= INV_SLOT_START && slotIndex < USE_ROW_SLOT_END)
		{
			if (!moveItemStackTo(stack, INPUT_SLOT, RESULT_SLOT, false))
			{
				return nullptr;
			}
		}
		if (stack->count == 0)
		{
			slot->set(nullptr);
		}
		else
		{
			slot->setChanged();
		}
		if (stack->count == clicked->count)
		{
			return nullptr;
		}
		else
		{
			slot->onTake(player, stack);
		}
	}
	return clicked;
}
