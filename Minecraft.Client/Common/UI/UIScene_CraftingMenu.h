#pragma once

#include "UIScene.h"
#include "UIControl_SlotList.h"
#include "UIControl_Label.h"
#include "IUIScene_CraftingMenu.h"

#define CRAFTING_INVENTORY_SLOT_START 0
#define CRAFTING_INVENTORY_SLOT_END (CRAFTING_INVENTORY_SLOT_START + 27)

#define CRAFTING_HOTBAR_SLOT_START CRAFTING_INVENTORY_SLOT_END
#define CRAFTING_HOTBAR_SLOT_END (CRAFTING_HOTBAR_SLOT_START + 9)

// Ingredients etc should go here
#define CRAFTING_INGREDIENTS_DESCRIPTION_START CRAFTING_HOTBAR_SLOT_END
#define CRAFTING_INGREDEINTS_DESCRIPTION_END (CRAFTING_INGREDIENTS_DESCRIPTION_START + 9)

#define CRAFTING_OUTPUT_SLOT_START CRAFTING_INGREDEINTS_DESCRIPTION_END
#define CRAFTING_OUTPUT_SLOT_END (CRAFTING_OUTPUT_SLOT_START + 1)

#define CRAFTING_INGREDIENTS_LAYOUT_START CRAFTING_OUTPUT_SLOT_END
#define CRAFTING_INGREDIENTS_LAYOUT_END (CRAFTING_INGREDIENTS_LAYOUT_START+9)

#define CRAFTING_SLOT_EXTRA_START CRAFTING_INGREDIENTS_LAYOUT_END
#define CRAFTING_SLOT_EXTRA_END (CRAFTING_SLOT_EXTRA_START+3)

#define CRAFTING_V_SLOT_START CRAFTING_SLOT_EXTRA_END
#define CRAFTING_V_SLOT_END (CRAFTING_V_SLOT_START+3)

// H slots should go last in the count as it's dependent on which size of crafting panel we have
#define CRAFTING_H_SLOT_START CRAFTING_V_SLOT_END

class UIScene_CraftingMenu : public UIScene, public IUIScene_CraftingMenu
{
private:
	typedef struct _SlotInfo
	{
		shared_ptr<ItemInstance> item;
		unsigned int alpha;
		bool show;

		_SlotInfo() : item(nullptr), alpha(31), show(true) {}
		_SlotInfo(shared_ptr<ItemInstance> _item) : item(_item), alpha(31), show(true) { }
	} SlotInfo;

	SlotInfo m_hSlotsInfo[m_iMaxHSlotC];
	SlotInfo m_vSlotsInfo[m_iMaxDisplayedVSlotC];
	SlotInfo m_ingredientsSlotsInfo[m_iIngredients3x3SlotC];
	SlotInfo m_craftingOutputSlotInfo;
	SlotInfo m_ingredientsInfo[9];

	typedef struct _GroupIcons {
		SlotInfo VSlot;
		std::vector<SlotInfo> HSlots;
	} GroupIcons;

	GroupIcons m_vGroupIconCache[4];

	AbstractContainerMenu *m_menu;

	int m_iMenuInventoryStart;
	int m_iMenuHotBarStart;

	// One-shot tab re-issue state (see tick()): the tab setup issued from the
	// constructor/handleReload can land before the movie's frame scripts have
	// run, leaving the AS3 iggy-slot placeholders visible as blue boxes.
	// m_bCraftingTabsReissueArmed marks that a full tick has elapsed with the
	// movie live; m_bCraftingTabsReissued marks the re-issue as done.
	bool m_bCraftingTabsReissueArmed;
	bool m_bCraftingTabsReissued;

public:
	UIScene_CraftingMenu(int iPad, void *initData, UILayer *parentLayer);

	virtual void handleDestroy();

	virtual EUIScene getSceneType();

	virtual void customDraw(IggyCustomDrawCallbackRegion *region);

#ifdef __PSVITA__
	virtual void handleTouchInput(unsigned int iPad, S32 x, S32 y, int iId, bool bPressed, bool bRepeat, bool bReleased);
	virtual void handleTouchBoxRebuild();
	virtual void handleTimerComplete(int id);
#endif
#if defined(__PSVITA__) || defined(_WINDOWS64)
	virtual UIControl* GetMainPanel();
#endif
#ifdef _WINDOWS64
	virtual bool handleMouseClick(F32 x, F32 y);
	// Cached from customDraw — H slot bounding boxes in SWF space
	F32 m_hSlotX0, m_hSlotY0, m_hSlotY1;
	F32 m_hSlotSpacing; // x distance between slot 0 and slot 1
	bool m_hSlotBoundsValid;

	//cached from swf at startup
	UIControl m_GroupH[2];
	UIControl m_GroupV[2];
	
	void handleCraftingGroupSwitch();
	void handleCraftingSlotClick(int iIndex);
	void handleLeftMouseClick(F32 x, F32 y, F32 sceneX, F32 sceneY);
#endif

protected:
	UIControl m_controlMainPanel, m_controlDescriptionPanel;
	UIControl m_control1Selector, m_control2Selector, m_control3Selector;
	UIControl_SlotList m_slotListCraftingHSlots;
	UIControl_SlotList m_slotListCrafting1VSlots, m_slotListCrafting2VSlots[2], m_slotListCrafting3VSlots[3];
	UIControl_SlotList m_slotListIngredientsLayout, m_slotListIngredientsLayoutSmall, m_slotListCraftingOutput;
	UIControl_SlotList m_slotListIngredients[9];
	UIControl_SlotList m_Slot2_1, m_Slot2_2;
	UIControl_SlotList m_Slot3_1, m_Slot3_2, m_Slot3_3;
	UIControl_SlotList m_slotListInventory, m_slotListHotBar, m_slotPreviewSlot;
	UIControl_Label m_labelIngredientsDesc[9];
	UIControl_HTMLLabel m_labelDescriptionText, m_labelDescriptionTextTop;
	UIControl_Label m_labelGroupName, m_labelSpinnerTitle, m_labelItemName, m_labelItemNameSmall, m_labelInventory, m_labelPreview, m_labelIngredients;

	IggyName m_funcMoveSelector, m_funcSelectVerticalItem, m_funcSetActiveTab;
	IggyName m_funcShowPanelDisplay, m_funcShowIngredientSlot;

	IggyName m_funcInitCraftingMenu, m_funcSetCraftingMenu, m_funcEnableTab, m_funcShowSuperCraftingSlots;

#ifdef __PSVITA__
	enum ETouchInput
	{
		ETouchInput_TouchPanel_0,
		ETouchInput_TouchPanel_1,
		ETouchInput_TouchPanel_2,
		ETouchInput_TouchPanel_3,
		ETouchInput_TouchPanel_4,
		ETouchInput_TouchPanel_5,
		ETouchInput_TouchPanel_6,
		ETouchInput_CraftingHSlots,

		ETouchInput_Count,
	};
	UIControl_Touch m_TouchInput[ETouchInput_Count];
	S32 m_iCraftingSlotTouchStartY;
#endif

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_controlMainPanel, "MainPanel" )
		UI_BEGIN_MAP_CHILD_ELEMENTS( m_controlMainPanel )
			UI_MAP_ELEMENT( m_slotListCraftingHSlots, "CraftingHSlots")

			UI_MAP_ELEMENT( m_control3Selector, "SlotSelector3" )
			UI_BEGIN_MAP_CHILD_ELEMENTS( m_control3Selector)
				UI_MAP_ELEMENT( m_slotListCrafting3VSlots[0], "Crafting3VSlot1")
				UI_MAP_ELEMENT( m_slotListCrafting3VSlots[1], "Crafting3VSlot2")
				UI_MAP_ELEMENT( m_slotListCrafting3VSlots[2], "Crafting3VSlot3")
			UI_END_MAP_CHILD_ELEMENTS()

			UI_MAP_ELEMENT( m_control2Selector, "SlotSelector2" )
			UI_BEGIN_MAP_CHILD_ELEMENTS( m_control2Selector)
				UI_MAP_ELEMENT( m_slotListCrafting2VSlots[0], "Crafting2VSlot1")
				UI_MAP_ELEMENT( m_slotListCrafting2VSlots[1], "Crafting2VSlot2")
			UI_END_MAP_CHILD_ELEMENTS()

			UI_MAP_ELEMENT( m_control1Selector, "CraftingSelector" )
			UI_BEGIN_MAP_CHILD_ELEMENTS( m_control1Selector)
				UI_MAP_ELEMENT( m_slotListCrafting1VSlots, "Crafting1VSlot1")
			UI_END_MAP_CHILD_ELEMENTS()

			UI_MAP_ELEMENT( m_slotListIngredientsLayout, "IngredientsLayout")
			UI_MAP_ELEMENT( m_slotListIngredientsLayoutSmall, "IngredientsLayout_Small")
			UI_MAP_ELEMENT( m_slotListCraftingOutput, "CraftingOutput")

			UI_MAP_ELEMENT( m_slotListIngredients[0], "Ingredient1")
			UI_MAP_ELEMENT( m_slotListIngredients[1], "Ingredient2")
			UI_MAP_ELEMENT( m_slotListIngredients[2], "Ingredient3")
			UI_MAP_ELEMENT( m_slotListIngredients[3], "Ingredient4")
			UI_MAP_ELEMENT( m_slotListIngredients[4], "Ingredient5")
			UI_MAP_ELEMENT( m_slotListIngredients[5], "Ingredient6")
			UI_MAP_ELEMENT( m_slotListIngredients[6], "Ingredient7")
			UI_MAP_ELEMENT( m_slotListIngredients[7], "Ingredient8")
			UI_MAP_ELEMENT( m_slotListIngredients[8], "Ingredient9")

			UI_MAP_ELEMENT( m_labelIngredientsDesc[0], "Ingredient1Desc")
			UI_MAP_ELEMENT( m_labelIngredientsDesc[1], "Ingredient2Desc")
			UI_MAP_ELEMENT( m_labelIngredientsDesc[2], "Ingredient3Desc")
			UI_MAP_ELEMENT( m_labelIngredientsDesc[3], "Ingredient4Desc")
			UI_MAP_ELEMENT( m_labelIngredientsDesc[4], "Ingredient5Desc")
			UI_MAP_ELEMENT( m_labelIngredientsDesc[5], "Ingredient6Desc")
			UI_MAP_ELEMENT( m_labelIngredientsDesc[6], "Ingredient7Desc")
			UI_MAP_ELEMENT( m_labelIngredientsDesc[7], "Ingredient8Desc")
			UI_MAP_ELEMENT( m_labelIngredientsDesc[8], "Ingredient9Desc")

			UI_MAP_ELEMENT( m_controlDescriptionPanel, "DescriptionPanel" )

			UI_MAP_ELEMENT( m_Slot2_1, "Slot2_1")
			UI_MAP_ELEMENT( m_Slot2_2, "Slot2_2")

			UI_MAP_ELEMENT( m_Slot3_1, "Slot3_1")
			UI_MAP_ELEMENT( m_Slot3_2, "Slot3_2")
			UI_MAP_ELEMENT( m_Slot3_3, "Slot3_3")

			UI_MAP_ELEMENT( m_labelIngredients, "IngredientsLabel")

			UI_MAP_ELEMENT( m_labelDescriptionText, "DescriptionText")
			UI_MAP_ELEMENT( m_labelDescriptionTextTop, "DescriptionTextTop")

			UI_MAP_ELEMENT( m_slotListInventory, "Inventory")
			UI_MAP_ELEMENT( m_slotListHotBar, "HotBar")
			UI_MAP_ELEMENT( m_slotPreviewSlot, "PreviewSlot")

			UI_MAP_ELEMENT( m_labelSpinnerTitle, "SpinnerTitle")
			UI_MAP_ELEMENT( m_labelGroupName, "GroupName")
			UI_MAP_ELEMENT( m_labelItemName, "ItemName")
			UI_MAP_ELEMENT( m_labelItemNameSmall, "ItemNameSmall")
			UI_MAP_ELEMENT( m_labelInventory, "InventoryLabel")
			UI_MAP_ELEMENT( m_labelPreview, "PreviewLabel")

			UI_MAP_NAME( m_funcMoveSelector, L"MoveSelector")
			UI_MAP_NAME( m_funcSelectVerticalItem, L"SelectVerticalItem")
			UI_MAP_NAME( m_funcSetActiveTab, L"SetActiveTab")
			UI_MAP_NAME( m_funcShowPanelDisplay, L"showPanelDisplay")
			UI_MAP_NAME( m_funcShowIngredientSlot, L"ShowIngredient")

			UI_MAP_NAME( m_funcInitCraftingMenu, L"InitCraftingMenu" )
			UI_MAP_NAME( m_funcSetCraftingMenu,  L"SetCraftingMenu" )
			UI_MAP_NAME( m_funcEnableTab,        L"EnableTab" )
			UI_MAP_NAME( m_funcShowSuperCraftingSlots, L"ShowSuperCraftingSlots" )

#ifdef _WINDOWS64
			UI_MAP_ELEMENT(m_GroupH[0], "TabH_1")
			UI_MAP_ELEMENT(m_GroupH[1], "TabH_2")

			UI_MAP_ELEMENT(m_GroupV[0], "TabV_1")
			UI_MAP_ELEMENT(m_GroupV[1], "TabV_2")
#endif

#ifdef __PSVITA__
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_0], "TouchPanel_0" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_1], "TouchPanel_1" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_2], "TouchPanel_2" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_3], "TouchPanel_3" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_4], "TouchPanel_4" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_5], "TouchPanel_5" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_6], "TouchPanel_6" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_CraftingHSlots], "TouchPanel_CraftingHSlots" )
#endif

		UI_END_MAP_CHILD_ELEMENTS()
	UI_END_MAP_ELEMENTS_AND_NAMES()

	virtual wstring getMoviePath();
	virtual void handleReload();

	virtual bool allowRepeat(int key);
	void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

protected:
	virtual void tick();

	virtual int getPad();
	virtual void hideAllHSlots();
	virtual void hideAllVSlots();
	virtual void hideAllIngredientsSlots();
	virtual void setCraftHSlotItem(int iPad, int iIndex, shared_ptr<ItemInstance> item, unsigned int uiAlpha);
	virtual void setCraftVSlotItem(int iPad, int iIndex, shared_ptr<ItemInstance> item, unsigned int uiAlpha);
	virtual void setCraftingOutputSlotItem(int iPad, shared_ptr<ItemInstance> item);
	virtual void setCraftingOutputSlotRedBox(bool show);
	virtual void setIngredientSlotItem(int iPad, int index, shared_ptr<ItemInstance> item);
	virtual void setIngredientSlotRedBox(int index, bool show);
	virtual void setIngredientDescriptionItem(int iPad, int index, shared_ptr<ItemInstance> item);
	virtual void setSelectedBannerEdit(shared_ptr<ItemInstance> banner);
	virtual void setIngredientDescriptionRedBox(int index, bool show);
	virtual void setIngredientDescriptionText(int index, LPCWSTR text);
	virtual void setShowCraftHSlot(int iIndex, bool show);
	virtual void setHGroupHighlight(int iIndex);
	virtual void setVGroupHighlight(int iIndex);
	virtual void setGroupText(LPCWSTR text);
	virtual void setDescriptionText(LPCWSTR text);
	virtual void setDescriptionTextTop(LPCWSTR text);
	virtual void setItemText(LPCWSTR text);
	virtual void scrollDescriptionUp();
	virtual void scrollDescriptionDown();
	virtual void updateHighlightAndScrollPositions();
	virtual void updateVSlotPositions(int iSlots, int i);

	virtual void initializeCraftingTabs();

	virtual void UpdateMultiPanel();
	virtual void UpdateCraftingPanels();
	virtual void UpdateEnabledTabs();

	virtual void ShowSuperCraftingSlots(int iIndex);
	virtual void ShowDescriptionPanel(bool show);
	virtual void SetCraftingMenu();

	virtual void EnableTab(int iIndex, bool enabled);
	
	virtual void HandleMessage(EUIMessage message, void *data);
	void handleInventoryUpdated(LPVOID data);

	// 4J - TomK If update tooltips is called then make sure the correct parent is invoked! (both UIScene AND IUIScene_CraftingMenu have an instance of said function!)
	virtual void updateTooltips() { IUIScene_CraftingMenu::UpdateTooltips(); }
};
