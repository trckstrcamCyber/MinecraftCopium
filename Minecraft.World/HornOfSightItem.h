#pragma once
#include "Item.h"

// neoLegacy - Horn of Sight, from the Illager Invasion mod: blow it and monsters nearby show through walls. In the mod it gives them the vanilla Glowing outline; here the Glowing
// effect (id 28) draws a coloured aura (see EntityRenderDispatcher::renderGlow), and, following the Colored Glow Lib mod, the colour says what you are looking at:
// rainbow for bosses and illager captains, purple illagers, green undead, red the rest.
//   Blow:           every monster within 48 blocks glows for 10 seconds.
//   Blow sneaking:  the Illager Sense - only illagers, Ravagers, Witches and the Surrendered, but within 96 blocks and for 30 seconds. Made for raids and their captains.
// Seven seconds between blasts (kept on the horn itself). Not used up.
// The mod's horn is a goat horn variant; our fork has a goat_horn item (619) and the horn's recipe uses it, but the blast sound here is a low Pillager war cry.
class HornOfSightItem : public Item
{
public:
	static const int COOLDOWN_TICKS = 140;
	static const int RANGE = 48;
	static const int SENSE_RANGE = 96;
	static const int GLOW_TICKS = 200;
	static const int SENSE_GLOW_TICKS = 600;

	HornOfSightItem(int id);

	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
};
