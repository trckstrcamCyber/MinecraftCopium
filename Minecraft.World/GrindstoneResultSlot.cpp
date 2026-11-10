#include "stdafx.h"
#include "net.minecraft.world.inventory.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.enchantment.h"
#include "GrindstoneResultSlot.h"
#include "GrindstoneMenu.h"
#include "GrindstoneContainer.h"
#include "ExperienceOrb.h"

GrindstoneResultSlot::GrindstoneResultSlot(GrindstoneMenu *menu, int xt, int yt, int zt, shared_ptr<Container> container, int slot, int x, int y) : Slot(container, slot, x, y)
{
	m_menu = menu;
	this->xt = xt;
	this->yt = yt;
	this->zt = zt;
}

bool GrindstoneResultSlot::mayPlace(shared_ptr<ItemInstance> item)
{
	return false;
}

bool GrindstoneResultSlot::mayPickup(shared_ptr<Player> player)
{
	if (!hasItem()) return false;

	// xpReward < 0 means this result costs levels (book-absorb); >= 0
	// means it's free (refunds XP on pickup instead).
	if (m_menu->xpReward < 0)
	{
		return player->abilities.instabuild || player->experienceLevel >= -m_menu->xpReward;
	}
	return true;
}

void GrindstoneResultSlot::onTake(shared_ptr<Player> player, shared_ptr<ItemInstance> carried)
{
	// Snapshot what createResult() decided BEFORE touching any slot.
	//
	// Every setItem() below fires GrindstoneContainer::setChanged() ->
	// GrindstoneMenu::slotsChanged() -> createResult(), which immediately
	// recomputes the result and RESETS pendingAction / pendingEnchantId /
	// xpReward. Reading them afterwards meant that no XP orb was ever spawned,
	// the level cost of a book transfer was never charged, and (with a single
	// book) the transferred enchantment was never removed from the item.
	const GrindstoneMenu::EAction action = m_menu->pendingAction;
	const int enchantId = m_menu->pendingEnchantId;
	const int xpReward = m_menu->xpReward;
	Level *level = m_menu->level;

	switch (action)
	{
	case GrindstoneMenu::ACTION_COMBINE:
		m_menu->repairSlots->setItem(GrindstoneMenu::INPUT_SLOT, nullptr);
		m_menu->repairSlots->setItem(GrindstoneMenu::ADDITIONAL_SLOT, nullptr);
		break;

	case GrindstoneMenu::ACTION_STRIP:
		m_menu->repairSlots->setItem(GrindstoneMenu::INPUT_SLOT, nullptr);
		break;

	case GrindstoneMenu::ACTION_BOOK_ABSORB:
	{
		// Remove just the one moved enchantment from the item in slot A --
		// it keeps everything else (including any curses).
		shared_ptr<ItemInstance> item = m_menu->repairSlots->getItem(GrindstoneMenu::INPUT_SLOT);
		if (item != nullptr && enchantId >= 0)
		{
			unordered_map<int, int> *enchantments = EnchantmentHelper::getEnchantments(item);
			enchantments->erase(enchantId);
			EnchantmentHelper::setEnchantments(enchantments, item);
			delete enchantments;
			m_menu->repairSlots->setItem(GrindstoneMenu::INPUT_SLOT, item);
		}

		// Then consume one book from the additional slot.
		shared_ptr<ItemInstance> book = m_menu->repairSlots->getItem(GrindstoneMenu::ADDITIONAL_SLOT);
		if (book != nullptr)
		{
			if (book->count > 1)
			{
				book->count--;
				m_menu->repairSlots->setItem(GrindstoneMenu::ADDITIONAL_SLOT, book);
			}
			else
			{
				m_menu->repairSlots->setItem(GrindstoneMenu::ADDITIONAL_SLOT, nullptr);
			}
		}
		break;
	}

	default:
		break;
	}

	if (!level->isClientSide)
	{
		if (xpReward > 0)
		{
			level->addEntity(std::make_shared<ExperienceOrb>(
				level, xt + 0.5, yt + 0.5, zt + 0.5, xpReward));
		}
		else if (xpReward < 0 && !player->abilities.instabuild)
		{
			player->giveExperienceLevels(xpReward);
		}

		level->levelEvent(LevelEvent::SOUND_ANVIL_USED, xt, yt, zt, 0);
	}
}

bool GrindstoneResultSlot::mayCombine(shared_ptr<ItemInstance> item)
{
	return false;
}
