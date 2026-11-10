#include "stdafx.h"
#include "CamelModel.h"
#include "ModelPart.h"
#include "../Minecraft.World/net.minecraft.world.entity.animal.h"
#include "../Minecraft.World/Mth.h"

// Java camel model coordinates, flattened: the Java head/hump/tail are children of the body, but the
// body never rotates, so the head is a top-level part at body.pivot + head.pivot and the hump/tail
// are extra boxes of the body (box coords shifted by their child pivot).
//
// body pivot (0,4,9.5); head pivot = (0,4,9.5)+(0,-3,-19.5) = (0,1,-10).
CamelModel::CamelModel(float inflate, bool saddleLayer) : QuadrupedModel(12, 0)
{
	texWidth = 128;
	texHeight = 128;

	float g = inflate;

	if (saddleLayer)
	{
		// Same pivots as the camel, but only the saddle gear. (Box layout validated against camel_saddle.png: ~96% of
		// the texture's opaque pixels fall inside these boxes' UV areas.)
		body = new ModelPart(this, 0, 89);
		body->addBox(-7.5f, -12, -23.5f, 15, 12, 27, 0.05f);					// girth straps around the body
		body->texOffs(74, 64)->addBox(-4.5f, -17, -15.5f, 9, 5, 11, 0.5f);		// saddle pad over the hump
		body->texOffs(92, 114)->addBox(-3.5f, -21, -15.5f, 7, 3, 11, 0.5f);		// saddle top
		body->setPos(0, 4, 9.5f);

		head = new ModelPart(this, 60, 87);
		head->addBox(-3.5f, -7, -15, 7, 8, 19, 0.05f);							// bridle: jaw
		head->texOffs(21, 64)->addBox(-3.5f, -21, -15, 7, 14, 7, 0.05f);		// bridle: neck
		head->texOffs(50, 64)->addBox(-2.5f, -21, -21, 5, 5, 6, 0.05f);			// bridle: snout
		head->texOffs(74, 70)->addBox(2.45f, -18, -10, 0, 7, 5, 0.0f);			// reins (right)
		head->mirror();
		head->texOffs(74, 70)->addBox(-2.45f, -18, -10, 0, 7, 5, 0.0f);			// reins (left, mirrored)
		head->mirror();
		head->setPos(0, 1, -10);

		// No saddle gear on the legs, but they must exist for QuadrupedModel's animation/render.
		leg0 = new ModelPart(this, 0, 0);
		leg0->setPos(-4.9f, 1, 9.5f);
		leg1 = new ModelPart(this, 0, 0);
		leg1->setPos(4.9f, 1, 9.5f);
		leg2 = new ModelPart(this, 0, 0);
		leg2->setPos(-4.9f, 1, -10.5f);
		leg3 = new ModelPart(this, 0, 0);
		leg3->setPos(4.9f, 1, -10.5f);

		head->compile(1.0f/16.0f);
		body->compile(1.0f/16.0f);
		leg0->compile(1.0f/16.0f);
		leg1->compile(1.0f/16.0f);
		leg2->compile(1.0f/16.0f);
		leg3->compile(1.0f/16.0f);
		return;
	}

	body = new ModelPart(this, 0, 25);
	body->addBox(-7.5f, -12, -23.5f, 15, 12, 27, g);
	body->texOffs(74, 0)->addBox(-4.5f, -17, -15.5f, 9, 5, 11, g);		// hump   (child pivot 0,-12,-10)
	body->texOffs(122, 0)->addBox(-1.5f, -9, 3.5f, 3, 14, 0, g);		// tail   (child pivot 0,-9,3.5)
	body->setPos(0, 4, 9.5f);

	head = new ModelPart(this, 60, 24);
	head->addBox(-3.5f, -7, -15, 7, 8, 19, g);							// jaw / lower head
	head->texOffs(21, 0)->addBox(-3.5f, -21, -15, 7, 14, 7, g);			// neck
	head->texOffs(50, 0)->addBox(-2.5f, -21, -21, 5, 5, 6, g);			// snout
	head->texOffs(45, 0)->addBox(2, -20.5f, -10.5f, 3, 1, 2, g);		// left ear  (child pivot 2.5,-21,-9.5)
	head->texOffs(67, 0)->addBox(-5, -20.5f, -10.5f, 3, 1, 2, g);		// right ear
	head->setPos(0, 1, -10);

	leg0 = new ModelPart(this, 94, 16);		// right hind
	leg0->addBox(-2.5f, 2, -2.5f, 5, 21, 5, g);
	leg0->setPos(-4.9f, 1, 9.5f);

	leg1 = new ModelPart(this, 58, 16);		// left hind
	leg1->addBox(-2.5f, 2, -2.5f, 5, 21, 5, g);
	leg1->setPos(4.9f, 1, 9.5f);

	leg2 = new ModelPart(this, 0, 26);		// right front
	leg2->addBox(-2.5f, 2, -2.5f, 5, 21, 5, g);
	leg2->setPos(-4.9f, 1, -10.5f);

	leg3 = new ModelPart(this, 0, 0);		// left front
	leg3->addBox(-2.5f, 2, -2.5f, 5, 21, 5, g);
	leg3->setPos(4.9f, 1, -10.5f);

	head->compile(1.0f/16.0f);
	body->compile(1.0f/16.0f);
	leg0->compile(1.0f/16.0f);
	leg1->compile(1.0f/16.0f);
	leg2->compile(1.0f/16.0f);
	leg3->compile(1.0f/16.0f);
}

void CamelModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	QuadrupedModel::setupAnim(time, r, bob, yRot, xRot, scale, entity, uiBitmaskOverrideAnim);

	// The camel's body is already horizontal, and its long legs swing a bit less than a pig's.
	body->xRot = 0;
	leg0->xRot *= 0.7f;
	leg1->xRot *= 0.7f;
	leg2->xRot *= 0.7f;
	leg3->xRot *= 0.7f;

	// The Java head hangs from the body: keep the neck from over-rotating.
	if (head->xRot > 0.6f) head->xRot = 0.6f;
	if (head->xRot < -0.6f) head->xRot = -0.6f;

	// Reset for sitting adjustments.
	body->y = 4;
	head->y = 1;
	leg0->y = leg1->y = leg2->y = leg3->y = 1;

	if (entity != nullptr && entity->instanceof(eTYPE_CAMEL))
	{
		shared_ptr<Camel> camel = dynamic_pointer_cast<Camel>(entity);
		if (camel != nullptr && camel->isSitting())
		{
			// Sit: whole body lowers ~17 units, legs fold flat under it.
			const float drop = 17.0f;
			body->y += drop;
			head->y += drop;
			leg0->y = leg1->y = leg2->y = leg3->y = 1 + drop;
			leg0->xRot = leg1->xRot = 1.5f;		// hind legs folded back
			leg2->xRot = leg3->xRot = -1.5f;	// front legs folded forward
		}
	}
}
