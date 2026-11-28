#include "stdafx.h"

#include "../../../Minecraft.World/net.minecraft.world.item.crafting.h"
#include "../../../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../../../Minecraft.World/net.minecraft.stats.h"
#include "../../LocalPlayer.h"
#include "../../../Minecraft.World/ComplexCraftingMenu.h"
#include "../../../Minecraft.World/BannerItem.h"
#include "../../../Minecraft.World/BannerPatterns.h"
#include "../../../Minecraft.World/DyePowderItem.h"
#include "IUIScene_CraftingMenu.h"

#include "UI.h"

Recipy::_eGroupType IUIScene_CraftingMenu::m_GroupTypeMapping4GridA[IUIScene_CraftingMenu::m_iMaxGroupH2x2]=
{
	Recipy::eGroupType_Structure,
	Recipy::eGroupType_Tool,
	Recipy::eGroupType_Food,
	Recipy::eGroupType_Mechanism,
	Recipy::eGroupType_Transport,
	Recipy::eGroupType_Decoration,
};

Recipy::_eGroupType IUIScene_CraftingMenu::m_GroupTypeMapping9GridA[IUIScene_CraftingMenu::m_iMaxGroupH3x3]=
{
	Recipy::eGroupType_Structure,
	Recipy::eGroupType_Tool,
	Recipy::eGroupType_Food,
	Recipy::eGroupType_Armour,
	Recipy::eGroupType_Mechanism,
	Recipy::eGroupType_Transport,
	Recipy::eGroupType_Decoration,
};

Recipy::_eGroupType IUIScene_CraftingMenu::m_GroupTypeBannerMapping9GridA[m_iMaxBannerGroupH3x3] =
{
	Recipy::eGroupType_Banner_CustomLogic,
	Recipy::eGroupType_Banner_Stripes,
	Recipy::eGroupType_Banner_Halves,
	Recipy::eGroupType_Banner_Shapes,
	Recipy::eGroupType_Banner_BBG, //borders backgrounds and gradients
	Recipy::eGroupType_Banner_Symbols,
};


LPCWSTR IUIScene_CraftingMenu::m_GroupIconNameA[m_iMaxGroupH3x3]=
{
	L"Structures",//Recipy::eGroupType_Structure,
	L"Tools",//Recipy::eGroupType_Tool,
	L"Food",//Recipy::eGroupType_Food,
	L"Armour",//Recipy::eGroupType_Armour,
	L"Mechanisms",//Recipy::eGroupType_Mechanism,
	L"Transport",//Recipy::eGroupType_Transport,
	L"Decoration",//Recipy::eGroupType_Decoration,
};

IUIScene_CraftingMenu::_eGroupTab IUIScene_CraftingMenu::m_GroupTabBkgMapping2x2A[m_iMaxGroupH2x2]=
{
	eGroupTab_Left,
	eGroupTab_Middle,
	eGroupTab_Middle,
	eGroupTab_Middle,
	eGroupTab_Middle,
	eGroupTab_Right,
};

IUIScene_CraftingMenu::_eGroupTab IUIScene_CraftingMenu::m_GroupTabBkgMapping3x3A[m_iMaxGroupH3x3]=
{
	eGroupTab_Left,
	eGroupTab_Middle,
	eGroupTab_Middle,
	eGroupTab_Middle,
	eGroupTab_Middle,
	eGroupTab_Middle,
	eGroupTab_Right,
};


// mapping array to map the base objects to their description string
// This should map the enums
// enum
// {
// 	eBaseItemType_undefined=0,
// 	eBaseItemType_sword,
// 	eBaseItemType_shovel,
// 	eBaseItemType_pickaxe,
// 	eBaseItemType_hatchet,
// 	eBaseItemType_hoe,
// 	eBaseItemType_door,
// 	eBaseItemType_helmet,
// 	eBaseItemType_chestplate,
// 	eBaseItemType_leggings,
// 	eBaseItemType_boots,
// 	eBaseItemType_ingot,
// 	eBaseItemType_rail,
// 	eBaseItemType_block,
// 	eBaseItemType_pressureplate,
// 	eBaseItemType_stairs,
// 	eBaseItemType_cloth,
// 	eBaseItemType_dyepowder,
//  eBaseItemType_structplanks
// 	eBaseItemType_structblock,
// 	eBaseItemType_slab,
// 	eBaseItemType_halfslab,
// 	eBaseItemType_torch,
// 	eBaseItemType_bow,
// 	eBaseItemType_pockettool,
// 	eBaseItemType_utensil,
//
// }
// eBaseItemType;

IUIScene_CraftingMenu::IUIScene_CraftingMenu()
{
	m_iCurrentSlotHIndex=0;
	m_iCurrentSlotVIndex=1;

	for(int i=0;i<m_iMaxHSlotC;i++)
	{
		CanBeMadeA[i].iCount=0;
		CanBeMadeA[i].iItemBaseType=0;
	}
	memset(CanBeMadeA,0,sizeof(CANBEMADE)*m_iMaxHSlotC);
	m_iRecipeC=0;
	m_iGroupHIndex = 0;
	m_iGroupVIndex=0;
	m_bComplexTable=false;	// neoLegacy

	for(int i=0;i<m_iMaxDisplayedVSlotC;i++)
	{
		iVSlotIndexA[i]=i; // start with 0,1,2
	}

	{
		for (int t = 0; t < 5; t++) {
			for (int s = 0; s < 12; s++) {
				m_aBannerCraftPatterns[t][s] = -1;
			}
		}

		{ // stripes
			m_aBannerCraftPatterns[0][0] = 13; //cr
			m_aBannerCraftPatterns[0][1] = 12; //ss
			m_aBannerCraftPatterns[0][2] = 14; //sc
			m_aBannerCraftPatterns[0][3] = 4;  //bs
			m_aBannerCraftPatterns[0][4] = 8;  //cs
			m_aBannerCraftPatterns[0][5] = 11; //dls
			m_aBannerCraftPatterns[0][6] = 10; //drl
			m_aBannerCraftPatterns[0][7] = 6;  //ls
			m_aBannerCraftPatterns[0][8] = 9;  //ms
			m_aBannerCraftPatterns[0][9] = 7;  //rs
			m_aBannerCraftPatterns[0][10] = 5; //ts
		}
		{ // halves
			m_aBannerCraftPatterns[1][0] = 19; //ld
			m_aBannerCraftPatterns[1][1] = 20; //rd
			m_aBannerCraftPatterns[1][2] = 21; //lud
			m_aBannerCraftPatterns[1][3] = 22; //rud
			m_aBannerCraftPatterns[1][4] = 26; //hh
			m_aBannerCraftPatterns[1][5] = 28; //hhb
			m_aBannerCraftPatterns[1][6] = 25; //vh
			m_aBannerCraftPatterns[1][7] = 27; //vhr
		}
		{ // shapes
			m_aBannerCraftPatterns[2][0] = 23; //mc
			m_aBannerCraftPatterns[2][1] = 24; //mr
			m_aBannerCraftPatterns[2][2] = 0;  //bl
			m_aBannerCraftPatterns[2][3] = 1;  //br
			m_aBannerCraftPatterns[2][4] = 2;  //tl
			m_aBannerCraftPatterns[2][5] = 3;  //tr
			m_aBannerCraftPatterns[2][6] = 15; //bt
			m_aBannerCraftPatterns[2][7] = 16; //tt
			m_aBannerCraftPatterns[2][8] = 17; //bts
			m_aBannerCraftPatterns[2][9] = 18; //tts
		}
		{ // borders, backgrounds and gradients (neoLegacy: indices remapped to BannerPatterns table order)
			m_aBannerCraftPatterns[3][0] = 29; //bo
			m_aBannerCraftPatterns[3][1] = 34; //bri
			m_aBannerCraftPatterns[3][2] = 30; //cbo
			m_aBannerCraftPatterns[3][3] = 32; //gra
			m_aBannerCraftPatterns[3][4] = 33; //gru
		}
		{ // symbols (neoLegacy: cre remapped to BannerPatterns table order)
			m_aBannerCraftPatterns[4][0] = 31; //cre
			m_aBannerCraftPatterns[4][1] = 36; //flo
			m_aBannerCraftPatterns[4][2] = 37; //moj
			m_aBannerCraftPatterns[4][3] = 35; //sku
		}
	}

	m_iDisplayDescription=DISPLAY_INVENTORY;
	m_iIngredientsC=0;
}

LPCWSTR IUIScene_CraftingMenu::GetGroupNameText(int iGroupType)
{
	switch(iGroupType)
	{
	case ShapedRecipy::eGroupType_Tool:
		return app.GetString( IDS_GROUPNAME_TOOLS );
	case ShapedRecipy::eGroupType_Food:
		return app.GetString( IDS_GROUPNAME_FOOD);
	case ShapedRecipy::eGroupType_Structure:
		return app.GetString( IDS_GROUPNAME_STRUCTURES );
	case ShapedRecipy::eGroupType_Armour:
		return app.GetString( IDS_GROUPNAME_ARMOUR );
	case ShapedRecipy::eGroupType_Mechanism:
		return app.GetString( IDS_GROUPNAME_MECHANISMS );
	case ShapedRecipy::eGroupType_Transport:
		return app.GetString( IDS_GROUPNAME_TRANSPORT );
	case ShapedRecipy::eGroupType_Decoration:
		return app.GetString( IDS_GROUPNAME_DECORATIONS );
	case ShapedRecipy::eGroupType_Banner_Stripes:
		return L"Stripes";
	case ShapedRecipy::eGroupType_Banner_Halves:
		return L"Halves";
	case ShapedRecipy::eGroupType_Banner_Shapes:
		return L"Shapes";
	case ShapedRecipy::eGroupType_Banner_BBG: //borders backgrounds and gradients
		return L"Borders, Backgrounds and Gradients";
	case ShapedRecipy::eGroupType_Banner_Symbols:
		return L"Symbols";
	case ShapedRecipy::eGroupType_Banner_CustomLogic:
	default:
		return L"";
	}
}

static int bannerPatternColorOptionCount(int patternId)
{
	if (patternId < 0 || patternId >= BANNER_PATTERN_COUNT) return 16;
	const BannerPatternDef *def = BannerPatterns::get(patternId);
	return def != nullptr && def->itemId != 0 ? 17 : 16;
}

static void resetBannerPatternColorOptions(int* options, int patternId)
{
	const BannerPatternDef *def = BannerPatterns::get(patternId);
	if (def != nullptr && def->itemId != 0)
	{
		options[0] = 15;
		options[1] = -1;
		options[2] = 0;
	}
	else
	{
		options[0] = 15;
		options[1] = 0;
		options[2] = 1;
	}
}

static int stepBannerPatternColorOption(int patternId, int colorIndex, int direction)
{
	const BannerPatternDef *def = BannerPatterns::get(patternId);
	bool specialPattern = def != nullptr && def->itemId != 0;
	int optionCount = bannerPatternColorOptionCount(patternId);
	int optionIndex = specialPattern ? colorIndex + 1 : colorIndex;
	if (optionIndex < 0 || optionIndex >= optionCount) optionIndex = 0;
	optionIndex = (optionIndex + optionCount + direction) % optionCount;
	return specialPattern ? optionIndex - 1 : optionIndex;
}

// neoLegacy - replacement for the upstream preview-banner factory: a white
// banner (dye-order aux 15) carrying one pattern layer, for menu preview/icon slots. Color -1
// (the "no dye" option) stores BLACK, matching the upstream pattern-adder.
static shared_ptr<ItemInstance> makePatternPreviewBanner(const wchar_t *code, int color)
{
	shared_ptr<ItemInstance> banner = std::make_shared<ItemInstance>(Tile::standing_banner_Id, 1, 15);
	BannerItem::addPattern(banner, code, color < 0 ? DyePowderItem::BLACK : color);
	return banner;
}

bool IUIScene_CraftingMenu::handleKeyDown(int iPad, int iAction, bool bRepeat)
{
	bool bHandled = false;

	if(m_bIgnoreKeyPresses) return bHandled;

	// ignore key repeats of the X key - because it's X to open this menu, it can come through as a repeat on opening
	if(bRepeat &&(iAction==ACTION_MENU_X))
	{
		return S_OK;
	}

	Minecraft *pMinecraft = Minecraft::GetInstance();

	if( pMinecraft->localgameModes[getPad()] != nullptr )
	{
		Tutorial *tutorial = pMinecraft->localgameModes[getPad()]->getTutorial();
		if(tutorial != nullptr)
		{
			tutorial->handleUIInput(iAction);
			if(ui.IsTutorialVisible(getPad()) && !tutorial->isInputAllowed(iAction))
			{
				return S_OK;
			}
		}
	}


	switch(iAction)
	{
	case ACTION_MENU_Y:
		if (m_selectedBannerEdit != nullptr && ((int)BannerItem::getPatterns(m_selectedBannerEdit).size() + m_bannerPreviewData.size()) < 6) {
			int patternId = m_aBannerCraftPatterns[m_iGroupHIndex - 1][m_iCurrentSlotHIndex];
			if (patternId != -1) {
				PreviewBannerData data(patternId, iVSlotIndexA[m_iCurrentSlotVIndex]);
				vector<PreviewBannerData> previewData = m_bannerPreviewData;
				previewData.emplace_back(data);
				if (hasResourcesForBannerPatterns(previewData)) {
					m_bannerPreviewData.emplace_back(data);
					rebuildPreviewItem();
					DisplayIngredients();
					ui.PlayUISFX(eSFX_Focus);
				} else {
					ui.PlayUISFX(eSFX_CraftFail);
				}
			}
		}
		UpdateTooltips();
		break;
	case ACTION_MENU_X:
		if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex > 0) {
			if (m_bannerPreviewData.size() > 0) {
				m_bannerPreviewData.pop_back();
				rebuildPreviewItem();
				DisplayIngredients();
			} else {
				setSelectedBannerEdit(nullptr);
			}
		} else {
			if (m_iDisplayDescription != DISPLAY_PREVIEW) {
				m_iDisplayDescription++;
				if (m_iDisplayDescription == DISPLAY_PREVIEW) m_iDisplayDescription++;
			}

			if(m_iDisplayDescription==DISPLAY_MAX) m_iDisplayDescription=DISPLAY_INVENTORY;
		}
		ui.PlayUISFX(eSFX_Focus);
		UpdateMultiPanel();
		UpdateTooltips();
		break;
	case ACTION_MENU_PAUSEMENU:
	case ACTION_MENU_B:
		ui.ShowTooltip( iPad, eToolTipButtonX, false );
		ui.ShowTooltip( iPad, eToolTipButtonB, false );
		ui.ShowTooltip( iPad, eToolTipButtonA, false );
		ui.ShowTooltip( iPad, eToolTipButtonRB, false );
		// kill the crafting xui
		//ui.PlayUISFX(eSFX_Back);
		ui.CloseUIScenes(iPad);

		bHandled = true;
		break;
	case ACTION_MENU_A:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
		// Do some crafting!
		if(m_pPlayer && m_pPlayer->inventory)
		{
			if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex == 0 && m_iCurrentSlotHIndex == 1) {
				if (CurrentInventoryBanners.size() > 0) {
					setSelectedBannerEdit(CurrentInventoryBanners[m_iCurrentSlotVIndex-1]);
				}
			} else if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex > 0) {
				if (app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad()) & (1L << eDebugSetting_CraftAnything)) {
					if (m_bannerPreviewData.size() > 0) {
						shared_ptr<ItemInstance> pTempItemInst = ItemInstance::clone(m_selectedBannerEdit);
						for (PreviewBannerData& data : m_bannerPreviewData) {
							const BannerPatternDef *def = BannerPatterns::get(data.id);
							if (def != nullptr) BannerItem::addPattern(pTempItemInst, def->code, data.color < 0 ? DyePowderItem::BLACK : data.color);
						}

						if (pMinecraft->localgameModes[iPad] != nullptr) {
							Tutorial* tutorial = pMinecraft->localgameModes[iPad]->getTutorial();
							if (tutorial != nullptr) {
								tutorial->onCrafted(pTempItemInst);
							}
						}

						pMinecraft->localgameModes[iPad]->handleCraftBanner(m_selectedBannerEdit, m_bannerPreviewData, m_pPlayer);
						m_pPlayer->inventory->removeResources(m_selectedBannerEdit);

						if (m_pPlayer->inventory->add(pTempItemInst) == false)
						{
							// no room in inventory, so throw it down
							m_pPlayer->drop(pTempItemInst);
						}
						ui.PlayUISFX(eSFX_Craft);
					}
				} else if (m_bannerPreviewData.size() > 0) {
					bool canCraft = true;
					int requiredDyes[16] = {};
					std::vector<std::pair<int, int>> requiredSpecialItems;

					for (const PreviewBannerData& data : m_bannerPreviewData) {
						const BannerPatternDef *pattern = BannerPatterns::get(data.id);
						if (pattern == nullptr) continue;

						if (pattern->itemId != 0) {
							bool found = false;

							for (auto& ingredient : requiredSpecialItems) {
								if (ingredient.first == pattern->itemId && ingredient.second == pattern->itemAux) {
									found = true;
									break;
								}
							}

							if (!found) requiredSpecialItems.emplace_back(pattern->itemId, pattern->itemAux);
							if (data.color >= 0) ++requiredDyes[data.color & 15];
						} else {
							if (pattern->rows[0] == nullptr) continue;

							for (int row = 0; row < 3; ++row) {
								for (int col = 0; col < 3; ++col) {
									if (pattern->rows[row][col] != ' ')
										++requiredDyes[data.color & 15];
								}
							}
						}
					}

					for (const PreviewBannerData& data : m_bannerPreviewData) {
						const BannerPatternDef *pattern = BannerPatterns::get(data.id);
						if (pattern == nullptr) continue;

						if (pattern->itemId == 0) continue;
						int requiredCount = 0;

						for (const PreviewBannerData& other : m_bannerPreviewData) {
							const BannerPatternDef *otherPattern = BannerPatterns::get(other.id);
							if (otherPattern == nullptr) continue;

							if (otherPattern->itemId == pattern->itemId && otherPattern->itemAux == pattern->itemAux) {
								++requiredCount;
							}
						}

						if (!hasItem(pattern->itemId, requiredCount, pattern->itemAux)) {
							canCraft = false;
							break;
						}
					}

					if (canCraft) {
						for (int color = 0; color < 16; ++color) {
							if (requiredDyes[color] > 0 &&
								!hasItem(Item::dye_Id, requiredDyes[color], color)) {
								canCraft = false;
								break;
							}
						}
					}

					if (canCraft) {
						shared_ptr<ItemInstance> pTempItemInst = ItemInstance::clone(m_selectedBannerEdit);

						for (const PreviewBannerData& data : m_bannerPreviewData) {
							const BannerPatternDef *pattern = BannerPatterns::get(data.id);
							if (pattern == nullptr) continue;
							BannerItem::addPattern(pTempItemInst, pattern->code, data.color < 0 ? DyePowderItem::BLACK : data.color);
						}

						if (pMinecraft->localgameModes[iPad] != nullptr) {
							Tutorial* tutorial = pMinecraft->localgameModes[iPad]->getTutorial();

							if (tutorial != nullptr) tutorial->onCrafted(pTempItemInst);
						}

						for (int color = 0; color < 16; ++color) {
							for (int i = 0; i < requiredDyes[color]; ++i) {
								m_pPlayer->inventory->removeResource(Item::dye_Id, color);
							}
						}

						for (const PreviewBannerData& data : m_bannerPreviewData) {
							const BannerPatternDef *pattern = BannerPatterns::get(data.id);
							if (pattern == nullptr) continue;

							if (pattern->itemId != 0) {
								// neoLegacy - itemAux -1 means "any aux" (cbo/bri/flo); removeResource(id, -1)
								// never matches (getSlot requires an exact aux), so use the any-aux overload.
								if (pattern->itemAux < 0) m_pPlayer->inventory->removeResource(pattern->itemId);
								else m_pPlayer->inventory->removeResource(pattern->itemId, pattern->itemAux);
							}
						}

						pMinecraft->localgameModes[iPad]->handleCraftBanner(m_selectedBannerEdit, m_bannerPreviewData, m_pPlayer);
						m_pPlayer->inventory->removeResources(m_selectedBannerEdit);

						if (!m_pPlayer->inventory->add(pTempItemInst)) m_pPlayer->drop(pTempItemInst);

						ui.PlayUISFX(eSFX_Craft);

						m_bannerPreviewData.clear();
						setSelectedBannerEdit(nullptr);

						CheckRecipesAvailable();
						UpdateVerticalSlots();
						UpdateHighlight();
						DisplayIngredients();
					} else {
						ui.PlayUISFX(eSFX_CraftFail);
					}
				}
			} else {
				//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
				Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();
				// Force a make if the debug is on
				if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_CraftAnything))
				{
					if(CanBeMadeA[m_iCurrentSlotHIndex].iCount!=0)
					{
						int iSlot=iVSlotIndexA[m_iCurrentSlotVIndex];

						int iRecipe= CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
						shared_ptr<ItemInstance> pTempItemInst=pRecipeIngredientsRequired[iRecipe].pRecipy->assemble(nullptr);
						//int iIcon=pTempItemInst->getItem()->getIcon(pTempItemInst->getAuxValue());

						if( pMinecraft->localgameModes[iPad] != nullptr)
						{
							Tutorial *tutorial = pMinecraft->localgameModes[iPad]->getTutorial();
							if(tutorial != nullptr)
							{
								tutorial->onCrafted(pTempItemInst);
							}
						}

						pMinecraft->localgameModes[iPad]->handleCraftItem(iRecipe,m_pPlayer);

						if(m_pPlayer->inventory->add(pTempItemInst)==false)
						{
							// no room in inventory, so throw it down
							m_pPlayer->drop(pTempItemInst);
						}
						// play a sound
						//pMinecraft->soundEngine->playUI( L"random.pop", 1.0f, 1.0f);
						ui.PlayUISFX(eSFX_Craft);
					}
				}
				else if(CanBeMadeA[m_iCurrentSlotHIndex].iCount!=0)
				{
					int iSlot;
					if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
					{
						iSlot=iVSlotIndexA[m_iCurrentSlotVIndex];
					}
					else
					{
						iSlot=0;
					}
					int iRecipe= CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
					shared_ptr<ItemInstance> pTempItemInst=pRecipeIngredientsRequired[iRecipe].pRecipy->assemble(nullptr);
					//int iIcon=pTempItemInst->getItem()->getIcon(pTempItemInst->getAuxValue());

					if( pMinecraft->localgameModes[iPad] != nullptr )
					{
						Tutorial *tutorial = pMinecraft->localgameModes[iPad]->getTutorial();
						if(tutorial != nullptr)
						{
							tutorial->createItemSelected(pTempItemInst, pRecipeIngredientsRequired[iRecipe].bCanMake[iPad]);
						}
					}

					if(pRecipeIngredientsRequired[iRecipe].bCanMake[iPad])
					{
						pTempItemInst->onCraftedBy(m_pPlayer->level, dynamic_pointer_cast<Player>( m_pPlayer->shared_from_this() ), pTempItemInst->count );
						// TODO 4J Stu - handleCraftItem should do a lot more than what it does, loads of the "can we craft" code should also probably be
						// shifted to the GameMode
						pMinecraft->localgameModes[iPad]->handleCraftItem(iRecipe,m_pPlayer);

						// play a sound
						//pMinecraft->soundEngine->playUI( L"random.pop", 1.0f, 1.0f);
						ui.PlayUISFX(eSFX_Craft);

						if(pTempItemInst->id != Item::firework_charge_Id && pTempItemInst->id != Item::fireworks_Id)
						{
							// and remove those resources from your inventory
							for(int i=0;i<pRecipeIngredientsRequired[iRecipe].iIngC;i++)
							{
								for(int j=0;j<pRecipeIngredientsRequired[iRecipe].iIngValA[i];j++)
								{
									shared_ptr<ItemInstance> ingItemInst = nullptr;
									// do we need to remove a specific aux value?
									if(pRecipeIngredientsRequired[iRecipe].iIngAuxValA[i]!=Recipes::ANY_AUX_VALUE)
									{
										ingItemInst = m_pPlayer->inventory->getResourceItem( pRecipeIngredientsRequired[iRecipe].iIngIDA[i],pRecipeIngredientsRequired[iRecipe].iIngAuxValA[i] );
										m_pPlayer->inventory->removeResource(pRecipeIngredientsRequired[iRecipe].iIngIDA[i],pRecipeIngredientsRequired[iRecipe].iIngAuxValA[i]);
									}
									else
									{
										ingItemInst = m_pPlayer->inventory->getResourceItem( pRecipeIngredientsRequired[iRecipe].iIngIDA[i] );
										m_pPlayer->inventory->removeResource(pRecipeIngredientsRequired[iRecipe].iIngIDA[i]);
									}

									// 4J Stu - Fix for #13097 - Bug: Milk Buckets are removed when crafting Cake
									if (ingItemInst != nullptr)
									{
										if (ingItemInst->getItem()->hasCraftingRemainingItem())
										{
											// replace item with remaining result
											m_pPlayer->inventory->add(std::make_shared<ItemInstance>(ingItemInst->getItem()->getCraftingRemainingItem()));
										}

									}
								}
							}

							// 4J Stu - Fix for #13119 - We should add the item after we remove the ingredients
							if(m_pPlayer->inventory->add(pTempItemInst)==false )
							{
								// no room in inventory, so throw it down
								m_pPlayer->drop(pTempItemInst);
							}

							//4J Gordon: Achievements
							switch(pTempItemInst->id )
							{
							case Tile::crafting_table_Id:		m_pPlayer->awardStat(GenericStats::buildWorkbench(),		GenericStats::param_buildWorkbench());		break;
							case Item::wooden_pickaxe_Id:		m_pPlayer->awardStat(GenericStats::buildPickaxe(),			GenericStats::param_buildPickaxe());		break;
							case Tile::furnace_Id:			m_pPlayer->awardStat(GenericStats::buildFurnace(),			GenericStats::param_buildFurnace());		break;
							//case Item::wooden_hoe_Id:			m_pPlayer->awardStat(GenericStats::buildHoe(),				GenericStats::param_buildHoe());			break;
							case Item::bread_Id:			m_pPlayer->awardStat(GenericStats::makeBread(),				GenericStats::param_makeBread());			break;
							case Item::cake_Id:				m_pPlayer->awardStat(GenericStats::bakeCake(),				GenericStats::param_bakeCake());			break;
							case Item::stone_pickaxe_Id:	m_pPlayer->awardStat(GenericStats::buildBetterPickaxe(),	GenericStats::param_buildBetterPickaxe());	break;
							//case Item::wooden_sword_Id:		m_pPlayer->awardStat(GenericStats::buildSword(),			GenericStats::param_buildSword());			break;
							case Tile::dispenser_Id:		m_pPlayer->awardStat(GenericStats::dispenseWithThis(),		GenericStats::param_dispenseWithThis());	break;
							case Tile::enchanting_table_Id:		m_pPlayer->awardStat(GenericStats::enchantments(),			GenericStats::param_enchantments());		break;
							case Tile::bookshelf_Id:		m_pPlayer->awardStat(GenericStats::bookcase(),				GenericStats::param_bookcase());			break;
							}
							switch (pTempItemInst->getItem()->getBaseItemType()) {
							case Item::eBaseItemType_hoe:		m_pPlayer->awardStat(GenericStats::buildHoe(), GenericStats::param_buildHoe());			break;
							case Item::eBaseItemType_sword:		m_pPlayer->awardStat(GenericStats::buildSword(), GenericStats::param_buildSword());		break;
							}

							// We've used some ingredients from our inventory, so update the recipes we can make
							CheckRecipesAvailable();
							// don't reset the vertical slots - we want to stay where we are
							UpdateVerticalSlots();
							UpdateHighlight();
						}
					}
					else
					{
						//pMinecraft->soundEngine->playUI( L"btn.back", 1.0f, 1.0f);
						ui.PlayUISFX(eSFX_CraftFail);
					}
				}
			}
		}
		break;
	case ACTION_MENU_PAGEUP:
		if (m_iGroupVIndex == 0) {
			m_iGroupVIndex = (m_iContainerType == RECIPE_TYPE_3x3 ? m_iMaxGroupV3x3 - 1 : m_iMaxGroupV2x2);
		} else {
			// Already in Banner: step the V index back down to Workbench.
			// (Was m_iGroupHIndex--: a copy-paste of the H-switch that
			// silently moved H instead of leaving Banner.)
			m_iGroupVIndex--;
			if (m_iGroupVIndex < 0) m_iGroupVIndex = 0;
		}

		m_iGroupHIndex = 0; // Banner row has 6 tabs (0-5); the Workbench row's last tab (Decoration, 6) is out of range
		setVGroupHighlight(m_iGroupVIndex);

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
		}
		else {
			iVSlotIndexA[0] = CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
			iVSlotIndexA[1] = 0;
			iVSlotIndexA[2] = 1;
		}
		ui.PlayUISFX(eSFX_Focus);
		UpdateVerticalSlots();
		UpdateHighlight();
		setGroupText(GetGroupNameText(m_pGroupA[m_iGroupVIndex][m_iGroupHIndex]));
		UpdateTooltips();
		break;
	case ACTION_MENU_PAGEDOWN:
		m_iGroupVIndex++;
		if (m_iGroupVIndex >= (m_iContainerType == RECIPE_TYPE_3x3 ? m_iMaxGroupV3x3 : m_iMaxGroupV2x2)) {
			m_iGroupVIndex = 0;
		}

		m_iGroupHIndex = 0; // Banner row has 6 tabs (0-5); the Workbench row's last tab (Decoration, 6) is out of range
		setVGroupHighlight(m_iGroupVIndex);

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
		}
		else {
			iVSlotIndexA[0] = CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
			iVSlotIndexA[1] = 0;
			iVSlotIndexA[2] = 1;
		}
		ui.PlayUISFX(eSFX_Focus);
		UpdateVerticalSlots();
		UpdateHighlight();
		setGroupText(GetGroupNameText(m_pGroupA[m_iGroupVIndex][m_iGroupHIndex]));
		UpdateTooltips();
		break;
	case ACTION_MENU_LEFT_SCROLL:
		if (m_iGroupVIndex == eGroupVType_Workbench) {
			if (m_iGroupHIndex == 0) {
				m_iGroupHIndex = ((m_iContainerType == RECIPE_TYPE_3x3 ? m_iMaxGroupH3x3 : m_iMaxGroupH2x2) - 1);
			} else {
				m_iGroupHIndex--;
			}
		} else if (m_iGroupVIndex == eGroupVType_Banner) {
			if (m_selectedBannerEdit != nullptr) {
				if (m_iGroupHIndex == 1) {
					m_iGroupHIndex = m_iMaxBannerGroupH3x3 - 1;
				} else {
					m_iGroupHIndex--;
				}
			}
		}
		
		// turn on the new group
		setHGroupHighlight(m_iGroupHIndex);

		m_iCurrentSlotHIndex=0;
		m_iCurrentSlotVIndex=1;
		UpdateCraftingPanels();
		CheckRecipesAvailable();
		// reset the vertical slots
		if (m_iGroupVIndex == eGroupVType_Workbench) {
			iVSlotIndexA[0] = CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
			iVSlotIndexA[1] = 0;
			iVSlotIndexA[2] = 1;
		} else if (m_iGroupVIndex == eGroupVType_Banner) {
			int patternId = m_iGroupHIndex > 0 ? m_aBannerCraftPatterns[m_iGroupHIndex - 1][m_iCurrentSlotHIndex] : -1;
			resetBannerPatternColorOptions(iVSlotIndexA, patternId);
		}
		ui.PlayUISFX(eSFX_Focus);
		UpdateVerticalSlots();
		UpdateHighlight();
		setGroupText(GetGroupNameText(m_pGroupA[m_iGroupVIndex][m_iGroupHIndex]));

		break;
	case ACTION_MENU_RIGHT_SCROLL:
		m_iGroupHIndex++;
		if (m_iGroupVIndex == eGroupVType_Workbench) {
			if (m_iGroupHIndex >= (m_iContainerType == RECIPE_TYPE_3x3 ? m_iMaxGroupH3x3 : m_iMaxGroupH2x2)) {
				m_iGroupHIndex = 0;
			}
		} else if (m_iGroupVIndex == eGroupVType_Banner) {
			if (m_selectedBannerEdit != nullptr) {
				if (m_iGroupHIndex >= m_iMaxBannerGroupH3x3) {
					m_iGroupHIndex = 1;
                }
			} else {
				m_iGroupHIndex = 0;
			}
		}
		// turn on the new group
		setHGroupHighlight(m_iGroupHIndex);


		m_iCurrentSlotHIndex=0;
		m_iCurrentSlotVIndex=1;
		CheckRecipesAvailable();
		// reset the vertical slots
		if (m_iGroupVIndex == eGroupVType_Workbench) {
			iVSlotIndexA[0] = CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
			iVSlotIndexA[1] = 0;
			iVSlotIndexA[2] = 1;
		} else if (m_iGroupVIndex == eGroupVType_Banner) {
			int patternId = m_iGroupHIndex > 0 ? m_aBannerCraftPatterns[m_iGroupHIndex - 1][m_iCurrentSlotHIndex] : -1;
			resetBannerPatternColorOptions(iVSlotIndexA, patternId);
		}
		ui.PlayUISFX(eSFX_Focus);
		UpdateVerticalSlots();
		UpdateHighlight();
		setGroupText(GetGroupNameText(m_pGroupA[m_iGroupVIndex][m_iGroupHIndex]));
		break;
	}

	// 4J-Tomk - check if we've only got one vertical scroll slot (480, splits & Vita)
	bool bNoScrollSlots = false;
	if(m_bSplitscreen ||(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen()))
	{
		bNoScrollSlots = true;
	}
#ifdef __PSVITA__
	bNoScrollSlots = true;
#endif

	// 4J Stu - We did used to swap the thumsticks based on Southpaw in this scene, but ONLY in this scene
	switch(iAction)
	{
	case ACTION_MENU_OTHER_STICK_UP:
		scrollDescriptionUp();
		break;
	case ACTION_MENU_OTHER_STICK_DOWN:
		scrollDescriptionDown();
		break;
	case ACTION_MENU_RIGHT:
		{
			int iOldHSlot=m_iCurrentSlotHIndex;

			m_iCurrentSlotHIndex++;
			if(m_iCurrentSlotHIndex>=m_iCraftablesMaxHSlotC) m_iCurrentSlotHIndex=0;
			m_iCurrentSlotVIndex=1;
			// clear the indices
			if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex > 0) {
				int patternId = m_aBannerCraftPatterns[m_iGroupHIndex - 1][m_iCurrentSlotHIndex];
				resetBannerPatternColorOptions(iVSlotIndexA, patternId);
			} else {
				iVSlotIndexA[0] = CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
				iVSlotIndexA[1] = 0;
				iVSlotIndexA[2] = 1;
			}

			UpdateVerticalSlots();
			UpdateHighlight();
			// re-enable the old hslot
			//if(CanBeMadeA[iOldHSlot].iCount>0)
			//{
				setShowCraftHSlot(iOldHSlot,true);
			//}
			ui.PlayUISFX(eSFX_Focus);
			bHandled = true;
		}
		break;
	case ACTION_MENU_LEFT:
		{
			if(m_iCraftablesMaxHSlotC!=0)
			{
				int iOldHSlot=m_iCurrentSlotHIndex;
				if(m_iCurrentSlotHIndex==0) m_iCurrentSlotHIndex=m_iCraftablesMaxHSlotC-1;
				else m_iCurrentSlotHIndex--;
				m_iCurrentSlotVIndex=1;
				// clear the indices
				if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex > 0) {
					int patternId = m_aBannerCraftPatterns[m_iGroupHIndex - 1][m_iCurrentSlotHIndex];
					resetBannerPatternColorOptions(iVSlotIndexA, patternId);
				} else {
					iVSlotIndexA[0] = CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
					iVSlotIndexA[1] = 0;
					iVSlotIndexA[2] = 1;
				}

				UpdateVerticalSlots();
				UpdateHighlight();
				// re-enable the old hslot
				//if(CanBeMadeA[iOldHSlot].iCount>0)
				//{
					setShowCraftHSlot(iOldHSlot,true);
				//}
				ui.PlayUISFX(eSFX_Focus);
			}
			bHandled = true;
		}
		break;
	case ACTION_MENU_UP:
		{
			if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex == 0 && m_iCurrentSlotHIndex == 1) {
				if (CurrentInventoryBanners.size() > 1) {
					const int bannerCount = static_cast<int>(CurrentInventoryBanners.size());

					if (bNoScrollSlots) {
						iVSlotIndexA[1] = (iVSlotIndexA[1] + bannerCount - 1) % bannerCount;
						ui.PlayUISFX(eSFX_Focus);
						UpdateVerticalSlots();
						UpdateHighlight();
					} else {
						if (bannerCount > 2) {
							if (m_iCurrentSlotVIndex > 1) {
								m_iCurrentSlotVIndex--;
							} else {
								iVSlotIndexA[2] = iVSlotIndexA[1];
								iVSlotIndexA[1] = iVSlotIndexA[0];
								iVSlotIndexA[0] = (iVSlotIndexA[0] + bannerCount - 1) % bannerCount;

								m_iCurrentSlotVIndex = bannerCount;
							}

							ui.PlayUISFX(eSFX_Focus);
						} else {
							m_iCurrentSlotVIndex = (m_iCurrentSlotVIndex == 1) ? 2 : 1;
							ui.PlayUISFX(eSFX_Focus);
						}

						UpdateVerticalSlots();
						UpdateHighlight();
					}
				}
			} else if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex > 0) {
				int patternId = m_aBannerCraftPatterns[m_iGroupHIndex - 1][m_iCurrentSlotHIndex];
				if (bNoScrollSlots) {
					iVSlotIndexA[1] = stepBannerPatternColorOption(patternId, iVSlotIndexA[1], -1);
					ui.PlayUISFX(eSFX_Focus);
					UpdateVerticalSlots();
					UpdateHighlight();
				} else {
					if (m_iCurrentSlotVIndex > 1) {
						m_iCurrentSlotVIndex--;
					}
					else {
						iVSlotIndexA[1] = stepBannerPatternColorOption(patternId, iVSlotIndexA[1], -1);
						iVSlotIndexA[0] = stepBannerPatternColorOption(patternId, iVSlotIndexA[1], -1);
						iVSlotIndexA[2] = stepBannerPatternColorOption(patternId, iVSlotIndexA[1], 1);
					}

					ui.PlayUISFX(eSFX_Focus);

					UpdateVerticalSlots();
					UpdateHighlight();
				}
			} else {
				if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1) {
					if(bNoScrollSlots) {
						if(iVSlotIndexA[1]==0) {
							iVSlotIndexA[1]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
						} else {
							iVSlotIndexA[1]--;
						}
						ui.PlayUISFX(eSFX_Focus);
						UpdateVerticalSlots();
						UpdateHighlight();
					} else {
						if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>2) {
							{
								if(m_iCurrentSlotVIndex!=0) {
									// just move the highlight
									m_iCurrentSlotVIndex--;
									ui.PlayUISFX(eSFX_Focus);
								} else {
									//move the slots
									iVSlotIndexA[2]=iVSlotIndexA[1];
									iVSlotIndexA[1]=iVSlotIndexA[0];
									// on 0 and went up, so cycle the values
									if(iVSlotIndexA[0]==0) {
										iVSlotIndexA[0]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
									} else {
										iVSlotIndexA[0]--;
									}
									ui.PlayUISFX(eSFX_Focus);
								}
							}
						} else {
							if(m_iCurrentSlotVIndex!=1) {
								// just move the highlight
								m_iCurrentSlotVIndex--;
								ui.PlayUISFX(eSFX_Focus);
							}
						}
						UpdateVerticalSlots();
						UpdateHighlight();
					}
				}
			}
		}
		break;
	case ACTION_MENU_DOWN:
		{
			if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex == 0 && m_iCurrentSlotHIndex == 1) {
				if(CurrentInventoryBanners.size()>1) {
					if (bNoScrollSlots) {
						iVSlotIndexA[1] = (iVSlotIndexA[1] + 1) % CurrentInventoryBanners.size();
						ui.PlayUISFX(eSFX_Focus);
						UpdateVerticalSlots();
						UpdateHighlight();
					} else {
						const int bannerCount = static_cast<int>(CurrentInventoryBanners.size());

						if (bannerCount > 2) {
							if (m_iCurrentSlotVIndex < 2) {
								m_iCurrentSlotVIndex++;
							} else {
								iVSlotIndexA[0] = iVSlotIndexA[1];
								iVSlotIndexA[1] = iVSlotIndexA[2];
								iVSlotIndexA[2] = (iVSlotIndexA[2] + 1) % bannerCount;

								m_iCurrentSlotVIndex = 1;
							}

							ui.PlayUISFX(eSFX_Focus);
						} else {
							m_iCurrentSlotVIndex = (m_iCurrentSlotVIndex == 1) ? 2 : 1;
							ui.PlayUISFX(eSFX_Focus);
						}

						UpdateVerticalSlots();
						UpdateHighlight();
					}
				}
			} else if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex > 0) {
				int patternId = m_aBannerCraftPatterns[m_iGroupHIndex - 1][m_iCurrentSlotHIndex];
				if (bNoScrollSlots) {
					iVSlotIndexA[1] = stepBannerPatternColorOption(patternId, iVSlotIndexA[1], 1);
					ui.PlayUISFX(eSFX_Focus);
					UpdateVerticalSlots();
					UpdateHighlight();
				} else {
					if (m_iCurrentSlotVIndex < 1) {
						m_iCurrentSlotVIndex++;
					}
					else {
						iVSlotIndexA[1] = stepBannerPatternColorOption(patternId, iVSlotIndexA[1], 1);
						iVSlotIndexA[0] = stepBannerPatternColorOption(patternId, iVSlotIndexA[1], -1);
						iVSlotIndexA[2] = stepBannerPatternColorOption(patternId, iVSlotIndexA[1], 1);
					}

					ui.PlayUISFX(eSFX_Focus);

					UpdateVerticalSlots();
					UpdateHighlight();
				}
			} else {
				if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1) {
					if(bNoScrollSlots) {
						if(iVSlotIndexA[1]==(CanBeMadeA[m_iCurrentSlotHIndex].iCount-1)) {
							iVSlotIndexA[1]=0;
						} else {
							iVSlotIndexA[1]++;
						}
						ui.PlayUISFX(eSFX_Focus);
						UpdateVerticalSlots();
						UpdateHighlight();
					} else {
						if (CanBeMadeA[m_iCurrentSlotHIndex].iCount > 2) {
							if (m_iCurrentSlotVIndex != 2) {
								m_iCurrentSlotVIndex++;
								ui.PlayUISFX(eSFX_Focus);
							} else {
								iVSlotIndexA[0] = iVSlotIndexA[1];
								iVSlotIndexA[1] = iVSlotIndexA[2];
								if (iVSlotIndexA[m_iCurrentSlotVIndex] == (CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1)) {
									iVSlotIndexA[2] = 0;
								} else {
									iVSlotIndexA[2]++;
								}
								ui.PlayUISFX(eSFX_Focus);
							}
						} else {
							if (m_iCurrentSlotVIndex != (CanBeMadeA[m_iCurrentSlotHIndex].iCount)) {
								m_iCurrentSlotVIndex++;
								ui.PlayUISFX(eSFX_Focus);
							}
						}
						UpdateVerticalSlots();
						UpdateHighlight();
					}
				}
			}
		}
		break;
	}

	return bHandled;
}

bool IUIScene_CraftingMenu::hasItem(int id, int requiredCount, int aux)
{
	if (!m_pPlayer || !m_pPlayer->inventory) return false;

	int count = 0;

	for (unsigned int i = 0; i < m_pPlayer->inventory->items.length; ++i) {
		const auto& item = m_pPlayer->inventory->items[i];

		if (item == nullptr || item->id != id) continue;

		if (aux != -1 && item->getAuxValue() != aux) continue;

		count += item->count;

		if (count >= requiredCount) return true;
	}

	return false;
}

void IUIScene_CraftingMenu::rebuildPreviewItem() {
	m_bannerPreviewHolder = ItemInstance::clone(m_selectedBannerEdit);

	for (PreviewBannerData& data : m_bannerPreviewData) {
		const BannerPatternDef *def = BannerPatterns::get(data.id);
		if (def != nullptr) BannerItem::addPattern(m_bannerPreviewHolder, def->code, data.color < 0 ? DyePowderItem::BLACK : data.color);
	}
}

int IUIScene_CraftingMenu::hasItemCounts(int id, int requiredCount, int aux)
{
	if (!m_pPlayer || !m_pPlayer->inventory) return 0;

	int count = 0;

	for (unsigned int i = 0; i < m_pPlayer->inventory->items.length; ++i) {
		const auto& item = m_pPlayer->inventory->items[i];

		if (item == nullptr || item->id != id) continue;

		if (aux != -1 && item->getAuxValue() != aux) continue;

		count += item->count;

		if (requiredCount != -1 && count >= requiredCount) return count;
	}

	return count;
}

bool IUIScene_CraftingMenu::canCraftPattern(const BannerPatternDef& pattern, char color) {

	if (pattern.itemId != 0) {
		return hasItem(pattern.itemId, 1, pattern.itemAux) && (color < 0 || hasItem(Item::dye_Id, 1, color));
	} else {
		int requiredDye = 0;
		for (int row = 0; row < 3; ++row) {
			for (int col = 0; col < 3; ++col) {
				if (pattern.rows[row][col] != ' ') ++requiredDye;
			}
		}

		if (hasItem(Item::dye_Id, requiredDye, color)) return true;
	}

	return false;
}

bool IUIScene_CraftingMenu::hasResourcesForBannerPatterns(const vector<PreviewBannerData>& patterns)
{
	if (m_selectedBannerEdit == nullptr || !hasItem(Tile::standing_banner_Id)) return false;

	struct RequiredSpecialItem
	{
		int itemId;
		int aux;
		int count;
	};

	int requiredDyes[16] = {};
	vector<RequiredSpecialItem> requiredSpecialItems;
	for (const PreviewBannerData& data : patterns)
	{
		if (data.id < 0 || data.id >= BANNER_PATTERN_COUNT) return false;

		const BannerPatternDef *pattern = BannerPatterns::get(data.id);
		if (pattern == nullptr) return false;
		if (pattern->itemId != 0)
		{
			bool found = false;
			for (RequiredSpecialItem& required : requiredSpecialItems)
			{
				if (required.itemId == pattern->itemId && required.aux == pattern->itemAux)
				{
					++required.count;
					found = true;
					break;
				}
			}
			if (!found) requiredSpecialItems.push_back({ pattern->itemId, pattern->itemAux, 1 });

			if (data.color >= 0)
			{
				if (data.color >= 16) return false;
				++requiredDyes[data.color];
			}
		}
		else
		{
			if (data.color < 0 || data.color >= 16 || pattern->rows[0] == nullptr) return false;
			for (int row = 0; row < 3; ++row)
			{
				for (int col = 0; col < 3; ++col)
				{
					if (pattern->rows[row][col] != ' ') ++requiredDyes[data.color];
				}
			}
		}
	}

	for (const RequiredSpecialItem& required : requiredSpecialItems)
	{
		if (!hasItem(required.itemId, required.count, required.aux)) return false;
	}
	for (int color = 0; color < 16; ++color)
	{
		if (requiredDyes[color] > 0 && !hasItem(Item::dye_Id, requiredDyes[color], color)) return false;
	}

	return true;
}

static inline bool DoesPatternHaveVertical(int id) {
	return true;
}

//////////////////////////////////////////////////////////////////////////
//
//	CheckRecipesAvailable
//
//////////////////////////////////////////////////////////////////////////
void IUIScene_CraftingMenu::CheckRecipesAvailable()
{
	int iHSlotBrushControl=0;

	// clear the current list
	memset(CanBeMadeA,0,sizeof(CANBEMADE)*m_iCraftablesMaxHSlotC);
	CurrentInventoryBanners.clear();

	hideAllHSlots();

	if(m_pPlayer && m_pPlayer->inventory)
	{
		for (unsigned int k = 0; k < m_pPlayer->inventory->items.length; k++) {
			if (m_pPlayer->inventory->items[k] != nullptr) {
				if (m_pPlayer->inventory->items[k]->id == Tile::standing_banner_Id) {
					if ((int)BannerItem::getPatterns(m_pPlayer->inventory->items[k]).size() >= 6) continue;

					CurrentInventoryBanners.push_back(m_pPlayer->inventory->items[k]);
				}
			}
		}

		if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex > 0) {
			for (int i = 0; i < 12; i++) {
				int patternId = m_aBannerCraftPatterns[m_iGroupHIndex-1][i];
				if (patternId == -1) continue;

				const BannerPatternDef *pattern = BannerPatterns::get(patternId);
				if (pattern == nullptr) continue;
				int colorIndex = pattern->itemId != 0 ? -1 : DyePowderItem::BLACK;
				bool canCraft = canCraftPattern(*pattern, colorIndex) || (app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad()) & (1L << eDebugSetting_CraftAnything));
				setCraftHSlotItem(getPad(), i, makePatternPreviewBanner(pattern->code, colorIndex), canCraft ? 31 : 16);
			}
			return;
		}

		// dump out the inventory
		/*		for (unsigned int k = 0; k < m_pPlayer->inventory->items.length; k++)
		{
		if (m_pPlayer->inventory->items[k] != nullptr)
		{
		wstring itemstring=m_pPlayer->inventory->items[k]->toString();

		//printf("--- Player has ");
		OutputDebugStringW(itemstring.c_str());
		//printf(" with Aux val = %d, base type = %d, Material = %d\n",m_pPlayer->inventory->items[k]->getAuxValue(),m_pPlayer->inventory->items[k]->getItem()->getBaseItemType(),m_pPlayer->inventory->items[k]->getItem()->getMaterial());
		}
		}
		*/
		RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
		Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();
		int iRecipeC=static_cast<int>(recipes->size());
        auto itRecipe = recipes->begin();

        // dump out the recipe products

		// 		for (int i = 0; i < iRecipeC; i++)
		// 		{
		// 			shared_ptr<ItemInstance> pTempItemInst=pRecipeIngredientsRequired[i].pRecipy->assemble(nullptr);
		// 			if (pTempItemInst != nullptr)
		// 			{
		// 				wstring itemstring=pTempItemInst->toString();
		//
		// 				printf("Recipe [%d] = ",i);
		// 				OutputDebugStringW(itemstring.c_str());
		// 				if(pTempItemInst->id!=0)
		// 				{
		// 					if(pTempItemInst->id<256)
		// 					{
		// 						Tile *pTile=Tile::tiles[pTempItemInst->id];
		// 						printf("[TILE] ID\t%d\tAux val\t%d\tBase type\t%d\tMaterial\t%d\t Count=%d\n",pTempItemInst->id, pTempItemInst->getAuxValue(),pTile->getBaseItemType(),pTile->getMaterial(),pTempItemInst->GetCount());
		// 					}
		// 					else
		// 					{
		// 						printf("ID\t%d\tAux val\t%d\tBase type\t%d\tMaterial\t%d Count=%d\n",pTempItemInst->id, pTempItemInst->getAuxValue(),pTempItemInst->getItem()->getBaseItemType(),pTempItemInst->getItem()->getMaterial(),pTempItemInst->GetCount());
		// 					}
		//
		// 				}
		// 			}
		// 		}

		for(int i=0;i<iRecipeC;i++)
		{

			Recipy *r = *itRecipe;

			// If this recipe isn't in the current grouptype, skip it
			if(r->getGroup()!=m_pGroupA[m_iGroupVIndex][m_iGroupHIndex])
			{
				itRecipe++;
				pRecipeIngredientsRequired[i].bCanMake[getPad()]=false;
				continue;
			}
			// neoLegacy - Complex Table recipes are only listed at a Complex Table, and a
			// Complex Table lists nothing else. Everything else just skips them.
			if(Recipes::isComplexOnly(r) != m_bComplexTable)
			{
				itRecipe++;
				pRecipeIngredientsRequired[i].bCanMake[getPad()]=false;
				continue;
			}
			// if we are in the inventory menu, then we have 2x2 crafting available only
			if((m_iContainerType==RECIPE_TYPE_2x2) && (pRecipeIngredientsRequired[i].iType==RECIPE_TYPE_3x3))
			{
				// need a crafting table for this recipe
				itRecipe++;
				pRecipeIngredientsRequired[i].bCanMake[getPad()]=false;
				continue;
			}
			// clear the mask showing which ingredients are missing
			pRecipeIngredientsRequired[i].usBitmaskMissingGridIngredients[getPad()]=0;

			//bool bCanMakeRecipe=true;
			bool *bFoundA= new bool [pRecipeIngredientsRequired[i].iIngC];
			for(int j=0;j<pRecipeIngredientsRequired[i].iIngC;j++)
			{
				bFoundA[j]=false;
				int iTotalCount=0;

				// Does the player have this ingredient?
				for (unsigned int k = 0; k < m_pPlayer->inventory->items.length; k++)
				{
					if (m_pPlayer->inventory->items[k] != nullptr)
					{
						// do they have the ingredient, and the aux value matches, and enough off it?
						if((m_pPlayer->inventory->items[k]->id == pRecipeIngredientsRequired[i].iIngIDA[j]) &&
							// check if the ingredient required doesn't care about the aux value, or if it does, does the inventory item aux match it
								((pRecipeIngredientsRequired[i].iIngAuxValA[j]==Recipes::ANY_AUX_VALUE) || (pRecipeIngredientsRequired[i].iIngAuxValA[j]==m_pPlayer->inventory->items[k]->getAuxValue()))
								)
						{
							// do they have enough? We need to check the whole inventory, since they may have enough in different slots (milk isn't milkx3, but milk,milk,milk)
							if(m_pPlayer->inventory->items[k]->GetCount()>=pRecipeIngredientsRequired[i].iIngValA[j])
							{
								// they have enough with one slot
								bFoundA[j]=true;
							}
							else
							{
								// look at the combined value from the whole inventory

								for(unsigned int l=0;l<m_pPlayer->inventory->items.length;l++)
								{
									if (m_pPlayer->inventory->items[l] != nullptr)
									{
										if(
											(m_pPlayer->inventory->items[l]->id == pRecipeIngredientsRequired[i].iIngIDA[j]) &&
											( (pRecipeIngredientsRequired[i].iIngAuxValA[j]==Recipes::ANY_AUX_VALUE) || (pRecipeIngredientsRequired[i].iIngAuxValA[j]==m_pPlayer->inventory->items[l]->getAuxValue() ))
											)
										{
											iTotalCount+=m_pPlayer->inventory->items[l]->GetCount();
										}
									}
								}

								if(iTotalCount>=pRecipeIngredientsRequired[i].iIngValA[j])
								{
									bFoundA[j]=true;
								}
							}

							// 4J Stu - TU-1 hotfix
							// Fix for #13143 - Players are able to craft items they do not have enough ingredients for if they store the ingredients in multiple, smaller stacks
							break;
						}
					}
				}
				// if bFoundA[j] is false, then we didn't have enough of the ingredient required by the recipe, so mark the grid items we're short of
				if(bFoundA[j]==false)
				{
					int iMissing = pRecipeIngredientsRequired[i].iIngValA[j]-iTotalCount;
					int iGridIndex=0;
					while(iMissing!=0 && iGridIndex<9)
					{
						// need to check if there is an aux val and match that
						if(((pRecipeIngredientsRequired[i].uiGridA[iGridIndex]&0x00FFFFFF)==pRecipeIngredientsRequired[i].iIngIDA[j]) &&
							((pRecipeIngredientsRequired[i].iIngAuxValA[j]==Recipes::ANY_AUX_VALUE) ||((pRecipeIngredientsRequired[i].iIngAuxValA[j]&0xFF)== ((pRecipeIngredientsRequired[i].uiGridA[iGridIndex]&0xFF000000)>>24))) )
						{
							// this grid entry is the ingredient we don't have enough of
							pRecipeIngredientsRequired[i].usBitmaskMissingGridIngredients[getPad()]|=1<<iGridIndex;
							iMissing--;
						}
						iGridIndex++;
					}
				}
			}

			// so can we make it?
			bool bCanMake=true;
			for(int j=0;j<pRecipeIngredientsRequired[i].iIngC;j++)
			{
				if(bFoundA[j]==false)
				{
					bCanMake=false;
					break;
				}
			}

			pRecipeIngredientsRequired[i].bCanMake[getPad()]=bCanMake;

			// Add the recipe to the CanBeMade list of lists
			if(iHSlotBrushControl<=m_iCraftablesMaxHSlotC)
			{
				bool bFound=false;
				shared_ptr<ItemInstance> pTempItemInst=pRecipeIngredientsRequired[i].pRecipy->assemble(nullptr);
				//int iIcon=pTempItemInst->getItem()->getIcon(pTempItemInst->getAuxValue());
				int iID=pTempItemInst->getItem()->id;
				int iBaseType;

				if(Tile::isTileItemId(iID)) // is it a tile?
				{
					iBaseType=Tile::tiles[iID]->getBaseItemType();
				}
				else
				{
					iBaseType=pTempItemInst->getItem()->getBaseItemType();
				}

				// ignore for the misc base type - these have not been placed in a base type group
				if(iBaseType!=Item::eBaseItemType_undefined)
				{
					for(int k=0;k<iHSlotBrushControl;k++)
					{
						// if the item base type is the same as one already in, then add it to that list
						if(CanBeMadeA[k].iItemBaseType==iBaseType)
						{
							// base item type already in our list
							bFound=true;
							if(CanBeMadeA[k].iCount<m_iMaxVSlotC)
							{
								CanBeMadeA[k].iRecipeA[CanBeMadeA[k].iCount++]=i;
							}
							else
							{
								app.DebugPrintf("Need more V slots\n");
							}
							break;
						}
					}
				}

				if(!bFound)
				{
					if(iHSlotBrushControl<m_iCraftablesMaxHSlotC)
					{
						// add to the list
						CanBeMadeA[iHSlotBrushControl].iItemBaseType=iBaseType;
						CanBeMadeA[iHSlotBrushControl].iRecipeA[CanBeMadeA[iHSlotBrushControl].iCount++]=i;
						iHSlotBrushControl++;
					}
					else
					{
						app.DebugPrintf("Need more H slots - ");
#ifndef _CONTENT_PACKAGE
						OutputDebugStringW(app.GetString(pTempItemInst->getDescriptionId()));
#endif
						app.DebugPrintf("\n");

					}
				}
			}
			else
			{
				app.DebugPrintf("Need more HSlots\n");
			}

			delete [] bFoundA;
			itRecipe++;
		}
	}

	// run through the canbemade list and update the icons displayed
	int iIndex=0;
	//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
	Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();

	while((iIndex<m_iCraftablesMaxHSlotC) && CanBeMadeA[iIndex].iCount!=0)
	{
		shared_ptr<ItemInstance> pTempItemInst=pRecipeIngredientsRequired[CanBeMadeA[iIndex].iRecipeA[0]].pRecipy->assemble(nullptr);
		assert(pTempItemInst->id!=0);
		unsigned int uiAlpha;

		if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_CraftAnything))
		{
			uiAlpha = 31;
		}
		else
		{
			if(pRecipeIngredientsRequired[CanBeMadeA[iIndex].iRecipeA[0]].bCanMake[getPad()])
			{
				uiAlpha = 31;
			}
			else
			{
				uiAlpha= 16;
			}
		}

		// 4J Stu - For clocks and compasses we set the aux value to a special one that signals we should use a default texture
		// rather than the dynamic one for the player
		if( pTempItemInst->id == Item::clock_Id || pTempItemInst->id == Item::compass_Id )
		{
			pTempItemInst->setAuxValue( 255 );
		}
		setCraftHSlotItem(getPad(),iIndex,pTempItemInst,uiAlpha);

		iIndex++;
	}

	// 4J-PB - Removed - UpdateTooltips will do this
	// Update tooltips
	/*if(CanBeMadeA[m_iCurrentSlotHIndex].iCount!=0)
	{
		ui.ShowTooltip( getPad(), eToolTipButtonA, true );
		// 4J-PB - not implemented !
		//ui.EnableTooltip( getPad(), eToolTipButtonA, true );
	}
	else
	{
		ui.ShowTooltip( getPad(), eToolTipButtonA, false );
	}*/
}

//////////////////////////////////////////////////////////////////////////
//
//	UpdateHighlight
//
//////////////////////////////////////////////////////////////////////////
void IUIScene_CraftingMenu::UpdateHighlight()
{
	updateHighlightAndScrollPositions();

	if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex == 0 && m_iCurrentSlotHIndex == 1) {
		if (CurrentInventoryBanners.size() > 0) {
			wstring selectedBannerName = CurrentInventoryBanners[m_iCurrentSlotVIndex - 1]->getHoverName();
			setItemText(selectedBannerName.c_str());
			setDescriptionText(selectedBannerName.c_str());
		}
	} else if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex > 0) {
		setItemText(L"");
		setDescriptionText(L"");
	}else {
		bool bCanBeMade = CanBeMadeA[m_iCurrentSlotHIndex].iCount != 0;
		if (bCanBeMade)
		{
			//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
			Recipy::INGREDIENTS_REQUIRED* pRecipeIngredientsRequired = Recipes::getInstance()->getRecipeIngredientsArray();
			int iSlot;
			if (CanBeMadeA[m_iCurrentSlotHIndex].iCount > 1)
			{
				iSlot = iVSlotIndexA[m_iCurrentSlotVIndex];
			}
			else
			{
				iSlot = 0;
			}
			shared_ptr<ItemInstance> pTempItemInstAdditional = pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot]].pRecipy->assemble(nullptr);

			// special case for the torch coal/charcoal
			int id = pTempItemInstAdditional->getDescriptionId();
			wstring itemstring;

			switch (id)
			{
			case IDS_TILE_TORCH:
			{
				if (pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot]].iIngAuxValA[0] == 1)
				{
					itemstring = app.GetString(IDS_TILE_TORCHCHARCOAL);
				}
				else
				{
					itemstring = app.GetString(IDS_TILE_TORCHCOAL);
				}
			}
			break;
			case IDS_ITEM_FIREBALL:
			{
				if (pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot]].iIngAuxValA[2] == 1)
				{
					itemstring = app.GetString(IDS_ITEM_FIREBALLCHARCOAL);
				}
				else
				{
					itemstring = app.GetString(IDS_ITEM_FIREBALLCOAL);
				}
			}
			break;
			default:
				if (pTempItemInstAdditional->hasCustomHoverName()) {
					itemstring = pTempItemInstAdditional->getHoverName();
				}
				else {
					itemstring = app.GetString(id);
				}
				break;
			}

			setItemText(itemstring.c_str());
		}
		else
		{
			setItemText(L"");
		}
		UpdateDescriptionText(bCanBeMade);
	}
	DisplayIngredients();

	UpdateMultiPanel();

	UpdateTooltips();
}

//////////////////////////////////////////////////////////////////////////
//
//	UpdateVerticalSlots
//
//////////////////////////////////////////////////////////////////////////
void IUIScene_CraftingMenu::UpdateVerticalSlots()
{
	//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
	Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();

	// update the vertical items for the current horizontal slot
	hideAllVSlots();

	if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex > 0) {
		int patternId = m_aBannerCraftPatterns[m_iGroupHIndex - 1][m_iCurrentSlotHIndex];
		if (patternId != -1 && DoesPatternHaveVertical(patternId)) {
			setShowCraftHSlot(m_iCurrentSlotHIndex, false);
			const BannerPatternDef *pattern = BannerPatterns::get(patternId);
			if (pattern == nullptr) return;

			bool bNoScrollSlots = false;
			if (m_bSplitscreen || (!RenderManager.IsHiDef() && !RenderManager.IsWidescreen()))
			{
				bNoScrollSlots = true;
			}

			for (int i = 0; i < 3; i++) {
				if (bNoScrollSlots) {
					if (i != 1) continue;
				}

				int colorIndex = iVSlotIndexA[i];

				bool canCraft = canCraftPattern(*pattern, colorIndex) || (app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad()) & (1L << eDebugSetting_CraftAnything));
				setCraftVSlotItem(getPad(), i, makePatternPreviewBanner(pattern->code, colorIndex), canCraft ? 31 : 16);
				updateVSlotPositions(3, i);
			}
		}
	} else {

		// could have either 1 or 2 vertical slots, above and below the horizontal slot
		if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
		{
			// turn off the horizontal one since we could be cycling through others
			setShowCraftHSlot(m_iCurrentSlotHIndex,false);
			int iSlots=(CanBeMadeA[m_iCurrentSlotHIndex].iCount>2)?3:2;

			// 4J-Tomk - check if we've only got one vertical scroll slot (480, splits & Vita)
			bool bNoScrollSlots = false;
			if(m_bSplitscreen ||(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen()))
			{
				bNoScrollSlots = true;
			}
	#ifdef __PSVITA__
			bNoScrollSlots = true;
	#endif

			for(int i=0;i<iSlots;i++)
			{
				// 4J this check determines if the crafting scene has only one vertical scroll slot
				if(bNoScrollSlots)
				{
					if(i!=1) continue;
				}
				shared_ptr<ItemInstance> pTempItemInstAdditional=pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iVSlotIndexA[i]]].pRecipy->assemble(nullptr);

				assert(pTempItemInstAdditional->id!=0);
				unsigned int uiAlpha;

				if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_CraftAnything))
				{
					uiAlpha = 31;
				}
				else
				{
					if(pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iVSlotIndexA[i]]].bCanMake[getPad()])
					{
						uiAlpha = 31;
					}
					else
					{
						uiAlpha= 16;
					}
				}

				// 4J Stu - For clocks and compasses we set the aux value to a special one that signals we should use a default texture
				// rather than the dynamic one for the player
				if( pTempItemInstAdditional->id == Item::clock_Id || pTempItemInstAdditional->id == Item::compass_Id )
				{
					pTempItemInstAdditional->setAuxValue( 255 );
				}

				setCraftVSlotItem(getPad(),i,pTempItemInstAdditional,uiAlpha);

				updateVSlotPositions(iSlots, i);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
//	DisplayIngredients
//
//////////////////////////////////////////////////////////////////////////
void IUIScene_CraftingMenu::DisplayIngredients()
{

	if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex == 0 && m_iCurrentSlotHIndex == 1) {
		hideAllIngredientsSlots();

		if (CurrentInventoryBanners.size() >= 1) {
			auto selectedBanner = CurrentInventoryBanners[m_iCurrentSlotVIndex - 1];

			setCraftingOutputSlotItem(getPad(), selectedBanner);
			setCraftingOutputSlotRedBox(false);

			// if it's a 2x2 , only clear the 4 m_pCraftingIngredientA slots
			int iIngredientsSlots = (m_iContainerType == RECIPE_TYPE_2x2) ? 4 : m_iIngredients3x3SlotC;
			for (int i = 0; i < iIngredientsSlots; i++) {
				setIngredientSlotRedBox(i, false);
				setIngredientSlotItem(getPad(), i, nullptr);
			}

			int centerSlot = (m_iContainerType == RECIPE_TYPE_2x2) ? 4 : 5;
			setIngredientSlotItem(getPad(), centerSlot-1, selectedBanner);
		}
	} else if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex > 0) {
		hideAllIngredientsSlots();

		int patternId = m_aBannerCraftPatterns[m_iGroupHIndex - 1][m_iCurrentSlotHIndex];
		if (patternId != -1) {
			const BannerPatternDef *pPattern = BannerPatterns::get(patternId);
			if (pPattern == nullptr) return; // stale H index / bad pattern id — ingredient slots already hidden above
			const BannerPatternDef &pattern = *pPattern;

			auto whiteBanner = std::make_shared<ItemInstance>(Tile::standing_banner_Id, 1, 15);

			if (pattern.itemId != 0) {
				const int bannerSlot = 0;
				const int specialItemSlot = 1;
				const int dyeSlot = 2;
				const int specialAux = pattern.itemAux == -1 ? 0 : pattern.itemAux;
				const int colorIndex = iVSlotIndexA[m_iCurrentSlotVIndex];
				auto specialItem = std::make_shared<ItemInstance>(pattern.itemId, 1, specialAux);

				for (int slot = 0; slot < 9; ++slot) {
					setIngredientSlotItem(getPad(), slot, nullptr);
					setIngredientSlotRedBox(slot, false);
				}
				setIngredientSlotItem(getPad(), bannerSlot, whiteBanner);
				setIngredientSlotItem(getPad(), specialItemSlot, specialItem);
				setIngredientSlotRedBox(bannerSlot, !hasItem(Tile::standing_banner_Id));
				setIngredientSlotRedBox(specialItemSlot, !hasItem(pattern.itemId, 1, pattern.itemAux));

				if (colorIndex >= 0) {
					auto dyeItem = std::make_shared<ItemInstance>(Item::dye_Id, 1, colorIndex);
					setIngredientSlotItem(getPad(), dyeSlot, dyeItem);

					int dyeAlreadyConsumed = 0;
					for (const PreviewBannerData& data : m_bannerPreviewData) {
						if (data.color != colorIndex) continue;
						const BannerPatternDef &pendingPattern = *BannerPatterns::get(data.id);
						if (pendingPattern.itemId != 0) {
							++dyeAlreadyConsumed;
						} else if (pendingPattern.rows[0] != nullptr) {
							for (int row = 0; row < 3; ++row) {
								for (int col = 0; col < 3; ++col) {
									if (pendingPattern.rows[row][col] != ' ') ++dyeAlreadyConsumed;
								}
							}
						}
					}

					bool hasDye = hasItemCounts(Item::dye_Id, -1, colorIndex) > dyeAlreadyConsumed;
					setIngredientSlotRedBox(dyeSlot, !hasDye && !(app.DebugSettingsOn() && (app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad()) & (1L << eDebugSetting_CraftAnything))));
				}

				setCraftingOutputSlotItem(getPad(), makePatternPreviewBanner(pattern.code, colorIndex));
				setCraftingOutputSlotRedBox(!((canCraftPattern(pattern, colorIndex)) || (app.DebugSettingsOn() && (app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad()) & (1L << eDebugSetting_CraftAnything)))));
			} else {
				int colorIndex = iVSlotIndexA[m_iCurrentSlotVIndex];
				if (colorIndex == -1) colorIndex = DyePowderItem::BLACK;

				auto dyeItem = std::make_shared<ItemInstance>(Item::dye_Id, 1, colorIndex);
				const int bannerPositions[] = { 4, 1, 7, 8, 6, 5, 3, 2, 0 };
				int bannerSlot = -1;

				for (int position : bannerPositions) {
					int row = position / 3;
					int col = position % 3;
					if (pattern.rows[row][col] == ' ') {
						bannerSlot = position;
						break;
					}
				}

				int dyeCount = hasItemCounts(dyeItem->id, -1, dyeItem->getAuxValue());
				int dyeAlreadyConsumed = 0;
				for (PreviewBannerData& data : m_bannerPreviewData) {
					if (data.color != colorIndex) continue;
					const BannerPatternDef &pendingPattern = *BannerPatterns::get(data.id);
					if (pendingPattern.rows[0] == nullptr) continue;
					for (int row = 0; row < 3; ++row) {
						for (int col = 0; col < 3; ++col) {
							if (pendingPattern.rows[row][col] != ' ') ++dyeAlreadyConsumed;
						}
					}
				}
				dyeCount -= dyeAlreadyConsumed;

				for (int row = 0; row < 3; ++row) {
					for (int col = 0; col < 3; ++col) {
						int slot = row * 3 + col;
						bool enableRedSlot = false;
						if (slot == bannerSlot) {
							setIngredientSlotItem(getPad(), slot, whiteBanner);
						} else if (pattern.rows[row][col] != ' ') {
							setIngredientSlotItem(getPad(), slot, dyeItem);
							if (dyeCount <= 0 && !(app.DebugSettingsOn() && (app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad()) & (1L << eDebugSetting_CraftAnything)))) enableRedSlot = true;
							else dyeCount--;
						} else {
							setIngredientSlotItem(getPad(), slot, nullptr);
						}
						setIngredientSlotRedBox(slot, enableRedSlot);
					}
				}

				setCraftingOutputSlotItem(getPad(), makePatternPreviewBanner(pattern.code, colorIndex));
				setCraftingOutputSlotRedBox(!((canCraftPattern(pattern, colorIndex)) || (app.DebugSettingsOn() && (app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad()) & (1L << eDebugSetting_CraftAnything)))));
			}
		} else {
			setCraftingOutputSlotItem(getPad(), nullptr);
			setCraftingOutputSlotRedBox(false);
		}
	} else {
		//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
		Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();

		// hide the previous ingredients
		hideAllIngredientsSlots();

		if(CanBeMadeA[m_iCurrentSlotHIndex].iCount!=0)
		{
			int iSlot,iRecipy;
			if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
			{
				iSlot=iVSlotIndexA[m_iCurrentSlotVIndex];
				iRecipy=CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
			}
			else
			{
				iSlot=0;
				iRecipy=CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[0];
			}

			// show the 2x2 or 3x3 to make the current item
			int iBoxWidth=(m_iContainerType==RECIPE_TYPE_2x2)?2:3;
			int iRecipe=CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
			bool bCanMakeRecipe  = pRecipeIngredientsRequired[iRecipe].bCanMake[getPad()];
			shared_ptr<ItemInstance> pTempItemInst=pRecipeIngredientsRequired[iRecipe].pRecipy->assemble(nullptr);

			m_iIngredientsC=pRecipeIngredientsRequired[iRecipe].iIngC;

			// update the ingredients required - these will all be hidden until cycled by the user
			for(int i=0;i<pRecipeIngredientsRequired[iRecipe].iIngC;i++)
			{
				int id=pRecipeIngredientsRequired[iRecipe].iIngIDA[i];
				int iAuxVal=pRecipeIngredientsRequired[iRecipe].iIngAuxValA[i];
				// neoLegacy - never build an ItemInstance from a missing item (a bad recipe ingredient used to crash here)
				Item *item = (id > 0 && id < Item::ITEM_NUM_COUNT) ? Item::items[id] : nullptr;
				if (item == nullptr)
				{
					app.DebugPrintf("Recipe %d has an ingredient with no item (id %d), skipping it\n", iRecipe, id);
					continue;
				}

				shared_ptr<ItemInstance> itemInst= std::make_shared<ItemInstance>(item, pRecipeIngredientsRequired[iRecipe].iIngValA[i], iAuxVal);

				// 4J-PB - a very special case - the bed can use any kind of wool, so we can't use the item description
				// and the same goes for the painting
				int idescID;

				// neoLegacy - beds are now crafted from three wool of one specific color, so only the painting still takes "any wool"
				if( ((pTempItemInst->id==Item::painting_Id)	&&(id==Tile::wool_Id)) )
				{
					idescID=IDS_ANY_WOOL;
				}
				else if((pTempItemInst->id==Item::firework_charge_Id) && (id==Item::dye_Id))
				{
					idescID=IDS_ITEM_DYE_POWDER;
					iAuxVal = 1;
				}
				else
				{
					idescID=itemInst->getDescriptionId();
				}
				setIngredientDescriptionText(i,app.GetString(idescID));


				if( (iAuxVal & 0xFF) == 0xFF) // 4J Stu - If the aux value is set to match any
					iAuxVal = 0;

				// 4J Stu - For clocks and compasses we set the aux value to a special one that signals we should use a default texture
				// rather than the dynamic one for the player
				if( id == Item::clock_Id || id == Item::compass_Id )
				{
					iAuxVal = 0xFF;
				}
				itemInst->setAuxValue(iAuxVal);

				setIngredientDescriptionItem(getPad(),i,itemInst);
				setIngredientDescriptionRedBox(i,false);
			}

			// 4J Stu - For clocks and compasses we set the aux value to a special one that signals we should use a default texture
			// rather than the dynamic one for the player
			if( pTempItemInst->id == Item::clock_Id || pTempItemInst->id == Item::compass_Id )
			{
				pTempItemInst->setAuxValue( 255 );
			}

			// don't grey out the output icon
			setCraftingOutputSlotItem(getPad(), pTempItemInst);

			if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_CraftAnything))
			{
				setCraftingOutputSlotRedBox(false);
			}
			else
			{
				if(bCanMakeRecipe==false)
				{
					setCraftingOutputSlotRedBox(true);
				}
				else
				{
					setCraftingOutputSlotRedBox(false);
				}

			}
			for (int x = 0; x < iBoxWidth; x++)
			{
				for (int y = 0; y < iBoxWidth; y++)
				{
					int index = x+y*iBoxWidth;
					if(pRecipeIngredientsRequired[iRecipy].uiGridA[x+y*3]!=0)
					{
						int id=pRecipeIngredientsRequired[iRecipy].uiGridA[x+y*3]&0x00FFFFFF;
						assert(id!=0);
						int iAuxVal=(pRecipeIngredientsRequired[iRecipy].uiGridA[x+y*3]&0xFF000000)>>24;
						for (int k = 0; k < pRecipeIngredientsRequired[iRecipy].iIngC; k++)
						{
							int fullAux = pRecipeIngredientsRequired[iRecipy].iIngAuxValA[k];
							if (pRecipeIngredientsRequired[iRecipy].iIngIDA[k] == id && fullAux != Recipes::ANY_AUX_VALUE && (fullAux & 0xFF) == iAuxVal)
							{
								iAuxVal = fullAux;
								break;
							}
						}

						// 4J Stu - For clocks and compasses we set the aux value to a special one that signals we should use a default texture
						// rather than the dynamic one for the player
						if( id == Item::clock_Id || id == Item::compass_Id )
						{
							iAuxVal = 0xFF;
						}
						else if( pTempItemInst->id==Item::firework_charge_Id && id == Item::dye_Id)
						{
							iAuxVal = 1;
						}
						shared_ptr<ItemInstance> itemInst= std::make_shared<ItemInstance>(id, 1, iAuxVal);
						setIngredientSlotItem(getPad(),index,itemInst);
						// show the ingredients we don't have if we can't make the recipe
						if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_CraftAnything))
						{
							setIngredientSlotRedBox(index, false);
						}
						else
						{
							if((pRecipeIngredientsRequired[iRecipy].usBitmaskMissingGridIngredients[getPad()]&(1<<(x+y*3)))!=0)
							{
								setIngredientSlotRedBox(index, true);
							}
							else
							{
								setIngredientSlotRedBox(index, false);
							}
						}
					}
					else
					{
						setIngredientSlotRedBox(index, false);
						setIngredientSlotItem(getPad(),index,nullptr);
					}
				}
			}
		}
		else
		{
			setCraftingOutputSlotItem(getPad(), nullptr);
			setCraftingOutputSlotRedBox(false);
			m_iIngredientsC=0;
			int iIngredientsSlots;
			// if it's a 2x2 , only clear the 4 m_pCraftingIngredientA slots
			if(m_iContainerType==RECIPE_TYPE_2x2)
			{
				iIngredientsSlots=4;
			}
			else
			{
				iIngredientsSlots=m_iIngredients3x3SlotC;
			}

			for(int i=0;i<iIngredientsSlots;i++)
			{
				setIngredientSlotRedBox(i, false);
				setIngredientSlotItem(getPad(),i,nullptr);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//	UpdateDescriptionText
//
//////////////////////////////////////////////////////////////////////////
void IUIScene_CraftingMenu::UpdateDescriptionText(bool bCanBeMade)
{
	int iIDSString=0;
	//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
	Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();

	if(bCanBeMade)
	{
		int iSlot;//,iRecipy;
		if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
		{
			iSlot=iVSlotIndexA[m_iCurrentSlotVIndex];
			//iRecipy=CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
		}
		else
		{
			iSlot=0;
			//iRecipy=CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[0];
		}

		shared_ptr<ItemInstance> pTempItemInst=pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot]].pRecipy->assemble(nullptr);
		int iID=pTempItemInst->getItem()->id;
		int iAuxVal=pTempItemInst->getAuxValue();
		int iBaseType;

		if(Tile::isTileItemId(iID)) // is it a tile?
		{
			iBaseType=Tile::tiles[iID]->getBaseItemType();

			iIDSString = Tile::tiles[iID]->getUseDescriptionId();
		}
		else
		{
			iBaseType=pTempItemInst->getItem()->getBaseItemType();

			iIDSString = pTempItemInst->getUseDescriptionId();
		}

		// A few special cases where the description required is specific to crafting, rather than the normal description
		if(iBaseType!=Item::eBaseItemType_undefined)
		{
			switch(iBaseType)
			{
			case Item::eBaseItemType_cloth:
				switch(iAuxVal)
				{
				case 0:
					iIDSString=IDS_DESC_WOOLSTRING;
					break;
				}
				break;
			}
		}

		// set the string mapped to by the base object mapping array

		if(iIDSString>=0)
		{
			// this is an html control now, so set the font size and colour
			//wstring wsText=app.GetString(iIDSString);
			wstring wsText=app.FormatHTMLString(getPad(),app.GetString(iIDSString));

			// 12 for splitscreen, 14 for normal
			EHTMLFontSize size = eHTMLSize_Normal;
			if(m_bSplitscreen ||(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen()))
			{
				size = eHTMLSize_Splitscreen;
			}
			wchar_t startTags[64];
			swprintf(startTags,64,L"<font color=\"#%08x\"><P ALIGN=LEFT>",app.GetHTMLColour(eHTMLColor_Black));
			wsText= startTags + wsText + L"</P>";

			setDescriptionText(wsText.c_str());
		}
		else
		{
			/// Missing string!
#ifdef _DEBUG
			setDescriptionText(L"This is some placeholder description text about the craftable item.");
#else
			setDescriptionText(L"");
#endif
		}
	}
	else
	{
		setDescriptionText(L"");
	}
}

//////////////////////////////////////////////////////////////////////////
//
//	UpdateTooltips
//
//////////////////////////////////////////////////////////////////////////
void IUIScene_CraftingMenu::UpdateTooltips()
{
	if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex == 0 && m_iCurrentSlotHIndex == 1) {
		ui.SetTooltips(getPad(), -1, IDS_TOOLTIPS_EXIT, -1, IDS_TOOLTIPS_SELECT, -1, -1, -1, -1);
	} else if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex > 0) {
		int tooltipA = -1;
		int tooltipX = -1;
		int tooltipY = -1;

		if (m_selectedBannerEdit != nullptr && !m_bannerPreviewData.empty())
		{
			tooltipA = IDS_TOOLTIPS_CREATE;
		}
		if (m_selectedBannerEdit != nullptr) tooltipX = IDS_TOOLTIPS_REMOVE;

		int patternId = m_aBannerCraftPatterns[m_iGroupHIndex - 1][m_iCurrentSlotHIndex];
		if (m_selectedBannerEdit != nullptr && patternId != -1 &&
			((int)BannerItem::getPatterns(m_selectedBannerEdit).size() + m_bannerPreviewData.size()) < 6)
		{
			PreviewBannerData data(patternId, iVSlotIndexA[m_iCurrentSlotVIndex]);
			vector<PreviewBannerData> previewData = m_bannerPreviewData;
			previewData.emplace_back(data);
			if (hasResourcesForBannerPatterns(previewData)) tooltipY = IDS_TOOLTIPS_ADD;
		}

		ui.SetTooltips(getPad(), tooltipA, IDS_TOOLTIPS_EXIT, tooltipX, tooltipY, -2, IDS_TOOLTIPS_CHANGE_TYPE, -2, IDS_TOOLTIPS_CHANGE_GROUP);
	} else {
		//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
		Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();
		// Update tooltips

		bool bDisplayCreate;

		if(CanBeMadeA[m_iCurrentSlotHIndex].iCount!=0)
		{
			int iSlot;
			if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
			{
				iSlot=iVSlotIndexA[m_iCurrentSlotVIndex];
			}
			else
			{
				iSlot=0;
			}

			if(pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot]].bCanMake[getPad()])
			{
				bDisplayCreate=true;
			}
			else
			{
				bDisplayCreate=false;
			}
		}
		else
		{
			bDisplayCreate=false;
		}

		if (m_iGroupVIndex == eGroupVType_Banner && m_iGroupHIndex == 0 && m_iCurrentSlotHIndex == 0) {
			ui.SetTooltips(getPad(), bDisplayCreate ? IDS_TOOLTIPS_CREATE : -1, IDS_TOOLTIPS_EXIT, -1, -1, (m_iContainerType == RECIPE_TYPE_3x3) ? -2 : -1, (m_iContainerType == RECIPE_TYPE_3x3) ? IDS_TOOLTIPS_CHANGE_TYPE : -1, -1, -1);
		} else {
			switch(m_iDisplayDescription)
			{
			case DISPLAY_INVENTORY:
				ui.SetTooltips( getPad(), bDisplayCreate?IDS_TOOLTIPS_CREATE:-1,IDS_TOOLTIPS_EXIT, IDS_TOOLTIPS_SHOW_DESCRIPTION,-1, (m_iContainerType==RECIPE_TYPE_3x3)?-2:-1,(m_iContainerType==RECIPE_TYPE_3x3)?IDS_TOOLTIPS_CHANGE_TYPE:-1,-2, IDS_TOOLTIPS_CHANGE_GROUP);
				break;
			case DISPLAY_DESCRIPTION:
				ui.SetTooltips( getPad(), bDisplayCreate?IDS_TOOLTIPS_CREATE:-1,IDS_TOOLTIPS_EXIT, IDS_TOOLTIPS_SHOW_INGREDIENTS,-1,(m_iContainerType==RECIPE_TYPE_3x3)?-2:-1,(m_iContainerType==RECIPE_TYPE_3x3)?IDS_TOOLTIPS_CHANGE_TYPE:-1,-2, IDS_TOOLTIPS_CHANGE_GROUP);
				break;
			case DISPLAY_INGREDIENTS:
				ui.SetTooltips( getPad(), bDisplayCreate?IDS_TOOLTIPS_CREATE:-1,IDS_TOOLTIPS_EXIT, IDS_TOOLTIPS_SHOW_INVENTORY,-1,(m_iContainerType==RECIPE_TYPE_3x3)?-2:-1,(m_iContainerType==RECIPE_TYPE_3x3)?IDS_TOOLTIPS_CHANGE_TYPE:-1,-2, IDS_TOOLTIPS_CHANGE_GROUP);
				break;
			}
		}

		/*if(CanBeMadeA[m_iCurrentSlotHIndex].iCount!=0)
		{
			int iSlot;
			if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
			{
				iSlot=iVSlotIndexA[m_iCurrentSlotVIndex];
			}
			else
			{
				iSlot=0;
			}

			if(pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot]].bCanMake[getPad()])
			{
				ui.EnableTooltip( getPad(), eToolTipButtonA, true );
			}
			else
			{
				ui.EnableTooltip( getPad(), eToolTipButtonA, false );
			}
		}
		else
		{
			ui.ShowTooltip( getPad(), eToolTipButtonA, false );
		}*/
	}
}

void IUIScene_CraftingMenu::HandleInventoryUpdated()
{
	// Check which recipes are available with the resources we have
	CheckRecipesAvailable();
	UpdateVerticalSlots();
	UpdateHighlight();
	UpdateTooltips();
}

bool IUIScene_CraftingMenu::isItemSelected(int itemId)
{
	bool isSelected = false;
	if(m_pPlayer && m_pPlayer->inventory)
	{
		//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
		Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();

		if(CanBeMadeA[m_iCurrentSlotHIndex].iCount!=0)
		{
			int iSlot;
			if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
			{
				iSlot=iVSlotIndexA[m_iCurrentSlotVIndex];
			}
			else
			{
				iSlot=0;
			}
			int iRecipe= CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
			ItemInstance *pTempItemInst = (ItemInstance *)pRecipeIngredientsRequired[iRecipe].pRecipy->getResultItem();

			if(pTempItemInst->id == itemId)
			{
				isSelected = true;
			}
		}
	}
	return isSelected;
}
