#include "stdafx.h"
#include "Gui.h"
#include "ItemRenderer.h"
#include "ChickenModel.h"
#include "GameRenderer.h"
#include "Options.h"
#include "MultiPlayerLocalPlayer.h"
#include "LocalPlayer.h"
#include "ArcadeGameSession.h"
#include <algorithm>
#include <vector>
#include "Textures.h"
#include "TextureAtlas.h"
#include "GameMode.h"
#include "Lighting.h"
#include "ChatScreen.h"
#include "MultiPlayerLevel.h"
#include "../Minecraft.World/JavaMath.h"
#include "../Minecraft.World/net.minecraft.world.entity.player.h"
#include "../Minecraft.World/net.minecraft.world.effect.h"
#include "../Minecraft.World/net.minecraft.world.food.h"
#include "../Minecraft.World/net.minecraft.world.item.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "../Minecraft.World/LevelData.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/System.h"
#include "../Minecraft.World/Language.h"
#include "EntityRenderDispatcher.h"
#include "../Minecraft.World/Dimension.h"
#include "../Minecraft.World/net.minecraft.world.entity.boss.enderdragon.h"
#include "EnderDragonRenderer.h"
#include "../Minecraft.World/net.minecraft.h"
#include "../Minecraft.World/net.minecraft.world.h"
#include "../Minecraft.World/LevelChunk.h"
#include "../Minecraft.World/Biome.h"
#include "../Minecraft.World/HitResult.h"
#include <Common/UI/UI.h>
#include "../Minecraft.World/Tile.h"
#include "../Minecraft.World/DyePowderItem.h"
#include "../Minecraft.World/BlockStateDecoderRegistry.h"
#include "../Minecraft.World/BlockStateDecoder.h"
#include <map>
#include <set>
#include <cwctype>

ResourceLocation Gui::PUMPKIN_BLUR_LOCATION = ResourceLocation(TN__BLUR__MISC_PUMPKINBLUR);
ResourceLocation Gui::FREEZE_OVERLAY_LOCATION = ResourceLocation(TN_MISC_FREEZE_OVERLAY);	// neoLegacy
ResourceLocation Gui::SPYSCOPE_OVERLAY_LOCATION = ResourceLocation(TN__BLUR__MISC_SPYSCOPE);	// neoLegacy

#define RENDER_HUD 0
//#ifndef _XBOX
//#undef RENDER_HUD
//#define RENDER_HUD 1
//#endif

float Gui::currentGuiBlendFactor = 1.0f;	// 4J added
float Gui::currentGuiScaleFactor = 1.0f;	// 4J added
ItemRenderer *Gui::itemRenderer = new ItemRenderer();

Gui::Gui(Minecraft *minecraft)
{
	// 4J - initialisers added
    random = new Random();
	tickCount = 0;
    overlayMessageTime = 0;
    animateOverlayMessageColor = false;
	progress = 0.0f;
	tbr = 1.0f;
	fAlphaIncrementPerCent=255.0f/100.0f;

	this->minecraft = minecraft;

	lastTickA = 0.0f;
}

void Gui::render(float a, bool mouseFree, int xMouse, int yMouse)
{
	// 4J Stu - I have copied this code for XUI_BaseScene. If/when it gets changed it should be broken out
	// 4J - altered to force full screen mode to 3X scaling, and any split screen modes to 2X scaling. This is so that the further scaling by 0.5 that
	// happens in split screen modes results in a final scaling of 1 rather than 1.5.
	
	if (minecraft->player == nullptr)
	{
		return;
	}
	
	int splitYOffset;// = 20;	// This offset is applied when doing the 2X scaling above to move the gui out of the way of the tool tips
	int guiScale;// = ( minecraft->player->m_iScreenSection == C4JRender::VIEWPORT_TYPE_FULLSCREEN ? 3 : 2 );
	int iPad=minecraft->player->GetXboxPad();
	int iWidthOffset=0,iHeightOffset=0; // used to get the interface looking right on a 2 player split screen game

 	// 4J-PB - selected the gui scale based on the slider settings
	if(minecraft->player->m_iScreenSection == C4JRender::VIEWPORT_TYPE_FULLSCREEN)
	{
		guiScale=app.GetGameSettings(iPad,eGameSetting_UISize) + 2;
	}
	else
	{
		guiScale=app.GetGameSettings(iPad,eGameSetting_UISizeSplitscreen) + 2;
	}


	ScreenSizeCalculator ssc(minecraft->options, minecraft->width, minecraft->height, guiScale );
	int screenWidth = ssc.getWidth();
	int screenHeight = ssc.getHeight();
	int iSafezoneXHalf=0,iSafezoneYHalf=0,iSafezoneTopYHalf=0;
	int iTooltipsYOffset=0;
	int quickSelectWidth=182;
	int quickSelectHeight=22;
	float fScaleFactorWidth=1.0f,fScaleFactorHeight=1.0f;
	bool bTwoPlayerSplitscreen=false;
	currentGuiScaleFactor = static_cast<float>(guiScale);		// Keep static copy of scale so we know how gui coordinates map to physical pixels - this is also affected by the viewport

	switch(guiScale)
	{
	case 3:
		splitYOffset = 0;
		break;
	case 4:
		splitYOffset = -5;
		break;
	default: // 2
		splitYOffset = 10;
		break;
	}

	// Check which screen section this player is in
	switch(minecraft->player->m_iScreenSection)
	{
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
		// single player
		iSafezoneXHalf = screenWidth/20; // 5%
		iSafezoneYHalf = screenHeight/20; // 5%
		iSafezoneTopYHalf = iSafezoneYHalf;
		iTooltipsYOffset=40+splitYOffset;
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
		iSafezoneXHalf = screenWidth/10; // 5%  (need to treat the whole screen is 2x this screen)
		iSafezoneYHalf = splitYOffset;
		iSafezoneTopYHalf = screenHeight/10;
		fScaleFactorWidth=0.5f;
		iWidthOffset=static_cast<int>((float)screenWidth * (1.0f - fScaleFactorWidth));
		iTooltipsYOffset=44;
		bTwoPlayerSplitscreen=true;
		currentGuiScaleFactor *= 0.5f;
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
		iSafezoneXHalf = screenWidth/10; // 5% (need to treat the whole screen is 2x this screen)
		iSafezoneYHalf = splitYOffset + screenHeight/10;// 5%  (need to treat the whole screen is 2x this screen)
		iSafezoneTopYHalf = 0;
		fScaleFactorWidth=0.5f;
		iWidthOffset=static_cast<int>((float)screenWidth * (1.0f - fScaleFactorWidth));
		iTooltipsYOffset=44;
		bTwoPlayerSplitscreen=true;
		currentGuiScaleFactor *= 0.5f;
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
		iSafezoneXHalf = screenWidth/10; // 5% (the whole screen is 2x this screen)
		iSafezoneYHalf = splitYOffset + screenHeight/10;// 5% (need to treat the whole screen is 2x this screen)
		iSafezoneTopYHalf = screenHeight/10;
		fScaleFactorHeight=0.5f;
		iHeightOffset=screenHeight;
		iTooltipsYOffset=44;
		bTwoPlayerSplitscreen=true;
		currentGuiScaleFactor *= 0.5f;
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
		iSafezoneXHalf = 0;
		iSafezoneYHalf = splitYOffset + screenHeight/10;// 5% (need to treat the whole screen is 2x this screen)
		iSafezoneTopYHalf = splitYOffset + screenHeight/10;
		fScaleFactorHeight=0.5f;
		iHeightOffset=screenHeight;
		iTooltipsYOffset=44;
		bTwoPlayerSplitscreen=true;
		currentGuiScaleFactor *= 0.5f;
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
		iSafezoneXHalf = screenWidth/10; // 5% (the whole screen is 2x this screen)
		iSafezoneYHalf = splitYOffset;
		iSafezoneTopYHalf = screenHeight/10;
		iTooltipsYOffset=44;
		currentGuiScaleFactor *= 0.5f;
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
		iSafezoneXHalf = 0;
		iSafezoneYHalf = splitYOffset; // 5%
		iSafezoneTopYHalf = screenHeight/10;
		iTooltipsYOffset=44;
		currentGuiScaleFactor *= 0.5f;
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
		iSafezoneXHalf = screenWidth/10; // 5%  (the whole screen is 2x this screen)
		iSafezoneYHalf = splitYOffset + screenHeight/10; // 5% (the whole screen is 2x this screen)
		iSafezoneTopYHalf = 0;
		iTooltipsYOffset=44;
		currentGuiScaleFactor *= 0.5f;
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
		iSafezoneXHalf = 0;
		iSafezoneYHalf = splitYOffset + screenHeight/10; // 5%  (the whole screen is 2x this screen)
		iSafezoneTopYHalf = 0;
		iTooltipsYOffset=44;
		currentGuiScaleFactor *= 0.5f;
		break;

	}

	// Copium Arcade replaces the normal HUD while a player is inside a cabinet.
    // Gate this per LocalPlayer so split-screen players who are not playing keep their HUD.
    shared_ptr<LocalPlayer> arcadeHudPlayer = dynamic_pointer_cast<LocalPlayer>(minecraft->player);
    bool bArcadeSessionActive = arcadeHudPlayer != nullptr && arcadeHudPlayer->getArcadeGameSession().isActive();
	// 4J-PB - turn off the slot display if a xui menu is up, if we're autosaving, or in Copium Arcade.
	bool bDisplayGui=!bArcadeSessionActive && !ui.GetMenuDisplayed(iPad) && !(app.GetXuiAction(iPad)==eAppAction_AutosaveSaveGameCapturedThumbnail);

	// if tooltips are off, set the y offset to zero
	if(app.GetGameSettings(iPad,eGameSetting_Tooltips)==0 && bDisplayGui)
	{
		switch(minecraft->player->m_iScreenSection)
		{
		case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
			iTooltipsYOffset=screenHeight/10;
			break;
		default:
			//iTooltipsYOffset=screenHeight/10;
			switch(guiScale)
			{
			case 3:
				iTooltipsYOffset=28;//screenHeight/10;
				break;
			case 4:
				iTooltipsYOffset=28;//screenHeight/10;
				break;
			default: // 2
				iTooltipsYOffset=14;//screenHeight/10;
				break;
			}
			break;
		}
	}

	// 4J-PB - Turn off interface if eGameSetting_DisplayHUD is off - for screen shots/videos.
	if ( app.GetGameSettings(iPad,eGameSetting_DisplayHUD)==0 )
	{
		bDisplayGui = false;
	}

    Font *font = minecraft->font;


    minecraft->gameRenderer->setupGuiScreen(guiScale);



    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// 4J - added - this did actually get set in renderVignette but that code is currently commented out

    if (Minecraft::useFancyGraphics())
	{
		renderVignette(minecraft->player->getBrightness(a), screenWidth, screenHeight);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// Display the pumpkin screen effect
	/////////////////////////////////////////////////////////////////////////////////////

    shared_ptr<ItemInstance> headGear = minecraft->player->inventory->getArmor(3);

	// 4J-PB - changing this to be per player
	//if (!minecraft->options->thirdPersonView && headGear != NULL && headGear->id == Tile::pumpkin_Id) renderPumpkin(screenWidth, screenHeight);
	if ((minecraft->player->ThirdPersonView()==0) && headGear != NULL && headGear->id == Tile::pumpkin_Id) renderPumpkin(screenWidth, screenHeight);

	// neoLegacy - Freezing: frost creeps in from the screen edges as the effect builds up
	if ((minecraft->player->ThirdPersonView() == 0) && minecraft->player->freezeTicks > 0) renderFreeze(minecraft->player->getFreezeProgress(), screenWidth, screenHeight);

	// neoLegacy - Spyglass: scope overlay while the spyglass is being used in first person (same getUseItem check as the FOV zoom in GameRenderer.cpp)
	shared_ptr<ItemInstance> usedItem = minecraft->player->getUseItem();
	if ((minecraft->player->ThirdPersonView() == 0) && usedItem != nullptr && usedItem->id == Item::spyglass_Id) renderScope(screenWidth, screenHeight);
	if (!minecraft->player->hasEffect(MobEffect::confusion))
	{
		float pt = minecraft->player->oPortalTime + (minecraft->player->portalTime - minecraft->player->oPortalTime) * a;
		if (pt > 0)
		{
			renderTp(pt, screenWidth, screenHeight);
		}
	}

	if (!minecraft->gameMode->isCutScene())
	{
		if(bDisplayGui && bTwoPlayerSplitscreen)
		{
			// need to apply scale factors depending on the mode
			glPushMatrix();
			glScalef(fScaleFactorWidth, fScaleFactorHeight, fScaleFactorWidth);
		}
#if RENDER_HUD
		/////////////////////////////////////////////////////////////////////////////////////
		// Display the quick select background, the quick select selection, and the crosshair
		/////////////////////////////////////////////////////////////////////////////////////

		glColor4f(1, 1, 1, 1);

		// 4J - this is where to set the blend factor for gui things
		// use the primary player's settings
		unsigned char ucAlpha=app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_InterfaceOpacity);

		// If the user has started to navigate their quickselect bar, ignore the alpha setting, and display at default value
		float fVal=fAlphaIncrementPerCent*(float)ucAlpha;
		if(ucAlpha<80)
		{
			// check if we have the timer running for the opacity
			unsigned int uiOpacityTimer=app.GetOpacityTimer(iPad);
			if(uiOpacityTimer!=0)
			{
				if(uiOpacityTimer<10)
				{
					float fStep=(80.0f-(float)ucAlpha)/10.0f;
					fVal=fAlphaIncrementPerCent*(80.0f-((10.0f-(float)uiOpacityTimer)*fStep));
				}
				else
				{
					fVal=fAlphaIncrementPerCent*80.0f;
				}
			}
			else
			{
				fVal=fAlphaIncrementPerCent*(float)ucAlpha;
			}
		}
		else
		{
			fVal=fAlphaIncrementPerCent*(float)ucAlpha;
		}

		RenderManager.StateSetBlendFactor(0xffffff |(((unsigned int)fVal)<<24));
		currentGuiBlendFactor = fVal / 255.0f;
	//	RenderManager.StateSetBlendFactor(0x40ffffff);
		glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);

		blitOffset = -90;

		/////////////////////////////////////////////////////////////////////////////////////
		// Display the quick select background, the quick select selection, and the crosshair
		/////////////////////////////////////////////////////////////////////////////////////
		if(bDisplayGui)
		{
			MemSect(31);
			minecraft->textures->bindTexture(TN_GUI_GUI);	// 4J was L"/gui/gui.png"
			MemSect(0);

			shared_ptr<Inventory> inventory = minecraft->player->inventory;
			if(bTwoPlayerSplitscreen)
			{
				// need to apply scale factors depending on the mode

				// 4J Stu - Moved this push and scale further up as we still need to do it for the few HUD components not replaced by xui
				//glPushMatrix();
				//glScalef(fScaleFactorWidth, fScaleFactorHeight, fScaleFactorWidth);

				// 4J-PB - move into the safe zone, and account for 2 player splitscreen
				blit(iWidthOffset + (screenWidth - quickSelectWidth)/2, iHeightOffset + screenHeight - iSafezoneYHalf - iTooltipsYOffset , 0, 0, 182, 22);
				blit(iWidthOffset + (screenWidth - quickSelectWidth)/2 - 1 + inventory->selected * 20, iHeightOffset + screenHeight - iSafezoneYHalf - iTooltipsYOffset  - 1, 0, 22, 24, 22);
			}
			else
			{
				blit(iWidthOffset + screenWidth / 2 - quickSelectWidth / 2, iHeightOffset + screenHeight - iSafezoneYHalf - iTooltipsYOffset , 0, 0, 182, 22);
				blit(iWidthOffset + screenWidth / 2 - quickSelectWidth / 2 - 1 + inventory->selected * 20, iHeightOffset + screenHeight - iSafezoneYHalf - iTooltipsYOffset  - 1, 0, 22, 24, 22);
			}


			MemSect(31);
			minecraft->textures->bindTexture(TN_GUI_ICONS);//L"/gui/icons.png"));
			MemSect(0);
			glEnable(GL_BLEND);
			RenderManager.StateSetBlendFactor(0xffffff |(((unsigned int)fVal)<<24));
			glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
			//glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
			// 4J Stu - We don't want to adjust the cursor by the safezone, we want it centred
			if(bTwoPlayerSplitscreen)
			{
				blit(iWidthOffset + screenWidth / 2 - 7, (iHeightOffset + screenHeight) / 2 - 7, 0, 0, 16, 16);
			}
			else
			{
				blit(screenWidth / 2 - 7, screenHeight / 2 - 7, 0, 0, 16, 16);
			}
			glDisable(GL_BLEND);

	// 		if(bTwoPlayerSplitscreen)
	// 		{
	// 			glPopMatrix();
	// 		}

		}

		bool blink = minecraft->player->invulnerableTime / 3 % 2 == 1;
		if (minecraft->player->invulnerableTime < 10) blink = false;
		int iHealth = minecraft->player->getHealth();
		int iLastHealth = minecraft->player->lastHealth;
		random->setSeed(tickCount * 312871);

		bool foodBlink = false;
		FoodData *foodData = minecraft->player->getFoodData();
		int food = foodData->getFoodLevel();
		int oldFood = foodData->getLastFoodLevel();

// 		if (false) //(true)
// 		{
// 			renderBossHealth();
// 		}

		/////////////////////////////////////////////////////////////////////////////////////
		// Display the experience, food, armour, health and the air bubbles
		/////////////////////////////////////////////////////////////////////////////////////
		if(bDisplayGui)
		{
			// 4J - added blend for fading gui
			glEnable(GL_BLEND);
			glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);

			if (minecraft->gameMode->canHurtPlayer())
			{
				int xLeft, xRight;
				// 4J Stu - TODO Work out proper positioning for splitscreen
				if(bTwoPlayerSplitscreen)
				{
					xLeft = iWidthOffset + (screenWidth - quickSelectWidth)/2;
					xRight = iWidthOffset + (screenWidth + quickSelectWidth)/2;
				}
				else
				{
					xLeft = (screenWidth - quickSelectWidth)/2;
					xRight = (screenWidth + quickSelectWidth) / 2;
				}

				// render experience bar
				int xpNeededForNextLevel = minecraft->player->getXpNeededForNextLevel();
				if (xpNeededForNextLevel > 0)
				{
					int w = 182;

					int progress = (int) (minecraft->player->experienceProgress * (float) (w + 1));

					int yo = screenHeight - iSafezoneYHalf - iTooltipsYOffset - 8;
					if(bTwoPlayerSplitscreen)
					{
						yo+=iHeightOffset;
					}
					blit(xLeft, yo, 0, 64, w, 5);
					if (progress > 0)
					{
						blit(xLeft, yo, 0, 69, progress, 5);
					}
				}

				int yLine1, yLine2;
				if(bTwoPlayerSplitscreen)
				{
					//yo = iHeightOffset + screenHeight - 10 - iSafezoneYHalf - iTooltipsYOffset;
					yLine1 = iHeightOffset + screenHeight - 18 - iSafezoneYHalf - iTooltipsYOffset;
					yLine2 = yLine1 - 10;
				}
				else
				{
					//yo = screenHeight - 10 - iSafezoneYHalf - iTooltipsYOffset;
					yLine1 = screenHeight - 18 - iSafezoneYHalf - iTooltipsYOffset;
					yLine2 = yLine1 - 10;
				}

				double maxHealth = minecraft->localplayers[iPad]->getAttribute(SharedMonsterAttributes.MAX_HEALTH);

				double totalAbsorption = minecraft->localplayers[iPad]->getAbsorptionAmount();
				const double healthHalves = (maxHealth + totalAbsorption) / 2.0;
				int numHealthRows = Mth.ceil(healthHalves / (float) NUM_HEARTS_PER_ROW);
				int healthRowHeight = Math.max(10 - (numHealthRows - 2), 3);
				int yLine2 = yLine1 - (numHealthRows - 1) * healthRowHeight - 10;
				absorption = totalAbsorption;

				int armor = minecraft->player->getArmorValue();
				int heartOffsetIndex = -1;
				if (minecraft->player->hasEffect(MobEffect::regeneration))
				{
					heartOffsetIndex = tickCount % (int) ceil(maxHealth + 5);
				}

				// render health and armor
				//minecraft.profiler.push("armor");
				for (int i = 0; i < Player::MAX_HEALTH / 2; i++)
				{
					if (armor > 0)
					{
						int xo = xLeft + i * 8;
						if (i * 2 + 1 < armor) blit(xo, yLine2, 16 + 2 * 9, 9, 9, 9);
						if (i * 2 + 1 == armor) blit(xo, yLine2, 16 + 1 * 9, 9, 9, 9);
						if (i * 2 + 1 > armor) blit(xo, yLine2, 16 + 0 * 9, 9, 9, 9);
					}
				}

				//minecraft.profiler.popPush("health");
				for (int i = (int)Mth.ceil(healthHalves) - 1; i >= 0; i--)
				{
					int healthTexBaseX = 16;
					if (minecraft.player.hasEffect(MobEffect.poison))
					{
						healthTexBaseX += 4 * 9;
					}
					else if (minecraft.player.hasEffect(MobEffect.wither))
					{
						healthTexBaseX += 8 * 9;
					}

					int bg = 0;
					if (blink) bg = 1;
					int rowIndex = Mth.ceil((i + 1)  / (float) NUM_HEARTS_PER_ROW) - 1;
					int xo = xLeft + (i % NUM_HEARTS_PER_ROW) * 8;
					int yo = yLine1 - rowIndex * healthRowHeight;
					if (currentHealth <= 4)
					{
						yo += random.nextInt(2);
					}

					if (i == heartOffsetIndex)
					{
						yo -= 2;
					}

					int y0 = 0;

					//if (minecraft->level->getLevelData()->isHardcore())
					//{
					//	y0 = 5;
					//}

					blit(xo, yo, 16 + bg * 9, 9 * y0, 9, 9);
					if (blink)
					{
						if (i * 2 + 1 < oldHealth) blit(xo, yo, healthTexBaseX + 6 * 9, 9 * y0, 9, 9);
						if (i * 2 + 1 == oldHealth) blit(xo, yo, healthTexBaseX + 7 * 9, 9 * y0, 9, 9);
					}

					if (absorption > 0)
					{
						if (absorption == totalAbsorption && totalAbsorption % 2 == 1)
						{
							blit(xo, yo, healthTexBaseX + 17 * 9, 9 * y0, 9, 9);
						}
						else
						{
							blit(xo, yo, healthTexBaseX + 16 * 9, 9 * y0, 9, 9);
						}
						absorption -= 2;
					}
					else
					{
						if (i * 2 + 1 < currentHealth) blit(xo, yo, healthTexBaseX + 4 * 9, 9 * y0, 9, 9);
						if (i * 2 + 1 == currentHealth) blit(xo, yo, healthTexBaseX + 5 * 9, 9 * y0, 9, 9);
					}
				}

				std::shared_ptr<Entity> riding = minecraft->localplayers[iPad].get()->riding;
				std::shared_ptr<LivingEntity> living = dynamic_pointer_cast<LivingEntity>(riding);
				if (riding == NULL)
				{
					// render food
					for (int i = 0; i < FoodConstants::MAX_FOOD / 2; i++)
					{
						int yo = yLine1;


						int texBaseX = 16;
						int bg = 0;
						if (minecraft->player->hasEffect(MobEffect::hunger))
						{
							texBaseX += 4 * 9;
							bg = 13;
						}

						if (minecraft->player->getFoodData()->getSaturationLevel() <= 0)
						{
							if ((tickCount % (food * 3 + 1)) == 0)
							{
								yo += random->nextInt(3) - 1;
							}
						}

						if (foodBlink) bg = 1;
						int xo = xRight - i * 8 - 9;
						blit(xo, yo, 16 + bg * 9, 9 * 3, 9, 9);
						if (foodBlink)
						{
							if (i * 2 + 1 < oldFood) blit(xo, yo, texBaseX + 6 * 9, 9 * 3, 9, 9);
							if (i * 2 + 1 == oldFood) blit(xo, yo, texBaseX + 7 * 9, 9 * 3, 9, 9);
						}
						if (i * 2 + 1 < food) blit(xo, yo, texBaseX + 4 * 9, 9 * 3, 9, 9);
						if (i * 2 + 1 == food) blit(xo, yo, texBaseX + 5 * 9, 9 * 3, 9, 9);
					}
				}
				else if (living != nullptr)
				{
					// Render mount health

					int riderCurrentHealth = (int) ceil(living.get()->GetHealth());
					float maxRiderHealth = living->GetMaxHealth();
					int hearts = (int) (maxRiderHealth + .5f) / 2;
					if (hearts > 30)
					{
						hearts = 30;
					}

					int yo = yLine1;
					int baseHealth = 0;

					while (hearts > 0)
					{
						int rowHearts = min(hearts, 10);
						hearts -= rowHearts;

						for (int i = 0; i < rowHearts; i++)
						{
							int texBaseX = 52;
							int bg = 0;

							if (foodBlink) bg = 1;
							int xo = xRight - i * 8 - 9;
							blit(xo, yo, texBaseX + bg * 9, 9 * 1, 9, 9);
							if (i * 2 + 1 + baseHealth < riderCurrentHealth) blit(xo, yo, texBaseX + 4 * 9, 9 * 1, 9, 9);
							if (i * 2 + 1 + baseHealth == riderCurrentHealth) blit(xo, yo, texBaseX + 5 * 9, 9 * 1, 9, 9);
						}
						yo -= 10;
						baseHealth += 20;
					}
				}

				// render air bubbles
				if (minecraft->player->isUnderLiquid(Material::water))
				{
					const int airSupply = minecraft->player->getAirSupply();
					const float airScale = 10.0f / Player::TOTAL_AIR_SUPPLY;
					const float airSupplyScaled = airSupply * airScale;
					int count = (int) ceil((airSupply - 2) * airScale);
					int extra = (int) ceil(airSupplyScaled) - count;
					for (int i = 0; i < count + extra; i++)
					{
						// Air bubbles
						if (i < count) blit(xRight - i * 8 - 9, yLine2, 16, 9 * 2, 9, 9);
						else blit(xRight - i * 8 - 9, yLine2, 16 + 9, 9 * 2, 9, 9);
					}
				}
			}

		}

		// 4J-PB - turn off the slot display if a xui menu is up

		////////////////////////////
		// render the slot contents
		////////////////////////////
		if(bDisplayGui)
		{
	//		glDisable(GL_BLEND);		4J - removed - we want to be able to fade our gui

			glEnable(GL_RESCALE_NORMAL);

			Lighting::turnOnGui();


			int x,y;

			for (int i = 0; i < 9; i++)
			{
				if(bTwoPlayerSplitscreen)
				{
					x = iWidthOffset + screenWidth / 2 - 9 * 10 + i * 20 + 2;
					y = iHeightOffset + screenHeight - iSafezoneYHalf - iTooltipsYOffset - 16 - 3 + 22;
				}
				else
				{
					x = screenWidth / 2 - 9 * 10 + i * 20 + 2;
					y = screenHeight - iSafezoneYHalf - iTooltipsYOffset - 16 - 3 + 22;
				}
				this->renderSlot(i, x, y, a);
			}
			Lighting::turnOff();
			glDisable(GL_RESCALE_NORMAL);
		}
#endif // RENDER_HUD

		// neoLegacy - MAC-10 HUD. The stock HUD is drawn by the Iggy scene (RENDER_HUD is 0), so the gun's own readouts are drawn here:
		// a hit marker around the crosshair and an ammo counter in the bottom right corner.
		if (bDisplayGui)
		{
			Mac10Client &gun = minecraft->player->getMac10();
			bool markerHead = false;
			float marker = gun.getHitMarker(a, markerHead);
			int gMag = 0, gCap = 0, gReserve = 0;
			bool gReloading = false;
			bool gunHud = gun.getHud(gMag, gCap, gReserve, gReloading);
			if (marker > 0.01f || gunHud)
			{
				glColor4f(1, 1, 1, 1);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				if (marker > 0.01f)
				{
					int cx = bTwoPlayerSplitscreen ? (iWidthOffset + screenWidth / 2) : (screenWidth / 2);
					int cy = bTwoPlayerSplitscreen ? ((iHeightOffset + screenHeight) / 2) : (screenHeight / 2);
					int colour = (((int)(marker * 255.0f)) << 24) | (markerHead ? 0xFF3030 : 0xFFFFFF);
					for (int i = 4; i < 8; ++i)
					{
						fill(cx + i, cy + i, cx + i + 2, cy + i + 2, colour);
						fill(cx - i - 1, cy + i, cx - i + 1, cy + i + 2, colour);
						fill(cx + i, cy - i - 1, cx + i + 2, cy - i + 1, colour);
						fill(cx - i - 1, cy - i - 1, cx - i + 1, cy - i + 1, colour);
					}
				}
				if (gunHud)
				{
					wstring line = std::to_wstring(gMag) + L" / " + (gReserve < 0 ? wstring(L"INF") : std::to_wstring(gReserve));
					int colour = 0xFFFFFF;
					if (gMag <= 0) colour = 0xFF4040;
					else if (gMag * 3 <= gCap) colour = 0xFFD040;
					int w = minecraft->font->width(line);
					int hx = iWidthOffset + screenWidth - iSafezoneXHalf - w - 6;
					int hy = iHeightOffset + screenHeight - iSafezoneYHalf - iTooltipsYOffset - 30;
					minecraft->font->drawShadow(line, hx, hy, colour);
					// diagnostics (Mac10Item.h: MAC10_DEBUG_LOG): the client's own hit scan and what the server reported back for the last shot
					{
						std::wstring debugLine;
						if (gun.getDebugLine(minecraft->player->inventory->getSelected(), debugLine))
							minecraft->font->drawShadow(debugLine, iWidthOffset + iSafezoneXHalf + 6, iHeightOffset + screenHeight - iSafezoneYHalf - iTooltipsYOffset - 42, 0x80FF80);
					}
					if (gReloading)
					{
						wstring reloadText = L"RELOADING";
						minecraft->font->drawShadow(reloadText, iWidthOffset + screenWidth - iSafezoneXHalf - minecraft->font->width(reloadText) - 6, hy - 11, 0xAAAAAA);
					}
					else if (gMag <= 0 && gReserve != 0)
					{
						wstring reloadText = L"CLICK TO RELOAD";
						minecraft->font->drawShadow(reloadText, iWidthOffset + screenWidth - iSafezoneXHalf - minecraft->font->width(reloadText) - 6, hy - 11, 0xFF8080);
					}
				}
				glDisable(GL_BLEND);
				glColor4f(1, 1, 1, 1);
			}
		}


		// 4J - do render of crouched player. This code is largely taken from the inventory render of the player, with some special hard-coded positions
		// worked out by hand from the xui implementation of the crouch icon

		if(app.GetGameSettings(iPad,eGameSetting_AnimatedCharacter))
		{
			//int playerIdx = minecraft->player->GetXboxPad();

			static int characterDisplayTimer[4] = {0};
			if( !bDisplayGui )
			{
				characterDisplayTimer[iPad] = 0;
			}
			else if( minecraft->player->isSneaking() )
			{
				characterDisplayTimer[iPad] = 30;
			}
			else if( minecraft->player->isSprinting() )
			{
				characterDisplayTimer[iPad] = 30;
			}
			else if( minecraft->player->abilities.flying)
			{
				characterDisplayTimer[iPad] = 5; // quickly get rid of the player display if they stop flying
			}
			else if( characterDisplayTimer[iPad] > 0 )
			{
				--characterDisplayTimer[iPad];
			}
			bool displayCrouch = minecraft->player->isSneaking() || ( characterDisplayTimer[iPad] > 0 );
			bool displaySprint = minecraft->player->isSprinting() || ( characterDisplayTimer[iPad] > 0 );
			bool displayFlying = minecraft->player->abilities.flying || ( characterDisplayTimer[iPad] > 0 );

			if( bDisplayGui && (displayCrouch || displaySprint || displayFlying) )
			{
				EntityRenderDispatcher::instance->prepare(minecraft->level, minecraft->textures, minecraft->font, minecraft->cameraTargetPlayer, minecraft->crosshairPickMob, minecraft->options, a);
				glEnable(GL_RESCALE_NORMAL);
				glEnable(GL_COLOR_MATERIAL);

				// 4J - TomK now using safe zone values directly instead of the magic number calculation that lived here before (which only worked for medium scale, the other two were off!)
				int xo = iSafezoneXHalf + 10; // TODO: fix relative scaling for atrocious aspect ratios
				int yo = iSafezoneTopYHalf + 10; // TODO: fix relative scaling for atrocious aspect ratios

#ifdef __PSVITA__
				// align directly with corners, there are no safe zones on vita
				xo = 10;
				yo = 10;
#endif

				glPushMatrix();
				glTranslatef(static_cast<float>(xo), static_cast<float>(yo), 50);
				
				// correct paper doll aspect ratio
				float ss = 12.0f;
				float aspectScaleX = 1.0f;
				float aspectScaleY = 1.0f;

				extern int g_rScreenWidth;
				extern int g_rScreenHeight;

				if (g_rScreenWidth > 0 && g_rScreenHeight > 0) {
					float screenAspect = (float)g_rScreenWidth / (float)g_rScreenHeight;
					const float targetAspect = 16.0f / 9.0f;
					
					// apply correction if window is not already at a 16:9 aspect ratio
					if (fabs(screenAspect - targetAspect) > 0.01f) {
						if (screenAspect > targetAspect)
							aspectScaleX = targetAspect / screenAspect;
						else
							aspectScaleY = screenAspect / targetAspect;
					}
				}

				glScalef(-ss * aspectScaleX, ss * aspectScaleY, ss);
				glRotatef(180, 0, 0, 1);

				float oyr = minecraft->player->yRot;
				float oyrO = minecraft->player->yRotO;
				float oxr = minecraft->player->xRot;
				int ofire = minecraft->player->onFire;
				bool ofireflag = minecraft->player->getSharedFlag(Entity::FLAG_ONFIRE);

				float xd = -40;
				float yd = 10;

				// 4J Stu - This is all based on the inventory player renderer, with changes to ensure that capes render correctly
				// by minimising the changes to member variables of the player which are all related

				glRotatef(45 + 90, 0, 1, 0);
				Lighting::turnOn();
				glRotatef(-45 - 90, 0, 1, 0);

				const float xRotAngle = -(float) atan(yd / 40.0f) * 20;
				glRotatef(xRotAngle, 1, 0, 0);
				float bodyRot = (minecraft->player->yBodyRotO + (minecraft->player->yBodyRot - minecraft->player->yBodyRotO));
				// Fixed rotation angle of degrees, adjusted by bodyRot to negate the rotation that occurs in the renderer
				// bodyRot in the rotation below is a simplification of "180 - (180 - bodyRot)" where the first 180 is EntityRenderDispatcher::instance->playerRotY that we set below
				// and (180 - bodyRot) is the angle of rotation that is performed within the mob renderer
				glRotatef( bodyRot - ( (float) atan(xd / 40.0f) * 20), 0, 1, 0);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

				// Set head rotation to body rotation to make head static
				minecraft->player->yRot = bodyRot;
				minecraft->player->yRotO = minecraft->player->yRot;
				minecraft->player->xRot = xRotAngle;

				minecraft->player->onFire = 0;
				minecraft->player->setSharedFlag(Entity::FLAG_ONFIRE, false);

				// 4J - TomK don't offset the player. it's easier to align it with the safe zones that way!
				//glTranslatef(0, minecraft->player->heightOffset, 0);
				glTranslatef(0, 0, 0);
				EntityRenderDispatcher::instance->playerRotY = 180;
				EntityRenderDispatcher::instance->isGuiRender = true;
				EntityRenderDispatcher::instance->render(minecraft->player, 0, 0, 0, 0, 1);
				EntityRenderDispatcher::instance->isGuiRender = false;

				minecraft->player->yRot = oyr;
				minecraft->player->yRotO = oyrO;
				minecraft->player->xRot = oxr;
				minecraft->player->onFire = ofire;
				minecraft->player->setSharedFlag(Entity::FLAG_ONFIRE,ofireflag);
				glPopMatrix();
				Lighting::turnOff();
				glDisable(GL_RESCALE_NORMAL);
			}
		}
	}

#if RENDER_HUD
	// Moved so the opacity blend is applied to it
	if (bDisplayGui && minecraft->gameMode->hasExperience() && minecraft->player->experienceLevel > 0)
	{
		if (true)
		{
			bool blink = false;
			int col = blink ? 0xffffff : 0x80ff20;
			wchar_t formatted[10];
			swprintf(formatted, 10, L"%d",minecraft->player->experienceLevel);

			wstring str = formatted;
			int x = iWidthOffset + (screenWidth - font->width(str)) / 2;
			int y = screenHeight - iSafezoneYHalf - iTooltipsYOffset;
			// If we're in creative mode, we don't need to offset the XP display so much
			if (minecraft->gameMode->canHurtPlayer())
			{
				y-=18;
			}
			else
			{
				y-=13;
			}

			if(bTwoPlayerSplitscreen)
			{
				y+=iHeightOffset;
			}
			//int y = screenHeight - 31 - 4;
			font->draw(str, x + 1, y, 0x000000);
			font->draw(str, x - 1, y, 0x000000);
			font->draw(str, x, y + 1, 0x000000);
			font->draw(str, x, y - 1, 0x000000);
			// 			font->draw(str, x + 1, y + 1, 0x000000);
			// 			font->draw(str, x - 1, y + 1, 0x000000);
			// 			font->draw(str, x + 1, y - 1, 0x000000);
			// 			font->draw(str, x - 1, y - 1, 0x000000);
			font->draw(str, x, y, col);
		}
	}
#endif // RENDER_HUD

	// 4J - added to disable blends, which we have enabled previously to allow gui fading
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // if the player is falling asleep we render a dark overlay
    if (minecraft->player->getSleepTimer() > 0)
	{
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_ALPHA_TEST);
        int timer = minecraft->player->getSleepTimer();
        float amount = static_cast<float>(timer) / static_cast<float>(Player::SLEEP_DURATION);
        if (amount > 1)
		{
            // waking up
            amount = 1.0f - (static_cast<float>(timer - Player::SLEEP_DURATION) / static_cast<float>(Player::WAKE_UP_DURATION));
        }

        int color = static_cast<int>(220.0f * amount) << 24 | (0x101020);
        fill(0, 0, screenWidth/fScaleFactorWidth, screenHeight/fScaleFactorHeight, color);
        glEnable(GL_ALPHA_TEST);
        glEnable(GL_DEPTH_TEST);
    }

	// 4J-PB - Request from Mojang to have a red death screen
	if (!minecraft->player->isAlive())
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		int timer = minecraft->player->getDeathFadeTimer();
		float amount = static_cast<float>(timer) / static_cast<float>(Player::DEATHFADE_DURATION);

		int color = static_cast<int>(220.0f * amount) << 24 | (0x200000);
		fill(0, 0, screenWidth/fScaleFactorWidth, screenHeight/fScaleFactorHeight, color);
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_DEPTH_TEST);

	}


	//        {
	//            String str = "" + minecraft.player.getFoodData().getExhaustionLevel() + ", " + minecraft.player.getFoodData().getSaturationLevel();
	//            int x = (screenWidth - font.width(str)) / 2;
	//            int y = screenHeight - 64;
	//            font.draw(str, x + 1, y, 0xffffff);
	//        }


	lastTickA = a;
	// 4J Stu - This is now displayed in a xui scene
#if 0
// Jukebox CD message
    if (overlayMessageTime > 0)
	{
        float t = overlayMessageTime - a;
        int alpha = (int) (t * 256 / 20);
        if (alpha > 255) alpha = 255;
        if (alpha > 0)
		{
            glPushMatrix();

			if(bTwoPlayerSplitscreen)
			{
				glTranslatef((float)((screenWidth / 2)+iWidthOffset), ((float)(screenHeight+iHeightOffset)) - iTooltipsYOffset -12 -iSafezoneYHalf, 0);
			}
			else
			{
				glTranslatef(((float)screenWidth) / 2, ((float)screenHeight) - iTooltipsYOffset - 12 -iSafezoneYHalf, 0);
			}
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            int col = 0xffffff;
            if (animateOverlayMessageColor)
			{
                col = Color::HSBtoRGB(t / 50.0f, 0.7f, 0.6f) & 0xffffff;
            }
			// 4J-PB - this is the string displayed when cds are placed in a jukebox
            font->draw(overlayMessageString,-font->width(overlayMessageString) / 2, -20, col + (alpha << 24));
            glDisable(GL_BLEND);
            glPopMatrix();
        }
    }
#endif

    unsigned int max = 10;
    bool isChatting = false;
    if (dynamic_cast<ChatScreen *>(minecraft->screen) != NULL)
	{
        max = 20;
        isChatting = true;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_ALPHA_TEST);

#if 0 // defined(_WINDOWS64) // Temporarily disable this chat in favor of iggy chat until we have better visual parity
    glPushMatrix();
    glTranslatef(0.0f, static_cast<float>(screenHeight - iSafezoneYHalf - iTooltipsYOffset - 16 - 3 + 22) - 24.0f, 0.0f);

	if(bDisplayGui)
	{
		int iPad=minecraft->player->GetXboxPad();
		for (unsigned int i = 0; i < guiMessages[iPad].size() && i < max; i++)
		{
			if (guiMessages[iPad][i].ticks < 20 * 10 || isChatting)
			{
				double t = guiMessages[iPad][i].ticks / (20 * 10.0);
				t = 1 - t;
				t = t * 10;
				if (t < 0) t = 0;
				if (t > 1) t = 1;
				t = t * t;
				int alpha = static_cast<int>(255 * t);
				if (isChatting) alpha = 255;

				if (alpha > 0)
				{
					int x = iSafezoneXHalf+2;
					int y = -(static_cast<int>(i)) * 9;
					if(bTwoPlayerSplitscreen)
					{
						y+= iHeightOffset;
					}

					wstring msg = guiMessages[iPad][i].string;
					this->fill(0, y - 1, screenWidth/fScaleFactorWidth, y + 8, (alpha / 2) << 24);
					glEnable(GL_BLEND);
					font->drawShadow(msg, iSafezoneXHalf+4, y, 0xffffff + (alpha << 24));
				}
			}
		}
	}
    glPopMatrix();
#endif

	// 4J Stu - Copied over but not used
#if 0
	if (minecraft.player instanceof MultiplayerLocalPlayer && minecraft.options.keyPlayerList.isDown)
	{
		ClientConnection connection = ((MultiplayerLocalPlayer) minecraft.player).connection;
		List<PlayerInfo> playerInfos = connection.playerInfos;
		int slots = connection.maxPlayers;

		int rows = slots;
		int cols = 1;
		while (rows > 20) {
			cols++;
			rows = (slots + cols - 1) / cols;
		}

		/*
		* int fakeCount = 39; while (playerInfos.size() > fakeCount)
		* playerInfos.remove(playerInfos.size() - 1); while (playerInfos.size() <
		* fakeCount) playerInfos.add(new PlayerInfo("fiddle"));
		*/

		int slotWidth = 300 / cols;
		if (slotWidth > 150) slotWidth = 150;

		int xxo = (screenWidth - cols * slotWidth) / 2;
		int yyo = 10;
		fill(xxo - 1, yyo - 1, xxo + slotWidth * cols, yyo + 9 * rows, 0x80000000);
		for (int i = 0; i < slots; i++) {
			int xo = xxo + i % cols * slotWidth;
			int yo = yyo + i / cols * 9;

			fill(xo, yo, xo + slotWidth - 1, yo + 8, 0x20ffffff);
			glColor4f(1, 1, 1, 1);
			glEnable(GL_ALPHA_TEST);

			if (i < playerInfos.size()) {
				PlayerInfo pl = playerInfos.get(i);
				font.drawShadow(pl.name, xo, yo, 0xffffff);
				minecraft.textures.bind(minecraft.textures.loadTexture("/gui/icons.png"));
				int xt = 0;
				int yt = 0;
				xt = 0;
				yt = 0;
				if (pl.latency < 0) yt = 5;
				else if (pl.latency < 150) yt = 0;
				else if (pl.latency < 300) yt = 1;
				else if (pl.latency < 600) yt = 2;
				else if (pl.latency < 1000) yt = 3;
				else yt = 4;

				blitOffset += 100;
				blit(xo + slotWidth - 12, yo, 0 + xt * 10, 176 + yt * 8, 10, 8);
				blitOffset -= 100;
			}
		}
	}
#endif

	if(bDisplayGui && bTwoPlayerSplitscreen)
	{
		// pop the scaled matrix
		glPopMatrix();
	}

#ifndef _FINAL_BUILD
	MemSect(31);
    if (true)
    {
        // Real window dimensions updated on every WM_SIZE — always current
        extern int g_rScreenWidth;
        extern int g_rScreenHeight;

        // Set up a fresh projection using physical pixel coordinates so the debug
        // text is never distorted regardless of aspect ratio, splitscreen layout,
        // or menu state. 1 coordinate unit = 1 physical pixel.
        // Compute the actual viewport dimensions for this player's screen section.
        // glOrtho must match the viewport exactly for 1 unit = 1 physical pixel.
        int vpW = g_rScreenWidth;
        int vpH = g_rScreenHeight;
        switch (minecraft->player->m_iScreenSection)
        {
        case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
        case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
            vpH /= 2;
            break;
        case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
        case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
            vpW /= 2;
            break;
        case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
        case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
        case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
        case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
            vpW /= 2;
            vpH /= 2;
            break;
        default: // VIEWPORT_TYPE_FULLSCREEN
            break;
        }

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, vpW, vpH, 0, 1000, 3000);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0, 0, -2000);

        // Font was designed for guiScale px/unit; scale up so characters appear
        // at the same physical size as the rest of the HUD at 0.5x.
        const float fontScale = static_cast<float>(guiScale) * 1.0f;
        const int debugLeft = 1;
        const int debugTop = 1;

        glTranslatef(static_cast<float>(debugLeft), static_cast<float>(debugTop), 0.f);
        glScalef(fontScale, fontScale, 1.f);
        glTranslatef(static_cast<float>(-debugLeft), static_cast<float>(-debugTop), 0.f);

        vector<wstring> lines;

        // Only show version/branch for player 0 to avoid cluttering each splitscreen viewport
        if (iPad == 0 && ClientConstants::SHOW_VERSION_WATERMARK)
        {
            lines.push_back(ClientConstants::VERSION_STRING);
            lines.push_back(ClientConstants::BRANCH_STRING);
        }

        const eXuiAction action = app.GetXuiAction(iPad);
        const bool capturingSaveThumbnail =
            action == eAppAction_ExitWorldCapturedThumbnail
            || action == eAppAction_SaveGameCapturedThumbnail
            || action == eAppAction_AutosaveSaveGameCapturedThumbnail;
        if (!capturingSaveThumbnail && minecraft->options->renderDebug
            && minecraft->player != nullptr && minecraft->level != nullptr)
        {
            lines.push_back(std::wstring(VER_BRANCHVERSION_STR_W) + L"/" + std::wstring(VER_PRODUCTVERSION_STR_W));
            lines.push_back(minecraft->fpsString);
            lines.push_back(L"E: " + std::to_wstring(minecraft->level->getAllEntities().size()));
            int renderDistance = app.GetGameSettings(iPad, eGameSetting_RenderDistance);
            // Calculate the chunk sections using 16 * (2n + 1)^2
            lines.push_back(L"C: " + std::to_wstring(16 * (2 * renderDistance + 1) * (2 * renderDistance + 1)) + L" D: " + std::to_wstring(renderDistance));
            lines.push_back(minecraft->gatherStats4());

            // Dimension
            wstring dimension = L"unknown";
            switch (minecraft->player->dimension)
            {
            case -1:
                dimension = L"minecraft:the_nether";
                break;
            case 0:
                dimension = L"minecraft:overworld";
                break;
            case 1:
                dimension = L"minecraft:the_end";
                break;
            }
            lines.push_back(dimension);

            lines.push_back(L""); // Spacer

            // Players block pos
            int xBlockPos = Mth::floor(minecraft->player->x);
            int yBlockPos = Mth::floor(minecraft->player->y);
            int zBlockPos = Mth::floor(minecraft->player->z);

            // Chunk player is in
            int xChunkPos = xBlockPos >> 4;
            int yChunkPos = yBlockPos >> 4;
            int zChunkPos = zBlockPos >> 4;

            // Players offset within the chunk
            int xChunkOffset = xBlockPos & 15;
            int yChunkOffset = yBlockPos & 15;
            int zChunkOffset = zBlockPos & 15;

            // Format the position like java with limited decumal places
            WCHAR posString[44]; // Allows upto 7 digit positions (+-9_999_999)
            swprintf(posString, 44, L"%.3f / %.5f / %.3f", minecraft->player->x, minecraft->player->y, minecraft->player->z);

            lines.push_back(L"XYZ: " + std::wstring(posString));
            lines.push_back(L"Block: " + std::to_wstring(xBlockPos) + L" " + std::to_wstring(yBlockPos) + L" " + std::to_wstring(zBlockPos));
            lines.push_back(L"Chunk: " + std::to_wstring(xChunkOffset) + L" " + std::to_wstring(yChunkOffset) + L" " + std::to_wstring(zChunkOffset) + L" in " + std::to_wstring(xChunkPos) + L" " + std::to_wstring(yChunkPos) + L" " + std::to_wstring(zChunkPos));

			// Wrap the yRot to 360 then adjust to (-180 to 180) range to match java
            float yRotDisplay = fmod(minecraft->player->yRot, 360.0f);
            if (yRotDisplay >  180.0f) yRotDisplay -= 360.0f;
            if (yRotDisplay < -180.0f) yRotDisplay += 360.0f;
            // Generate the angle string in the format "yRot / xRot" with one decimal place, similar to java edition
            WCHAR angleString[16];
            swprintf(angleString, 16, L"%.1f / %.1f", yRotDisplay, minecraft->player->xRot);

			// Work out the named direction
            int direction = Mth::floor(minecraft->player->yRot * 4.0f / 360.0f + 0.5) & 0x3;
            const wchar_t* cardinals[] = { L"south", L"west", L"north", L"east" };
            lines.push_back(L"Facing: " + std::wstring(cardinals[direction]) + L" (" + angleString + L")");

			// We have to limit y to 256 as we don't get any information past that
			// target block state
			if (minecraft->hitResult != nullptr && minecraft->hitResult->type == HitResult::TILE)
			{
				int hx = minecraft->hitResult->x;
				int hy = minecraft->hitResult->y;
				int hz = minecraft->hitResult->z;
				if (minecraft->level != NULL && minecraft->level->hasChunkAt(hx, hy, hz))
				{
					int tid = minecraft->level->getTile(hx, hy, hz);
					if (tid >= 0 && tid < Tile::TILE_NUM_COUNT)
					{
						Tile *t = Tile::tiles[tid];
						if (t != nullptr)
						{
							Tile::BlockState st = t->getBlockState(minecraft->level, hx, hy, hz);
							// check registry so we dont end up with random integers
							std::wstring decoded = BlockStateDecoderRegistry::decode(tid, st.value);
				
							if (decoded.empty()) {
								if (tid == Tile::wooden_door_Id || tid == Tile::iron_door_Id || tid == Tile::spruce_door_Id || tid == Tile::birch_door_Id || tid == Tile::jungle_door_Id || tid == Tile::acacia_door_Id || tid == Tile::dark_oak_door_Id || tid == Tile::bamboo_door_Id) {
									decoded = BlockStateDecoder::doorPropsToString(BlockStateDecoder::decodeDoor(st.value));
								}
							}

							if (!decoded.empty())
							{
								std::map<std::wstring, std::wstring> props;
								std::set<std::wstring> shownProps;
								auto appendProp = [&](const std::wstring &key) {
									auto it = props.find(key);
									if (it != props.end()) {
										lines.push_back(key + L": " + it->second);
										shownProps.insert(key);
									}
								};
								size_t start = 0;
								while (start < decoded.size()) {
									size_t pos = decoded.find(L'\n', start);
									std::wstring line = (pos == std::wstring::npos) ? decoded.substr(start) : decoded.substr(start, pos - start);
									size_t colon = line.find(L':');
									if (colon != std::wstring::npos) {
										std::wstring key = line.substr(0, colon);
										std::wstring val = line.substr(colon + 1);
										auto trim = [](std::wstring &s) {
											size_t i = 0;
											while (i < s.size() && iswspace(s[i])) ++i;
											if (i) s.erase(0, i);
											// right
											if (!s.empty()) {
												size_t j = s.size() - 1;
												while (j != (size_t)-1 && iswspace(s[j])) --j;
												s.erase(j + 1);
											}
										};
										trim(key);
										trim(val);
										props[key] = val;
									}
									if (pos == std::wstring::npos) break;
									start = pos + 1;
								}
								lines.push_back(L"State:");
								appendProp(L"age");
								appendProp(L"moisture");
								appendProp(L"facing");
								appendProp(L"part");
								appendProp(L"occupied");
								appendProp(L"north");
								appendProp(L"south");
								appendProp(L"east");
								appendProp(L"west");
								appendProp(L"type");
								appendProp(L"variant");
								appendProp(L"axis");
								appendProp(L"hinge");
								appendProp(L"half");
								appendProp(L"shape");
								appendProp(L"up");
								appendProp(L"extended");
								appendProp(L"open");
								appendProp(L"in_wall");
								appendProp(L"attached");
								appendProp(L"powered");
								appendProp(L"power");
								appendProp(L"triggered");
								appendProp(L"explode");
								appendProp(L"bites");
								appendProp(L"mode");
								appendProp(L"delay");
								appendProp(L"enabled");
								appendProp(L"eye");
								appendProp(L"bottle_0");
								appendProp(L"bottle_1");
								appendProp(L"bottle_2");
								appendProp(L"has_record");
								for (const auto &entry : props) {
									if (shownProps.find(entry.first) == shownProps.end()) {
										lines.push_back(entry.first + L": " + entry.second);
									}
								}
							}
						}
					}
				}
			}
            if (minecraft->level != NULL && minecraft->level->hasChunkAt(xBlockPos, fmod(yBlockPos, 256), zBlockPos))
            {
                LevelChunk *chunkAt = minecraft->level->getChunkAt(xBlockPos, zBlockPos);
                if (chunkAt != NULL)
                {
                    int skyLight = chunkAt->getBrightness(LightLayer::Sky, xChunkOffset, yChunkOffset, zChunkOffset);
                    int blockLight = chunkAt->getBrightness(LightLayer::Block, xChunkOffset, yChunkOffset, zChunkOffset);
                    int maxLight = fmax(skyLight, blockLight);
                    lines.push_back(L"Light: " + std::to_wstring(maxLight) + L" (" + std::to_wstring(skyLight) + L" sky, " + std::to_wstring(blockLight) + L" block)");

                    lines.push_back(L"CH S: " + std::to_wstring(chunkAt->getHeightmap(xChunkOffset, zChunkOffset)));

                    Biome *biome = chunkAt->getBiome(xChunkOffset, zChunkOffset, minecraft->level->getBiomeSource());
					if (biome != nullptr && biome->id >= 0) 
						lines.push_back(L"Biome: " + biome->m_name + L" (" + std::to_wstring(biome->id) + L")");
					else
						lines.push_back(L"Biome: Unknown");
                    
                    lines.push_back(L"Difficulty: " + std::to_wstring(minecraft->level->difficulty) + L" (Day " + std::to_wstring(minecraft->level->getGameTime() / Level::TICKS_PER_DAY) + L")");
                }
            }

			// This is all LCE only stuff, it was never on java
            lines.push_back(L""); // Spacer
            lines.push_back(L"Seed: " + std::to_wstring(minecraft->level->getLevelData()->getSeed()));
            lines.push_back(minecraft->gatherStats1()); // Time to autosave
            lines.push_back(minecraft->gatherStats2()); // Empty currently - CPlatformNetworkManagerStub::GatherStats()
            lines.push_back(minecraft->gatherStats3()); // RTT

#ifdef _DEBUG // Only show terrain features in debug builds not release
			
			// No point trying to render this when not in the overworld
            if (minecraft->level->dimension->id == 0)
            {
                wstring wfeature[eTerrainFeature_Count];
                wfeature[eTerrainFeature_Stronghold] = L"Stronghold: ";
                wfeature[eTerrainFeature_Mineshaft] = L"Mineshaft: ";
                wfeature[eTerrainFeature_Village] = L"Village: ";
                wfeature[eTerrainFeature_Ravine] = L"Ravine: ";
                wfeature[eTerrainFeature_OceanMonument] = L"Monument: ";

                // maxW in font units: physical width divided by font scale
                float maxW = (static_cast<float>(g_rScreenWidth) - debugLeft - 8) / fontScale;
                float maxWForContent = maxW - static_cast<float>(font->width(L"..."));
                bool truncated[eTerrainFeature_Count] = {};

                for (size_t i = 0; i < app.m_vTerrainFeatures.size(); i++)
                {
                    FEATURE_DATA *pFeatureData = app.m_vTerrainFeatures[i];
                    int type = pFeatureData->eTerrainFeature;
                    if (type < eTerrainFeature_Stronghold || type > eTerrainFeature_OceanMonument) continue;
                    if (truncated[type]) continue;

                    wstring itemInfo = L"[" + std::to_wstring(pFeatureData->x * 16) + L", " + std::to_wstring(pFeatureData->z * 16) + L"] ";
                    if (font->width(wfeature[type] + itemInfo) <= maxWForContent)
                    {
                        wfeature[type] += itemInfo;
                    }
                    else
                    {
                        wfeature[type] += L"...";
                        truncated[type] = true;
                    }
                }

                lines.push_back(L""); // Spacer
                for (int i = eTerrainFeature_Stronghold; i <= static_cast<int>(eTerrainFeature_OceanMonument); i++)
                {
                    lines.push_back(wfeature[i]);
                }
                lines.push_back(L""); // Spacer
            }
#endif
        }

		// Disable the depth test so the text shows on top of the paperdoll
        glDisable(GL_DEPTH_TEST);
#ifdef _WINDOWS64
		float scaleWidth = (g_rScreenWidth / 1920.0f);
		float scaleHeight = (g_rScreenHeight / 1080.0f);

		float scale = min(scaleWidth, scaleHeight); //stop stretching

		if (scale < 0.5f) scale = 0.5f; // force minimum scale
        if (scale > 1.2f) // resolutions over 1296 pixels tall
        {
			scale = scale - 0.33f; // tame overscaling on 1440p
		}
			
		glScalef(scale, scale, 1);
#endif
        // Loop through the lines and draw them all on screen
        int yPos = debugTop;
        for (const auto &line : lines)
        {
            drawString(font, line, debugLeft, yPos, 0xffffff);
            yPos += 10;
        }

#ifdef _WINDOWS64
		glScalef(1, 1, 1);
#endif
		// Restore the depth test
        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
    MemSect(0);
#endif

    // Arcade overlay: draw each cabinet as a readable, self-contained mini-game HUD.
    // The owning LocalPlayer carries its own session, preserving split-screen isolation.
    shared_ptr<LocalPlayer> arcadePlayer = dynamic_pointer_cast<LocalPlayer>(minecraft->player);
    if (arcadePlayer != nullptr && arcadePlayer->getArcadeGameSession().isActive())
    {
        ArcadeGameSession &game = arcadePlayer->getArcadeGameSession();
        game.setViewportSize(static_cast<float>(screenWidth), static_cast<float>(screenHeight));

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_ALPHA_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        fill(0, 0, screenWidth, screenHeight, 0xE50A1119);

        // The legacy Font renderer depends on texture/alpha/blend state that GUI fills and
        // item renderers may change. Re-establish it for every label and use literal drawing
        // so HUD text doesn't get interpreted as formatting codes.
        auto arcadeText = [&](const wstring &text, int x, int y, int color)
        {
            if (font == nullptr || text.empty()) return;
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_ALPHA_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            drawStringLiteral(font, text, x, y, color);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        };
        auto arcadeCenteredText = [&](const wstring &text, int x, int y, int color)
        {
            if (font == nullptr || text.empty()) return;
            arcadeText(text, x - font->width(text) / 2, y, color);
        };

        // Unlike drawItem(), this uses the terrain/block atlas UVs directly. It is used for
        // cactus pipes and obsidian walls so they look like Minecraft blocks, not inventory icons.
        auto drawTerrainTiledRect = [&](Icon *icon, int x, int y, int width, int height)
        {
            if (icon == nullptr || width <= 0 || height <= 0) return;
            minecraft->textures->bindTexture(&TextureAtlas::LOCATION_BLOCKS);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_ALPHA_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            Tesselator *t = Tesselator::getInstance();
            t->begin();
            for (int dy = 0; dy < height; dy += 16)
            {
                int tileH = (std::min)(16, height - dy);
                float v0 = icon->getV0(true);
                float v1 = icon->getV(tileH, true);
                for (int dx = 0; dx < width; dx += 16)
                {
                    int tileW = (std::min)(16, width - dx);
                    float u0 = icon->getU0(true);
                    float u1 = icon->getU(tileW, true);
                    float x0 = static_cast<float>(x + dx);
                    float y0 = static_cast<float>(y + dy);
                    float x1 = x0 + static_cast<float>(tileW);
                    float y1 = y0 + static_cast<float>(tileH);
                    t->vertexUV(x0, y1, 0.0f, u0, v1);
                    t->vertexUV(x1, y1, 0.0f, u1, v1);
                    t->vertexUV(x1, y0, 0.0f, u1, v0);
                    t->vertexUV(x0, y0, 0.0f, u0, v0);
                }
            }
            t->end();
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        };
        auto drawItem = [&](int itemId, float x, float y, float scale)
        {
            if (itemId >= 0 && itemId < Item::ITEM_NUM_COUNT && Item::items[itemId] != nullptr)
            {
                glEnable(GL_TEXTURE_2D);
                glEnable(GL_ALPHA_TEST);
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                shared_ptr<ItemInstance> iconItem = std::make_shared<ItemInstance>(itemId, 1, 0);
                itemRenderer->renderGuiItem(minecraft->font, minecraft->textures, iconItem, x, y, scale, 1.0f);
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            }
        };
        auto fmtDouble = [](double value) -> wstring
        {
            if (value >= 1000000000.0) return std::to_wstring(static_cast<long long>(value / 1000000000.0)) + L"B";
            if (value >= 1000000.0) return std::to_wstring(static_cast<long long>(value / 1000000.0)) + L"M";
            if (value >= 1000.0) return std::to_wstring(static_cast<long long>(value / 1000.0)) + L"K";
            return std::to_wstring(static_cast<long long>(value));
        };

        // Shared top strip: opaque backing makes the title and exit hint readable even
        // after rendering a model or a stitched block texture.
        fill(6, 4, screenWidth - 6, 25, 0xF51B2A39);
        arcadeCenteredText(L"COPIUM ARCADE", screenWidth / 2, 8, 0xFFFFFFFF);
        arcadeText(L"ESC: EXIT", (std::max)(8, screenWidth - 82), 9, 0xFFFFD166);

        if (game.getGameType() == ARCADE_GAME_COOKIE_MINER)
        {
            int left = 8, top = 30, bottom = screenHeight - 25;
            int third = (std::max)(1, (screenWidth - 24) / 3);
            int xCookie = left, xStats = left + third + 4, xShop = left + third * 2 + 8;
            int shopW = screenWidth - 8 - xShop;

            fill(xCookie, top, xCookie + third, bottom, 0xFF24394B);
            fill(xStats, top, xStats + third, bottom, 0xFF182A39);
            fill(xShop, top, screenWidth - 8, bottom, 0xFF24394B);
            fill(xCookie, top, xCookie + third, top + 24, 0xFF365268);
            fill(xStats, top, xStats + third, top + 24, 0xFF2A4052);
            fill(xShop, top, screenWidth - 8, top + 24, 0xFF365268);
            arcadeCenteredText(L"COOKIE MINER", xCookie + third / 2, top + 8, 0xFFFFE0A0);
            arcadeCenteredText(L"CLICK COOKIE", xCookie + third / 2, top + 33, 0xFFFFFFFF);
            arcadeCenteredText(L"+1 COOKIE / CLICK", xCookie + third / 2, top + 46, 0xFFBFE8FF);
            drawItem(Item::cookie_Id, static_cast<float>(xCookie + third / 2 - 25), static_cast<float>(top + (bottom - top) / 2 - 14), 3.0f);
            fill(xCookie + 8, bottom - 33, xCookie + third - 8, bottom - 8, 0xE51A2835);
            arcadeCenteredText(L"CLICK TO BAKE", xCookie + third / 2, bottom - 25, 0xFFFFFFFF);

            fill(xStats + 8, top + 32, xStats + third - 8, top + 76, 0xFF101D28);
            arcadeCenteredText(L"COOKIES", xStats + third / 2, top + 7, 0xFFFFFFFF);
            arcadeCenteredText(fmtDouble(game.getCookies()), xStats + third / 2, top + 39, 0xFFFFD166);
            arcadeCenteredText(L"TOTAL COOKIES", xStats + third / 2, top + 60, 0xFFDAE7F0);
            arcadeText(L"Rate / second", xStats + 12, top + 91, 0xFFCFDCE5);
            arcadeText(fmtDouble(game.getCookiesPerSecond()), xStats + 12, top + 105, 0xFFFFFFFF);
            arcadeText(L"Click value", xStats + 12, top + 130, 0xFFCFDCE5);
            arcadeText(L"x" + std::to_wstring(static_cast<int>(game.getClickYield())), xStats + 12, top + 144, 0xFFFFFFFF);
            if (game.getGoldenBoostTicks() > 0)
            {
                fill(xStats + 7, top + 170, xStats + third - 7, top + 195, 0xFF755322);
                arcadeCenteredText(L"GOLDEN x7: " + std::to_wstring(game.getGoldenBoostTicks() / 20) + L"s", xStats + third / 2, top + 178, 0xFFFFE0A0);
            }
            fill(xStats + 8, bottom - 33, xStats + third - 8, bottom - 8, 0xE5101D28);
            arcadeCenteredText(L"B / RIGHT-CLICK: BUY", xStats + third / 2, bottom - 25, 0xFFFFFFFF);

            arcadeCenteredText(L"UPGRADE SHOP", xShop + shopW / 2, top + 8, 0xFFFFFFFF);
            for (int u = 0; u < 4; ++u)
            {
                int rowY = top + 25 + u * 42;
                bool selected = game.getSelectedUpgrade() == u;
                fill(xShop + 3, rowY - 2, screenWidth - 11, rowY + 37, selected ? 0xFF596E80 : 0xFF1B2B38);
                if (selected) fill(xShop + 3, rowY - 2, xShop + 6, rowY + 37, 0xFFFFD166);
                drawItem(game.getUpgradeIconId(u), static_cast<float>(xShop + 7), static_cast<float>(rowY + 8), 1.0f);
                arcadeText(game.getUpgradeName(u), xShop + 29, rowY + 1, 0xFFFFFFFF);
                arcadeText(L"Owned " + std::to_wstring(game.getUpgradeOwned(u)), xShop + 29, rowY + 12, 0xFFDAE7F0);
                arcadeText(L"Cost " + fmtDouble(game.getUpgradeCost(u)), xShop + 29, rowY + 23, 0xFFFFD166);
            }
            if (game.isGoldenAppleVisible())
            {
                // The target stays in the same top strip used by primaryActionAtCursor().
                int gx = static_cast<int>(game.getGoldenAppleX() * screenWidth);
                fill(8, top + 1, (std::min)(screenWidth - 8, 250), top + 23, 0xF56A4B14);
                drawItem(Item::golden_apple_Id, static_cast<float>(gx), static_cast<float>(top + 4), 1.0f);
                arcadeText(L"GOLDEN APPLE! CLICK FOR 7x", 32, top + 7, 0xFFFFFFFF);
            }
            fill(6, screenHeight - 21, screenWidth - 6, screenHeight - 3, 0xF51B2A39);
            arcadeCenteredText(L"W/S OR STICK: SELECT   |   B / RIGHT-CLICK: BUY   |   SHIFT / ESC: EXIT", screenWidth / 2, screenHeight - 16, 0xFFFFFFFF);
        }
        else if (game.getGameType() == ARCADE_GAME_FLAPPY_CRAFT)
        {
            int left = 10, top = 28, right = screenWidth - 10, bottom = screenHeight - 28;
            int playW = right - left, playH = bottom - top - 12;
            fill(left, top, right, bottom, 0xFF95D9F4);
            // Light-blue concrete uses the actual terrain atlas. If unavailable, the sky fill remains.
            Icon *skyIcon = (Tile::concrete != nullptr) ? Tile::concrete->getTexture(Facing::UP, DyePowderItem::LIGHT_BLUE) : nullptr;
            if (skyIcon != nullptr) drawTerrainTiledRect(skyIcon, left, top, playW, playH);

            // Clouds add depth without turning the playfield into a patchwork of UI squares.
            int cloudDrift = (tickCount * 2) % (playW + 100);
            auto drawCloud = [&](int cx, int cy)
            {
                fill(cx, cy + 5, cx + 38, cy + 17, 0xFFF2FAFF);
                fill(cx + 7, cy, cx + 29, cy + 18, 0xFFF2FAFF);
                fill(cx + 25, cy + 5, cx + 49, cy + 17, 0xFFF2FAFF);
            };
            drawCloud(left + playW - cloudDrift, top + 52);
            drawCloud(left + ((cloudDrift + playW / 2) % (playW + 70)) - 35, top + 118);
            fill(left, bottom - 12, right, bottom, 0xFF6E9F45);
            fill(left, bottom - 12, right, bottom - 8, 0xFF98C85E);

            const std::vector<ArcadeGameSession::Obstacle> &obs = game.getObstacles();
            Icon *cactusSide = (Tile::cactus != nullptr) ? Tile::cactus->getTexture(Facing::NORTH) : nullptr;
            Icon *cactusTop = (Tile::cactus != nullptr) ? Tile::cactus->getTexture(Facing::UP) : nullptr;
            Icon *cactusBottom = (Tile::cactus != nullptr) ? Tile::cactus->getTexture(Facing::DOWN) : nullptr;
            int pipeW = (std::max)(32, static_cast<int>(playW * 0.12f));
            for (size_t i = 0; i < obs.size(); ++i)
            {
                int ox = left + static_cast<int>(obs[i].x * playW);
                int gapTop = top + static_cast<int>((obs[i].gapY - obs[i].gapHeight * 0.5f) * playH);
                int gapBottom = top + static_cast<int>((obs[i].gapY + obs[i].gapHeight * 0.5f) * playH);
                int upperH = (std::max)(0, gapTop - top);
                int lowerH = (std::max)(0, (bottom - 12) - gapBottom);
                // Dark outlines make the obstacle silhouette clear, while the body and the
                // gap-side endcaps come from cactus_side, cactus_bottom and cactus_top UVs.
                if (upperH > 0)
                {
                    fill(ox - 2, top, ox + pipeW + 2, gapTop, 0xFF244D2B);
                    int capH = (std::min)(16, upperH);
                    drawTerrainTiledRect(cactusSide, ox, top, pipeW, upperH - capH);
                    drawTerrainTiledRect(cactusBottom, ox, gapTop - capH, pipeW, capH);
                }
                if (lowerH > 0)
                {
                    fill(ox - 2, gapBottom, ox + pipeW + 2, bottom - 12, 0xFF244D2B);
                    int capH = (std::min)(16, lowerH);
                    drawTerrainTiledRect(cactusTop, ox, gapBottom, pipeW, capH);
                    drawTerrainTiledRect(cactusSide, ox, gapBottom + capH, pipeW, lowerH - capH);
                }
                if (obs[i].hasApple && !obs[i].appleCollected)
                    drawItem(Item::golden_apple_Id, static_cast<float>(ox + pipeW / 2 - 8), static_cast<float>(top + obs[i].appleY * playH - 8.0f), 1.0f);
            }

            int birdX = left + static_cast<int>(0.24f * playW);
            int birdY = top + static_cast<int>(game.getBirdY() * playH);
            {
                static ChickenModel arcadeChickenModel;
                float birdSize = (std::max)(18.0f, (std::min)(28.0f, screenWidth * 0.045f));
                glPushMatrix();
                glTranslatef(static_cast<float>(birdX), static_cast<float>(birdY - static_cast<int>(birdSize)), 0.0f);
                glScalef(birdSize, birdSize, birdSize);
                // ChickenModel's beak points toward local -Z; -90 degrees turns it right on screen.
                glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glEnable(GL_TEXTURE_2D);
                minecraft->textures->bind(minecraft->textures->loadTexture(TN_MOB_CHICKEN));
                float wingFlap = (tickCount % 12 < 6) ? 0.28f : -0.28f;
                arcadeChickenModel.render(shared_ptr<Entity>(), static_cast<float>(tickCount), 0.35f,
                    wingFlap, 0.0f, -8.0f, 1.0f / 16.0f, true);
                glPopMatrix();
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            }
            fill(left + 5, top + 5, left + 166, top + 39, 0xD91B2A39);
            arcadeText(L"FLAPPY CRAFT", left + 12, top + 8, 0xFFFFFFFF);
            arcadeText(L"SCORE " + std::to_wstring(game.getScore()) + L"   BEST " + std::to_wstring(game.getFlappyHighScore()), left + 12, top + 22, 0xFFFFD166);
            if (game.hasFlappyShield())
            {
                fill(right - 160, top + 5, right - 5, top + 29, 0xE56A4B14);
                drawItem(Item::golden_apple_Id, static_cast<float>(right - 155), static_cast<float>(top + 8), 0.85f);
                arcadeText(L"SHIELD ACTIVE", right - 132, top + 12, 0xFFFFFFFF);
            }
            fill(left + 6, bottom - 32, right - 6, bottom - 16, 0xD91B2A39);
            arcadeCenteredText(L"SPACE / CLICK / A: FLAP", (left + right) / 2, bottom - 28, 0xFFFFFFFF);
            if (game.isGameOver())
            {
                int boxW = (std::min)(230, screenWidth - 24);
                fill((screenWidth - boxW) / 2 - 2, screenHeight / 2 - 42, (screenWidth + boxW) / 2 + 2, screenHeight / 2 + 40, 0xF5101720);
                fill((screenWidth - boxW) / 2, screenHeight / 2 - 40, (screenWidth + boxW) / 2, screenHeight / 2 + 38, 0xF52C3B49);
                arcadeCenteredText(L"GAME OVER", screenWidth / 2, screenHeight / 2 - 23, 0xFFFFFFFF);
                arcadeCenteredText(L"SCORE " + std::to_wstring(game.getScore()) + L"  |  BEST " + std::to_wstring(game.getFlappyHighScore()), screenWidth / 2, screenHeight / 2 - 7, 0xFFFFD166);
                arcadeCenteredText(L"CLICK / A TO RETRY", screenWidth / 2, screenHeight / 2 + 13, 0xFFFFFFFF);
            }
        }
        else
        {
            int cell = (std::max)(8, (std::min)((screenWidth - 24) / ArcadeGameSession::SNAKE_GRID_WIDTH, (screenHeight - 58) / ArcadeGameSession::SNAKE_GRID_HEIGHT));
            int boardW = cell * ArcadeGameSession::SNAKE_GRID_WIDTH;
            int boardH = cell * ArcadeGameSession::SNAKE_GRID_HEIGHT;
            int left = (screenWidth - boardW) / 2;
            int top = (std::max)(29, (screenHeight - boardH) / 2);
            fill(6, 4, screenWidth - 6, 25, 0xF51B2A39);
            arcadeCenteredText(L"MINESNAKE  |  COAL " + std::to_wstring(game.getScore()) + L"  |  BEST " + std::to_wstring(game.getSnakeHighScore()), screenWidth / 2, 8, 0xFFFFFFFF);
            fill(left, top, left + boardW, top + boardH, 0xFF101A22);

            Icon *obsidianSide = (Tile::obsidian != nullptr) ? Tile::obsidian->getTexture(Facing::NORTH) : nullptr;
            // Draw walls from the terrain atlas, not the square inventory icon.
            drawTerrainTiledRect(obsidianSide, left, top, boardW, cell);
            drawTerrainTiledRect(obsidianSide, left, top + boardH - cell, boardW, cell);
            drawTerrainTiledRect(obsidianSide, left, top + cell, cell, boardH - 2 * cell);
            drawTerrainTiledRect(obsidianSide, left + boardW - cell, top + cell, cell, boardH - 2 * cell);

            // Subtle grid lines clarify the movement cells without overwhelming the carts.
            for (int gx = 1; gx < ArcadeGameSession::SNAKE_GRID_WIDTH - 1; ++gx)
                fill(left + gx * cell, top + cell, left + gx * cell + 1, top + boardH - cell, 0x28364A57);
            for (int gy = 1; gy < ArcadeGameSession::SNAKE_GRID_HEIGHT - 1; ++gy)
                fill(left + cell, top + gy * cell, left + boardW - cell, top + gy * cell + 1, 0x28364A57);

            drawItem(Item::coal_Id, static_cast<float>(left + game.getFood().x * cell), static_cast<float>(top + game.getFood().y * cell), static_cast<float>(cell) / 16.0f);
            const std::vector<ArcadeGameSession::Cell> &snake = game.getSnake();
            for (size_t i = 0; i < snake.size(); ++i)
            {
                int iconId = (i == 0) ? Item::furnace_minecart_Id : Item::minecart_Id;
                int sx = left + snake[i].x * cell;
                int sy = top + snake[i].y * cell;
                if (i == 0) fill(sx + 1, sy + 1, sx + cell - 1, sy + cell - 1, 0xFF476B4A);
                drawItem(iconId, static_cast<float>(sx), static_cast<float>(sy), static_cast<float>(cell) / 16.0f);
            }
            fill(6, screenHeight - 21, screenWidth - 6, screenHeight - 3, 0xF51B2A39);
            arcadeCenteredText(L"WASD / ARROWS / STICK: MOVE   |   CLICK / A: RESTART   |   ESC: EXIT", screenWidth / 2, screenHeight - 16, 0xFFFFFFFF);
            if (game.isGameOver())
            {
                int boxW = (std::min)(220, screenWidth - 24);
                fill((screenWidth - boxW) / 2 - 2, screenHeight / 2 - 38, (screenWidth + boxW) / 2 + 2, screenHeight / 2 + 34, 0xF5101720);
                fill((screenWidth - boxW) / 2, screenHeight / 2 - 36, (screenWidth + boxW) / 2, screenHeight / 2 + 32, 0xF52C3B49);
                arcadeCenteredText(L"GAME OVER", screenWidth / 2, screenHeight / 2 - 18, 0xFFFFFFFF);
                arcadeCenteredText(L"COAL " + std::to_wstring(game.getScore()) + L"  |  BEST " + std::to_wstring(game.getSnakeHighScore()), screenWidth / 2, screenHeight / 2 - 2, 0xFFFFD166);
                arcadeCenteredText(L"CLICK / A TO RESTART", screenWidth / 2, screenHeight / 2 + 14, 0xFFFFFFFF);
            }
        }

        // Hardware mouse stays captured for raw input. This high-contrast pixel cursor is
        // drawn above every cabinet panel and remains local to this player's game session.
        int cursorX = static_cast<int>(game.getCursorX() * screenWidth);
        int cursorY = static_cast<int>(game.getCursorY() * screenHeight);
        for (int row = 0; row < 9; ++row)
        {
            int arrowWidth = row + 1;
            fill(cursorX - 1, cursorY + row - 1, cursorX + arrowWidth + 1, cursorY + row + 1, 0xFF101010);
            fill(cursorX, cursorY + row, cursorX + arrowWidth, cursorY + row + 1, 0xFFFFFFFF);
        }
        fill(cursorX + 4, cursorY + 6, cursorX + 8, cursorY + 14, 0xFF101010);
        fill(cursorX + 5, cursorY + 7, cursorX + 7, cursorY + 13, 0xFFFFFFFF);
        glEnable(GL_DEPTH_TEST);
    }

    glColor4f(1, 1, 1, 1);
    glDisable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
}

// Moved to the xui base scene
// void Gui::renderBossHealth(void)
// {
// 	if (EnderDragonRenderer::bossInstance == NULL) return;
//
// 	shared_ptr<EnderDragon> boss = EnderDragonRenderer::bossInstance;
// 	EnderDragonRenderer::bossInstance = NULL;
//
// 	Minecraft *pMinecraft=Minecraft::GetInstance();
//
// 	Font *font = pMinecraft->font;
//
// 	ScreenSizeCalculator ssc(pMinecraft->options, pMinecraft->width_phys, pMinecraft->height_phys);
// 	int screenWidth = ssc.getWidth();
//
// 	int w = 182;
// 	int xLeft = screenWidth / 2 - w / 2;
//
// 	int progress = (int) (boss->getSynchedHealth() / (float) boss->getMaxHealth() * (float) (w + 1));
//
// 	int yo = 12;
// 	blit(xLeft, yo, 0, 74, w, 5);
// 	blit(xLeft, yo, 0, 74, w, 5);
// 	if (progress > 0)
// 	{
// 		blit(xLeft, yo, 0, 79, progress, 5);
// 	}
//
// 	wstring msg = L"Boss health - NON LOCALISED";
// 	font->drawShadow(msg, screenWidth / 2 - font->width(msg) / 2, yo - 10, 0xff00ff);
// 	glColor4f(1, 1, 1, 1);
// 	glBindTexture(GL_TEXTURE_2D, pMinecraft->textures->loadTexture(TN_GUI_ICONS) );//"/gui/icons.png"));
//
// }

void Gui::renderPumpkin(int w, int h)
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1, 1, 1, 1);
    glDisable(GL_ALPHA_TEST);

	MemSect(31);
    minecraft->textures->bindTexture(&PUMPKIN_BLUR_LOCATION);
	MemSect(0);
    Tesselator *t = Tesselator::getInstance();
    t->begin();
    t->vertexUV(static_cast<float>(0), static_cast<float>(h), static_cast<float>(-90), static_cast<float>(0), static_cast<float>(1));
    t->vertexUV(static_cast<float>(w), static_cast<float>(h), static_cast<float>(-90), static_cast<float>(1), static_cast<float>(1));
    t->vertexUV(static_cast<float>(w), static_cast<float>(0), static_cast<float>(-90), static_cast<float>(1), static_cast<float>(0));
    t->vertexUV(static_cast<float>(0), static_cast<float>(0), static_cast<float>(-90), static_cast<float>(0), static_cast<float>(0));
    t->end();
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glColor4f(1, 1, 1, 1);

}

// neoLegacy - full-screen frost overlay, same quad as the pumpkin blur; opacity follows how frozen you are
void Gui::renderFreeze(float progress, int w, int h)
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1, 1, 1, progress);
    glDisable(GL_ALPHA_TEST);

    MemSect(31);
    minecraft->textures->bindTexture(&FREEZE_OVERLAY_LOCATION);
    MemSect(0);
    Tesselator *t = Tesselator::getInstance();
    t->begin();
    t->vertexUV(static_cast<float>(0), static_cast<float>(h), static_cast<float>(-90), static_cast<float>(0), static_cast<float>(1));
    t->vertexUV(static_cast<float>(w), static_cast<float>(h), static_cast<float>(-90), static_cast<float>(1), static_cast<float>(1));
    t->vertexUV(static_cast<float>(w), static_cast<float>(0), static_cast<float>(-90), static_cast<float>(1), static_cast<float>(0));
    t->vertexUV(static_cast<float>(0), static_cast<float>(0), static_cast<float>(-90), static_cast<float>(0), static_cast<float>(0));
    t->end();
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glColor4f(1, 1, 1, 1);
}

// neoLegacy - spyglass scope overlay, same fullscreen quad as the pumpkin blur; fully opaque like the pumpkin blur
void Gui::renderScope(int w, int h)
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1, 1, 1, 1);
    glDisable(GL_ALPHA_TEST);

	MemSect(31);
    minecraft->textures->bindTexture(&SPYSCOPE_OVERLAY_LOCATION);
	MemSect(0);
    Tesselator *t = Tesselator::getInstance();
    t->begin();
    t->vertexUV(static_cast<float>(0), static_cast<float>(h), static_cast<float>(-90), static_cast<float>(0), static_cast<float>(1));
    t->vertexUV(static_cast<float>(w), static_cast<float>(h), static_cast<float>(-90), static_cast<float>(1), static_cast<float>(1));
    t->vertexUV(static_cast<float>(w), static_cast<float>(0), static_cast<float>(-90), static_cast<float>(1), static_cast<float>(0));
    t->vertexUV(static_cast<float>(0), static_cast<float>(0), static_cast<float>(-90), static_cast<float>(0), static_cast<float>(0));
    t->end();
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glColor4f(1, 1, 1, 1);

}

void Gui::renderVignette(float br, int w, int h)
{
    br = 1 - br;
    if (br < 0) br = 0;
    if (br > 1) br = 1;
    tbr += (br - tbr) * 0.01f;

#if 0  // 4J - removed - TODO put back when we have blend functions implemented
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
    glColor4f(tbr, tbr, tbr, 1);
    glBindTexture(GL_TEXTURE_2D, minecraft->textures->loadTexture(TN__BLUR__MISC_VIGNETTE));//L"%blur%/misc/vignette.png"));
    Tesselator *t = Tesselator::getInstance();
    t->begin();
    t->vertexUV((float)(0), (float)( h), (float)( -90), (float)( 0), (float)( 1));
    t->vertexUV((float)(w), (float)( h), (float)( -90), (float)( 1), (float)( 1));
    t->vertexUV((float)(w), (float)( 0), (float)( -90), (float)( 1), (float)( 0));
    t->vertexUV((float)(0), (float)( 0), (float)( -90), (float)( 0), (float)( 0));
    t->end();
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
    glColor4f(1, 1, 1, 1);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
}

void Gui::renderTp(float br, int w, int h)
{
    if (br < 1)
	{
        br = br * br;
        br = br * br;
        br = br * 0.8f + 0.2f;
    }

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1, 1, 1, br);
	MemSect(31);
    minecraft->textures->bindTexture(&TextureAtlas::LOCATION_BLOCKS);
	MemSect(0);

	Icon *slot = Tile::portalTile->getTexture(Facing::UP);
    float u0 = slot->getU0();
    float v0 = slot->getV0();
    float u1 = slot->getU1();
    float v1 = slot->getV1();
    Tesselator *t = Tesselator::getInstance();
    t->begin();
    t->vertexUV(static_cast<float>(0), static_cast<float>(h), static_cast<float>(-90), (float)( u0), (float)( v1));
    t->vertexUV(static_cast<float>(w), static_cast<float>(h), static_cast<float>(-90), (float)( u1), (float)( v1));
    t->vertexUV(static_cast<float>(w), static_cast<float>(0), static_cast<float>(-90), (float)( u1), (float)( v0));
    t->vertexUV(static_cast<float>(0), static_cast<float>(0), static_cast<float>(-90), (float)( u0), (float)( v0));
    t->end();
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glColor4f(1, 1, 1, 1);

}

void Gui::renderSlot(int slot, int x, int y, float a)
{
    shared_ptr<ItemInstance> item = minecraft->player->inventory->items[slot];
    if (item == NULL) return;

    float pop = item->popTime - a;
    if (pop > 0)
	{
        glPushMatrix();
        float squeeze = 1 + pop / static_cast<float>(Inventory::POP_TIME_DURATION);
        glTranslatef(static_cast<float>(x + 8), static_cast<float>(y + 12), 0);
        glScalef(1 / squeeze, (squeeze + 1) / 2, 1);
        glTranslatef(static_cast<float>(-(x + 8)), static_cast<float>(-(y + 12)), 0);
    }

    itemRenderer->renderAndDecorateItem(minecraft->font, minecraft->textures, item, x, y);

    if (pop > 0)
	{
        glPopMatrix();
    }

    itemRenderer->renderGuiItemDecorations(minecraft->font, minecraft->textures, item, x, y);

}

void Gui::tick()
{
    if (overlayMessageTime > 0) overlayMessageTime--;
    tickCount++;

	for(int iPad=0;iPad<XUSER_MAX_COUNT;iPad++)
	{
		// 4J Stu - Fix for #10929 - MP LAB: Network Disconnects: Host does not receive an error message stating the client left the game when viewing the Pause Menu.
		// We don't show the guiMessages when a menu is up, so don't fade them out
		if(!ui.GetMenuDisplayed(iPad))
		{
            for (auto& it : guiMessages[iPad])
            {
				it.ticks++;
			}
		}
	}
}

void Gui::clearMessages(int iPad)
{
	if(iPad==-1)
	{
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			if(minecraft->localplayers[i])
			{
				guiMessages[i].clear();
			}
		}
	}
	else
	{
		guiMessages[iPad].clear();
	}
}

int getVisibleMessageLength(const wstring& _string) {
	int visibleMessageLength = 0;
	bool inHtmlTag = false;

	for (wchar_t _char : _string) {
		if (_char == L'<') inHtmlTag = true;
		if (_char == L'>') inHtmlTag = false;

		if (!inHtmlTag) visibleMessageLength++;
	}

	return visibleMessageLength;
}

int getVisibleIndexToRaw(const wstring& _string, size_t target) {
	int visibleMessageLength = 0;
	bool inHtmlTag = false;

	for (size_t i = 0; i < _string.size(); i++) {
		if (_string[i] == L'<') inHtmlTag = true;
		if (_string[i] == L'>') inHtmlTag = false;

		if (!inHtmlTag) {
			if (visibleMessageLength == target) return i;

			visibleMessageLength++;
		}
	}
	return _string.size();
}


void Gui::addMessage(const wstring& _string,int iPad,bool bIsDeathMessage)
{
	{ char buf[32]; sprintf_s(buf, "[CHAT] Display (pad=%d): ", iPad); OutputDebugStringA(buf); }
	OutputDebugStringW(_string.c_str());
	OutputDebugStringA("\n");
	wstring string = _string;	// 4J - Take copy of input as it is const
	//int iScale=1;

	//if((minecraft->player->m_iScreenSection==C4JRender::VIEWPORT_TYPE_SPLIT_TOP) ||
	//	(minecraft->player->m_iScreenSection==C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM))
	//{
	//	iScale=2;
	//}

 //   while (minecraft->font->width(string) > (m_iMaxMessageWidth*iScale))
	//{
 //       unsigned int i = 1;
 //       while (i < string.length() && minecraft->font->width(string.substr(0, i + 1)) <= (m_iMaxMessageWidth*iScale))
	//	{
 //           i++;
 //       }
	//	int iLast=string.find_last_of(L" ",i);

	//	// if a space was found, include the space on this line
	//	if(iLast!=i)
	//	{
	//		iLast++;
	//	}
	//	addMessage(string.substr(0, iLast), iPad);
	//	string = string.substr(iLast);
 //   }

	int maximumChars;

	switch(minecraft->player->m_iScreenSection)
	{
	case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
	case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
		if(RenderManager.IsHiDef())
		{
			maximumChars = 105;
		}
		else
		{
			maximumChars = 55;
		}
#ifdef __PSVITA__
		maximumChars = 90;
#endif
		switch(XGetLanguage())
		{
		case XC_LANGUAGE_JAPANESE:
		case XC_LANGUAGE_TCHINESE:
		case XC_LANGUAGE_KOREAN:
			if(RenderManager.IsHiDef())
			{
				maximumChars = 70;
			}
			else
			{
				maximumChars = 35;
			}
#ifdef __PSVITA__
			maximumChars = 55;
#endif
			break;
		}
		break;
	default:
		 maximumChars = 55;
		 switch(XGetLanguage())
		 {
		 case XC_LANGUAGE_JAPANESE:
		 case XC_LANGUAGE_TCHINESE:
		 case XC_LANGUAGE_KOREAN:
			 maximumChars = 35;
			 break;
		 }
		break;
	}

	while (getVisibleMessageLength(string) > maximumChars)
	{
		size_t cutOffset = getVisibleIndexToRaw(string, maximumChars);

		size_t iLast=string.find_last_of(L" ", cutOffset);
		switch(XGetLanguage())
		{
			case XC_LANGUAGE_JAPANESE:
			case XC_LANGUAGE_TCHINESE:
			case XC_LANGUAGE_KOREAN:
				iLast = cutOffset;
				break;
			default:
				iLast=string.find_last_of(L" ", cutOffset);
				break;
		}

		// if a space was found, include the space on this line
		if(iLast!=cutOffset)
		{
			iLast++;
		}
		addMessage(string.substr(0, iLast), iPad, bIsDeathMessage);
		string = string.substr(iLast);
    }

	if(iPad==-1)
	{
		// add to all
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			if(minecraft->localplayers[i] && !(bIsDeathMessage && app.GetGameSettings(i,eGameSetting_DeathMessages)==0))
			{
				guiMessages[i].insert(guiMessages[i].begin(), GuiMessage(string));
				while (guiMessages[i].size() > 50)
				{
					guiMessages[i].pop_back();
				}
			}
		}
	}
	else if(!(bIsDeathMessage && app.GetGameSettings(iPad,eGameSetting_DeathMessages)==0))
	{
		guiMessages[iPad].insert(guiMessages[iPad].begin(), GuiMessage(string));
		while (guiMessages[iPad].size() > 50)
		{
			guiMessages[iPad].pop_back();
		}
	}


}

void Gui::setLastDeathMessage(int iPad, const wstring &message)
{
	if (iPad >= 0 && iPad < XUSER_MAX_COUNT)
	{
		m_lastDeathMessage[iPad] = message;
	}
	else
	{
		for (int i = 0; i < XUSER_MAX_COUNT; i++) m_lastDeathMessage[i] = message;
	}
}

wstring Gui::getLastDeathMessage(int iPad)
{
	return (iPad >= 0 && iPad < XUSER_MAX_COUNT) ? m_lastDeathMessage[iPad] : L"";
}

// 4J Added
float Gui::getOpacity(int iPad, DWORD index)
{
	float opacityPercentage = 0;
	if (guiMessages[iPad].size() > index && guiMessages[iPad][index].ticks < 20 * 10)
	{
		double t = guiMessages[iPad][index].ticks / (20 * 10.0);
		t = 1 - t;
		t = t * 10;
		if (t < 0) t = 0;
		if (t > 1) t = 1;
		t = t * t;
		opacityPercentage = t;
	}
	return opacityPercentage;
}

//just like java functionality it overwrites the jukebox label
void Gui::setActionBarMessage(wstring message)
{
	overlayMessageString = message;
	overlayMessageTime = 20 * 4; //idk how long it should last, need to check java usage
}

float Gui::getJukeboxOpacity(int iPad)
{
	float t = overlayMessageTime - lastTickA;
    int alpha = static_cast<int>(t * 256 / 20);
    if (alpha > 255) alpha = 255;
	alpha /= 255;

	return alpha;
}

void Gui::setNowPlaying(const wstring& string)
{
//	overlayMessageString = L"Now playing: " + string;
	overlayMessageString = app.GetString(IDS_NOWPLAYING) + string;
    overlayMessageTime = 20 * 3;
    animateOverlayMessageColor = true; //appears to be unused, @DrPerkyLegit plans to add in later pr
}

void Gui::displayClientMessage(int messageId, int iPad)
{
    //Language *language = Language::getInstance();
    wstring languageString = app.GetString(messageId);//language->getElement(messageId);

    addMessage(languageString, iPad);
}

// 4J Added
void Gui::renderGraph(int dataLength, int dataPos, int64_t *dataA, float dataAScale, int dataAWarning, int64_t *dataB, float dataBScale, int dataBWarning)
{
	int height = minecraft->height;
	// This causes us to cover xScale*dataLength pixels in the horizontal
	int xScale = 1;
	if(dataA != NULL && dataB != NULL) xScale = 2;

	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, static_cast<float>(minecraft->width), static_cast<float>(height), 0, 1000, 3000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -2000);

	glLineWidth(1);
	glDisable(GL_TEXTURE_2D);
	Tesselator *t = Tesselator::getInstance();

	t->begin(GL_LINES);
	for (int i = 0; i < dataLength; i++)
	{
		int col = ((i - dataPos) & (dataLength - 1)) * 255 / dataLength;
		int cc = col * col / 255;
		cc = cc * cc / 255;
		int cc2 = cc * cc / 255;
		cc2 = cc2 * cc2 / 255;

		if( dataA != NULL )
		{
			if (dataA[i] > dataAWarning)
			{
				t->color(0xff000000 + cc * 65536);
			}
			else
			{
				t->color(0xff000000 + cc * 256);
			}

			int64_t aVal = dataA[i] / dataAScale;

			t->vertex((float)(xScale*i + 0.5f), (float)( height - aVal + 0.5f), static_cast<float>(0));
			t->vertex((float)(xScale*i + 0.5f), (float)( height + 0.5f), static_cast<float>(0));
		}

		if( dataB != NULL )
		{
			if (dataB[i]>dataBWarning)
			{
				t->color(0xff000000 + cc * 65536 + cc * 256 + cc * 1);
			}
			else
			{
				t->color(0xff808080 + cc/2 * 256);
			}

			int64_t bVal = dataB[i] / dataBScale;

			t->vertex((float)(xScale*i + (xScale - 1) + 0.5f), (float)( height - bVal + 0.5f), static_cast<float>(0));
			t->vertex((float)(xScale*i + (xScale - 1) + 0.5f), (float)( height + 0.5f), static_cast<float>(0));
		}
	}
	t->end();

	glEnable(GL_TEXTURE_2D);
}

void Gui::renderStackedGraph(int dataPos, int dataLength, int dataSources, int64_t (*func)(unsigned int dataPos, unsigned int dataSource) )
{
	int height = minecraft->height;

	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, static_cast<float>(minecraft->width), static_cast<float>(height), 0, 1000, 3000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -2000);

	glLineWidth(1);
	glDisable(GL_TEXTURE_2D);
	Tesselator *t = Tesselator::getInstance();

	t->begin(GL_LINES);
	int64_t thisVal = 0;
	int64_t topVal = 0;
	for (int i = 0; i < dataLength; i++)
	{
		thisVal = 0;
		topVal = 0;
		int col = ((i - dataPos) & (dataLength - 1)) * 255 / dataLength;
		int cc = col * col / 255;
		cc = cc * cc / 255;
		int cc2 = cc * cc / 255;
		cc2 = cc2 * cc2 / 255;


		for(unsigned int source = 0; source < dataSources; ++source )
		{
			thisVal = func( i, source );

			if( thisVal > 0 )
			{
				float vary = static_cast<float>(source)/dataSources;
				int fColour = floor(vary * 0xffffff);

				int colour = 0xff000000 + fColour;
				//printf("Colour is %x\n", colour);
				t->color(colour);

				t->vertex((float)(i + 0.5f), (float)( height - topVal - thisVal + 0.5f), static_cast<float>(0));
				t->vertex((float)(i + 0.5f), (float)( height - topVal + 0.5f), static_cast<float>(0));

				topVal += thisVal;
			}
		}

		// Draw some horizontals
		for(unsigned int horiz = 1; horiz < 7; ++horiz )
		{
			t->color(0xff000000);

			t->vertex((float)(0 + 0.5f), (float)( height - (horiz*100) + 0.5f), static_cast<float>(0));
			t->vertex((float)(dataLength + 0.5f), (float)( height - (horiz*100) + 0.5f), static_cast<float>(0));
		}
	}
	t->end();

	glEnable(GL_TEXTURE_2D);
}
