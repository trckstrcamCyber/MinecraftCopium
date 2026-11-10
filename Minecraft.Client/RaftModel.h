#pragma once
#include "Model.h"
#include "ModelPart.h"

// neoLegacy - the Bamboo Raft (boat wood type 6): two stacked layers of bamboo planks instead of a hull, plus the same two paddles.
// 128x64 texture, same layout as Java's raft: planks at (0,0), paddles at (0,24) and (40,24).
class RaftModel : public Model
{
public:
	ModelPart *cubes[3];	// [0] = the two plank layers, [1] / [2] = paddles
	RaftModel();
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
};
