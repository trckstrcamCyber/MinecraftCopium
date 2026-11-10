#pragma once
#include "IUIScene_AbstractContainerMenu.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.ContainerListener.h"

// The 0-indexed slot in the inventory list that lines up with each top-row slot.
// Mirrors IUIScene_AnvilMenu's ANVIL_SCENE_* offsets -- same three-slots-wide
// top row shape (two inputs + a result), just without the Name row above it.
#define GRINDSTONE_SCENE_RESULT_SLOT_UP_OFFSET 5
#define GRINDSTONE_SCENE_RESULT_SLOT_DOWN_OFFSET 5
#define GRINDSTONE_SCENE_ITEM1_SLOT_UP_OFFSET 3
#define GRINDSTONE_SCENE_ITEM1_SLOT_DOWN_OFFSET 3
#define GRINDSTONE_SCENE_ITEM2_SLOT_UP_OFFSET 4
#define GRINDSTONE_SCENE_ITEM2_SLOT_DOWN_OFFSET 4

class Inventory;
class GrindstoneMenu;

class IUIScene_GrindstoneMenu : public virtual IUIScene_AbstractContainerMenu, public net_minecraft_world_inventory::ContainerListener
{
protected:
	shared_ptr<Inventory> m_inventory;
	GrindstoneMenu *m_grindstoneMenu;

protected:
	IUIScene_GrindstoneMenu();

	virtual ESceneSection GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY );
	int getSectionStartOffset(ESceneSection eSection);
	bool IsSectionSlotList( ESceneSection eSection );

	// The cost label / red cross are refreshed by
	// UIScene_GrindstoneMenu::updateCostLabel(); nothing else to do per tick.
	// Kept as a named hook so tick() has the same shape as every sibling scene's.
	void handleTick() {}

	// ContainerListener. Anvil uses these to keep its editable-name text box
	// in sync with the server; Grindstone has no equivalent state to track,
	// so these are intentionally empty -- present only to satisfy the
	// interface that addSlotListener(this) requires.
	void refreshContainer(AbstractContainerMenu *container, vector<shared_ptr<ItemInstance> > *items) {}
	void slotChanged(AbstractContainerMenu *container, int slotIndex, shared_ptr<ItemInstance> item) {}
	void setContainerData(AbstractContainerMenu *container, int id, int value) {}
};
