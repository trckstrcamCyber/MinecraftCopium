#pragma once

#include "Entity.h"

class EnderCrystal : public Entity
{
public:
	eINSTANCEOF GetType() { return eTYPE_ENDER_CRYSTAL; };
	static Entity *create(Level *level) { return new EnderCrystal(level); }
private:
	static const int MAX_LIFE = 5;

public:
	static const int serialVersionUID = 0;
	int time;
	int life;

private:
	static const int DATA_REMAINING_LIFE = 8;
	static const int DATA_SHOW_BOTTOM = 9;
	static const int DATA_BEAM_TARGET_X = 10;
	static const int DATA_BEAM_TARGET_Y = 11;
	static const int DATA_BEAM_TARGET_Z = 12;

	void _init(Level *level);

public:
	EnderCrystal(Level *level);
	EnderCrystal(Level *level, double x, double y, double z);

protected:
	virtual bool makeStepSound();
	virtual void defineSynchedData();

public:
	void tick();

protected:
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

public:
	virtual float getShadowHeightOffs();
	virtual bool isPickable();
	virtual bool hurt(DamageSource *source, float damage);

	void setShowBottom(bool show);
	bool showsBottom();

	void setBeamTarget(int x, int y, int z);
	void clearBeamTarget();
	bool hasBeamTarget();
	int getBeamTargetX();
	int getBeamTargetY();
	int getBeamTargetZ();
};