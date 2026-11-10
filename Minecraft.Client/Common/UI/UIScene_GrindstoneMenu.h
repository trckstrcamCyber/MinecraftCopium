#pragma once

#include "UIScene_AbstractContainerMenu.h"
#include "IUIScene_GrindstoneMenu.h"

class UIScene_GrindstoneMenu : public UIScene_AbstractContainerMenu, public IUIScene_GrindstoneMenu
{
public:
	UIScene_GrindstoneMenu(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_GrindstoneMenu;}

protected:
	UIControl_SlotList m_slotListItem1, m_slotListItem2, m_slotListResult;

	// GrindstoneMenu*.swf are byte-for-byte copies of the matching AnvilMenu*.swf
	// (minus the anvil hammer icon), so every element keeps its Anvil name:
	//   "AnvilText"      - the title label; re-labelled here
	//   "AnvilTextInput" - Anvil's rename box; hidden, the grindstone has no use for it
	// and the movie's own SetCostLabel / ShowRedCross functions are reused to show
	// the level cost of moving an enchantment onto a book.
	// Nothing may be deleted from the movie's Panel sprite: the Panel class
	// assigns to its children when it is constructed.
	UIControl_Label m_labelGrindstone;
	UIControl m_hiddenNameInput;

	IggyName m_funcShowRedCross, m_funcSetCostLabel;

	bool m_showingCross;
	bool m_costLabelValid;
	bool m_lastCostExpensive;
	wstring m_lastCostString;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene_AbstractContainerMenu)
		UI_BEGIN_MAP_CHILD_ELEMENTS( m_controlMainPanel )
			UI_MAP_ELEMENT( m_slotListItem1, "Ingredient")
			UI_MAP_ELEMENT( m_slotListItem2, "Ingredient2")
			UI_MAP_ELEMENT( m_slotListResult, "Result")
			UI_MAP_ELEMENT( m_labelGrindstone, "AnvilText")
			UI_MAP_ELEMENT( m_hiddenNameInput, "AnvilTextInput")
		UI_END_MAP_CHILD_ELEMENTS()

		UI_MAP_NAME(m_funcShowRedCross, L"ShowRedCross")
		UI_MAP_NAME(m_funcSetCostLabel, L"SetCostLabel")
	UI_END_MAP_ELEMENTS_AND_NAMES()

	virtual wstring getMoviePath();
	virtual void handleReload();

	virtual void tick();

	virtual int getSectionColumns(ESceneSection eSection);
	virtual int getSectionRows(ESceneSection eSection);
	virtual void GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition );
	virtual void GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize );
	virtual void handleSectionClick(ESceneSection eSection) {}
	virtual void setSectionSelectedSlot(ESceneSection eSection, int x, int y);

	virtual UIControl *getSection(ESceneSection eSection);

	void hideUnusedControls();
	void updateCostLabel();
	void setCostLabel(const wstring &label, bool canAfford);
	void showCross(bool show);
};
