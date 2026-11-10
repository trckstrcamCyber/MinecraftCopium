#include "stdafx.h"
#include "BoatModel.h"

// neoLegacy - uses the modern boat texture layout (128x64):
//   floor (0,0) | stern (0,19) | bow (0,27) | sides (0,35) (0,43) | paddles (62,0) (62,20)
// The boat's bow points along model +x, same as the old model, so BoatRenderer needs no extra rotation.
BoatModel::BoatModel() : Model()
{
	texWidth = 128;
	texHeight = 64;

	cubes[0] = new ModelPart(this, 0, 0);
	cubes[1] = new ModelPart(this, 0, 19);
	cubes[2] = new ModelPart(this, 0, 27);
	cubes[3] = new ModelPart(this, 0, 35);
	cubes[4] = new ModelPart(this, 0, 43);
	cubes[5] = new ModelPart(this, 62, 0);
	cubes[6] = new ModelPart(this, 62, 20);

	// floor
	cubes[0]->addBox(-14.0f, -9.0f, -3.0f, 28, 16, 3);
	cubes[0]->setPos(0, 3, 1);
	// stern / bow
	cubes[1]->addBox(-13.0f, -7.0f, -1.0f, 18, 6, 2);
	cubes[1]->setPos(-15, 4, 4);
	cubes[2]->addBox(-8.0f, -7.0f, -1.0f, 16, 6, 2);
	cubes[2]->setPos(15, 4, 0);
	// sides
	cubes[3]->addBox(-14.0f, -7.0f, -1.0f, 28, 6, 2);
	cubes[3]->setPos(0, 4, -9);
	cubes[4]->addBox(-14.0f, -7.0f, -1.0f, 28, 6, 2);
	cubes[4]->setPos(0, 4, 9);
	// paddles (shaft + blade share the same UV block, like the vanilla model)
	cubes[5]->addBox(-1.0f, 0.0f, -5.0f, 2, 2, 18);
	cubes[5]->addBox(-1.001f, -3.0f, 8.0f, 1, 6, 7);
	cubes[5]->setPos(3, -5, 9);
	cubes[6]->addBox(-1.0f, 0.0f, -5.0f, 2, 2, 18);
	cubes[6]->addBox(0.001f, -3.0f, 8.0f, 1, 6, 7);
	cubes[6]->setPos(3, -5, -9);

	cubes[0]->xRot = PI / 2;
	cubes[1]->yRot = PI / 2 * 3;
	cubes[2]->yRot = PI / 2 * 1;
	cubes[3]->yRot = PI / 2 * 2;
	// resting pose: paddles hang angled down toward the water (render() animates them while rowing)
	cubes[5]->xRot = -PI / 5;
	cubes[6]->xRot = -PI / 5;
	cubes[6]->yRot = PI;

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	for (int i = 0; i < 7; i++)
	{
		cubes[i]->compile(1.0f/16.0f);
	}
}

void BoatModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	// Paddle stroke (same ranges as the vanilla boat): dips the blade into the water and sweeps it back.
	// time = stroke phase in radians, r = 0..1 blend between the resting pose and the stroke.
	const float restX = -PI / 5;
	float strokeX = -PI / 3 + (-PI / 12 + PI / 3) * ((sinf(-time) + 1.0f) * 0.5f);
	float strokeY = -PI / 4 + (PI / 2) * ((sinf(-time + 1.0f) + 1.0f) * 0.5f);
	cubes[5]->xRot = restX + (strokeX - restX) * r;
	cubes[5]->yRot = 0.0f + (strokeY - 0.0f) * r;
	cubes[6]->xRot = restX + (strokeX - restX) * r;
	cubes[6]->yRot = PI + ((PI - strokeY) - PI) * r;

	for (int i = 0; i < 7; i++)
	{
		cubes[i]->render(scale, usecompiled);
	}
}
