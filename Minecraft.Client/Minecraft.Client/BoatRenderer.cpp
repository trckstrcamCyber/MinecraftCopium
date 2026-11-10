#include "stdafx.h"
#include "BoatRenderer.h"
#include "BoatModel.h"
#include "RaftModel.h"
#include "../Minecraft.World/net.minecraft.world.entity.item.h"
#include "../Minecraft.World/Mth.h"

ResourceLocation BoatRenderer::BOAT_LOCATION = ResourceLocation(TN_ITEM_BOAT);
ResourceLocation BoatRenderer::BOAT_WOOD_LOCATIONS[7] =
{
	ResourceLocation(TN_ITEM_BOAT),
	ResourceLocation(TN_ITEM_BOAT_SPRUCE),
	ResourceLocation(TN_ITEM_BOAT_BIRCH),
	ResourceLocation(TN_ITEM_BOAT_JUNGLE),
	ResourceLocation(TN_ITEM_BOAT_ACACIA),
	ResourceLocation(TN_ITEM_BOAT_DARK_OAK),
	ResourceLocation(TN_ITEM_BOAT_BAMBOO)
};

BoatRenderer::BoatRenderer()  : EntityRenderer()
{
    this->shadowRadius = 0.5f;
    model = new BoatModel();
    raftModel = new RaftModel();
}

void BoatRenderer::render(shared_ptr<Entity> _boat, double x, double y, double z, float rot, float a)
{
	// 4J - original version used generics and thus had an input parameter of type Boat rather than shared_ptr<Entity>  we have here - 
	// do some casting around instead
 	shared_ptr<Boat> boat = dynamic_pointer_cast<Boat>(_boat);

    glPushMatrix();

    glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));

    glRotatef(180-rot, 0, 1, 0);
    float hurt = boat->getHurtTime() - a;
    float dmg = boat->getDamage() - a;
    if (dmg<0) dmg = 0;
    if (hurt>0)
	{
        glRotatef(Mth::sin(hurt)*hurt*dmg/10*boat->getHurtDir(), 1, 0, 0);
    }

    float ss = 12/16.0f;
    glScalef(ss, ss, ss);
    glScalef(1/ss, 1/ss, 1/ss);

    bindTexture(boat);
    glScalef(-1, -1, 1);
	// neoLegacy - time = paddle stroke phase, r = how much the paddles are rowing (smoothed, partial-tick interpolated)
	float rowTime = boat->rowingTimeO + (boat->rowingTime - boat->rowingTimeO) * a;
	float rowBlend = boat->paddleBlendO + (boat->paddleBlend - boat->paddleBlendO) * a;
    // neoLegacy - the Bamboo Raft (wood type 6) has its own flat model
    Model *m = (boat->getWoodType() == 6) ? raftModel : model;
    m->render(boat, rowTime, rowBlend, -0.1f, 0, 0, 1 / 16.0f, true);
    glPopMatrix();
}

ResourceLocation *BoatRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	shared_ptr<Boat> boat = dynamic_pointer_cast<Boat>(mob);
	if (boat == nullptr) return &BOAT_LOCATION;
	return &BOAT_WOOD_LOCATIONS[boat->getWoodType()];
}