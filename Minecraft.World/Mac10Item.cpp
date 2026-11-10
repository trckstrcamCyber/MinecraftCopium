#include "stdafx.h"
#include "Mac10Item.h"
#include "CompoundTag.h"
#include "Inventory.h"
#include "Player.h"
#include "Entity.h"
#include "LivingEntity.h"
#include "Level.h"
#include "Vec3.h"
#include "AABB.h"
#include "HitResult.h"
#include "DamageSource.h"
#include "Enchantment.h"
#include "EnchantmentHelper.h"
#include "Mth.h"
#include <unordered_map>
#include <algorithm>
#include <cmath>

// Set MAC10_DEBUG_LOG to 0 in Mac10Item.h to silence the per-shot log lines.
#if MAC10_DEBUG_LOG
#define MAC10_LOG(...) app.DebugPrintf(__VA_ARGS__)
#else
#define MAC10_LOG(...) ((void)0)
#endif

// ======================================================================================================================
// Per-gun numbers. Everything that makes one gun feel different from another is in this table.
//
//   MAC-10     SMG. Fast, forgiving at the hip, short effective range, light bullets. Semi-auto; AutoFire makes it spray.
//   M16        Rifle. 3-round burst per trigger pull; AutoFire makes it full auto. Hits hard and far, tight when aimed, kicks up, slow to reload.
//   Flintlock  Single-shot pistols. One loaded ball, slow muzzle reload from gunpowder, heavy damage, shoves the target, very accurate when aimed.
//              Copper = balanced, Iron = hardest hitting and most durable but slowest to reload, Gold = quickest to reload, easiest to enchant, fragile.
// ======================================================================================================================
//                                            ammo                    mag  +1  +2  rate auto burst gap  reload     draw  dmg   hs    fall(start,end,min)   range  spread(hip,move,air)   ads   bloom(per,max) knock  ench  recoil(up,auto,side)  zoom  adsMove
static const GunStats GUN_TABLE[GUN_KIND_COUNT] =
{
    { "MAC-10",           Item::bullet_9mm_Id,    24,  6, 12,  3,  2,   0,   0,  45, 60,  22,  3.5f, 1.75f, 10.0f, 30.0f, 0.45f,  40.0f, 1.9f, 1.6f, 2.5f, 0.25f, 0.18f, 3.6f, false, 14,  0.85f, 0.62f, 0.30f, 0.84f, 0.62f },
    { "M16",              Item::bullet_556_Id,    30,  6, 12,  2,  3,   3,   8,  56, 56,  26,  5.0f, 2.00f, 25.0f, 70.0f, 0.55f,  80.0f, 2.6f, 2.2f, 3.5f, 0.12f, 0.15f, 4.0f, false, 12,  1.00f, 0.70f, 0.28f, 0.70f, 0.55f },
    { "Flintlock Copper", Item::gunpowder_Id,      1,  1,  2,  8,  0,   0,   0,  56, 56,  16,  8.0f, 2.25f,  8.0f, 28.0f, 0.50f,  50.0f, 3.2f, 2.5f, 3.0f, 0.10f, 0.00f, 0.0f, true,   8,  3.20f, 3.20f, 0.60f, 0.86f, 0.80f },
    { "Flintlock Iron",   Item::gunpowder_Id,      1,  1,  2, 10,  0,   0,   0,  64, 64,  18,  9.5f, 2.25f,  8.0f, 30.0f, 0.50f,  55.0f, 3.2f, 2.5f, 3.0f, 0.10f, 0.00f, 0.0f, true,  10,  3.60f, 3.60f, 0.60f, 0.86f, 0.80f },
    { "Flintlock Gold",   Item::gunpowder_Id,      1,  1,  2,  7,  0,   0,   0,  44, 44,  14,  7.5f, 2.25f,  8.0f, 26.0f, 0.50f,  48.0f, 2.8f, 2.3f, 2.8f, 0.10f, 0.00f, 0.0f, true,  22,  2.80f, 2.80f, 0.50f, 0.86f, 0.80f },
};

const GunStats &Mac10Item::statsFor(int kind)
{
    if (kind < 0 || kind >= GUN_KIND_COUNT) kind = GUN_MAC10;
    return GUN_TABLE[kind];
}

namespace
{
    // Only these two live in the stack NBT (they save and sync). Everything time-based is in GunState below.
    const wchar_t *TAG_MAG    = L"Mac10.Magazine";
    const wchar_t *TAG_WEAR   = L"Mac10.WearFraction";
    // Transient client -> server request, present only inside the UseItemPacket's copy of the stack.
    const wchar_t *TAG_ACTION = L"Mac10.PendingAction";
    const wchar_t *TAG_ADS    = L"Mac10.Ads";

    // Per-player timers. One table for the client side and one for the server side: the integrated server runs on its own
    // thread, and each thread only ever touches its own table, so no locking is needed.
    struct GunState
    {
        int  cooldown = 0;          // ticks until the next round may be fired
        int  reloadLeft = 0;        // ticks until the reload completes (0 = not reloading)
        int  selSlot = -1;          // hotbar slot the gun is currently held in
        int  shots = 0;             // rounds fired since the gun was equipped (spread seed)
        int  sinceShot = 1000;      // ticks since the last round
        int  burst = 0;             // consecutive rounds in the current burst (spread bloom)
        int  burstLeft = 0;         // rounds still to come in the running burst of a burst weapon
    };
    std::unordered_map<int, GunState> g_clientStates;
    std::unordered_map<int, GunState> g_serverStates;

    GunState &stateFor(Level *level, shared_ptr<Player> player)
    {
        return level->isClientSide ? g_clientStates[player->entityId] : g_serverStates[player->entityId];
    }
    GunState *peekState(Level *level, shared_ptr<Player> player)
    {
        if (!level || !player) return nullptr;
        std::unordered_map<int, GunState> &table = level->isClientSide ? g_clientStates : g_serverStates;
        auto it = table.find(player->entityId);
        return it == table.end() ? nullptr : &it->second;
    }

    int kindOf(shared_ptr<ItemInstance> item)
    {
        return Mac10Item::getKind(item);
    }

    int capacityFor(shared_ptr<ItemInstance> item)
    {
        const GunStats &s = Mac10Item::statsFor(kindOf(item));
        int level = EnchantmentHelper::getEnchantmentLevel(Enchantment::extendedMag->id, item);
        if (level >= 2) return s.magazine + s.magBonus2;
        if (level == 1) return s.magazine + s.magBonus1;
        return s.magazine;
    }

    // Same eye position / look direction maths as Item::getPlayerPOVHitResult, which is proven on both the client and the server.
    void eyeAndLook(shared_ptr<Player> player, double &ex, double &ey, double &ez, double &lx, double &ly, double &lz)
    {
        ex = player->x;
        ey = player->y + 1.62 - player->heightOffset;
        ez = player->z;
        float yCos = (float) cos(-player->yRot * Mth::RAD_TO_GRAD - PI);
        float ySin = (float) sin(-player->yRot * Mth::RAD_TO_GRAD - PI);
        float xCos = (float) -cos(-player->xRot * Mth::RAD_TO_GRAD);
        float xSin = (float) sin(-player->xRot * Mth::RAD_TO_GRAD);
        lx = ySin * xCos;
        ly = xSin;
        lz = yCos * xCos;
    }

    // Bullets ignore anything already dead: a dying mob keeps its hit box for the death animation and used to swallow shots meant for what stands behind it.
    bool isTargetable(const shared_ptr<Entity> &entity, const shared_ptr<Player> &shooter)
    {
        if (!entity || entity == shooter || entity->removed || !entity->isPickable() || !entity->isAttackable() || !entity->bb) return false;
        shared_ptr<LivingEntity> living = dynamic_pointer_cast<LivingEntity>(entity);
        if (living && (!living->isAlive() || living->getHealth() <= 0.0f)) return false;
        return true;
    }

    // Small deterministic hash -> [-1, 1]
    double hashSigned(unsigned int a, unsigned int b, unsigned int salt)
    {
        unsigned int h = a * 374761393u + b * 668265263u + salt * 2246822519u;
        h = (h ^ (h >> 13)) * 1274126177u;
        h ^= (h >> 16);
        return (double)(h & 0xFFFFu) / 32767.5 - 1.0;
    }
}

Mac10Item::ShotResult &Mac10Item::lastResult()
{
    static thread_local ShotResult result;
    return result;
}

Mac10Item::Mac10Item(int id, int gunKind) : Item(id), kind(gunKind) {}

int Mac10Item::getUseDuration(shared_ptr<ItemInstance> itemInstance) { return 0; }

// Without a positive enchantment value Item/EnchantmentHelper treat the item as "not enchantable" (cost 0), so the enchanting table never
// offers anything for the gun.
int Mac10Item::getEnchantmentValue() { return statsFor(kind).enchantValue; }

void Mac10Item::ensureData(shared_ptr<ItemInstance> itemInstance)
{
    if (!itemInstance) return;
    if (itemInstance->tag == nullptr) itemInstance->tag = new CompoundTag();
    if (!itemInstance->tag->contains(TAG_MAG))
    {
        // A fresh gun comes loaded (a flintlock holds its one ball).
        itemInstance->tag->putInt(TAG_MAG, statsFor(getKind(itemInstance)).magazine);
    }
    if (!itemInstance->tag->contains(TAG_WEAR)) itemInstance->tag->putFloat(TAG_WEAR, 0.0f);
}

bool Mac10Item::isMac10(shared_ptr<ItemInstance> itemInstance)
{
    return getKind(itemInstance) >= 0;
}

int Mac10Item::getKind(shared_ptr<ItemInstance> itemInstance)
{
    if (itemInstance == nullptr) return -1;
    switch (itemInstance->id)
    {
    case Item::mac10_Id: return GUN_MAC10;
    case Item::m16_Id: return GUN_M16;
    case Item::flintlock_copper_Id: return GUN_FLINTLOCK_COPPER;
    case Item::flintlock_iron_Id: return GUN_FLINTLOCK_IRON;
    case Item::flintlock_gold_Id: return GUN_FLINTLOCK_GOLD;
    default: return -1;
    }
}

int Mac10Item::getMagazineRounds(shared_ptr<ItemInstance> itemInstance)
{
    int k = getKind(itemInstance);
    if (k < 0) return 0;
    if (itemInstance->tag == nullptr || !itemInstance->tag->contains(TAG_MAG)) return statsFor(k).magazine;
    return itemInstance->tag->getInt(TAG_MAG);
}

int Mac10Item::getMagazineCapacity(shared_ptr<ItemInstance> itemInstance) { return capacityFor(itemInstance); }

int Mac10Item::getReserveAmmo(shared_ptr<Player> player)
{
    if (!player || !player->inventory) return 0;
    if (player->abilities.instabuild) return -1;
    int k = getKind(player->inventory->getSelected());
    if (k < 0) k = GUN_MAC10;
    return player->inventory->countResource(statsFor(k).ammoItemId, 0);
}

int Mac10Item::enchantLevel(int enchantmentId, shared_ptr<ItemInstance> itemInstance)
{
    return (itemInstance != nullptr) ? EnchantmentHelper::getEnchantmentLevel(enchantmentId, itemInstance) : 0;
}

bool Mac10Item::hasAutoFire(shared_ptr<ItemInstance> itemInstance)
{
    int k = getKind(itemInstance);
    return k >= 0 && statsFor(k).autoInterval > 0 && enchantLevel(Enchantment::autoFire->id, itemInstance) > 0;
}

int Mac10Item::getFireInterval(shared_ptr<ItemInstance> itemInstance)
{
    const GunStats &s = statsFor(getKind(itemInstance));
    return hasAutoFire(itemInstance) ? s.autoInterval : s.fireInterval;
}

int Mac10Item::getReloadTicks(shared_ptr<ItemInstance> itemInstance, bool emptyMagazine)
{
    const GunStats &s = statsFor(getKind(itemInstance));
    float ticks = (float)(emptyMagazine ? s.reloadEmptyTicks : s.reloadTicks);
    int rapid = enchantLevel(Enchantment::rapidReload->id, itemInstance);
    if (rapid == 1) ticks *= 0.85f;
    else if (rapid >= 2) ticks *= 0.73f;
    return (std::max)(8, (int)ceil(ticks));
}

bool Mac10Item::isReloading(Level *level, shared_ptr<Player> player)
{
    GunState *st = peekState(level, player);
    return st != nullptr && st->reloadLeft > 0;
}

bool Mac10Item::canFireNow(Level *level, shared_ptr<Player> player, shared_ptr<ItemInstance> itemInstance)
{
    GunState *st = peekState(level, player);
    int cooldown = st ? st->cooldown : 0;
    int reload = st ? st->reloadLeft : 0;
    return reload == 0 && cooldown <= 1 && getMagazineRounds(itemInstance) > 0;
}

int Mac10Item::getBurstShots(Level *level, shared_ptr<Player> player)
{
    GunState *st = peekState(level, player);
    return st ? st->burst : 0;
}

int Mac10Item::getBurstLeft(Level *level, shared_ptr<Player> player)
{
    GunState *st = peekState(level, player);
    return st ? st->burstLeft : 0;
}

void Mac10Item::setPendingAction(shared_ptr<ItemInstance> itemInstance, PendingAction action, bool aimingDownSights)
{
    ensureData(itemInstance);
    if (itemInstance && itemInstance->tag)
    {
        itemInstance->tag->putInt(TAG_ACTION, static_cast<int>(action));
        itemInstance->tag->putInt(TAG_ADS, aimingDownSights ? 1 : 0);
    }
}

void Mac10Item::clearPendingAction(shared_ptr<ItemInstance> itemInstance)
{
    if (itemInstance && itemInstance->tag)
    {
        itemInstance->tag->remove(TAG_ACTION);
        itemInstance->tag->remove(TAG_ADS);
    }
}

shared_ptr<ItemInstance> Mac10Item::use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
    lastResult() = ShotResult();
    if (!itemInstance || !level || !player) return itemInstance;
    ensureData(itemInstance);

    int action = itemInstance->tag->contains(TAG_ACTION) ? itemInstance->tag->getInt(TAG_ACTION) : ACTION_NONE;
    bool ads = itemInstance->tag->contains(TAG_ADS) && itemInstance->tag->getInt(TAG_ADS) != 0;
    if (action == ACTION_NONE) action = ACTION_FIRE;
    {
        GunState *dbg = peekState(level, player);
        MAC10_LOG("[MAC10 %s] %s use action=%d ads=%d mag=%d cooldown=%d reload=%d burstLeft=%d\n", level->isClientSide ? "client" : "server", statsFor(kind).name, action, ads ? 1 : 0,
                  getMagazineRounds(itemInstance), dbg ? dbg->cooldown : -1, dbg ? dbg->reloadLeft : -1, dbg ? dbg->burstLeft : -1);
    }

    switch (action)
    {
    case ACTION_RELOAD: beginReload(itemInstance, level, player); break;
    case ACTION_INSPECT: break;   // purely visual, handled by the client view model
    case ACTION_FIRE:
    default: fireRound(itemInstance, level, player, ads); break;
    }

    // The server consumes the request. The client clears it itself once UseItemPacket has copied the stack.
    if (!level->isClientSide) clearPendingAction(itemInstance);
    return itemInstance;
}

void Mac10Item::inventoryTick(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Entity> owner, int slot, bool selected)
{
    Item::inventoryTick(itemInstance, level, owner, slot, selected);
    if (!itemInstance || !level) return;
    shared_ptr<Player> player = dynamic_pointer_cast<Player>(owner);
    if (!player) return;
    ensureData(itemInstance);

    GunState &st = stateFor(level, player);
    if (!selected)
    {
        // Putting the gun away cancels a reload that has not finished yet (the ammo is only spent when it completes).
        if (st.selSlot == slot) { st.selSlot = -1; st.reloadLeft = 0; st.burstLeft = 0; }
        return;
    }

    if (st.selSlot != slot)
    {
        // Just equipped: cancel any reload, lock the trigger until the draw animation is mostly done.
        st.selSlot = slot;
        st.reloadLeft = 0;
        st.burstLeft = 0;
        st.cooldown = (std::max)(st.cooldown, statsFor(kind).drawLock);
        st.sinceShot = 1000;
        st.burst = 0;
        st.shots = 0;
    }

    // Exactly one simulation step per tick, driven by the selected gun's slot.
    if (st.cooldown > 0) st.cooldown--;
    if (st.sinceShot < 1000) st.sinceShot++;
    if (st.sinceShot > 6) st.burst = 0;
    if (st.sinceShot > 8) st.burstLeft = 0;           // an abandoned burst is forgotten
    if (st.reloadLeft > 0 && --st.reloadLeft == 0) finishReload(itemInstance, level, player);
}

void Mac10Item::beginReload(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
    if (!itemInstance || !level || !player || !player->inventory) return;
    GunState &st = stateFor(level, player);
    if (st.reloadLeft > 0) return;
    int rounds = itemInstance->tag->getInt(TAG_MAG);
    if (rounds >= capacityFor(itemInstance)) return;
    if (!player->abilities.instabuild && !player->inventory->hasResource(statsFor(kind).ammoItemId)) return;

    st.reloadLeft = getReloadTicks(itemInstance, rounds <= 0);
    st.burst = 0;
    st.burstLeft = 0;
    lastResult().reloadStarted = true;
}

void Mac10Item::finishReload(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
    if (!itemInstance || !level || !player || !player->inventory) return;
    ensureData(itemInstance);
    const GunStats &s = statsFor(getKind(itemInstance));
    int capacity = capacityFor(itemInstance);
    int rounds = itemInstance->tag->getInt(TAG_MAG);
    int needed = capacity - rounds;
    if (needed <= 0) return;

    int transfer = needed;
    if (!player->abilities.instabuild)
    {
        int ammo = player->inventory->countResource(s.ammoItemId, 0);
        transfer = (std::min)(needed, ammo);
        for (int i = 0; i < transfer; ++i) player->inventory->removeResource(s.ammoItemId);
    }
    itemInstance->tag->putInt(TAG_MAG, rounds + transfer);
    player->inventory->setChanged();
}

void Mac10Item::fireRound(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, bool aimingDownSights)
{
    if (!itemInstance || !level || !player) return;
    const GunStats &s = statsFor(kind);
    GunState &st = stateFor(level, player);
    ShotResult &result = lastResult();

    if (st.reloadLeft > 0) { MAC10_LOG("[MAC10 %s]   blocked: reloading (%d ticks left)\n", level->isClientSide ? "client" : "server", st.reloadLeft); return; }
    // One tick of tolerance: the request can arrive a tick early because client and server tick on slightly different phases.
    if (st.cooldown > 1) { MAC10_LOG("[MAC10 %s]   blocked: cooldown %d\n", level->isClientSide ? "client" : "server", st.cooldown); return; }

    int rounds = itemInstance->tag->getInt(TAG_MAG);
    if (rounds <= 0)
    {
        // Empty: click, and start reloading by itself if there is ammo to load.
        result.dryFire = true;
        st.cooldown = 6;
        st.burstLeft = 0;
        beginReload(itemInstance, level, player);
        return;
    }

    const int autoLevel = hasAutoFire(itemInstance) ? 1 : 0;
    // ---- cadence: semi-auto / full auto / burst ----
    if (autoLevel > 0)
    {
        st.cooldown = s.autoInterval;
        st.burstLeft = 0;
    }
    else if (s.burstCount > 0)
    {
        if (st.burstLeft > 0) st.burstLeft--;                     // inside a burst
        else st.burstLeft = s.burstCount - 1;                      // a new trigger pull starts a burst
        st.cooldown = (st.burstLeft == 0 || rounds <= 1) ? s.burstGap : s.fireInterval;
        if (rounds <= 1) st.burstLeft = 0;
    }
    else
    {
        st.cooldown = s.fireInterval;
    }
    st.sinceShot = 0;
    st.shots++;
    st.burst++;
    itemInstance->tag->putInt(TAG_MAG, rounds - 1);
    result.fired = true;
    player->inventory->setChanged();

    // ---- spread (degrees) -----------------------------------------------------------------------------------------
    double dxm = player->x - player->xo, dzm = player->z - player->zo;
    bool moving = (dxm * dxm + dzm * dzm) > 0.0016;
    double spreadDeg = s.spreadHip;
    if (!player->onGround) spreadDeg += s.spreadAir;
    else if (moving)       spreadDeg += s.spreadMoving;
    if (player->isSneaking()) spreadDeg *= 0.7;
    if (aimingDownSights)     spreadDeg *= s.adsMult;
    spreadDeg += (std::min)((double)(st.burst * s.bloomPerShot), (double)s.bloomMax);      // bloom while holding the trigger
    int precision = enchantLevel(Enchantment::precisionBarrel->id, itemInstance);
    if (precision > 0)
    {
        const float reductions[] = {0.0f, 0.12f, 0.22f, 0.30f};
        spreadDeg *= 1.0 - reductions[(precision > 3) ? 3 : precision];
    }

    double ex, ey, ez, lx, ly, lz;
    eyeAndLook(player, ex, ey, ez, lx, ly, lz);
    double spread = tan(spreadDeg * 3.14159265358979323846 / 180.0);
    double jx = hashSigned((unsigned)st.shots, (unsigned)player->entityId, 1u) * spread;
    double jy = hashSigned((unsigned)st.shots, (unsigned)player->entityId, 2u) * spread;
    double jz = hashSigned((unsigned)st.shots, (unsigned)player->entityId, 3u) * spread;
    Vec3 *direction = Vec3::newTemp(lx + jx, ly + jy, lz + jz)->normalize();
    const double range = s.maxRange;

    // ---- hit scan: nearest of (solid block, entity) ------------------------------------------------------------------
    Vec3 *from = Vec3::newTemp(ex, ey, ez);
    Vec3 *to = from->add(direction->x * range, direction->y * range, direction->z * range);
    // Level::clip() ADVANCES the vector it is given while it walks the ray, so it gets its own copies. (Reusing `from` afterwards put the ray
    // origin at the end of the ray: only entities standing where the block ray ended could be hit, which is why summoned mobs at the player's
    // feet died and natural ones far away seemed immune.)
    HitResult *blockResult = level->clip(Vec3::newTemp(ex, ey, ez), Vec3::newTemp(to->x, to->y, to->z), false, true);
    Vec3 *end = (blockResult != nullptr) ? blockResult->pos : to;
    double nearest = (blockResult != nullptr) ? from->distanceTo(end) : range + 0.001;
    shared_ptr<Entity> hitEntity = nullptr;
    bool headshot = false;

    // Candidate pool: the chunk lookup first; if that returns nothing (or only the shooter) fall back to every loaded entity near the ray.
    std::vector<shared_ptr<Entity> > pool;
    {
        vector<shared_ptr<Entity> > *objects = level->getEntities(player, player->bb->expand(to->x - from->x, to->y - from->y, to->z - from->z)->grow(1, 1, 1));
        if (objects != nullptr) pool = *objects;
        bool anyOther = false;
        for (size_t i = 0; i < pool.size(); ++i) if (pool[i] && pool[i] != player) { anyOther = true; break; }
        if (!anyOther)
        {
            pool.clear();
            const double reach = range + 2.0;
            for (size_t i = 0; i < level->entities.size(); ++i)
            {
                shared_ptr<Entity> e = level->entities[i];
                if (!e || e == player) continue;
                if (fabs(e->x - ex) > reach || fabs(e->y - ey) > reach || fabs(e->z - ez) > reach) continue;
                pool.push_back(e);
            }
        }
    }

    int candidates = 0;
    for (size_t i = 0; i < pool.size(); ++i)
    {
        shared_ptr<Entity> entity = pool[i];
        if (!isTargetable(entity, player)) continue;
        ++candidates;
        // forgiving hit box: wider with distance (targets move between what the shooter sees and what the server has, and they are small at range)
        double cdx = (entity->bb->x0 + entity->bb->x1) * 0.5 - ex, cdy = (entity->bb->y0 + entity->bb->y1) * 0.5 - ey, cdz = (entity->bb->z0 + entity->bb->z1) * 0.5 - ez;
        double toCenter = sqrt(cdx * cdx + cdy * cdy + cdz * cdz);
        double pad = 0.12 + 0.012 * (toCenter > range ? range : toCenter);
        HitResult *hit = entity->bb->grow(pad, pad, pad)->clip(Vec3::newTemp(ex, ey, ez), Vec3::newTemp(end->x, end->y, end->z));
        if (hit != nullptr)
        {
            double distance = from->distanceTo(hit->pos);
            if (distance < nearest)
            {
                nearest = distance;
                hitEntity = entity;
                double height = entity->bb->y1 - entity->bb->y0;
                headshot = hit->pos->y >= entity->bb->y0 + height * 0.78;
            }
            delete hit;
        }
    }

    // Fallback that does not rely on the AABB clip helpers: treat every living target as a capsule around its centre line and take the
    // closest one the ray passes through. Only used when the box test found nothing.
    if (hitEntity == nullptr)
    {
        const double blockLimit = (blockResult != nullptr) ? from->distanceTo(end) : range;
        double bestAlong = blockLimit + 0.001;
        for (size_t i = 0; i < pool.size(); ++i)
        {
            shared_ptr<Entity> entity = pool[i];
            if (!isTargetable(entity, player)) continue;
            double halfWidth = (entity->bb->x1 - entity->bb->x0) * 0.5 + 0.2;
            double height = entity->bb->y1 - entity->bb->y0;
            double cx = (entity->bb->x0 + entity->bb->x1) * 0.5, cy = (entity->bb->y0 + entity->bb->y1) * 0.5, cz = (entity->bb->z0 + entity->bb->z1) * 0.5;
            double vx = cx - ex, vy = cy - ey, vz = cz - ez;
            double along = vx * direction->x + vy * direction->y + vz * direction->z;
            if (along <= 0.05 || along >= bestAlong) continue;
            double px = vx - direction->x * along, py = vy - direction->y * along, pz = vz - direction->z * along;
            double horizontal = sqrt(px * px + pz * pz);
            if (horizontal <= halfWidth && fabs(py) <= height * 0.5 + 0.2)
            {
                bestAlong = along;
                hitEntity = entity;
                nearest = along;
                double hitY = ey + direction->y * along;
                headshot = hitY >= entity->bb->y0 + height * 0.78;
            }
        }
    }
    result.candidates = candidates;
    result.distance = hitEntity ? (float)nearest : -1.0f;
    delete blockResult;
    MAC10_LOG("[MAC10 %s]   shot: eye=(%.2f %.2f %.2f) dir=(%.3f %.3f %.3f) candidates=%d hit=%s dist=%.2f\n", level->isClientSide ? "client" : "server",
              ex, ey, ez, direction->x, direction->y, direction->z, candidates, hitEntity ? "YES" : "no", hitEntity ? nearest : -1.0);
#if MAC10_DEBUG_LOG
    if (!level->isClientSide)
    {
        // Server-side scan result, synced to the client with the stack and shown by the HUD debug line (Gui.cpp).
        itemInstance->tag->putInt(L"Mac10.DbgCand", candidates);
        itemInstance->tag->putInt(L"Mac10.DbgHit", hitEntity ? 1 : 0);
        itemInstance->tag->putFloat(L"Mac10.DbgDist", result.distance);
        itemInstance->tag->putInt(L"Mac10.DbgHurt", -1);
        itemInstance->tag->putFloat(L"Mac10.DbgHp", -1.0f);
    }
#endif

    shared_ptr<LivingEntity> target = hitEntity ? dynamic_pointer_cast<LivingEntity>(hitEntity) : nullptr;
    if (target && target->isAlive())
    {
        result.hitEntity = true;
        result.headshot = headshot;

        float falloff = 1.0f;
        if (nearest > s.falloffStart)
        {
            double t = ((std::min)(nearest, (double)s.falloffEnd) - s.falloffStart) / (double)(s.falloffEnd - s.falloffStart);
            falloff = 1.0f - (1.0f - s.falloffMin) * (float)t;
        }
        float damage = s.damage * falloff * (headshot ? s.headshotMult : 1.0f);
        int hollow = enchantLevel(Enchantment::hollowPoint->id, itemInstance);
        if (hollow > 0)
        {
            bool armored = target->getArmorValue() > 0;
            if (!armored) damage *= (hollow >= 2) ? 1.20f : 1.12f;
            else damage *= (hollow >= 2) ? 0.82f : 0.90f;
        }

        // Damage is server-authoritative; the client only predicts the hit marker.
        if (!level->isClientSide)
        {
            double ox = target->xd, oy = target->yd, oz = target->zd;
            target->invulnerableTime = 0;                     // bullets ignore the damage-immunity frames, so rapid fire adds up
            bool hurtOk = target->hurt(DamageSource::playerAttack(player), damage);
            MAC10_LOG("[MAC10 server]   damage=%.2f hurt()=%d targetHealth=%.1f\n", damage, hurtOk ? 1 : 0, target->getHealth());
#if MAC10_DEBUG_LOG
            itemInstance->tag->putInt(L"Mac10.DbgHurt", hurtOk ? 1 : 0);
            itemInstance->tag->putFloat(L"Mac10.DbgHp", target->getHealth());
#endif
            if (!s.knockback) { target->xd = ox; target->yd = oy; target->zd = oz; }   // bullets don't shove the target; a flintlock ball does
            result.killed = !target->isAlive();
        }
    }

    // ---- durability: fractional wear so AutoFire / Reinforced Parts can be tuned finely ----------------------------
    int reinforced = enchantLevel(Enchantment::reinforcedParts->id, itemInstance);
    const float durabilityBenefits[] = {1.0f, 1.20f, 1.40f, 1.65f};
    float wearMultiplier = (autoLevel > 0) ? 1.20f : 1.0f;
    wearMultiplier /= durabilityBenefits[(reinforced > 3) ? 3 : reinforced];
    float wear = itemInstance->tag->getFloat(TAG_WEAR) + wearMultiplier;
    int wholeWear = static_cast<int>(wear);
    itemInstance->tag->putFloat(TAG_WEAR, wear - wholeWear);
    if (wholeWear > 0) itemInstance->hurtAndBreak(wholeWear, dynamic_pointer_cast<LivingEntity>(player));
}
