#include "stdafx.h"
#include "SpyglassItem.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"

SpyglassItem::SpyglassItem(int id)
    : Item(id)
{
    maxStackSize = 1;
}

shared_ptr<ItemInstance> SpyglassItem::use(
    shared_ptr<ItemInstance> itemInstance,
    Level*,
    shared_ptr<Player> player)
{
    if (player != nullptr && itemInstance != nullptr)
        player->startUsingItem(itemInstance, getUseDuration(itemInstance));
    return itemInstance;
}

int SpyglassItem::getUseDuration(shared_ptr<ItemInstance>)
{
    // Long enough for the held-use action; releasing the button cancels it.
    return 20 * 60 * 60;
}

UseAnim SpyglassItem::getUseAnimation(shared_ptr<ItemInstance>)
{
    return UseAnim_none;
}
