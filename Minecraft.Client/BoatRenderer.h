#pragma once
#include "EntityRenderer.h"

class BoatRenderer : public EntityRenderer
{
private:
	static ResourceLocation BOAT_LOCATION;
	static ResourceLocation BOAT_WOOD_LOCATIONS[7];	// neoLegacy - oak, spruce, birch, jungle, acacia, dark oak, bamboo (the raft)

protected:
	Model *model;
	Model *raftModel;	// neoLegacy - used for wood type 6, the Bamboo Raft
public:
	BoatRenderer();

    virtual void render(shared_ptr<Entity> boat, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};