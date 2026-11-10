#pragma once

#include "Item.h"

class Mac10Item : public Item
{
public:
    enum PendingAction { ACTION_NONE = 0, ACTION_FIRE = 1, ACTION_RELOAD = 2, ACTION_INSPECT = 3 };
    explicit Mac10Item(int id);

    virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player) override;
    virtual void inventoryTick(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Entity> owner, int slot, bool selected) override;
    virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance) override;
    virtual void releaseUsing(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, int durationLeft) override;

    // Client input bridge: action is serialized in the item NBT during UseItemPacket creation.
    void setPendingAction(shared_ptr<ItemInstance> itemInstance, PendingAction action);
    void clearPendingAction(shared_ptr<ItemInstance> itemInstance);
    static bool isMac10(shared_ptr<ItemInstance> itemInstance);
    static int getMagazineRounds(shared_ptr<ItemInstance> itemInstance);
    static int getMagazineCapacity(shared_ptr<ItemInstance> itemInstance);

private:
    static void ensureData(shared_ptr<ItemInstance> itemInstance);
    static int enchantLevel(int enchantmentId, shared_ptr<ItemInstance> itemInstance);
    void executeAction(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, PendingAction action);
    void fireRound(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, bool automatic);
    void beginReload(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
    void finishReload(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
    void inspect(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
};
