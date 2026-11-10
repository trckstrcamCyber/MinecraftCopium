#include "stdafx.h"
#include "IUIScene_GrindstoneMenu.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../../Minecraft.World/InputOutputStream.h"
#include "../../../Minecraft.World/net.minecraft.network.packet.h"
#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../ClientConnection.h"

IUIScene_GrindstoneMenu::IUIScene_GrindstoneMenu()
{
	m_inventory = nullptr;
	m_grindstoneMenu = nullptr;
}

IUIScene_AbstractContainerMenu::ESceneSection IUIScene_GrindstoneMenu::GetSectionAndSlotInDirection( ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY )
{
	ESceneSection newSection = eSection;
	int xOffset = 0;

	// Find the new section if there is one
	switch( eSection )
	{
	case eSectionGrindstoneItem1:
		if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionGrindstoneInventory;
			xOffset = GRINDSTONE_SCENE_ITEM1_SLOT_DOWN_OFFSET;
		}
		else if(eTapDirection == eTapStateLeft)
		{
			newSection = eSectionGrindstoneResult;
		}
		else if(eTapDirection == eTapStateRight)
		{
			newSection = eSectionGrindstoneItem2;
		}
		break;
	case eSectionGrindstoneItem2:
		if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionGrindstoneInventory;
			xOffset = GRINDSTONE_SCENE_ITEM2_SLOT_DOWN_OFFSET;
		}
		else if(eTapDirection == eTapStateLeft)
		{
			newSection = eSectionGrindstoneItem1;
		}
		else if(eTapDirection == eTapStateRight)
		{
			newSection = eSectionGrindstoneResult;
		}
		break;
	case eSectionGrindstoneResult:
		if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionGrindstoneInventory;
			xOffset = GRINDSTONE_SCENE_RESULT_SLOT_DOWN_OFFSET;
		}
		else if(eTapDirection == eTapStateLeft)
		{
			newSection = eSectionGrindstoneItem2;
		}
		else if(eTapDirection == eTapStateRight)
		{
			newSection = eSectionGrindstoneItem1;
		}
		break;
	case eSectionGrindstoneInventory:
		if(eTapDirection == eTapStateDown)
		{
			newSection = eSectionGrindstoneUsing;
		}
		else if(eTapDirection == eTapStateUp)
		{
			if( *piTargetX <= GRINDSTONE_SCENE_ITEM1_SLOT_UP_OFFSET)
			{
				newSection = eSectionGrindstoneItem1;
			}
			else if( *piTargetX <= GRINDSTONE_SCENE_ITEM2_SLOT_UP_OFFSET)
			{
				newSection = eSectionGrindstoneItem2;
			}
			else if( *piTargetX >= GRINDSTONE_SCENE_RESULT_SLOT_UP_OFFSET)
			{
				newSection = eSectionGrindstoneResult;
			}
		}
		break;
	case eSectionGrindstoneUsing:
		if(eTapDirection == eTapStateUp)
		{
			newSection = eSectionGrindstoneInventory;
		}
		else if(eTapDirection == eTapStateDown)
		{
			if( *piTargetX <= GRINDSTONE_SCENE_ITEM1_SLOT_UP_OFFSET)
			{
				newSection = eSectionGrindstoneItem1;
			}
			else if( *piTargetX <= GRINDSTONE_SCENE_ITEM2_SLOT_UP_OFFSET)
			{
				newSection = eSectionGrindstoneItem2;
			}
			else if( *piTargetX >= GRINDSTONE_SCENE_RESULT_SLOT_UP_OFFSET)
			{
				newSection = eSectionGrindstoneResult;
			}
		}
		break;
	default:
		assert( false );
		break;
	}

	updateSlotPosition(eSection, newSection, eTapDirection, piTargetX, piTargetY, xOffset);

	return newSection;
}

int IUIScene_GrindstoneMenu::getSectionStartOffset(ESceneSection eSection)
{
	int offset = 0;
	switch( eSection )
	{
	case eSectionGrindstoneItem1:
		offset = GrindstoneMenu::INPUT_SLOT;
		break;
	case eSectionGrindstoneItem2:
		offset = GrindstoneMenu::ADDITIONAL_SLOT;
		break;
	case eSectionGrindstoneResult:
		offset = GrindstoneMenu::RESULT_SLOT;
		break;
	case eSectionGrindstoneInventory:
		offset = GrindstoneMenu::INV_SLOT_START;
		break;
	case eSectionGrindstoneUsing:
		offset = GrindstoneMenu::USE_ROW_SLOT_START;
		break;
	default:
		assert( false );
		break;
	}
	return offset;
}

bool IUIScene_GrindstoneMenu::IsSectionSlotList( ESceneSection eSection )
{
	switch( eSection )
	{
	case eSectionGrindstoneUsing:
	case eSectionGrindstoneInventory:
	case eSectionGrindstoneItem1:
	case eSectionGrindstoneItem2:
	case eSectionGrindstoneResult:
		return true;
	}
	return false;
}
