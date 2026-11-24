#pragma once
#include "PotionItem.h"

class TippedArrowItem : public PotionItem
{
public:
	TippedArrowItem(int id);

	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual bool TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
	virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance);
	virtual UseAnim getUseAnimation(shared_ptr<ItemInstance> itemInstance);
	virtual Icon *getIcon(int auxValue);
	virtual wstring getHoverName(shared_ptr<ItemInstance> itemInstance);
	virtual bool isFoil(shared_ptr<ItemInstance> itemInstance);
	virtual unsigned int getUseDescriptionId(shared_ptr<ItemInstance> instance);
	virtual void registerIcons(IconRegister *iconRegister);

protected:
	virtual float getTooltipDurationScale() { return 0.125f; }

private:
	Icon *iconBase;
};
