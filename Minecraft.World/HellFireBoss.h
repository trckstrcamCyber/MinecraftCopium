#pragma once
#include "Monster.h"
#include "RangedAttackMob.h"
#include "BossMob.h"

class HellFireBoss : public Monster, public RangedAttackMob, public BossMob
{
public:
    enum Phase { WILDFIRE = 0, SOULFIRE = 1 };
    static const int DATA_PHASE = 21;
    eINSTANCEOF GetType() { return eTYPE_HELLFIRE; }
    static Entity *create(Level *level) { return new HellFireBoss(level); }
    HellFireBoss(Level *level);
    virtual void defineSynchedData();
    virtual void aiStep();
    virtual bool hurt(DamageSource *source, float damage);
    virtual void performRangedAttack(shared_ptr<LivingEntity> target, float power);
    virtual void addAdditonalSaveData(CompoundTag *tag);
    virtual void readAdditionalSaveData(CompoundTag *tag);
    virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);
    virtual void checkDespawn();
    virtual void registerAttributes();
    int getPhase() const;
    int getActiveAnchorCount() const;
    void awakenSoulfire();
    virtual float getMaxHealth() { return Monster::getMaxHealth(); }
    virtual float getHealth() { return Monster::getHealth(); }
    virtual wstring getAName();
    virtual int getDimension() { return Entity::dimension; }
private:
    int findActiveAnchors() const;
    void convertAnchorsToSoulAnchors();
    int teleportCooldown;
    int anchorScanCooldown;
    int cachedAnchorCount;
    int attackCooldown;
    int attackTelegraph;
    int activeAttack;
    bool cataclysmUsed;
    void beginSpecialAttack();
    void resolveSpecialAttack();
    void damageNearby(float radius, float damage, double knockback, bool ignite, bool verticalLaunch);
};
