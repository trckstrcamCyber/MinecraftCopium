#pragma once
#include "../../../Minecraft.World/Recipy.h"
#include "../../../Minecraft.World/Item.h"
#include "../../../Minecraft.World/BannerPatternRecipe.h"

class LocalPlayer;

// 4J Stu - Crafting menu code that's shared across Iggy and XUI
struct BannerPatternDef;

class IUIScene_CraftingMenu
{
public:
	typedef struct _PreviewBannerData { char id = 0; char color = 0; _PreviewBannerData(char _id, char _color) : id(_id), color(_color) {} } PreviewBannerData;
protected:
#define DISPLAY_INVENTORY 0
#define DISPLAY_DESCRIPTION 1
#define DISPLAY_INGREDIENTS 2
#define DISPLAY_PREVIEW 3
#define DISPLAY_MAX 4

	enum _eGroupTab
	{
		eGroupTab_Left,
		eGroupTab_Middle,
		eGroupTab_Right
	};

	enum eGroupVType {
		eGroupVType_Workbench = 0,
		eGroupVType_Banner = 1,
		eGroupVType_Firework = 2,
		eGroupVType_Dye = 3,
	};

	static const int m_iMaxHSlotC = 40;
	static const int m_iMaxHCraftingSlotC = 40;
	static const int m_iMaxVSlotC = 99; 
	static const int m_iMaxDisplayedVSlotC = 3;
	static const int m_iIngredients3x3SlotC = 9;
	static const int m_iIngredients2x2SlotC = 4;

	static const int m_iMaxHSlot = 12;

	static const int m_iMaxGroupH3x3 = 7;
	static const int m_iMaxGroupH2x2 = 6;

	static const int m_iMaxGroupV2x2 = 0;
	static const int m_iMaxGroupV3x3 = 2;

	static const int m_iMaxBannerGroupH3x3 = 6;

	static int m_iBaseTypeMapA[Item::eBaseItemType_MAXTYPES];

	typedef struct  
	{
		int iCount;
		int iItemBaseType;
		int iRecipeA[m_iMaxVSlotC]; // tiers of item that can be made
	}
	CANBEMADE;

	CANBEMADE CanBeMadeA[m_iMaxHSlotC];
	std::vector<std::shared_ptr<ItemInstance>> CurrentInventoryBanners;

	shared_ptr<ItemInstance> m_selectedBannerEdit;

	vector<PreviewBannerData> m_bannerPreviewData;
	shared_ptr<ItemInstance> m_bannerPreviewHolder;
	


	int m_iCurrentSlotHIndex;
	int m_iCurrentSlotVIndex;
	int m_iRecipeC;
	int m_iContainerType; // 2x2 or 3x3
	bool m_bComplexTable;
	shared_ptr<LocalPlayer> m_pPlayer;

	char m_aBannerCraftPatterns[5][12];

	int m_iGroupHIndex;
	int m_iGroupVIndex;

	int iVSlotIndexA[3]; // index of the v slots currently displayed
	int iHSlotState[7];

	static LPCWSTR m_GroupIconNameA[m_iMaxGroupH3x3];
	static Recipy::_eGroupType m_GroupTypeMapping4GridA[m_iMaxGroupH2x2];
	static Recipy::_eGroupType m_GroupTypeMapping9GridA[m_iMaxGroupH3x3];
	static Recipy::_eGroupType m_GroupTypeBannerMapping9GridA[m_iMaxBannerGroupH3x3];
	Recipy::_eGroupType** m_pGroupA;

	static LPCWSTR m_GroupTabNameA[3];	
	static _eGroupTab m_GroupTabBkgMapping2x2A[m_iMaxGroupH2x2];
	static _eGroupTab m_GroupTabBkgMapping3x3A[m_iMaxGroupH3x3];
	_eGroupTab *m_pGroupTabA;
	int m_iCraftablesMaxHSlotC;
	int m_iIngredientsMaxSlotC;
	int m_iDisplayDescription;
	int m_iIngredientsC;
	bool m_bIgnoreKeyPresses;
	bool m_bSplitscreen;

	eTutorial_State m_previousTutorialState;

	bool handleKeyDown(int iPad, int iAction, bool bRepeat);

public:
	IUIScene_CraftingMenu();
	~IUIScene_CraftingMenu() {
		delete[] m_pGroupA;
	}

protected:
	LPCWSTR GetGroupNameText(int iGroupType);

	bool hasItem(int id, int requiredCount = 1, int aux = -1);
	int hasItemCounts(int id, int requiredCount = -1, int aux = -1);
	bool canCraftPattern(const BannerPatternDef& pattern, char color);
	bool hasResourcesForBannerPatterns(const vector<PreviewBannerData>& patterns);

	void CheckRecipesAvailable();
	void UpdateHighlight();
	void UpdateVerticalSlots();
	void DisplayIngredients();
	void UpdateTooltips();
	void UpdateDescriptionText(bool);
	void HandleInventoryUpdated();

public:
	Recipy::_eGroupType getCurrentGroup() { return m_pGroupA[m_iGroupVIndex][m_iGroupHIndex]; }
	bool isItemSelected(int itemId);

protected:
	virtual int getPad() = 0;
	virtual void hideAllHSlots() = 0;
	virtual void hideAllVSlots() = 0;
	virtual void hideAllIngredientsSlots() = 0;
	virtual void setCraftHSlotItem(int iPad, int iIndex, shared_ptr<ItemInstance> item, unsigned int uiAlpha) = 0;
	virtual void setCraftVSlotItem(int iPad, int iIndex, shared_ptr<ItemInstance> item, unsigned int uiAlpha) = 0;
	virtual void setCraftingOutputSlotItem(int iPad, shared_ptr<ItemInstance> item) = 0;
	virtual void setCraftingOutputSlotRedBox(bool show) = 0;
	virtual void setIngredientSlotItem(int iPad, int index, shared_ptr<ItemInstance> item) = 0;
	virtual void setIngredientSlotRedBox(int index, bool show) = 0;
	virtual void setIngredientDescriptionItem(int iPad, int index, shared_ptr<ItemInstance> item) = 0;
	virtual void setIngredientDescriptionRedBox(int index, bool show) = 0;
	virtual void setIngredientDescriptionText(int index, LPCWSTR text) = 0;
	virtual void setShowCraftHSlot(int iIndex, bool show) = 0;
	virtual void setHGroupHighlight(int iIndex) = 0;
	virtual void setVGroupHighlight(int iIndex) = 0;
	virtual void setGroupText(LPCWSTR text) = 0;
	virtual void setDescriptionText(LPCWSTR text) = 0;
	virtual void setDescriptionTextTop(LPCWSTR text) = 0;
	virtual void setItemText(LPCWSTR text) = 0;
	virtual void scrollDescriptionUp() = 0;
	virtual void scrollDescriptionDown() = 0;
	virtual void updateHighlightAndScrollPositions() = 0;
	virtual void updateVSlotPositions(int iSlots, int i) = 0;
	virtual void UpdateCraftingPanels() = 0;
	void rebuildPreviewItem();

	virtual void setSelectedBannerEdit(shared_ptr<ItemInstance> banner) = 0;

	virtual void UpdateMultiPanel() = 0;
};