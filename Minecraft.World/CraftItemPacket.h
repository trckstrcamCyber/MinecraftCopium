#pragma once

// 4J ADDED THIS PACKET

using namespace std;

#include "../Minecraft.Client/Common/UI/IUIScene_CraftingMenu.h"
#include "Packet.h"

class CraftItemPacket : public Packet, public enable_shared_from_this<CraftItemPacket>
{
public:
	bool isBanner;
	short uid;
	//banner recipe

	//banner that needs to be found for recipe
	short findBannerAux;
	vector<char> findPatternIndexs;
	vector<char> findPatternColors;

	//pattern from recipe
	vector<char> patternIndexs;
	vector<char> patternColors;

	//normal recipe
	int recipe;

	CraftItemPacket();
	~CraftItemPacket();
	CraftItemPacket(int recipe, short uid);
	CraftItemPacket(shared_ptr<ItemInstance> craftingBanner, vector<IUIScene_CraftingMenu::PreviewBannerData>& data, short uid);

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<CraftItemPacket>(); }
	virtual int getId() { return 150; }
};