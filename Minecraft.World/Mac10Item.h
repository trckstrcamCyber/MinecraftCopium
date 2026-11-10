#pragma once

#include "Item.h"

// 1 = print a few "[MAC10 ...]" lines per shot to the debug console (client prediction and server result). Set to 0 once the guns behave.
#ifndef MAC10_DEBUG_LOG
#define MAC10_DEBUG_LOG 0
#endif

// neoLegacy - firearms. Despite the file name this class is the engine for EVERY gun (MAC-10, M16, Flintlock Pistols): each Item is a Mac10Item
// with a different GunKind, and everything that differs between guns lives in the GunStats table below. Mac10Item::isMac10() means "is any gun".
//
// Design notes (read before changing the guns):
//  * Guns are server-authoritative. The client PREDICTS every action by running the very same code on its own copy of the stack
//    (MultiPlayerGameMode::useItem -> Item::use), then UseItemPacket carries the requested action to the server.
//  * Only the magazine count and durability wear live in the item NBT (so they save + sync). Every timer (fire cooldown, burst, reload progress,
//    draw lock, bloom) lives in a small per-player state table that exists separately on the client side and on the server side, counted in ticks.
//  * Guns never call Player::startUsingItem(): that would slow the player to 20% speed. Automatic fire / bursts are driven by Mac10Client::tick().
//  * Client-only feedback (view model, sounds, recoil, hit marker) is kept out of this class; the client reads lastResult().
//  * WARNING (this bit us): Level::clip() ADVANCES the Vec3 it is given as its first argument while it walks the ray. Never reuse a vector
//    after passing it to clip() - give clip() its own copies.
enum GunKind
{
    GUN_MAC10 = 0,
    GUN_M16,
    GUN_FLINTLOCK_COPPER,
    GUN_FLINTLOCK_IRON,
    GUN_FLINTLOCK_GOLD,
    GUN_KIND_COUNT
};

struct GunStats
{
    const char *name;
    int   ammoItemId;           // item consumed by a reload
    int   magazine;             // base capacity (Extended Mag adds magBonus[0..1])
    int   magBonus1, magBonus2;
    int   fireInterval;         // ticks between rounds (semi-auto, or inside a burst)
    int   autoInterval;         // ticks between rounds with the AutoFire enchantment (0 = AutoFire does nothing for this gun)
    int   burstCount;           // 0 = not a burst weapon, otherwise rounds per trigger pull (without AutoFire)
    int   burstGap;             // ticks after the last round of a burst
    int   reloadTicks;          // partial reload (matches the animation length where there is one)
    int   reloadEmptyTicks;
    int   drawLock;             // trigger locked while the gun is drawn
    float damage;               // before falloff / headshot / enchantments
    float headshotMult;
    float falloffStart, falloffEnd, falloffMin;   // blocks / fraction of damage that remains beyond falloffEnd
    float maxRange;             // blocks
    float spreadHip, spreadMoving, spreadAir;      // degrees (additive for moving / airborne)
    float adsMult;                                 // spread multiplier while aiming down sights
    float bloomPerShot, bloomMax;                  // degrees per consecutive round / cap
    bool  knockback;            // true = the target is shoved like a melee hit; false = bullets do not push
    int   enchantValue;         // > 0 lets the enchanting table accept the gun
    // client feel (used by Mac10Client)
    float recoilUp, recoilAutoUp, recoilSide;      // degrees of view kick per round
    float adsZoom;                                 // world FOV multiplier while aimed
    float adsMove;                                 // movement speed multiplier while aimed
};

class Mac10Item : public Item
{
public:
    enum PendingAction { ACTION_NONE = 0, ACTION_FIRE = 1, ACTION_RELOAD = 2, ACTION_INSPECT = 3 };

    // Kept for old call sites: the MAC-10's numbers.
    static const int BASE_MAGAZINE = 24;

    static const GunStats &statsFor(int kind);
    int getKind() const { return kind; }

    // What the last use() on THIS THREAD did. The client reads this right after predicting an action (the client and the
    // integrated server run on different threads, so each thread sees its own copy).
    struct ShotResult
    {
        bool fired = false;
        bool dryFire = false;
        bool reloadStarted = false;
        bool hitEntity = false;
        bool headshot = false;
        bool killed = false;
        int candidates = 0;        // entities the hit scan considered (diagnostics)
        float distance = -1.0f;    // distance to the entity that was hit, -1 if none
    };
    static ShotResult &lastResult();

    Mac10Item(int id, int kind);

    virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player) override;
    virtual void inventoryTick(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Entity> owner, int slot, bool selected) override;
    virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance) override;
    virtual int getEnchantmentValue() override;   // > 0 is what lets the enchanting table accept the gun

    // Client input bridge: the action (and the aim-down-sights flag) is serialized in the item NBT while the UseItemPacket
    // is created, and removed again immediately afterwards.
    void setPendingAction(shared_ptr<ItemInstance> itemInstance, PendingAction action, bool aimingDownSights = false);
    void clearPendingAction(shared_ptr<ItemInstance> itemInstance);

    // ---- queries (safe to call from the client every frame) ----------------------------------------------------------
    static bool isMac10(shared_ptr<ItemInstance> itemInstance);          // true for ANY gun
    static int  getKind(shared_ptr<ItemInstance> itemInstance);          // GunKind, -1 if not a gun
    static int  getMagazineRounds(shared_ptr<ItemInstance> itemInstance);
    static int  getMagazineCapacity(shared_ptr<ItemInstance> itemInstance);
    static int  getReserveAmmo(shared_ptr<Player> player);          // rounds carried (-1 = unlimited, creative)
    static bool hasAutoFire(shared_ptr<ItemInstance> itemInstance);  // AutoFire enchantment AND the gun supports it
    static int  getFireInterval(shared_ptr<ItemInstance> itemInstance);
    static int  getReloadTicks(shared_ptr<ItemInstance> itemInstance, bool emptyMagazine);
    static bool isReloading(Level *level, shared_ptr<Player> player);
    static bool canFireNow(Level *level, shared_ptr<Player> player, shared_ptr<ItemInstance> itemInstance);
    static int  getBurstShots(Level *level, shared_ptr<Player> player);
    static int  getBurstLeft(Level *level, shared_ptr<Player> player);   // rounds still to come in the running burst

private:
    int kind;
    static void ensureData(shared_ptr<ItemInstance> itemInstance);
    static int  enchantLevel(int enchantmentId, shared_ptr<ItemInstance> itemInstance);
    void fireRound(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, bool aimingDownSights);
    void beginReload(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
    static void finishReload(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
};
