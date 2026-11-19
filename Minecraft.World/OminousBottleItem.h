#pragma once
#include "Item.h"

// neoLegacy - Ominous Bottle, as in modern Minecraft: drink it to get Bad Omen. The bottle's level (I-V, stored as the item's data 0-4) becomes
// the level of the effect; a bottle of a lower level than the Bad Omen you already have changes nothing. No empty bottle is left.
// Raid captains drop one when a player kills them (Illager::die). With Bad Omen, walking into a village turns it into Raid Omen: a 30 second
// countdown, and when it ends the raid starts (RaidManager). Drinking milk cancels it.
class OminousBottleItem : public Item
{
public:
	static const int DRINK_DURATION = 32;
	static const int MAX_LEVEL = 5;
	static const int BAD_OMEN_TICKS = 120000;	// 100 minutes

	OminousBottleItem(int id);

	virtual shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance);
	virtual UseAnim getUseAnimation(shared_ptr<ItemInstance> itemInstance);
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual void appendHoverText(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, vector<HtmlString> *lines, bool advanced);

	static shared_ptr<ItemInstance> create(int level);	// level 1-5
	static int getLevel(shared_ptr<ItemInstance> itemInstance);
};
