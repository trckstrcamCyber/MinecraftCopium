#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.item.h"
#include "PacketListener.h"
#include "CraftItemPacket.h"
#include "BannerItem.h"



CraftItemPacket::~CraftItemPacket()
{
}

CraftItemPacket::CraftItemPacket()
{
	recipe = -1;
	uid = 0;
}

CraftItemPacket::CraftItemPacket(int recipe, short uid) {
	this->isBanner = false;
	this->recipe = recipe;
	this->uid = uid;
}

CraftItemPacket::CraftItemPacket(shared_ptr<ItemInstance> craftingBanner, vector<IUIScene_CraftingMenu::PreviewBannerData>& data, short uid) {
	this->isBanner = true;
	this->recipe = -1;
	this->uid = uid;

	this->findBannerAux = craftingBanner->getAuxValue();
	this->findPatternIndexs = BannerItem::getPatternIndexs(craftingBanner);
	this->findPatternColors = BannerItem::getPatternColors(craftingBanner);

	for (IUIScene_CraftingMenu::PreviewBannerData& entry : data) {
		this->patternIndexs.push_back(entry.id);
		this->patternColors.push_back(entry.color);
	}
}

void CraftItemPacket::handle(PacketListener *listener)
{
	listener->handleCraftItem(shared_from_this());
}

void CraftItemPacket::read(DataInputStream *dis) //throws IOException
{
	isBanner = dis->readBoolean();
	uid = dis->readShort();

	if (isBanner) {
		findBannerAux = dis->readShort();

		{
			{
				char size = dis->readChar();
				for (int i = 0; i < size; i++) {
					findPatternIndexs.push_back(dis->readChar());
				}
			}
			{
				char size = dis->readChar();
				for (int i = 0; i < size; i++) {
					findPatternColors.push_back(dis->readChar());
				}
			}
		}
		{
			{
				char size = dis->readChar();
				for (int i = 0; i < size; i++) {
					patternIndexs.push_back(dis->readChar());
				}
			}
			{
				char size = dis->readChar();
				for (int i = 0; i < size; i++) {
					patternColors.push_back(dis->readChar());
				}
			}
		}

	} else {
		recipe = dis->readInt();
	}

}

void CraftItemPacket::write(DataOutputStream *dos) // throws IOException
{
	dos->writeBoolean(isBanner);
	dos->writeShort(uid);

	if (isBanner) {
		dos->writeShort(findBannerAux);
		{
			{
				dos->writeChar(findPatternIndexs.size());
				for (int i = 0; i < findPatternIndexs.size(); i++) {
					dos->writeChar(findPatternIndexs[i]);
				}
			}
			{
				dos->writeChar(findPatternColors.size());
				for (int i = 0; i < findPatternColors.size(); i++) {
					dos->writeChar(findPatternColors[i]);
				}
			}
		}
		{
			{
				dos->writeChar(patternIndexs.size());
				for (int i = 0; i < patternIndexs.size(); i++) {
					dos->writeChar(patternIndexs[i]);
				}
			}
			{
				dos->writeChar(patternColors.size());
				for (int i = 0; i < patternColors.size(); i++) {
					dos->writeChar(patternColors[i]);
				}
			}
		}
		
	} else {
		dos->writeInt(recipe);
	}
}

int CraftItemPacket::getEstimatedSize() 
{
	if (isBanner) {
		return 1+2+2+(findPatternIndexs.size() + 1)+(findPatternColors.size() + 1)+(patternIndexs.size() + 1)+(patternColors.size() + 1);
	} else {
		return 1+2+4; //bool, short, int
	}
}
