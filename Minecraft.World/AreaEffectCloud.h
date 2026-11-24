#pragma once
using namespace std;

#include "Entity.h"
#include "ParticleTypes.h"

class LivingEntity;
class MobEffectInstance;
class CompoundTag;

class AreaEffectCloud : public Entity
{
public:
	eINSTANCEOF GetType() { return eTYPE_AREA_EFFECT_CLOUD; }
	static Entity *create(Level *level) { return new AreaEffectCloud(level); }

private:
	static const int DATA_RADIUS = 8;
	static const int DATA_COLOR = 9;
	static const int DATA_WAITING = 10;
	static const int DATA_PARTICLE = 11;
	static const int DATA_PARTICLE_RATE = 12;

	int potionValue;
	vector<MobEffectInstance *> extraEffects;
	unordered_map<int, int> reapplicationDelayMap;
	int duration;
	int waitTime;
	int reapplicationDelay;
	bool fixedColor;
	int durationOnUse;
	float radiusOnUse;
	float radiusPerTick;
	weak_ptr<LivingEntity> owner;

	void _init();
	void updateColor();
	void applyMobEffects();
	void generateParticles();

protected:
	virtual void defineSynchedData();
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void addAdditonalSaveData(CompoundTag *tag);

public:
	AreaEffectCloud(Level *level);
	AreaEffectCloud(Level *level, double x, double y, double z);
	virtual ~AreaEffectCloud();

	virtual void tick();
	virtual int getPistonPushReaction();

	void setRadius(float radius);
	float getRadius();
	bool isWaiting();
	void setWaiting(bool waiting);
	int getColor();
	void setFixedColor(int color);
	ePARTICLE_TYPE getParticle();
	void setParticle(ePARTICLE_TYPE particle);
	void setParticleSpawnRateOnTick(int rate);

	void setPotionValue(int potionValue);
	int getPotionValue() { return potionValue; }
	void addEffect(MobEffectInstance *effect);

	int getDuration() { return duration; }
	void setDuration(int duration) { this->duration = duration; }
	void setDurationOnUse(int duration) { durationOnUse = duration; }
	void setRadiusOnUse(float radius) { radiusOnUse = radius; }
	void setRadiusPerTick(float radius) { radiusPerTick = radius; }
	void setWaitTime(int waitTime) { this->waitTime = waitTime; }
	void setOwner(shared_ptr<LivingEntity> owner);
	shared_ptr<LivingEntity> getOwner();
};
