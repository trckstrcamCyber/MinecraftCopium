#include "stdafx.h"
#include "FrostbiteModel.h"
#include "ModelPart.h"
#include "../Minecraft.World/Mth.h"
#include "../Minecraft.World/net.minecraft.world.entity.h"
#include "../Minecraft.World/Frostbite.h"

// Replaces whatever box the part had with one at the given UV origin (mirrored boxes flip their texture like a mirrored left limb).
void FrostbiteModel::rebuild(ModelPart *part, int u, int v, bool mirror, float x, float y, float z, int w, int h, int d, float g)
{
	part->cubes.clear();	// the standard-skin box is dropped (models are built once, so the leak is a few bytes)
	part->texOffs(u, v);
	part->bMirror = mirror;
	part->addHumanoidBox(x, y, z, w, h, d, g);
	part->bMirror = false;
	part->compile(1.0f / 16.0f);
}

FrostbiteModel::FrostbiteModel() : ZombieModel(0.0f, false)	// 64x64 humanoid, not an armor model
{
	//           part      u   v  mirror   box (x, y, z, w, h, d)       inflate
	rebuild(head,    0,  0, false, -4, -8, -4, 8,  8, 8, 0.0f);
	rebuild(body,   32,  0, false, -4,  0, -2, 8, 12, 4, 0.0f);

	// the model's own "right" limbs (arm0 / leg0) are plain boxes, its "left" limbs (arm1 / leg1) use mirrored UVs
	rebuild(arm0,   16, 16, false, -3, -2, -2, 4, 12, 4, 0.0f);
	rebuild(arm1,   16, 32, true,  -1, -2, -2, 4, 12, 4, 0.0f);
	rebuild(leg0,    0, 32, false, -2,  0, -2, 4, 12, 4, 0.0f);
	rebuild(leg1,    0, 16, true,  -2,  0, -2, 4, 12, 4, 0.0f);

	// overlay layers: the existing sleeve / pants parts, inflated by 0.25 like the model file's *_overlay cubes
	if (sleeve0) rebuild(sleeve0, 32, 16, false, -3, -2, -2, 4, 12, 4, 0.25f);
	if (sleeve1) rebuild(sleeve1, 32, 32, true,  -1, -2, -2, 4, 12, 4, 0.25f);
	if (pants0)  rebuild(pants0,  48, 32, false, -2,  0, -2, 4, 12, 4, 0.25f);
	if (pants1)  rebuild(pants1,  48, 16, true,  -2,  0, -2, 4, 12, 4, 0.25f);

	// there is no hat or jacket layer on this skin: empty them so the humanoid renderer draws nothing for them
	if (hair)
	{
		hair->cubes.clear();
		hair->compile(1.0f / 16.0f);
	}
	if (jacket)
	{
		jacket->cubes.clear();
		jacket->compile(1.0f / 16.0f);
	}
}

// the sleeves / pants follow their limbs (the base class does this at the end of setupAnim, but we change the pose after that)
void FrostbiteModel::syncOverlays()
{
	if (sleeve0)
	{
		sleeve0->x = arm0->x; sleeve0->y = arm0->y; sleeve0->z = arm0->z;
		sleeve0->xRot = arm0->xRot; sleeve0->yRot = arm0->yRot; sleeve0->zRot = arm0->zRot;
	}
	if (sleeve1)
	{
		sleeve1->x = arm1->x; sleeve1->y = arm1->y; sleeve1->z = arm1->z;
		sleeve1->xRot = arm1->xRot; sleeve1->yRot = arm1->yRot; sleeve1->zRot = arm1->zRot;
	}
	if (pants0)
	{
		pants0->x = leg0->x; pants0->y = leg0->y; pants0->z = leg0->z;
		pants0->xRot = leg0->xRot; pants0->yRot = leg0->yRot; pants0->zRot = leg0->zRot;
	}
	if (pants1)
	{
		pants1->x = leg1->x; pants1->y = leg1->y; pants1->z = leg1->z;
		pants1->xRot = leg1->xRot; pants1->yRot = leg1->yRot; pants1->zRot = leg1->zRot;
	}
}

void FrostbiteModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity, unsigned int uiBitmaskOverrideAnim)
{
	// "idle_alert" in the model file: arms straight out in front (and swinging when it attacks) while it hunts something. The target only
	// exists on the server, so the Frostbite keeps a synched "aggressive" flag that this reads; otherwise plain humanoid walking.
	shared_ptr<Frostbite> frostbite = dynamic_pointer_cast<Frostbite>(entity);
	bool alert = frostbite != nullptr && frostbite->isAggressive();

	if (alert)
	{
		ZombieModel::setupAnim(time, r, bob, yRot, xRot, scale, entity, uiBitmaskOverrideAnim);
	}
	else
	{
		HumanoidModel::setupAnim(time, r, bob, yRot, xRot, scale, entity, uiBitmaskOverrideAnim);
		// "idle": arms hang a touch forward
		arm0->xRot -= 0.09f;
		arm1->xRot -= 0.09f;
	}

	// the constant shiver from the model's idle animation: a one degree wobble of the whole body about its vertical axis
	float shiver = Mth::sin(bob * 0.9f) * 0.02f;
	head->yRot += shiver;
	body->yRot += shiver;
	arm0->yRot += shiver;
	arm1->yRot += shiver;
	leg0->yRot += shiver;
	leg1->yRot += shiver;

	syncOverlays();
}
