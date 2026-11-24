#pragma once

#include "RotatedPillarTile.h"

class PurpurPillarTile : public RotatedPillarTile
{
	friend class ChunkRebuildData;
public:
	PurpurPillarTile(int id);

protected:
	virtual Icon *getTypeTexture(int type) override;

public:
	virtual void registerIcons(IconRegister *iconRegister) override;
};
