#include "stdafx.h"
#include "HellFireBoss.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "SmallFireball.h"
#include "Mth.h"
#include "AABB.h"

HellFireBoss::HellFireBoss(Level *level) : Monster(level)
{
    defineSynchedData();
    registerAttributes();
    setHealth(getMaxHealth());
    setSize(1.55f, 3.1f);
    fireImmune = true;
    getNavigation()->setCanFloat(true);
    teleportCooldown = 0;
    anchorScanCooldown = 0;
    cachedAnchorCount = 4;
    attackCooldown = 100;
    attackTelegraph = 0;
    activeAttack = 0;
    cataclysmUsed = false;
    goalSelector.addGoal(0, new FloatGoal(this));
    goalSelector.addGoal(2, new MeleeAttackGoal(this, 1.0f, false));
    goalSelector.addGoal(3, new RangedAttackGoal(this, this, 1.0, 38, 16.0f));
    goalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 24.0f));
    goalSelector.addGoal(7, new RandomLookAroundGoal(this));
    targetSelector.addGoal(1, new HurtByTargetGoal(this, false));
    targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 0, false, false));
    xpReward = Enemy::XP_REWARD_BOSS;
}

void HellFireBoss::defineSynchedData()
{
    Monster::defineSynchedData();
    entityData->define(DATA_PHASE, (int)WILDFIRE);
}

void HellFireBoss::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(100.0f);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.34f);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(12.0f);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(40.0f);
}

int HellFireBoss::getPhase() const { return entityData->getInteger(DATA_PHASE); }

wstring HellFireBoss::getAName()
{
    if (hasCustomName()) return getCustomName();
    return app.GetString(IDS_HELLFIRE_BOSS);
}

int HellFireBoss::findActiveAnchors() const
{
    if (level == nullptr) return 0;
    int wanted = getPhase() == WILDFIRE ? Tile::hellfire_anchor_Id : Tile::soul_anchor_Id;
    int count = 0;
    int cx = Mth::floor(x), cy = Mth::floor(y), cz = Mth::floor(z);
    for (int bx = cx - 18; bx <= cx + 18; ++bx)
        for (int bz = cz - 18; bz <= cz + 18; ++bz)
            for (int by = cy - 8; by <= cy + 14; ++by)
                if (level->getTile(bx, by, bz) == wanted) ++count;
    return count;
}

int HellFireBoss::getActiveAnchorCount() const { return cachedAnchorCount; }

void HellFireBoss::convertAnchorsToSoulAnchors()
{
    if (level == nullptr) return;
    int cx = Mth::floor(x), cy = Mth::floor(y), cz = Mth::floor(z);
    for (int bx = cx - 18; bx <= cx + 18; ++bx)
        for (int bz = cz - 18; bz <= cz + 18; ++bz)
            for (int by = cy - 8; by <= cy + 14; ++by)
                if (level->getTile(bx, by, bz) == Tile::hellfire_anchor_Id)
                    level->setTileAndData(bx, by, bz, Tile::soul_anchor_Id, 0, Tile::UPDATE_CLIENTS);
}

void HellFireBoss::awakenSoulfire()
{
    if (getPhase() != WILDFIRE || level->isClientSide) return;
    entityData->set(DATA_PHASE, (int)SOULFIRE);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(120.0f);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.48f);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(15.0f);
    setHealth(120.0f);
    convertAnchorsToSoulAnchors();
    cachedAnchorCount = findActiveAnchors();
    teleportCooldown = 30;
    attackCooldown = 55;
    attackTelegraph = 0;
    activeAttack = 0;
    for (int i = 0; i < 40; ++i)
        level->addParticle(eParticleType_mobSpell, x + random->nextGaussian() * 1.2,
            y + random->nextFloat() * 3.0f, z + random->nextGaussian() * 1.2,
            0.20f, 0.72f, 1.0f);
}

bool HellFireBoss::hurt(DamageSource *source, float damage)
{
    if (isInvulnerable() || source == nullptr || source == DamageSource::drown) return false;
    cachedAnchorCount = findActiveAnchors();
    float adjusted = damage;
    if (cachedAnchorCount > 0) adjusted *= (getPhase() == WILDFIRE ? 0.75f : 0.60f);
    if (getPhase() == WILDFIRE && adjusted >= getHealth())
        adjusted = getHealth() > 1.0f ? getHealth() - 1.0f : 0.0f;
    bool tookDamage = Monster::hurt(source, adjusted);
    if (!level->isClientSide && getPhase() == WILDFIRE && getHealth() <= 1.0f)
        awakenSoulfire();
    return tookDamage;
}

// Special attacks use a short visible wind-up before applying their effect.
// The warning is deliberately server-independent; damage is only resolved server-side.
void HellFireBoss::beginSpecialAttack()
{
    if (getTarget() == nullptr || attackTelegraph > 0) return;
    bool soul = getPhase() == SOULFIRE;
    if (!soul)
        activeAttack = random->nextInt(3) + 1; // slam, rush, ember burst
    else
        activeAttack = random->nextInt(4) + 4; // soul chains, wraith burst, wave, cataclysm

    if (soul && getHealth() <= getMaxHealth() * 0.25f && !cataclysmUsed)
    {
        activeAttack = 8;
        cataclysmUsed = true;
    }
    attackTelegraph = activeAttack == 8 ? 100 : (soul ? 32 : 42);
}

void HellFireBoss::damageNearby(float radius, float damage, double knockback, bool ignite, bool verticalLaunch)
{
    if (level == nullptr || level->isClientSide) return;
    AABB *area = AABB::newTemp(x - radius, y - 2.0, z - radius,
        x + radius, y + bbHeight + 3.0, z + radius);
    vector<shared_ptr<Entity> > *entities = level->getEntitiesOfClass(typeid(LivingEntity), area);
    if (entities == nullptr) return;
    shared_ptr<LivingEntity> self = dynamic_pointer_cast<LivingEntity>(shared_from_this());
    for (vector<shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        shared_ptr<LivingEntity> victim = dynamic_pointer_cast<LivingEntity>(*it);
        if (victim == nullptr || victim.get() == this || !victim->isAlive()) continue;
        double dx = victim->x - x;
        double dz = victim->z - z;
        double distSq = dx * dx + dz * dz;
        if (distSq > radius * radius) continue;
        if (victim->hurt(DamageSource::mobAttack(self), damage))
        {
            double dist = sqrt(distSq);
            if (dist < 0.01) { dx = 0.0; dz = 1.0; dist = 1.0; }
            victim->knockback(self, (float)knockback, dx / dist, dz / dist);
            if (verticalLaunch) victim->yd += 0.32;
            if (ignite) victim->setOnFire(4);
        }
    }
    delete entities;
}

void HellFireBoss::resolveSpecialAttack()
{
    if (level == nullptr || level->isClientSide) return;
    switch (activeAttack)
    {
    case 1: // Hellfire Slam
        damageNearby(6.0f, 10.0f, 1.8, true, true);
        for (int i = 0; i < 32; ++i)
            level->addParticle(eParticleType_mobSpell, x + random->nextGaussian() * 3.0,
                y + 0.15, z + random->nextGaussian() * 3.0, 0.0f, 0.04f, 0.0f);
        break;
    case 2: // Inferno Rush: a readable forward burst, not an unavoidable tracking hit.
    {
        shared_ptr<LivingEntity> target = getTarget();
        if (target != nullptr)
        {
            double dx = target->x - x, dz = target->z - z;
            double len = sqrt(dx * dx + dz * dz);
            if (len > 0.01) { xd += dx / len * 1.15; zd += dz / len * 1.15; }
        }
        damageNearby(3.2f, 8.0f, 1.2, true, false);
        break;
    }
    case 3: // Ember burst
        for (int i = 0; i < 3; ++i)
            performRangedAttack(getTarget(), 1.0f);
        break;
    case 4: // Soul Chains
        damageNearby(4.5f, 9.0f, 1.0, false, true);
        for (int i = 0; i < 24; ++i)
            level->addParticle(eParticleType_mobSpell, x + random->nextGaussian() * 3.5,
                y + 0.2, z + random->nextGaussian() * 3.5, 0.16f, 0.72f, 1.0f);
        break;
    case 5: // Wraith Orbs
        for (int i = 0; i < 5; ++i)
            performRangedAttack(getTarget(), 1.0f);
        break;
    case 6: // Soulfire Wave
        damageNearby(8.0f, 11.0f, 1.5, false, true);
        for (int i = 0; i < 40; ++i)
            level->addParticle(eParticleType_mobSpell, x + random->nextGaussian() * 5.0,
                y + 0.25, z + random->nextGaussian() * 5.0, 0.16f, 0.72f, 1.0f);
        break;
    case 7: // Eternal Flame: close-range burst with a longer warning.
        damageNearby(5.5f, 12.0f, 1.7, false, true);
        break;
    case 8: // Soulfire Cataclysm, used once below 25% health.
        damageNearby(10.0f, 18.0f, 2.2, false, true);
        for (int i = 0; i < 64; ++i)
            level->addParticle(eParticleType_mobSpell, x + random->nextGaussian() * 7.0,
                y + random->nextFloat() * 2.0f, z + random->nextGaussian() * 7.0,
                0.12f, 0.70f, 1.0f);
        break;
    default: break;
    }
    activeAttack = 0;
}

void HellFireBoss::aiStep()
{
    Monster::aiStep();
    if (level == nullptr) return;
    bool soul = getPhase() == SOULFIRE;
    float red = soul ? 0.18f : 1.0f;
    float green = soul ? 0.72f : 0.16f;
    float blue = soul ? 1.0f : 0.04f;
    for (int i = 0; i < 3; ++i)
        level->addParticle(eParticleType_mobSpell, x + random->nextGaussian() * 0.65,
            y + random->nextFloat() * bbHeight, z + random->nextGaussian() * 0.65,
            red, green, blue);
    if (level->isClientSide) return;
    if (attackTelegraph > 0)
    {
        --attackTelegraph;
        float pr = soul ? 0.12f : 1.0f;
        float pg = soul ? 0.70f : 0.12f;
        float pb = soul ? 1.0f : 0.02f;
        for (int i = 0; i < 5; ++i)
            level->addParticle(soul ? eParticleType_mobSpell : eParticleType_mobSpell,
                x + random->nextGaussian() * (activeAttack == 8 ? 5.0 : 2.0),
                y + 0.15f, z + random->nextGaussian() * (activeAttack == 8 ? 5.0 : 2.0),
                pr, pg, pb);
        if (attackTelegraph == 0) resolveSpecialAttack();
    }
    else if (--attackCooldown <= 0)
    {
        beginSpecialAttack();
        attackCooldown = soul ? 105 : 125;
    }
    if (--anchorScanCooldown <= 0)
    {
        cachedAnchorCount = findActiveAnchors();
        anchorScanCooldown = 10;
    }
    if (soul && cachedAnchorCount > 0 && tickCount % 20 == 0) heal(1.0f);
    shared_ptr<Entity> target = getAttackTarget();
    // Soulfire levitates toward the target's height instead of remaining grounded.
    if (soul && target != nullptr)
    {
        double verticalError = (target->y + 1.35) - y;
        yd = Mth::clamp(static_cast<float>(verticalError * 0.06), -0.18f, 0.18f);
    }
    else if (soul) yd = Mth::clamp(static_cast<float>(yd), -0.12f, 0.12f);
    if (soul && target != nullptr && --teleportCooldown <= 0)
    {
        double tx = target->x + (random->nextBoolean() ? 3.0 : -3.0);
        double tz = target->z + (random->nextBoolean() ? 2.0 : -2.0);
        setPos(tx, target->y, tz);
        teleportCooldown = 100;
    }
}

void HellFireBoss::performRangedAttack(shared_ptr<LivingEntity> target, float power)
{
    if (target == nullptr || level->isClientSide) return;
    double dx = target->x - x;
    double dy = target->y + target->getHeadHeight() - (y + getHeadHeight());
    double dz = target->z - z;
    shared_ptr<LivingEntity> self = dynamic_pointer_cast<LivingEntity>(shared_from_this());
    shared_ptr<SmallFireball> shot = std::make_shared<SmallFireball>(level, self, dx, dy, dz);
    shot->setPos(x, y + getHeadHeight(), z);
    level->addEntity(shot);
    if (getPhase() == SOULFIRE)
        for (int i = 0; i < 8; ++i)
            level->addParticle(eParticleType_mobSpell, x, y + getHeadHeight(), z, 0.16f, 0.72f, 1.0f);
}

void HellFireBoss::addAdditonalSaveData(CompoundTag *tag)
{
    Monster::addAdditonalSaveData(tag);
    tag->putInt(L"HellFirePhase", getPhase());
    tag->putBoolean(L"HellFireCataclysmUsed", cataclysmUsed);
}

void HellFireBoss::readAdditionalSaveData(CompoundTag *tag)
{
    Monster::readAdditionalSaveData(tag);
    int phase = tag->getInt(L"HellFirePhase");
    cataclysmUsed = tag->getBoolean(L"HellFireCataclysmUsed");
    entityData->set(DATA_PHASE, phase == SOULFIRE ? (int)SOULFIRE : (int)WILDFIRE);
    if (getPhase() == SOULFIRE)
    {
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(120.0f);
        getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.48f);
        getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(15.0f);
    }
}

void HellFireBoss::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
    Mob::dropDeathLoot(wasKilledByPlayer, playerBonusLevel);
    if (getPhase() == SOULFIRE) spawnAtLocation(Item::jdemon_fragment_Id, 1);
}

void HellFireBoss::checkDespawn() { noActionTime = 0; }
