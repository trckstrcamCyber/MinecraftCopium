#pragma once
#include "Model.h"
#include "ModelPart.h"

// neoLegacy - modern (1.9+) boat model: 128x64 texture, hull + two paddles.
//   cubes[0] = floor, cubes[1] = stern, cubes[2] = bow, cubes[3]/[4] = sides, cubes[5]/[6] = paddles.
class BoatModel : public Model
{
public:
    ModelPart *cubes[7];
    BoatModel();
    virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
};
