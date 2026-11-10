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
#include "SoundTypes.h"

namespace
{
    const wchar_t *TAG_MAG = L"Mac10.Magazine";
    const wchar_t *TAG_ACTION = L"Mac10.PendingAction";
    const wchar_t *TAG_NEXT_SHOT = L"Mac10.NextShotTick";
    const wchar_t *TAG_RELOAD_END = L"Mac10.ReloadEndTick";
    const wchar_t *TAG_RELOAD_EMPTY = L"Mac10.ReloadWasEmpty";
    const wchar_t *TAG_INSPECT_END = L"Mac10.InspectEndTick";
    const wchar_t *TAG_SHOT_COUNT = L"Mac10.ShotCount";
    const wchar_t *TAG_WEAR_FRACTION = L"Mac10.WearFraction";
    const wchar_t *TAG_ANIMATION = L"Mac10.Animation";
    const wchar_t *TAG_ANIMATION_END = L"Mac10.AnimationEndTick";
    const wchar_t *TAG_ANIMATION_NEXT = L"Mac10.AnimationNext";
    const wchar_t *TAG_WAS_SELECTED = L"Mac10.WasSelected";
    const int BASE_MAGAZINE = 24;
    const double MAX_RANGE = 30.0;

    int capacityFor(shared_ptr<ItemInstance> item)
    {
        int level = EnchantmentHelper::getEnchantmentLevel(Enchantment::extendedMag->id, item);
        if (level >= 2) return 36;
        if (level == 1) return 30;
        return BASE_MAGAZINE;
    }

    bool isUsingThisGun(shared_ptr<Player> player)
    {
        if (!player || !player->isUsingItem()) return false;
        shared_ptr<ItemInstance> used = player->getUseItem();
        return used != nullptr && used->id == Item::mac10_Id;
    }

    void setAnimationEvent(shared_ptr<ItemInstance> item, Level *level, const wstring &clip, int durationTicks = 0, const wstring &next = L"")
    {
        if (!item || !level) return;
        if (item->tag == nullptr) item->tag = new CompoundTag();
        item->tag->putString(TAG_ANIMATION, clip);
        item->tag->putLong(TAG_ANIMATION_END, durationTicks > 0 ? level->getGameTime() + durationTicks : 0);
        item->tag->putString(TAG_ANIMATION_NEXT, next);
    }
}

Mac10Item::Mac10Item(int id) : Item(id) {}

void Mac10Item::ensureData(shared_ptr<ItemInstance> itemInstance)
{
    if (!itemInstance) return;
    if (itemInstance->tag == nullptr) itemInstance->tag = new CompoundTag();
    if (!itemInstance->tag->contains(TAG_MAG)) itemInstance->tag->putInt(TAG_MAG, BASE_MAGAZINE);
    if (!itemInstance->tag->contains(TAG_ACTION)) itemInstance->tag->putInt(TAG_ACTION, ACTION_NONE);
    if (!itemInstance->tag->contains(TAG_NEXT_SHOT)) itemInstance->tag->putLong(TAG_NEXT_SHOT, 0);
    if (!itemInstance->tag->contains(TAG_RELOAD_END)) itemInstance->tag->putLong(TAG_RELOAD_END, 0);
    if (!itemInstance->tag->contains(TAG_SHOT_COUNT)) itemInstance->tag->putInt(TAG_SHOT_COUNT, 0);
    if (!itemInstance->tag->contains(TAG_WEAR_FRACTION)) itemInstance->tag->putFloat(TAG_WEAR_FRACTION, 0.0f);
    if (!itemInstance->tag->contains(TAG_ANIMATION)) itemInstance->tag->putString(TAG_ANIMATION, L"");
    if (!itemInstance->tag->contains(TAG_ANIMATION_END)) itemInstance->tag->putLong(TAG_ANIMATION_END, 0);
    if (!itemInstance->tag->contains(TAG_ANIMATION_NEXT)) itemInstance->tag->putString(TAG_ANIMATION_NEXT, L"");
    if (!itemInstance->tag->contains(TAG_WAS_SELECTED)) itemInstance->tag->putInt(TAG_WAS_SELECTED, 0);
}

bool Mac10Item::isMac10(shared_ptr<ItemInstance> itemInstance)
{
    return itemInstance != nullptr && itemInstance->id == Item::mac10_Id;
}

int Mac10Item::getMagazineRounds(shared_ptr<ItemInstance> itemInstance)
{
    if (!isMac10(itemInstance) || itemInstance->tag == nullptr || !itemInstance->tag->contains(TAG_MAG)) return BASE_MAGAZINE;
    return itemInstance->tag->getInt(TAG_MAG);
}

int Mac10Item::getMagazineCapacity(shared_ptr<ItemInstance> itemInstance)
{
    return capacityFor(itemInstance);
}

int Mac10Item::getUseDuration(shared_ptr<ItemInstance> itemInstance)
{
    return 72000;
}

void Mac10Item::setPendingAction(shared_ptr<ItemInstance> itemInstance, PendingAction action)
{
    ensureData(itemInstance);
    if (itemInstance && itemInstance->tag) itemInstance->tag->putInt(TAG_ACTION, static_cast<int>(action));
}

void Mac10Item::clearPendingAction(shared_ptr<ItemInstance> itemInstance)
{
    if (itemInstance && itemInstance->tag) itemInstance->tag->putInt(TAG_ACTION, ACTION_NONE);
}

shared_ptr<ItemInstance> Mac10Item::use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
    if (!itemInstance || !level || !player) return itemInstance;
    ensureData(itemInstance);
    PendingAction action = static_cast<PendingAction>(itemInstance->tag->getInt(TAG_ACTION));
    if (action == ACTION_NONE) action = ACTION_FIRE;
    executeAction(itemInstance, level, player, action);
    // The client deliberately leaves the action in NBT until LocalPlayer has serialized
    // UseItemPacket. The authoritative server clears it after executing the matching request.
    if (!level->isClientSide) clearPendingAction(itemInstance);
    return itemInstance;
}

void Mac10Item::executeAction(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, PendingAction action)
{
    switch (action)
    {
    case ACTION_RELOAD: beginReload(itemInstance, level, player); break;
    case ACTION_INSPECT: inspect(itemInstance, level, player); break;
    case ACTION_FIRE:
    default:
        if (itemInstance->tag->getLong(TAG_RELOAD_END) > 0) break;
        fireRound(itemInstance, level, player, false);
        if (enchantLevel(Enchantment::autoFire->id, itemInstance) > 0 && getMagazineRounds(itemInstance) > 0)
            player->startUsingItem(itemInstance, getUseDuration(itemInstance));
        break;
    }
}

int Mac10Item::enchantLevel(int enchantmentId, shared_ptr<ItemInstance> itemInstance)
{
    return (itemInstance != nullptr) ? EnchantmentHelper::getEnchantmentLevel(enchantmentId, itemInstance) : 0;
}

void Mac10Item::inventoryTick(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Entity> owner, int slot, bool selected)
{
    Item::inventoryTick(itemInstance, level, owner, slot, selected);
    if (!itemInstance || !level) return;
    shared_ptr<Player> player = dynamic_pointer_cast<Player>(owner);
    if (!player) return;
    ensureData(itemInstance);

    // Animation events are stored with the stack so a future/custom held-model renderer can
    // consume them without changing the inventory/packet schema. Actual GLTF playback requires
    // the dedicated renderer described in MAC10_INSTALL.md.
    int wasSelected = itemInstance->tag->getInt(TAG_WAS_SELECTED);
    if (selected && !wasSelected)
        setAnimationEvent(itemInstance, level, L"animation.mac10.draw", 6,
            itemInstance->tag->getInt(TAG_MAG) <= 0 ? L"animation.mac10.idle_empty" : L"");
    else if (!selected && wasSelected)
        setAnimationEvent(itemInstance, level, L"animation.mac10.put_away", 4);
    itemInstance->tag->putInt(TAG_WAS_SELECTED, selected ? 1 : 0);

    int64_t animationEnd = itemInstance->tag->getLong(TAG_ANIMATION_END);
    if (selected && animationEnd > 0 && level->getGameTime() >= animationEnd)
    {
        wstring nextClip = itemInstance->tag->getString(TAG_ANIMATION_NEXT);
        if (nextClip.empty() && itemInstance->tag->getInt(TAG_MAG) <= 0) nextClip = L"animation.mac10.idle_empty";
        setAnimationEvent(itemInstance, level, nextClip);
    }

    // Reload work is committed by the simulation tick only when this gun remains selected.
    int64_t reloadEnd = itemInstance->tag->getLong(TAG_RELOAD_END);
    if (reloadEnd > 0)
    {
        if (!selected)
        {
            // Switching away cancels the reload before the insertion point, preserving ammo.
            itemInstance->tag->putLong(TAG_RELOAD_END, 0);
            itemInstance->tag->putInt(TAG_RELOAD_EMPTY, 0);
        }
        else if (level->getGameTime() >= reloadEnd)
        {
            finishReload(itemInstance, level, player);
        }
        return;
    }

    if (!selected || !isUsingThisGun(player)) return;
    if (enchantLevel(Enchantment::autoFire->id, itemInstance) <= 0) return;
    if (itemInstance->tag->getString(TAG_ANIMATION) != L"animation.mac10.auto_fire")
        setAnimationEvent(itemInstance, level, L"animation.mac10.auto_fire");
    if (getMagazineRounds(itemInstance) <= 0)
    {
        player->stopUsingItem();
        return;
    }
    int64_t now = level->getGameTime();
    if (now >= itemInstance->tag->getLong(TAG_NEXT_SHOT))
        fireRound(itemInstance, level, player, true);
}

void Mac10Item::releaseUsing(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, int durationLeft)
{
    // Releasing mouse button ends AutoFire. The Item NBT cooldown prevents repeat-click exploits.
    if (itemInstance && level)
        setAnimationEvent(itemInstance, level, getMagazineRounds(itemInstance) <= 0 ? L"animation.mac10.idle_empty" : L"");
}

void Mac10Item::fireRound(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, bool automatic)
{
    if (!itemInstance || !level || !player) return;
    ensureData(itemInstance);
    if (itemInstance->tag->getLong(TAG_RELOAD_END) > 0) return;

    int rounds = itemInstance->tag->getInt(TAG_MAG);
    if (rounds <= 0) return;
    const int64_t now = level->getGameTime();
    const int64_t nextAllowed = itemInstance->tag->getLong(TAG_NEXT_SHOT);
    if (now < nextAllowed) return;

    const int autoLevel = enchantLevel(Enchantment::autoFire->id, itemInstance);
    if (automatic && autoLevel <= 0) return;

    itemInstance->tag->putInt(TAG_MAG, rounds - 1);
    int shotCount = itemInstance->tag->getInt(TAG_SHOT_COUNT) + 1;
    itemInstance->tag->putInt(TAG_SHOT_COUNT, shotCount);
    // TU43 advances simulation in 50-ms ticks: 5 ticks ~= 4 shots/sec, 2 ticks ~= 10 shots/sec.
    itemInstance->tag->putLong(TAG_NEXT_SHOT, now + (autoLevel > 0 ? 2 : 5));
    if (automatic)
        setAnimationEvent(itemInstance, level, L"animation.mac10.auto_fire");
    else
        setAnimationEvent(itemInstance, level, L"animation.mac10.fire", 4,
            rounds - 1 <= 0 ? L"animation.mac10.idle_empty" : L"");
    if (automatic && rounds - 1 <= 0)
        setAnimationEvent(itemInstance, level, L"animation.mac10.idle_empty");
    player->inventory->setChanged();

    int capacity = capacityFor(itemInstance);
    int precision = enchantLevel(Enchantment::precisionBarrel->id, itemInstance);
    double spreadDeg = player->isSneaking() ? 1.2 : 1.8;
    if (player->isSprinting()) spreadDeg = 2.8;
    if (automatic) spreadDeg += 0.45 + (std::min(shotCount, 7) * 0.12);
    if (precision > 0)
    {
        const float reductions[] = {0.0f, 0.12f, 0.22f, 0.30f};
        spreadDeg *= 1.0 - reductions[(precision > 3) ? 3 : precision];
    }

    // Deterministic spread seed allows visual effects to line up between client and server.
    Vec3 *look = player->getViewVector(1.0f);
    double radians = spreadDeg * 3.14159265358979323846 / 180.0;
    double spread = tan(radians);
    double seed = static_cast<double>((now + 1) * 31 + shotCount * 17);
    double jx = sin(seed * 12.9898) * spread;
    double jy = cos(seed * 78.233) * spread;
    Vec3 *direction = Vec3::newTemp(look->x + jx, look->y + jy, look->z + jx * 0.23)->normalize();

    Vec3 *from = Vec3::newTemp(player->x, player->y + player->getEyeHeight(), player->z);
    Vec3 *to = from->add(direction->x * MAX_RANGE, direction->y * MAX_RANGE, direction->z * MAX_RANGE);
    HitResult *blockResult = level->clip(from, to, false, true);
    Vec3 *end = (blockResult != nullptr) ? blockResult->pos : to;
    double nearest = (blockResult != nullptr) ? from->distanceTo(end) : MAX_RANGE + 0.001;
    shared_ptr<Entity> hitEntity = nullptr;
    vector<shared_ptr<Entity>> *objects = level->getEntities(player, player->bb->expand(to->x - from->x, to->y - from->y, to->z - from->z)->grow(1, 1, 1));
    for (auto &entity : *objects)
    {
        if (!entity || entity == player || !entity->isPickable() || !entity->isAttackable()) continue;
        if (!entity->bb) continue;
        HitResult *hit = entity->bb->grow(0.3, 0.3, 0.3)->clip(from, end);
        if (hit != nullptr)
        {
            double distance = from->distanceTo(hit->pos);
            if (distance < nearest)
            {
                nearest = distance;
                hitEntity = entity;
            }
            delete hit;
        }
    }
    delete blockResult;

    // Combat/damage is server-authoritative; the client only predicts recoil and the action.
    if (!level->isClientSide && hitEntity != nullptr)
    {
        shared_ptr<LivingEntity> target = dynamic_pointer_cast<LivingEntity>(hitEntity);
        if (target && target->isAlive())
        {
            float falloff = nearest <= 12.0 ? 1.0f : (nearest <= 24.0 ? 0.72f : 0.43f);
            float damage = 7.0f * falloff;
            int hollow = enchantLevel(Enchantment::hollowPoint->id, itemInstance);
            if (hollow > 0)
            {
                bool armored = target->getArmorValue() > 0;
                if (!armored) damage *= (hollow >= 2) ? 1.20f : 1.12f;
                else damage *= (hollow >= 2) ? 0.82f : 0.90f;
            }
            target->hurt(DamageSource::playerAttack(player), damage);
        }
    }

    // Temporary placeholder uses the bow-release sound until MAC-10 sound IDs are registered.
    if (level->isClientSide)
    {
        level->playEntitySound(player, eSoundType_RANDOM_BOW, automatic ? 0.55f : 0.72f, automatic ? 1.65f : 1.9f);
        player->xRot -= automatic ? (0.45f + std::min(shotCount, 7) * 0.12f) : 0.55f;
    }

    // Deterministic durability accumulator supports fractional AutoFire wear and Reinforced Parts.
    int reinforced = enchantLevel(Enchantment::reinforcedParts->id, itemInstance);
    const float durabilityBenefits[] = {1.0f, 1.20f, 1.40f, 1.65f};
    float wearMultiplier = automatic ? 1.20f : 1.0f;
    wearMultiplier /= durabilityBenefits[(reinforced > 3) ? 3 : reinforced];
    float wear = itemInstance->tag->getFloat(TAG_WEAR_FRACTION) + wearMultiplier;
    int wholeWear = static_cast<int>(wear);
    itemInstance->tag->putFloat(TAG_WEAR_FRACTION, wear - wholeWear);
    if (wholeWear > 0) itemInstance->hurtAndBreak(wholeWear, dynamic_pointer_cast<LivingEntity>(player));

    if (rounds - 1 <= 0 && automatic) player->stopUsingItem();
}

void Mac10Item::beginReload(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
    if (!itemInstance || !level || !player || !player->inventory) return;
    ensureData(itemInstance);
    if (itemInstance->tag->getLong(TAG_RELOAD_END) > 0) return;
    if (isUsingThisGun(player)) player->stopUsingItem();
    int capacity = capacityFor(itemInstance);
    int rounds = itemInstance->tag->getInt(TAG_MAG);
    if (rounds >= capacity || !player->inventory->hasResource(Item::bullet_9mm_Id)) return;

    bool empty = rounds <= 0;
    float seconds = empty ? 2.15f : 1.65f;
    int rapid = enchantLevel(Enchantment::rapidReload->id, itemInstance);
    if (rapid == 1) seconds *= 0.85f;
    else if (rapid >= 2) seconds *= 0.73f;
    int ticks = static_cast<int>(ceil(seconds * 20.0f));
    itemInstance->tag->putInt(TAG_RELOAD_EMPTY, empty ? 1 : 0);
    itemInstance->tag->putLong(TAG_RELOAD_END, level->getGameTime() + ticks);
    setAnimationEvent(itemInstance, level, empty ? L"animation.mac10.reload_empty" : L"animation.mac10.reload",
        ticks, empty ? L"animation.mac10.idle_boltout" : L"");
}

void Mac10Item::finishReload(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
    if (!itemInstance || !level || !player || !player->inventory) return;
    int capacity = capacityFor(itemInstance);
    int rounds = itemInstance->tag->getInt(TAG_MAG);
    bool wasEmpty = itemInstance->tag->getInt(TAG_RELOAD_EMPTY) != 0;
    int needed = capacity - rounds;
    int ammo = player->inventory->countResource(Item::bullet_9mm_Id, 0);
    int transfer = std::min(needed, ammo);
    for (int i = 0; i < transfer; ++i)
        player->inventory->removeResource(Item::bullet_9mm_Id);
    itemInstance->tag->putInt(TAG_MAG, rounds + transfer);
    itemInstance->tag->putLong(TAG_RELOAD_END, 0);
    itemInstance->tag->putInt(TAG_RELOAD_EMPTY, 0);
    setAnimationEvent(itemInstance, level, wasEmpty ? L"animation.mac10.idle_boltout" : L"");
    player->inventory->setChanged();
    // Empty reloads finish by displaying `idle_boltout`; regular reloads return to idle.
}

void Mac10Item::inspect(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
    if (!itemInstance || !level) return;
    ensureData(itemInstance);
    itemInstance->tag->putLong(TAG_INSPECT_END, level->getGameTime() + 30);
    setAnimationEvent(itemInstance, level, L"animation.mac10.inspect", 30,
        getMagazineRounds(itemInstance) <= 0 ? L"animation.mac10.idle_empty" : L"");
    // This event is ready for the custom held-model renderer; the current legacy renderer does not play GLTF clips.
}
