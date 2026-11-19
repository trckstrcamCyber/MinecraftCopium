#include "stdafx.h"
#include "UI.h"
#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../../Minecraft.World/ComplexCraftingMenu.h"	// neoLegacy - Complex Table
#include "../../../Minecraft.World/BannerItem.h"
#include "../../../Minecraft.World/BannerPatterns.h"
#include "../../../Minecraft.World/DyePowderItem.h"
#include "../../../Minecraft.World/Tile.h"
#include "../../../Minecraft.World/BannerPatternRecipe.h"
#include "UIScene_CraftingMenu.h"
#ifdef _WINDOWS64
#include "../../Windows64/Iggy/gdraw/gdraw_d3d11.h"
#endif

#ifdef __PSVITA__
#define GAME_CRAFTING_TOUCHUPDATE_TIMER_ID 0
#define GAME_CRAFTING_TOUCHUPDATE_TIMER_TIME 100
#endif

UIScene_CraftingMenu::UIScene_CraftingMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
#ifdef _WINDOWS64
	m_hSlotBoundsValid = false;
	m_hSlotX0 = m_hSlotY0 = m_hSlotY1 = 0;
	m_hSlotSpacing = 0;
#endif
	m_bIgnoreKeyPresses = false;
	m_bCraftingTabsReissueArmed = false;
	m_bCraftingTabsReissued = false;

	CraftingPanelScreenInput* initData = static_cast<CraftingPanelScreenInput *>(_initData);
	m_iContainerType=initData->iContainerType;
	m_bComplexTable=initData->bComplexTable;	// neoLegacy - Complex Table
	m_pPlayer=initData->player;
	m_bSplitscreen=initData->bSplitscreen;

	// Setup all the Iggy references we need for this scene
	initialiseMovie();
	
	for(unsigned int i = 0; i < 9; ++i)	m_labelIngredientsDesc[i].init(L"");
	m_labelDescriptionText.init(L"");
	m_labelDescriptionTextTop.init(L"");
	m_labelGroupName.init(L"");
	m_labelSpinnerTitle.init(L"");
	m_labelItemName.init(L"");
	m_labelItemNameSmall.init(L"");
	m_labelInventory.init( app.GetString(IDS_INVENTORY) );
	m_labelPreview.init(L"Preview");
	m_labelIngredients.init(app.GetString(IDS_INGREDIENTS));

	if(m_iContainerType==RECIPE_TYPE_2x2)
	{
		m_menu = m_pPlayer->inventoryMenu;
		m_iMenuInventoryStart = InventoryMenu::INV_SLOT_START;
		m_iMenuHotBarStart = InventoryMenu::USE_ROW_SLOT_START;
	}
	else
	{
		// neoLegacy - a Complex Table needs its own menu so recipe matching uses the Complex Table recipes
		CraftingMenu *menu = m_bComplexTable
			? new ComplexCraftingMenu(m_pPlayer->inventory, m_pPlayer->level, initData->x, initData->y, initData->z)
			: new CraftingMenu(m_pPlayer->inventory, m_pPlayer->level, initData->x, initData->y, initData->z);
		Minecraft::GetInstance()->localplayers[m_iPad]->containerMenu = menu;

		m_menu = menu;
		m_iMenuInventoryStart = CraftingMenu::INV_SLOT_START;
		m_iMenuHotBarStart = CraftingMenu::USE_ROW_SLOT_START;
	}
	m_slotListInventory.addSlots(CRAFTING_INVENTORY_SLOT_START,CRAFTING_INVENTORY_SLOT_END - CRAFTING_INVENTORY_SLOT_START);
	m_slotListHotBar.addSlots(CRAFTING_HOTBAR_SLOT_START, CRAFTING_HOTBAR_SLOT_END - CRAFTING_HOTBAR_SLOT_START);

	{
		int maxGroupVLimit = ((m_iContainerType == RECIPE_TYPE_2x2) ? m_iMaxGroupV2x2 : m_iMaxGroupV3x3);
		{
			m_vGroupIconCache[0].VSlot.item = make_shared<ItemInstance>(Tile::workBench, 1);
			m_vGroupIconCache[0].VSlot.show = (1 <= maxGroupVLimit);
		}
		{
			m_vGroupIconCache[1].VSlot.item = make_shared<ItemInstance>(Tile::standing_banner_Id, 1, 15);
			m_vGroupIconCache[1].VSlot.show = (2 <= maxGroupVLimit);

			{
				m_vGroupIconCache[1].HSlots.push_back(SlotInfo(make_shared<ItemInstance>(Tile::standing_banner_Id, 1, 15)));
			}

			{
				SlotInfo slotInfo = SlotInfo(make_shared<ItemInstance>(Tile::standing_banner_Id, 1, 15));
				BannerItem::addPattern(slotInfo.item, L"cr", DyePowderItem::BLACK);

				m_vGroupIconCache[1].HSlots.push_back(slotInfo);
			}
			{
				SlotInfo slotInfo = SlotInfo(make_shared<ItemInstance>(Tile::standing_banner_Id, 1, 15));
				BannerItem::addPattern(slotInfo.item, L"ld", DyePowderItem::BLACK);

				m_vGroupIconCache[1].HSlots.push_back(slotInfo);
			}
			{
				SlotInfo slotInfo = SlotInfo(make_shared<ItemInstance>(Tile::standing_banner_Id, 1, 15));
				BannerItem::addPattern(slotInfo.item, L"mc", DyePowderItem::BLACK);

				m_vGroupIconCache[1].HSlots.push_back(slotInfo);
			}
			{
				SlotInfo slotInfo = SlotInfo(make_shared<ItemInstance>(Tile::standing_banner_Id, 1, 15));
				BannerItem::addPattern(slotInfo.item, L"bo", DyePowderItem::BLACK);

				m_vGroupIconCache[1].HSlots.push_back(slotInfo);
			}
			{
				SlotInfo slotInfo = SlotInfo(make_shared<ItemInstance>(Tile::standing_banner_Id, 1, 15));
				BannerItem::addPattern(slotInfo.item, L"cre", DyePowderItem::BLACK);

				m_vGroupIconCache[1].HSlots.push_back(slotInfo);
			}
		}
		{
			m_vGroupIconCache[2].VSlot.item = make_shared<ItemInstance>(Item::fireworks_Id, 1, 0);
			m_vGroupIconCache[2].VSlot.show = (3 <= maxGroupVLimit);
		}
		{
			m_vGroupIconCache[3].VSlot.item = make_shared<ItemInstance>(Item::dye_Id, 1, DyePowderItem::CYAN);
			m_vGroupIconCache[3].VSlot.show = (4 <= maxGroupVLimit);
		}
	}

#if TO_BE_IMPLEMENTED
	// if we are in splitscreen, then we need to figure out if we want to move this scene
	if(m_bSplitscreen)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);	
	}

	XuiElementSetShow(m_hGrid,TRUE);
	XuiElementSetShow(m_hPanel,TRUE);
#endif

	m_pGroupA = new Recipy::_eGroupType*[m_iMaxGroupV3x3];

	if(m_iContainerType==RECIPE_TYPE_3x3)
	{
		m_iIngredientsMaxSlotC = m_iIngredients3x3SlotC;
		m_pGroupTabA=(_eGroupTab *)&m_GroupTabBkgMapping3x3A;

		m_pGroupA[eGroupVType::eGroupVType_Workbench] = (Recipy::_eGroupType*)&m_GroupTypeMapping9GridA;
		m_pGroupA[eGroupVType::eGroupVType_Banner] = (Recipy::_eGroupType*)&m_GroupTypeBannerMapping9GridA;
	}
	else
	{
		m_iIngredientsMaxSlotC = m_iIngredients2x2SlotC;
		m_pGroupTabA=(_eGroupTab *)&m_GroupTabBkgMapping2x2A;

		m_pGroupA[eGroupVType::eGroupVType_Workbench] = (Recipy::_eGroupType*)&m_GroupTypeMapping4GridA;
	}
	m_iCraftablesMaxHSlotC = m_iMaxHSlot;

	initializeCraftingTabs();

	app.SetRichPresenceContext(m_iPad,CONTEXT_GAME_STATE_CRAFTING);
	setGroupText(GetGroupNameText(m_pGroupA[m_iGroupVIndex][m_iGroupHIndex]));

	// Update the tutorial state
	Minecraft *pMinecraft = Minecraft::GetInstance();

	if( pMinecraft->localgameModes[m_iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[m_iPad]);
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		if(m_iContainerType==RECIPE_TYPE_2x2)
		{
			gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_2x2Crafting_Menu, this);
		}
		else
		{
			gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_3x3Crafting_Menu, this);
		}
	}

#ifdef _TO_BE_IMPLEMENTED
	XuiSetTimer(m_hObj,IGNORE_KEYPRESS_TIMERID,IGNORE_KEYPRESS_TIME);
#endif

	for(unsigned int i = 0; i < 9; ++i)
	{
		m_slotListIngredients[i].addSlot(CRAFTING_INGREDIENTS_DESCRIPTION_START + i);
	}
	m_slotListCraftingOutput.addSlot(CRAFTING_OUTPUT_SLOT_START);
	if (m_iContainerType == RECIPE_TYPE_3x3) {
		m_slotListIngredientsLayout.addSlots(CRAFTING_INGREDIENTS_LAYOUT_START, m_iIngredientsMaxSlotC);
		m_slotListIngredientsLayoutSmall.setVisible(false);
	} else {
		m_slotListIngredientsLayoutSmall.addSlots(CRAFTING_INGREDIENTS_LAYOUT_START, m_iIngredientsMaxSlotC);
		m_slotListIngredientsLayout.setVisible(false);
	}

	m_Slot2_1.addSlot(CRAFTING_SLOT_EXTRA_START + 0);
	m_Slot2_2.addSlot(CRAFTING_SLOT_EXTRA_START + 1);

	m_Slot3_1.addSlot(CRAFTING_SLOT_EXTRA_START + 0);
	m_Slot3_2.addSlot(CRAFTING_SLOT_EXTRA_START + 1);
	m_Slot3_3.addSlot(CRAFTING_SLOT_EXTRA_START + 2);

	m_slotPreviewSlot.addSlot(CRAFTING_SLOT_EXTRA_START + 0);

	// 3 Slot vertical scroll
	m_slotListCrafting3VSlots[0].addSlot(CRAFTING_V_SLOT_START + 0);
	m_slotListCrafting3VSlots[1].addSlot(CRAFTING_V_SLOT_START + 1);
	m_slotListCrafting3VSlots[2].addSlot(CRAFTING_V_SLOT_START + 2);

	// 2 Slot vertical scroll
	// 2 slot scroll has swapped order
	m_slotListCrafting2VSlots[0].addSlot(CRAFTING_V_SLOT_START + 1);
	m_slotListCrafting2VSlots[1].addSlot(CRAFTING_V_SLOT_START + 0);

	// 1 Slot scroll (for 480 mainly)
	m_slotListCrafting1VSlots.addSlot(CRAFTING_V_SLOT_START);

	m_slotListCraftingHSlots.addSlots(CRAFTING_H_SLOT_START,m_iCraftablesMaxHSlotC);

	// Check which recipes are available with the resources we have
	CheckRecipesAvailable();
	// reset the vertical slots
	iVSlotIndexA[0]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
	iVSlotIndexA[1]=0;
	iVSlotIndexA[2]=1;
	UpdateVerticalSlots();
	UpdateHighlight();

	if(initData) delete initData;

	// in this scene, we override the press sound with our own for crafting success or fail
	ui.OverrideSFX(m_iPad,ACTION_MENU_A,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_OK,true);
#ifdef __ORBIS__
	ui.OverrideSFX(m_iPad,ACTION_MENU_TOUCHPAD_PRESS,true);
#endif
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT_SCROLL,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT_SCROLL,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_UP,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_DOWN,true);

	// 4J-PB - Must be after the CanBeMade list has been set up with CheckRecipesAvailable
	UpdateTooltips();

	UpdateCraftingPanels();

#ifdef __PSVITA__
	// initialise vita touch controls with ids
	for(unsigned int i = 0; i < ETouchInput_Count; ++i)
	{
		m_TouchInput[i].init(i);
	}
	ui.TouchBoxRebuild(this);
#endif
}

void UIScene_CraftingMenu::handleDestroy()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();

	if( pMinecraft->localgameModes[m_iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[m_iPad]);
		if(gameMode != nullptr) gameMode->getTutorial()->changeTutorialState(m_previousTutorialState);
	}

	// We need to make sure that we call closeContainer() anytime this menu is closed, even if it is forced to close by some other reason (like the player dying)	
	if(Minecraft::GetInstance()->localplayers[m_iPad] != nullptr && Minecraft::GetInstance()->localplayers[m_iPad]->containerMenu->containerId == m_menu->containerId)
	{
		Minecraft::GetInstance()->localplayers[m_iPad]->closeContainer();
	}

	ui.OverrideSFX(m_iPad,ACTION_MENU_A,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_OK,false);
#ifdef __ORBIS__
	ui.OverrideSFX(m_iPad,ACTION_MENU_TOUCHPAD_PRESS,false);
#endif
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT_SCROLL,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT_SCROLL,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_UP,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_DOWN,false);
}

EUIScene UIScene_CraftingMenu::getSceneType()
{
	return eUIScene_CraftingMenu;
}

wstring UIScene_CraftingMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		m_bSplitscreen = true;
		return L"CraftingMenuSplit";
	}
	else
	{
		return L"CraftingMenu";
	}
}

#if defined(__PSVITA__) || defined(_WINDOWS64)
UIControl* UIScene_CraftingMenu::GetMainPanel()
{
	return &m_controlMainPanel;
}
#endif

#ifdef _WINDOWS64
void UIScene_CraftingMenu::handleCraftingGroupSwitch() {
	m_iCurrentSlotHIndex = 0;
	m_iCurrentSlotVIndex = 1;
	UpdateCraftingPanels();
	CheckRecipesAvailable();
	if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex > 0) {
	int patternId = m_aBannerCraftPatterns[m_iGroupHIndex - 1][m_iCurrentSlotHIndex];
	const BannerPatternDef *specialDef = BannerPatterns::get(patternId);
	bool specialPattern = specialDef != nullptr && specialDef->itemId != 0;
	iVSlotIndexA[0] = 15;
	iVSlotIndexA[1] = specialPattern ? -1 : 0;
	iVSlotIndexA[2] = specialPattern ? 0 : 1;
    } else {
        iVSlotIndexA[0] = CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
        iVSlotIndexA[1] = 0;
        iVSlotIndexA[2] = 1;
    }
	ui.PlayUISFX(eSFX_Focus);
	UpdateVerticalSlots();
	UpdateHighlight();
	setGroupText(GetGroupNameText(m_pGroupA[m_iGroupVIndex][m_iGroupHIndex]));
	UpdateTooltips();
}

void UIScene_CraftingMenu::handleCraftingSlotClick(int iIndex) {
	if (iIndex == m_iCurrentSlotHIndex) {
		// Click on already-selected slot — craft the item

		if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex > 0) {
			handleKeyDown(m_iPad, ACTION_MENU_Y, false);
		} else {
			handleKeyDown(m_iPad, ACTION_MENU_A, false);
		}
	} else {
		int iOldHSlot = m_iCurrentSlotHIndex;
		m_iCurrentSlotHIndex = iIndex;
		m_iCurrentSlotVIndex = 1;
		if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex > 0) {
            iVSlotIndexA[0] = 15; // White
            iVSlotIndexA[1] = 0;  // Black (center)
            iVSlotIndexA[2] = 1;  // Red
        } else {
            iVSlotIndexA[0] = CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
            iVSlotIndexA[1] = 0;
            iVSlotIndexA[2] = 1;
        }
		UpdateVerticalSlots();
		UpdateHighlight();
		//if (CanBeMadeA[iOldHSlot].iCount > 0) {
			setShowCraftHSlot(iOldHSlot, true);
		//}
		ui.PlayUISFX(eSFX_Focus);
	}
}

bool isClickInsideControl(F32 x, F32 y, UIControl& control) {
	F32 rectX = control.getXPos();
	F32 rectY = control.getYPos();

	return x >= rectX && x < rectX + control.getWidth() && y >= rectY && y < rectY + control.getHeight();
}

void UIScene_CraftingMenu::handleLeftMouseClick(F32 x, F32 y, F32 sceneX, F32 sceneY) {
	{
		int maxHorizontalTabs = (m_iContainerType == RECIPE_TYPE_3x3) ? m_iMaxGroupH3x3 : m_iMaxGroupH2x2;

		F32 groupRowX1 = m_GroupH[0].getXPos();
		F32 groupRowWidth = m_GroupH[0].getWidth();

		F32 tabStep = groupRowWidth + (m_GroupH[1].getXPos() - (groupRowX1 + groupRowWidth));
		F32 rowY = m_GroupH[0].getYPos();
		F32 rowHeight = m_GroupH[0].getHeight();

		if (sceneY >= rowY && sceneY < rowY + rowHeight && sceneX >= groupRowX1 && tabStep > 0.0f) {
			F32 offsetX = sceneX - groupRowX1;
			int tabIndex = static_cast<int>(offsetX / tabStep);

			if (tabIndex >= 0 && tabIndex < maxHorizontalTabs && tabIndex != m_iGroupHIndex) {
				F32 tabOffset = offsetX - tabIndex * tabStep;

				if (tabOffset < groupRowWidth) {
					if (iHSlotState[tabIndex]) {
						m_iGroupHIndex = tabIndex;
						setHGroupHighlight(m_iGroupHIndex);

						handleCraftingGroupSwitch();
					}
					return;
				}
			}
		}
	}
	{
		int maxVerticalTabs = (m_iContainerType == RECIPE_TYPE_3x3) ? m_iMaxGroupV3x3 : m_iMaxGroupV2x2;

		F32 groupRowY1 = m_GroupV[0].getYPos();
		F32 groupRowHeight = m_GroupV[0].getHeight();

		F32 tabStep = groupRowHeight + (m_GroupV[1].getYPos() - (groupRowY1 + groupRowHeight));
		F32 rowX = m_GroupV[0].getXPos();
		F32 rowWidth = m_GroupV[0].getWidth();

		if (sceneY >= groupRowY1 && sceneX >= rowX && sceneX < rowX + rowWidth && tabStep > 0.0f) {
			F32 offsetY = sceneY - groupRowY1;
			int tabIndex = static_cast<int>(offsetY / tabStep);

			if (tabIndex >= 0 && tabIndex < maxVerticalTabs && tabIndex != m_iGroupVIndex) {
				F32 tabOffset = offsetY - tabIndex * tabStep;

				if (tabOffset < groupRowHeight) {
					m_iGroupHIndex = 0;
					m_iGroupVIndex = tabIndex;
					setVGroupHighlight(m_iGroupVIndex);

					handleCraftingGroupSwitch();
					return;
				}
			}
		}
	}

	bool showingSpecial3 = (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex == 0);
	bool showingSpecial2 = false;
	bool showingNormalSlots = (!showingSpecial3 && !showingSpecial2);

	if (showingSpecial3) {
		if (isClickInsideControl(sceneX, sceneY, m_Slot3_1)) {
			handleCraftingSlotClick(0);
			UpdateCraftingPanels();
		} else if (isClickInsideControl(sceneX, sceneY, m_Slot3_2)) {
			handleCraftingSlotClick(1);
			UpdateCraftingPanels();
		} else if (isClickInsideControl(sceneX, sceneY, m_Slot3_3)) {
			handleCraftingSlotClick(2);
			UpdateCraftingPanels();
		}
	} else if (showingSpecial2) {

	} else if (showingNormalSlots) {
		if (m_hSlotBoundsValid && m_hSlotSpacing > 0) {
			F32 rowWidth = m_hSlotSpacing * m_iCraftablesMaxHSlotC;
			if (x >= m_hSlotX0 && x < m_hSlotX0 + rowWidth && y >= m_hSlotY0 && y < m_hSlotY1) {
				int iNewSlot = (int)((x - m_hSlotX0) / m_hSlotSpacing);
				if (iNewSlot >= 0 && iNewSlot < m_iCraftablesMaxHSlotC) {
					// Only interact with populated slots
					//if (CanBeMadeA[iNewSlot].iCount != 0) {
						handleCraftingSlotClick(iNewSlot);
					//}
				}
			}
		}
	}
	
}

bool UIScene_CraftingMenu::handleMouseClick(F32 x, F32 y)
{
	// Route the click to the tab/slot hit-testing. handleMouseClick only
	// receives scene coords (UIController passes sceneMouseX/Y), and both
	// coordinate pairs in handleLeftMouseClick are in that same scene
	// render space (tab controls + cached H-slot bounds), so pass the
	// same coords for both pairs.
	handleLeftMouseClick(x, y, x, y);
	// Consume all mouse clicks so misses don't generate ACTION_MENU_A
	// and accidentally craft. Only blocks mouse-originated presses.
	return true;
}
#endif

void UIScene_CraftingMenu::handleReload()
{
	// The movie was just reloaded and its frame scripts have not run yet, so
	// the tab setup below can race exactly like the constructor-time one.
	// Re-arm the tick-time re-issue so the tabs are re-asserted once the
	// new movie is live.
	m_bCraftingTabsReissueArmed = false;
	m_bCraftingTabsReissued = false;

	m_slotListInventory.addSlots(CRAFTING_INVENTORY_SLOT_START,CRAFTING_INVENTORY_SLOT_END - CRAFTING_INVENTORY_SLOT_START);
	m_slotListHotBar.addSlots(CRAFTING_HOTBAR_SLOT_START, CRAFTING_HOTBAR_SLOT_END - CRAFTING_HOTBAR_SLOT_START);

	for(unsigned int i = 0; i < 9; ++i)
	{
		m_slotListIngredients[i].addSlot(CRAFTING_INGREDIENTS_DESCRIPTION_START + i);
	}
	m_slotListCraftingOutput.addSlot(CRAFTING_OUTPUT_SLOT_START);
	if (m_iContainerType == RECIPE_TYPE_3x3) {
		m_slotListIngredientsLayout.addSlots(CRAFTING_INGREDIENTS_LAYOUT_START, m_iIngredientsMaxSlotC);
		m_slotListIngredientsLayoutSmall.setVisible(false);
	} else {
		m_slotListIngredientsLayoutSmall.addSlots(CRAFTING_INGREDIENTS_LAYOUT_START, m_iIngredientsMaxSlotC);
		m_slotListIngredientsLayout.setVisible(false);
	}

	m_Slot2_1.addSlot(CRAFTING_SLOT_EXTRA_START + 0);
	m_Slot2_2.addSlot(CRAFTING_SLOT_EXTRA_START + 1);

	m_Slot3_1.addSlot(CRAFTING_SLOT_EXTRA_START + 0);
	m_Slot3_2.addSlot(CRAFTING_SLOT_EXTRA_START + 1);
	m_Slot3_3.addSlot(CRAFTING_SLOT_EXTRA_START + 2);

	m_slotPreviewSlot.addSlot(CRAFTING_SLOT_EXTRA_START + 0);

	// 3 Slot vertical scroll
	m_slotListCrafting3VSlots[0].addSlot(CRAFTING_V_SLOT_START + 0);
	m_slotListCrafting3VSlots[1].addSlot(CRAFTING_V_SLOT_START + 1);
	m_slotListCrafting3VSlots[2].addSlot(CRAFTING_V_SLOT_START + 2);

	// 2 Slot vertical scroll
	// 2 slot scroll has swapped order
	m_slotListCrafting2VSlots[0].addSlot(CRAFTING_V_SLOT_START + 1);
	m_slotListCrafting2VSlots[1].addSlot(CRAFTING_V_SLOT_START + 0);

	// 1 Slot scroll (for 480 mainly)
	m_slotListCrafting1VSlots.addSlot(CRAFTING_V_SLOT_START);

	m_slotListCraftingHSlots.addSlots(CRAFTING_H_SLOT_START,m_iCraftablesMaxHSlotC);

	initializeCraftingTabs();

	app.DebugPrintf(app.USER_SR,"Reloading MultiPanel\n");
	int temp = m_iDisplayDescription;
	m_iDisplayDescription = m_iDisplayDescription==0?1:0;
	UpdateMultiPanel();
	m_iDisplayDescription = temp;
	UpdateMultiPanel();

	app.DebugPrintf(app.USER_SR,"Reloading Highlight and scroll\n");

	// reset the vertical slots
	m_iCurrentSlotHIndex = 0;
	m_iCurrentSlotVIndex = 1;
	iVSlotIndexA[0]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
	iVSlotIndexA[1]=0;
	iVSlotIndexA[2]=1;
	UpdateVerticalSlots();
	UpdateHighlight();

	app.DebugPrintf(app.USER_SR,"Reloading tabs\n");
	setVGroupHighlight(m_iGroupVIndex);
	UpdateCraftingPanels();
}

void UIScene_CraftingMenu::customDraw(IggyCustomDrawCallbackRegion *region)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft->localplayers[m_iPad] == nullptr || pMinecraft->localgameModes[m_iPad] == nullptr) return;

	shared_ptr<ItemInstance> item = nullptr;
	float alpha = 1.0f;
	bool inventoryItem = false;
	bool decorations = true;

	int slotId = -1;
	int iTabV = -1;
	int iTabH = -1;

	if (swscanf(static_cast<wchar_t*>(region->name), L"slot_%d", &slotId) == 1) {
		//if (m_iGroupVIndex != eGroupVType_Workbench) return;

		if (slotId == -1)
		{
			app.DebugPrintf("This is not the control we are looking for\n");
		}
		else if(slotId >= CRAFTING_INVENTORY_SLOT_START && slotId < CRAFTING_INVENTORY_SLOT_END)
		{
			int iIndex = slotId - CRAFTING_INVENTORY_SLOT_START;
			iIndex += m_iMenuInventoryStart;
			Slot *slot = m_menu->getSlot(iIndex);
			item = slot->getItem();
			inventoryItem = true;
		}
		else if(slotId >= CRAFTING_HOTBAR_SLOT_START && slotId < CRAFTING_HOTBAR_SLOT_END)
		{
			int iIndex = slotId - CRAFTING_HOTBAR_SLOT_START;
			iIndex += m_iMenuHotBarStart;
			Slot *slot = m_menu->getSlot(iIndex);
			item = slot->getItem();
			inventoryItem = true;
		}
		else if(slotId >= CRAFTING_V_SLOT_START && slotId < CRAFTING_V_SLOT_END )
		{
			decorations = false;
			int iIndex = slotId - CRAFTING_V_SLOT_START;
			if(m_vSlotsInfo[iIndex].show)
			{
				item = m_vSlotsInfo[iIndex].item;
				alpha = static_cast<float>(m_vSlotsInfo[iIndex].alpha)/31.0f;
			}
		}
		else if(slotId >= CRAFTING_H_SLOT_START && slotId < (CRAFTING_H_SLOT_START + m_iCraftablesMaxHSlotC) )
		{
			decorations = false;
			int iIndex = slotId - CRAFTING_H_SLOT_START;
	#ifdef _WINDOWS64
			// Cache H slot SWF-space positions from the custom draw transform matrix
			if (iIndex == 0 || iIndex == 1)
			{
				F32 mat[16];
				gdraw_D3D11_CalculateCustomDraw_4J(region, mat);
				// Matrix to SWF coords (same formula as setupCustomDrawMatrices)
				F32 sw = (F32)getRenderWidth();
				F32 sh = (F32)getRenderHeight();
				F32 swfX = sw * (1.0f + mat[3]) / 2.0f;
				F32 swfY = sh * (1.0f - mat[7]) / 2.0f;
				if (iIndex == 0)
				{
					m_hSlotX0 = swfX;
					m_hSlotY0 = swfY;
					// Slot visual height from matrix scale and region height
					F32 slotH = sh * (-mat[5]) / 2.0f * region->y1;
					m_hSlotY1 = swfY + slotH;
				}
				else
				{
					m_hSlotSpacing = swfX - m_hSlotX0;
					m_hSlotBoundsValid = (m_hSlotSpacing > 0);
				}
			}
	#endif
			if(m_hSlotsInfo[iIndex].show)
			{
				item = m_hSlotsInfo[iIndex].item;
				alpha = static_cast<float>(m_hSlotsInfo[iIndex].alpha)/31.0f;
			}
		}
		else if(slotId >= CRAFTING_INGREDIENTS_LAYOUT_START && slotId < (CRAFTING_INGREDIENTS_LAYOUT_START + m_iIngredientsMaxSlotC) )
		{
			int iIndex = slotId - CRAFTING_INGREDIENTS_LAYOUT_START;
			if(m_ingredientsSlotsInfo[iIndex].show)
			{
				item = m_ingredientsSlotsInfo[iIndex].item;
				alpha = static_cast<float>(m_ingredientsSlotsInfo[iIndex].alpha)/31.0f;
			}
		}
		else if (slotId >= CRAFTING_SLOT_EXTRA_START && slotId < CRAFTING_SLOT_EXTRA_END) {
			int iIndex = (slotId - CRAFTING_SLOT_EXTRA_START);
			if (iIndex == 0) {
				if (m_iGroupHIndex > 0) {
					item = m_bannerPreviewHolder;
					alpha = 1.0f;
				} else {
					if (m_hSlotsInfo[0].show) {
						item = m_hSlotsInfo[0].item;
						alpha = static_cast<float>(m_hSlotsInfo[0].alpha) / 31.0f;
					} else {
						if (m_craftingOutputSlotInfo.show)
						{
							item = m_craftingOutputSlotInfo.item;
							alpha = static_cast<float>(m_hSlotsInfo[0].alpha) / 31.0f;
						}
					}
				}
			} else if (iIndex == 1) {
				if (CurrentInventoryBanners.size() > 0) {
					if (m_iCurrentSlotHIndex == 1) {
						item = CurrentInventoryBanners[m_iCurrentSlotVIndex-1];
					} else {
						item = CurrentInventoryBanners[0];
					}
				}
			}
		}
		else if (slotId >= CRAFTING_INGREDIENTS_DESCRIPTION_START && slotId < (CRAFTING_INGREDIENTS_DESCRIPTION_START + 9))
		{
			int iIndex = slotId - CRAFTING_INGREDIENTS_DESCRIPTION_START;
			if(m_ingredientsInfo[iIndex].show)
			{
				item = m_ingredientsInfo[iIndex].item;
				alpha = static_cast<float>(m_ingredientsInfo[iIndex].alpha)/31.0f;
			}
		}
		else if(slotId == CRAFTING_OUTPUT_SLOT_START )
		{
			if(m_craftingOutputSlotInfo.show)
			{
				item = m_craftingOutputSlotInfo.item;
				alpha = static_cast<float>(m_craftingOutputSlotInfo.alpha)/31.0f;
			}
		}
	} else if (swscanf(static_cast<wchar_t*>(region->name), L"TabV_%d", &iTabV) == 1) {
		GroupIcons& vGroupIconInfo = m_vGroupIconCache[iTabV - 1];

		if (vGroupIconInfo.VSlot.show) {
			item = vGroupIconInfo.VSlot.item;
			alpha = static_cast<float>(vGroupIconInfo.VSlot.alpha)/31.0f;
		}
	} else if (swscanf(static_cast<wchar_t*>(region->name), L"TabH_%d", &iTabH) == 1) {
		GroupIcons& vGroupIconInfo = m_vGroupIconCache[m_iGroupVIndex];

		if (m_selectedBannerEdit == nullptr) {
			if (vGroupIconInfo.HSlots.size() >= iTabH && iTabH == 1) {
				item = vGroupIconInfo.HSlots[iTabH - 1].item;
			}
		} else {
			if (vGroupIconInfo.HSlots.size() >= iTabH && iTabH != 1) {
				item = vGroupIconInfo.HSlots[iTabH - 1].item;
			}
		}

		//alpha = static_cast<float>(vGroupIconInfo.HSlots[iTabH - 1].alpha)/31.0f;
	}

	if(item != nullptr) {
		if(!inventoryItem) {
			if( item->id == Item::clock_Id || item->id == Item::compass_Id ) {
				// 4J Stu - For clocks and compasses we set the aux value to a special one that signals we should use a default texture
				// rather than the dynamic one for the player
				item->setAuxValue(0xFF);
			} else if( (item->getAuxValue() & 0xFF) == 0xFF) {
				// 4J Stu - If the aux value is set to match any
				item->setAuxValue(0);
			}
		}
		customDrawSlotControl(region,m_iPad,item,alpha,item->isFoil(),decorations);
	}
}

void UIScene_CraftingMenu::tick()
{
	UIScene::tick();

	// One-shot re-issue of the tab setup once the movie is live. The
	// constructor/handleReload calls can land before the movie's frame
	// scripts have run, leaving the AS3 iggy-slot placeholder sprites
	// visible (floating blue boxes). m_hasTickedOnce is set only after
	// IggyPlayerTickRS has run, and the arming tick guarantees a full tick
	// has elapsed since the movie was (re)loaded, so this fires strictly
	// after the movie is settled. Idempotent: AS3 SetupIggySlot handles
	// re-runs, and both calls re-assert the current group indices.
	if (!m_bCraftingTabsReissued && m_hasTickedOnce && hasMovie())
	{
		if (m_bCraftingTabsReissueArmed)
		{
			m_bCraftingTabsReissued = true;
			initializeCraftingTabs();
			UpdateCraftingPanels();
		}
		else
		{
			m_bCraftingTabsReissueArmed = true;
		}
	}
}

int UIScene_CraftingMenu::getPad()
{
	return m_iPad;
}

bool UIScene_CraftingMenu::allowRepeat(int key)
{
	switch(key)
	{
	// X is used to open this menu, so don't let it repeat
	case ACTION_MENU_X:
		return false;
	}
	return true;
}

void UIScene_CraftingMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_InventoryMenu handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	//if (m_iGroupVIndex == eGroupVType_Banner && m_iCurrentSlotHIndex == 1) {

	//} else {
		switch(key)
		{
		case ACTION_MENU_OTHER_STICK_UP:
		case ACTION_MENU_OTHER_STICK_DOWN:
			sendInputToMovie(key,repeat,pressed,released);
			break;
		default:
			if(pressed)
			{
				handled = handleKeyDown(m_iPad, key, repeat);
			}
			break;
		};
	//}


}

void UIScene_CraftingMenu::hideAllHSlots()
{
	for(unsigned int iIndex = 0; iIndex < m_iMaxHSlotC; ++iIndex)
	{
		m_hSlotsInfo[iIndex].item = nullptr;
		m_hSlotsInfo[iIndex].alpha = 31;
		m_hSlotsInfo[iIndex].show = false;
	}
}

void UIScene_CraftingMenu::hideAllVSlots()
{
	for(unsigned int iIndex = 0; iIndex < m_iMaxDisplayedVSlotC; ++iIndex)
	{
		m_vSlotsInfo[iIndex].item = nullptr;
		m_vSlotsInfo[iIndex].alpha = 31;
		m_vSlotsInfo[iIndex].show = false;
	}
}

void UIScene_CraftingMenu::hideAllIngredientsSlots()
{
	for(int i=0;i<m_iIngredientsC;i++)
	{
		m_ingredientsInfo[i].item = nullptr;
		m_ingredientsInfo[i].alpha = 31;
		m_ingredientsInfo[i].show = false;

		m_labelIngredientsDesc[i].setLabel(L"");

		IggyDataValue result;
		IggyDataValue value[2];

		value[0].type = IGGY_DATATYPE_number;
		value[0].number = i;

		value[1].type = IGGY_DATATYPE_boolean;
		value[1].boolval = false;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ) , m_funcShowIngredientSlot , 2 , value );
	}
}

void UIScene_CraftingMenu::setCraftHSlotItem(int iPad, int iIndex, shared_ptr<ItemInstance> item, unsigned int uiAlpha)
{
	m_hSlotsInfo[iIndex].item = item;
	m_hSlotsInfo[iIndex].alpha = uiAlpha;
	m_hSlotsInfo[iIndex].show = true;
}

void UIScene_CraftingMenu::setCraftVSlotItem(int iPad, int iIndex, shared_ptr<ItemInstance> item, unsigned int uiAlpha)
{
	m_vSlotsInfo[iIndex].item = item;
	m_vSlotsInfo[iIndex].alpha = uiAlpha;
	m_vSlotsInfo[iIndex].show = true;
}

void UIScene_CraftingMenu::setCraftingOutputSlotItem(int iPad, shared_ptr<ItemInstance> item)
{
	if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex == 0) {
		// neoLegacy - item can be nullptr when the Banner group has no recipes (e.g. Complex Table filters out the plain-banner recipes); never dereference it
		setDescriptionTextTop(item ? item->getHoverName().c_str() : L"");
	}
	m_craftingOutputSlotInfo.item = item;
	m_craftingOutputSlotInfo.alpha = 31;
	m_craftingOutputSlotInfo.show = item != nullptr;
}

void UIScene_CraftingMenu::setCraftingOutputSlotRedBox(bool show)
{
	m_slotListCraftingOutput.showSlotRedBox(0,show);
}

void UIScene_CraftingMenu::setIngredientSlotItem(int iPad, int index, shared_ptr<ItemInstance> item)
{
	m_ingredientsSlotsInfo[index].item = item;
	m_ingredientsSlotsInfo[index].alpha = 31;
	m_ingredientsSlotsInfo[index].show = item != nullptr;
}

void UIScene_CraftingMenu::setIngredientSlotRedBox(int index, bool show)
{
	m_slotListIngredientsLayout.showSlotRedBox(index,show);
}

void UIScene_CraftingMenu::setIngredientDescriptionItem(int iPad, int index, shared_ptr<ItemInstance> item)
{
	m_ingredientsInfo[index].item = item;
	m_ingredientsInfo[index].alpha = 31;
	m_ingredientsInfo[index].show = item != nullptr;

	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = index;

	value[1].type = IGGY_DATATYPE_boolean;
	value[1].boolval = m_ingredientsInfo[index].show;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ) , m_funcShowIngredientSlot , 2 , value );
}

void UIScene_CraftingMenu::setIngredientDescriptionRedBox(int index, bool show)
{
	m_slotListIngredients[index].showSlotRedBox(0,show);
}

void UIScene_CraftingMenu::setIngredientDescriptionText(int index, LPCWSTR text)
{
	m_labelIngredientsDesc[index].setLabel(text);
}


void UIScene_CraftingMenu::setShowCraftHSlot(int iIndex, bool show)
{
	m_hSlotsInfo[iIndex].show = show;
}

void UIScene_CraftingMenu::setHGroupHighlight(int iIndex)
{
	IggyDataValue result;
	IggyDataValue value[2];
		
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iIndex;
	value[1].type = IGGY_DATATYPE_number;
	value[1].number = m_iGroupVIndex;

	IggyResult out = IggyPlayerCallMethodRS(getMovie(), &result, IggyPlayerRootPath(getMovie()), m_funcSetActiveTab, 2, value);
}

void UIScene_CraftingMenu::setVGroupHighlight(int iIndex) {
	setHGroupHighlight(m_iGroupHIndex);
	m_selectedBannerEdit = nullptr;
	m_bannerPreviewHolder = nullptr;
}


void UIScene_CraftingMenu::setGroupText(LPCWSTR text)
{
	m_labelGroupName.setLabel(text);
}

void UIScene_CraftingMenu::setDescriptionText(LPCWSTR text)
{
	m_labelDescriptionText.setLabel(text);
}

void UIScene_CraftingMenu::setDescriptionTextTop(LPCWSTR text)
{
	m_labelDescriptionTextTop.setLabel(text);
}

void UIScene_CraftingMenu::setItemText(LPCWSTR text)
{
	//if (m_iContainerType == RECIPE_TYPE_2x2) {
	//	m_labelItemNameSmall.setLabel(text);
	//	m_labelItemName.setLabel(L"");
	//} else {
		m_labelItemName.setLabel(text);
		m_labelItemNameSmall.setLabel(L"");
	//}
}

void UIScene_CraftingMenu::UpdateMultiPanel()
{
	// Call Iggy function to show the current panel
	IggyDataValue result;
	IggyDataValue value[1];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = m_iDisplayDescription;

	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ) , m_funcShowPanelDisplay , 1 , value );
}

void UIScene_CraftingMenu::scrollDescriptionUp()
{
	// handled differently
}

void UIScene_CraftingMenu::scrollDescriptionDown()
{
	// handled differently
}

void UIScene_CraftingMenu::updateHighlightAndScrollPositions()
{
	{	
		IggyDataValue result;
		IggyDataValue value[2];

		value[0].type = IGGY_DATATYPE_number;
		value[0].number = m_iCurrentSlotHIndex;

		int selectorType = 0;

		if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex == 0 && m_iCurrentSlotHIndex == 1) {
			if (CurrentInventoryBanners.size() == 2) {
				selectorType = 1;
			} else if (CurrentInventoryBanners.size() > 2) {
				selectorType = 2;
			}
		} else if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex > 0) {
			selectorType = 2;
		} else {
			if(CanBeMadeA[m_iCurrentSlotHIndex].iCount == 2) {
				selectorType = 1;
			} else if( CanBeMadeA[m_iCurrentSlotHIndex].iCount > 2) {
				selectorType = 2;
			}
		}

		value[1].type = IGGY_DATATYPE_number;
		value[1].number = selectorType;

		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ) , m_funcMoveSelector , 2 , value );
	}

	{
		IggyDataValue result;
		IggyDataValue value[1];

		value[0].type = IGGY_DATATYPE_number;
		value[0].number = m_iCurrentSlotVIndex;

		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ) , m_funcSelectVerticalItem , 1 , value );
	}
}

void UIScene_CraftingMenu::initializeCraftingTabs() {
	IggyDataValue result;
	IggyDataValue value[1];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = (m_iContainerType == RECIPE_TYPE_3x3) ? 1 : 0;
	IggyResult out = IggyPlayerCallMethodRS(getMovie(), &result, IggyPlayerRootPath(getMovie()), m_funcInitCraftingMenu, 1, value);

	SetCraftingMenu();
	setHGroupHighlight(m_iGroupHIndex);
}

void UIScene_CraftingMenu::HandleMessage(EUIMessage message, void *data)
{
	switch(message)
	{
	case eUIMessage_InventoryUpdated:
		handleInventoryUpdated(data);
		break;
	};
}

void UIScene_CraftingMenu::handleInventoryUpdated(LPVOID data)
{
	HandleInventoryUpdated();

	if (m_selectedBannerEdit != nullptr) {
		bool stillHasEditingBanner = false;
		for (unsigned int i = 0; i < CurrentInventoryBanners.size(); i++) {
			if (CurrentInventoryBanners[i] == nullptr) continue;
			if (CurrentInventoryBanners[i].get() == m_selectedBannerEdit.get()) {
				stillHasEditingBanner = true;
				break;
			}
		}

		if (!stillHasEditingBanner) setSelectedBannerEdit(nullptr);
	}
}

void UIScene_CraftingMenu::updateVSlotPositions(int iSlots, int i)
{
	// Not needed
}

void UIScene_CraftingMenu::ShowSuperCraftingSlots(int iIndex) {
	IggyDataValue result;
	IggyDataValue value[1];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iIndex;
	IggyResult out = IggyPlayerCallMethodRS(getMovie(), &result, IggyPlayerRootPath(getMovie()), m_funcShowSuperCraftingSlots, 1, value);
}

void UIScene_CraftingMenu::SetCraftingMenu() {
	IggyDataValue result;
	IggyDataValue value[3];

	int currentHGroups = (m_iContainerType == RECIPE_TYPE_3x3 ? m_iMaxGroupH3x3 : m_iMaxGroupH2x2);
	if (m_iGroupVIndex == eGroupVType_Banner) {
		currentHGroups = m_iMaxBannerGroupH3x3;
	}

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = currentHGroups;
	value[1].type = IGGY_DATATYPE_number;
	value[1].number = (m_iContainerType == RECIPE_TYPE_3x3 ? m_iMaxGroupV3x3 : m_iMaxGroupV2x2);
	value[2].type = IGGY_DATATYPE_boolean;
	value[2].boolval = (m_iGroupVIndex == eGroupVType::eGroupVType_Banner);	// false = static icons, true = iggy custom draw ("TabH_N" regions)
	IggyResult out = IggyPlayerCallMethodRS(getMovie(), &result, IggyPlayerRootPath(getMovie()), m_funcSetCraftingMenu, 3, value);
}


void UIScene_CraftingMenu::UpdateCraftingPanels() {
	UpdateEnabledTabs();

	if (m_iGroupVIndex == eGroupVType_Workbench) {
		m_iDisplayDescription = DISPLAY_INVENTORY;
		ShowSuperCraftingSlots(0);
		ShowDescriptionPanel(false);
	} else if (m_iGroupVIndex == eGroupVType_Banner) {
		ShowDescriptionPanel(m_iGroupHIndex == 0);

		if (m_iGroupHIndex == 0) {
			ShowSuperCraftingSlots(3);
			m_iDisplayDescription = DISPLAY_INVENTORY;

			if (m_iCurrentSlotHIndex == 0) {
				m_labelSpinnerTitle.setLabel(L"Create Banner");
			} else if (m_iCurrentSlotHIndex == 1) {
				m_labelSpinnerTitle.setLabel(L"Select Banner");
			}
		} else {
			ShowSuperCraftingSlots(0);
			m_iDisplayDescription = DISPLAY_PREVIEW;
		}

		
	}

	SetCraftingMenu();
	UpdateMultiPanel();
	updateHighlightAndScrollPositions();
}

void UIScene_CraftingMenu::UpdateEnabledTabs() {
	int maxHGroups = (m_iContainerType == RECIPE_TYPE_2x2) ? m_iMaxGroupH2x2 : m_iMaxGroupH3x3;
	for (int i = 0; i < maxHGroups; i++) {
		EnableTab(i, true);
	}

	if (m_iGroupVIndex == eGroupVType_Banner) {
		if (m_selectedBannerEdit == nullptr) {
			for (int i = 1; i < maxHGroups; i++) {
				EnableTab(i, false);
			}
		} else {
			EnableTab(0, false);
		}
	}
}

void UIScene_CraftingMenu::EnableTab(int iIndex, bool enabled) {
	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iIndex;
	value[1].type = IGGY_DATATYPE_boolean;
	value[1].boolval = enabled;
	IggyResult out = IggyPlayerCallMethodRS(getMovie(), &result, IggyPlayerRootPath(getMovie()), m_funcEnableTab, 2, value);

	iHSlotState[iIndex] = enabled;

}

void UIScene_CraftingMenu::ShowDescriptionPanel(bool show) {
	m_controlDescriptionPanel.setVisible(show);
	m_labelDescriptionTextTop.setVisible(show);
}

void UIScene_CraftingMenu::setSelectedBannerEdit(shared_ptr<ItemInstance> banner) {
	m_selectedBannerEdit = banner;

	if (m_selectedBannerEdit == nullptr) {
		m_iGroupHIndex = 0;
		m_bannerPreviewHolder = nullptr;
	} else {
		m_iGroupHIndex = 1;
		m_bannerPreviewHolder = ItemInstance::clone(m_selectedBannerEdit);
	}
	setHGroupHighlight(m_iGroupHIndex);
	handleCraftingGroupSwitch();
}
