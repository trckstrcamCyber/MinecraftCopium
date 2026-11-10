#include "stdafx.h"
#include "GoatModel.h"
#include "ModelPart.h"
#include "../Minecraft.World/net.minecraft.world.entity.animal.h"
#include "../Minecraft.World/Mth.h"

// Port of the Java GoatModel.createBodyLayer() (64x64). Leg slots follow the pig convention used by
// QuadrupedModel: leg0 = right hind, leg1 = left hind, leg2 = right front, leg3 = left front.
GoatModel::GoatModel() : QuadrupedModel(6, 0)
{
	texWidth = 64;
	texHeight = 64;

	float g = 0;

	// Head: ears + goatee on the head itself, horns and the angled nose as part of it.
	head = new ModelPart(this, 2, 61);
	head->addBox(-6, -11, -10, 3, 2, 1, g);					// right ear
	head->mirror();
	head->texOffs(2, 61)->addBox(2, -11, -10, 3, 2, 1, g);	// left ear (mirrored)
	head->mirror();
	head->texOffs(23, 52)->addBox(-0.5f, -3, -14, 0, 7, 5, g);	// goatee
	head->texOffs(12, 55)->addBox(-0.01f, -16, -10, 2, 7, 2, g);	// left horn
	head->texOffs(12, 55)->addBox(-2.99f, -16, -10, 2, 7, 2, g);	// right horn
	head->setPos(1, 14, 0);

	nose = new ModelPart(this, 34, 46);
	nose->addBox(-3, -4, -8, 5, 7, 10, g);
	nose->setPos(0, -8, -8);
	nose->xRot = 0.9599f;										// ~55 degrees down
	head->addChild(nose);

	// Body: the long back block plus the taller chest block over the front half.
	body = new ModelPart(this, 1, 1);
	body->addBox(-4, -17, -7, 9, 11, 16, g);
	body->texOffs(0, 28)->addBox(-5, -18, -8, 11, 14, 11, g);
	body->setPos(0, 24, 0);

	leg0 = new ModelPart(this, 49, 29);		// right hind
	leg0->addBox(0, 4, 0, 3, 6, 3, g);
	leg0->setPos(-3, 14, 4);

	leg1 = new ModelPart(this, 36, 29);		// left hind
	leg1->addBox(0, 4, 0, 3, 6, 3, g);
	leg1->setPos(1, 14, 4);

	leg2 = new ModelPart(this, 35, 2);		// right front
	leg2->addBox(0, 0, 0, 3, 10, 3, g);
	leg2->setPos(-3, 14, -6);

	leg3 = new ModelPart(this, 49, 2);		// left front
	leg3->addBox(0, 0, 0, 3, 10, 3, g);
	leg3->setPos(1, 14, -6);

	// 4J added - compile now to avoid random performance hit first time cubes are rendered
	nose->compile(1.0f/16.0f);
	head->compile(1.0f/16.0f);
	body->compile(1.0f/16.0f);
	leg0->compile(1.0f/16.0f);
	leg1->compile(1.0f/16.0f);
	leg2->compile(1.0f/16.0f);
	leg3->compile(1.0f/16.0f);
}

void GoatModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	QuadrupedModel::setupAnim(time, r, bob, yRot, xRot, scale, entity, uiBitmaskOverrideAnim);

	// QuadrupedModel stands its body box up (90 degrees); the goat's body is already horizontal.
	body->xRot = 0;

	// Charging pose: head bowed, horns forward.
	if (entity != nullptr && entity->instanceof(eTYPE_GOAT))
	{
		shared_ptr<Goat> goat = dynamic_pointer_cast<Goat>(entity);
		if (goat != nullptr && goat->isRamming())
		{
			head->xRot = 0.55f;
		}
	}
}
