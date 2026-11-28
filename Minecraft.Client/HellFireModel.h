#pragma once
#include "Model.h"
#include "ModelPart.h"

// Runtime conversion of the Blockbench Wildfire/Soulfire models supplied
// in boss.zip. Geometry, pivots, rotations and UV offsets match the Java
// Blockbench exports; the two phase hierarchies are kept separate so the
// renderer can switch forms without rebuilding model data.
class HellFireModel : public Model
{
private:
    ModelPart *wildfireRoot;
    ModelPart *wildfireTorso;
    ModelPart *wildfireHead;
    ModelPart *wildfireUnderbite;
    ModelPart *wildfireShieldRoot;
    ModelPartArray wildfireShields;

    ModelPart *soulfireRoot;
    ModelPart *soulfireTorso;
    ModelPart *soulfireHead;
    ModelPart *soulfireShieldRoot;
    ModelPartArray soulfireShields;

public:
    HellFireModel();
    int modelVersion();

    virtual void render(shared_ptr<Entity> entity, float time, float r, float bob,
                        float yRot, float xRot, float scale, bool usecompiled);
    virtual void setupAnim(float time, float r, float bob, float yRot, float xRot,
                           float scale, shared_ptr<Entity> entity,
                           unsigned int uiBitmaskOverrideAnim = 0);
};
