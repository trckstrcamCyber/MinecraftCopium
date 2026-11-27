#pragma once

#include "AbstractContainerMenu.h"

class GrindstoneMenu : public AbstractContainerMenu
{
	friend class GrindstoneResultSlot;
private:
	// What createResult() decided the current pair of inputs means. Decided
	// up front so GrindstoneResultSlot::onTake() knows exactly what to
	// consume/mutate without re-deriving it (and possibly disagreeing with
	// what the player was shown in the result slot).
	enum EAction
	{
		ACTION_NONE,
		ACTION_STRIP,			// one item, its non-curse enchantments removed
		ACTION_COMBINE,			// two matching items, durability combined +5%, non-curse enchantments removed
		ACTION_BOOK_ABSORB,		// item + plain book -> enchanted book carrying one enchantment, item keeps the rest
		ACTION_NETHERITE_UPGRADE	// diamond tool/armor + netherite ingot -> netherite tool/armor, enchantments + damage kept
	};

public:
	static const int INPUT_SLOT = 0;
	static const int ADDITIONAL_SLOT = 1;
	static const int RESULT_SLOT = 2;

	static const int INV_SLOT_START = RESULT_SLOT + 1;
	static const int INV_SLOT_END = INV_SLOT_START + 9 * 3;
	static const int USE_ROW_SLOT_START = INV_SLOT_END;
	static const int USE_ROW_SLOT_END = USE_ROW_SLOT_START + 9;

private:
	shared_ptr<Container> resultSlots;
	shared_ptr<Container> repairSlots;		// GrindstoneContainer, 2 slots

	Level *level;
	int x, y, z;
	shared_ptr<Player> player;

	EAction pendingAction;
	int pendingEnchantId;
	int pendingEnchantLevel;

public:
	int xpReward;		// XP points granted to the player when the result is taken

	using AbstractContainerMenu::slotsChanged;

	GrindstoneMenu(shared_ptr<Inventory> inventory, Level *level, int xt, int yt, int zt, shared_ptr<Player> player);

	void slotsChanged(shared_ptr<Container> container);
	void createResult();
	void addSlotListener(ContainerListener *listener);
	void removed(shared_ptr<Player> player);
	bool stillValid(shared_ptr<Player> player);
	shared_ptr<ItemInstance> quickMoveStack(shared_ptr<Player> player, int slotIndex);
};
