#pragma once

#include "StructureStart.h"

class MineShaftStart : public StructureStart
{
private:
	bool mesaWood;

public:
	static StructureStart *Create() { return new MineShaftStart(); }
	virtual EStructureStart GetType() { return eStructureStart_MineShaftStart; }

	MineShaftStart();
	MineShaftStart(Level *level, Random *random, int chunkX, int chunkZ);
	virtual void postProcess(Level *level, Random *random, BoundingBox *chunkBB);

protected:
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditonalSaveData(CompoundTag *tag);
};