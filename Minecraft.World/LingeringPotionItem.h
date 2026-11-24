#pragma once
#include "PotionItem.h"

class LingeringPotionItem : public PotionItem
{
public:
	static const wstring LINGERING_ICON;

private:
	Icon *iconLingering;

public:
	LingeringPotionItem(int id);

	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual bool TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
	virtual Icon *getIcon(int auxValue);
	virtual wstring getHoverName(shared_ptr<ItemInstance> itemInstance);
	virtual void registerIcons(IconRegister *iconRegister);

protected:
	virtual float getTooltipDurationScale() { return 0.25f; }
};
