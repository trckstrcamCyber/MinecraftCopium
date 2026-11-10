#include "stdafx.h"
#include "RaftModel.h"

RaftModel::RaftModel() : Model()
{
	texWidth = 128;
	texHeight = 64;

	cubes[0] = new ModelPart(this, 0, 0);
	cubes[1] = new ModelPart(this, 0, 24);
	cubes[2] = new ModelPart(this, 40, 24);

	// the raft deck: two layers of planks that share the same texture area (the upper one is 4 wider/lower layer, like Java)
	cubes[0]->addBox(-14.0f, -11.0f, -4.0f, 28, 20, 4);
	cubes[0]->addBox(-14.0f, -9.0f, -8.0f, 28, 16, 4);
	cubes[0]->setPos(0, -2.1f, 1);
	cubes[0]->xRot = PI / 2;

	// paddles (shaft + blade), a little higher than on the boat
	cubes[1]->addBox(-1.0f, 0.0f, -5.0f, 2, 2, 18);
	cubes[1]->addBox(-1.001f, -3.0f, 8.0f, 1, 6, 7);
	cubes[1]->setPos(3, -4, 9);
	cubes[2]->addBox(-1.0f, 0.0f, -5.0f, 2, 2, 18);
	cubes[2]->addBox(0.001f, -3.0f, 8.0f, 1, 6, 7);
	cubes[2]->setPos(3, -4, -9);
	cubes[1]->xRot = -PI / 5;
	cubes[2]->xRot = -PI / 5;
	cubes[2]->yRot = PI;

	for (int i = 0; i < 3; i++)
	{
		cubes[i]->compile(1.0f / 16.0f);
	}
}

void RaftModel::render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled)
{
	// same paddle stroke as BoatModel: time = stroke phase, r = 0..1 blend between the resting pose and rowing
	const float restX = -PI / 5;
	float strokeX = -PI / 3 + (-PI / 12 + PI / 3) * ((sinf(-time) + 1.0f) * 0.5f);
	float strokeY = -PI / 4 + (PI / 2) * ((sinf(-time + 1.0f) + 1.0f) * 0.5f);
	cubes[1]->xRot = restX + (strokeX - restX) * r;
	cubes[1]->yRot = 0.0f + (strokeY - 0.0f) * r;
	cubes[2]->xRot = restX + (strokeX - restX) * r;
	cubes[2]->yRot = PI + ((PI - strokeY) - PI) * r;

	for (int i = 0; i < 3; i++)
	{
		cubes[i]->render(scale, usecompiled);
	}
}
