#pragma once

#include "Item.h"

class SpyglassItem : public Item
{
public:
    explicit SpyglassItem(int id);

    virtual shared_ptr<ItemInstance> use(
        shared_ptr<ItemInstance> itemInstance,
        Level* level,
        shared_ptr<Player> player) override;

    virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance) override;
    virtual UseAnim getUseAnimation(shared_ptr<ItemInstance> itemInstance) override;
};
