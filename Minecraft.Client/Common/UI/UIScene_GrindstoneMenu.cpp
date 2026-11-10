#include "stdafx.h"
#include "UI.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "MultiPlayerLocalPlayer.h"
#include "../../Minecraft.h"
#include "../../../Minecraft.World/GrindstoneMenu.h"
#include "UIScene_GrindstoneMenu.h"

UIScene_GrindstoneMenu::UIScene_GrindstoneMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene_AbstractContainerMenu(iPad, parentLayer)
{
	app.DebugPrintf("[GRINDSTONE DIAG] 6/6 UIScene_GrindstoneMenu::UIScene_GrindstoneMenu -- scene constructed\n");

	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_showingCross = false;
	m_costLabelValid = false;
	m_lastCostExpensive = false;

	// The movie is a copy of Anvil's: its title label is still called "AnvilText"...
	m_labelGrindstone.init( app.GetString(IDS_GRINDSTONE) );

	// ...and it still contains Anvil's rename box, which the grindstone has no use for.
	hideUnusedControls();

	GrindstoneScreenInput *initData = static_cast<GrindstoneScreenInput *>(_initData);
	m_inventory = initData->inventory;

	Minecraft *pMinecraft = Minecraft::GetInstance();

	// UIScene_AbstractContainerMenu::handleDestroy() hands m_previousTutorialState
	// back to the tutorial when the menu closes, so it MUST hold a real value
	// (it is an uninitialised member of the base class). There is no
	// grindstone-specific tutorial state -- Anvil switches to
	// e_Tutorial_State_Anvil_Menu here -- so just remember the current state;
	// restoring it on close is then a no-op.
	m_previousTutorialState = e_Tutorial_State_Gameplay;
	if( pMinecraft->localgameModes[iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[iPad]);
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
	}

	// (Anvil also sets a Rich Presence context here; the constant for a
	// grindstone doesn't exist. Optional polish, not needed for the menu.)

	m_grindstoneMenu = new GrindstoneMenu( initData->inventory, initData->level, initData->x, initData->y, initData->z, pMinecraft->localplayers[iPad] );
	m_grindstoneMenu->addSlotListener(this);

	Initialize( iPad, m_grindstoneMenu, true, GrindstoneMenu::INV_SLOT_START, eSectionGrindstoneUsing, eSectionGrindstoneMax );

	m_slotListItem1.addSlots(GrindstoneMenu::INPUT_SLOT, 1);
	m_slotListItem2.addSlots(GrindstoneMenu::ADDITIONAL_SLOT, 1);
	m_slotListResult.addSlots(GrindstoneMenu::RESULT_SLOT, 1);

	// Put the movie's cost label / cross into a known (empty) state, as Anvil's constructor does.
	setCostLabel(L"", false);

	if(initData) delete initData;

	setIgnoreInput(false);
}

wstring UIScene_GrindstoneMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"GrindstoneMenuSplit";
	}
	else
	{
		return L"GrindstoneMenu";
	}
}

void UIScene_GrindstoneMenu::handleReload()
{
	Initialize( m_iPad, m_menu, true, GrindstoneMenu::INV_SLOT_START, eSectionGrindstoneUsing, eSectionGrindstoneMax  );

	m_slotListItem1.addSlots(GrindstoneMenu::INPUT_SLOT, 1);
	m_slotListItem2.addSlots(GrindstoneMenu::ADDITIONAL_SLOT, 1);
	m_slotListResult.addSlots(GrindstoneMenu::RESULT_SLOT, 1);

	// The movie was re-created, so re-apply everything we changed on it.
	m_showingCross = false;
	m_costLabelValid = false;
	hideUnusedControls();
}

void UIScene_GrindstoneMenu::tick()
{
	UIScene_AbstractContainerMenu::tick();

	updateCostLabel();
	handleTick();
}

void UIScene_GrindstoneMenu::hideUnusedControls()
{
	// UIControl::setVisible() ignores a request that matches its cached state,
	// and after a movie reload the cache says "hidden" while the fresh movie
	// shows the box again -- so flip it on and back off to be sure the call
	// always reaches Iggy.
	m_hiddenNameInput.setVisible(true);
	m_hiddenNameInput.setVisible(false);
}

void UIScene_GrindstoneMenu::updateCostLabel()
{
	if(m_grindstoneMenu == nullptr) return;

	wstring costString = L"";
	bool cannotAfford = false;

	// Only moving an enchantment onto a book costs levels (xpReward < 0); every
	// other result is free and pays out XP instead.
	Slot *resultSlot = m_grindstoneMenu->getSlot(GrindstoneMenu::RESULT_SLOT);
	if(m_grindstoneMenu->xpReward < 0 && resultSlot != nullptr && resultSlot->hasItem())
	{
		LPCWSTR costFormat = app.GetString(IDS_REPAIR_COST);
		wchar_t temp[256];
		swprintf(temp, 256, costFormat, -m_grindstoneMenu->xpReward);
		costString = temp;

		cannotAfford = !resultSlot->mayPickup(dynamic_pointer_cast<Player>(m_inventory->player->shared_from_this()));
	}

	if(!m_costLabelValid || cannotAfford != m_lastCostExpensive || costString != m_lastCostString)
	{
		setCostLabel(costString, cannotAfford);
		m_lastCostString = costString;
		m_lastCostExpensive = cannotAfford;
		m_costLabelValid = true;
	}

	showCross(cannotAfford);
}

void UIScene_GrindstoneMenu::setCostLabel(const wstring &label, bool canAfford)
{
	if (!getMovie()) return;

	IggyDataValue result;
	IggyDataValue value[2];

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].type = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;

	value[1].type = IGGY_DATATYPE_boolean;
	value[1].boolval = canAfford;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetCostLabel , 2 , value );
}

void UIScene_GrindstoneMenu::showCross(bool show)
{
	if(m_showingCross != show)
	{
		if (!getMovie()) return;

		IggyDataValue result;
		IggyDataValue value[1];

		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = show;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcShowRedCross , 1 , value );

		m_showingCross = show;
	}
}

int UIScene_GrindstoneMenu::getSectionColumns(ESceneSection eSection)
{
	int cols = 0;
	switch( eSection )
	{
	case eSectionGrindstoneItem1:
		cols = 1;
		break;
	case eSectionGrindstoneItem2:
		cols = 1;
		break;
	case eSectionGrindstoneResult:
		cols = 1;
		break;
	case eSectionGrindstoneInventory:
		cols = 9;
		break;
	case eSectionGrindstoneUsing:
		cols = 9;
		break;
	default:
		assert( false );
		break;
	}
	return cols;
}

int UIScene_GrindstoneMenu::getSectionRows(ESceneSection eSection)
{
	int rows = 0;
	switch( eSection )
	{
	case eSectionGrindstoneItem1:
		rows = 1;
		break;
	case eSectionGrindstoneItem2:
		rows = 1;
		break;
	case eSectionGrindstoneResult:
		rows = 1;
		break;
	case eSectionGrindstoneInventory:
		rows = 3;
		break;
	case eSectionGrindstoneUsing:
		rows = 1;
		break;
	default:
		assert( false );
		break;
	}
	return rows;
}

void UIScene_GrindstoneMenu::GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition )
{
	switch( eSection )
	{
	case eSectionGrindstoneItem1:
		pPosition->x = m_slotListItem1.getXPos();
		pPosition->y = m_slotListItem1.getYPos();
		break;
	case eSectionGrindstoneItem2:
		pPosition->x = m_slotListItem2.getXPos();
		pPosition->y = m_slotListItem2.getYPos();
		break;
	case eSectionGrindstoneResult:
		pPosition->x = m_slotListResult.getXPos();
		pPosition->y = m_slotListResult.getYPos();
		break;
	case eSectionGrindstoneInventory:
		pPosition->x = m_slotListInventory.getXPos();
		pPosition->y = m_slotListInventory.getYPos();
		break;
	case eSectionGrindstoneUsing:
		pPosition->x = m_slotListHotbar.getXPos();
		pPosition->y = m_slotListHotbar.getYPos();
		break;
	default:
		assert( false );
		break;
	}
}

void UIScene_GrindstoneMenu::GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize )
{
	UIVec2D sectionSize;

	switch( eSection )
	{
	case eSectionGrindstoneItem1:
		sectionSize.x = m_slotListItem1.getWidth();
		sectionSize.y = m_slotListItem1.getHeight();
		break;
	case eSectionGrindstoneItem2:
		sectionSize.x = m_slotListItem2.getWidth();
		sectionSize.y = m_slotListItem2.getHeight();
		break;
	case eSectionGrindstoneResult:
		sectionSize.x = m_slotListResult.getWidth();
		sectionSize.y = m_slotListResult.getHeight();
		break;
	case eSectionGrindstoneInventory:
		sectionSize.x = m_slotListInventory.getWidth();
		sectionSize.y = m_slotListInventory.getHeight();
		break;
	case eSectionGrindstoneUsing:
		sectionSize.x = m_slotListHotbar.getWidth();
		sectionSize.y = m_slotListHotbar.getHeight();
		break;
	default:
		assert( false );
		break;
	}

	if(IsSectionSlotList(eSection))
	{
		int rows = getSectionRows(eSection);
		int cols = getSectionColumns(eSection);

		pSize->x = sectionSize.x/cols;
		pSize->y = sectionSize.y/rows;

		int itemCol = iItemIndex % cols;
		int itemRow = iItemIndex/cols;

		pPosition->x = itemCol * pSize->x;
		pPosition->y = itemRow * pSize->y;
	}
	else
	{
		GetPositionOfSection(eSection, pPosition);
		pSize->x = sectionSize.x;
		pSize->y = sectionSize.y;
	}
}

void UIScene_GrindstoneMenu::setSectionSelectedSlot(ESceneSection eSection, int x, int y)
{
	int cols = getSectionColumns(eSection);

	int index = (y * cols) + x;

	UIControl_SlotList *slotList = nullptr;
	switch( eSection )
	{
	case eSectionGrindstoneItem1:
		slotList = &m_slotListItem1;
		break;
	case eSectionGrindstoneItem2:
		slotList = &m_slotListItem2;
		break;
	case eSectionGrindstoneResult:
		slotList = &m_slotListResult;
		break;
	case eSectionGrindstoneInventory:
		slotList = &m_slotListInventory;
		break;
	case eSectionGrindstoneUsing:
		slotList = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}

	slotList->setHighlightSlot(index);
}

UIControl *UIScene_GrindstoneMenu::getSection(ESceneSection eSection)
{
	UIControl *control = nullptr;
	switch( eSection )
	{
	case eSectionGrindstoneItem1:
		control = &m_slotListItem1;
		break;
	case eSectionGrindstoneItem2:
		control = &m_slotListItem2;
		break;
	case eSectionGrindstoneResult:
		control = &m_slotListResult;
		break;
	case eSectionGrindstoneInventory:
		control = &m_slotListInventory;
		break;
	case eSectionGrindstoneUsing:
		control = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}
	return control;
}
