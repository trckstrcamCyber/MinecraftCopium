#include "stdafx.h"
#include "BedRenderer.h"
#include "StrawBedModelData.h"
#include "Model.h"
#include "ModelPart.h"
#include "Tesselator.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/BedTile.h"
#include "../Minecraft.World/BedTileEntity.h"

// ---------------------------------------------------------------------------------------------------------------------------
// The bed model, as in Java Edition (BedRenderer.createHeadLayer / createFootLayer), 64x64 texture:
//   head half  main box texOffs(0, 0),  legs texOffs(50, 6) and (50, 18)
//   foot half  main box texOffs(0, 22), legs texOffs(50, 0) and (50, 12)
// Each half is a 16x16x6 box (model z = thickness) plus two 3x3x3 legs at its far corners.
// ---------------------------------------------------------------------------------------------------------------------------
class BedModel : public Model
{
public:
	ModelPart *headMain, *headLeftLeg, *headRightLeg;
	ModelPart *footMain, *footLeftLeg, *footRightLeg;

	BedModel()
	{
		texWidth = 64;
		texHeight = 64;
		// (PI is the project-wide macro from Definitions.h)

		headMain = new ModelPart(this, 0, 0);
		headMain->addBox(0, 0, 0, 16, 16, 6);

		headLeftLeg = new ModelPart(this, 50, 6);
		headLeftLeg->addBox(0, 6, 0, 3, 3, 3);
		headLeftLeg->xRot = PI / 2;
		headLeftLeg->zRot = PI / 2;

		headRightLeg = new ModelPart(this, 50, 18);
		headRightLeg->addBox(-16, 6, 0, 3, 3, 3);
		headRightLeg->xRot = PI / 2;
		headRightLeg->zRot = PI;

		footMain = new ModelPart(this, 0, 22);
		footMain->addBox(0, 0, 0, 16, 16, 6);

		footLeftLeg = new ModelPart(this, 50, 0);
		footLeftLeg->addBox(0, 6, -16, 3, 3, 3);
		footLeftLeg->xRot = PI / 2;

		footRightLeg = new ModelPart(this, 50, 12);
		footRightLeg->addBox(-16, 6, -16, 3, 3, 3);
		footRightLeg->xRot = PI / 2;
		footRightLeg->zRot = PI * 1.5f;

		// compile now to avoid a hitch the first time a bed is drawn
		headMain->compile(1.0f / 16.0f);
		headLeftLeg->compile(1.0f / 16.0f);
		headRightLeg->compile(1.0f / 16.0f);
		footMain->compile(1.0f / 16.0f);
		footLeftLeg->compile(1.0f / 16.0f);
		footRightLeg->compile(1.0f / 16.0f);
	}

	void renderPiece(bool head, bool useCompiled)
	{
		const float s = 1.0f / 16.0f;
		if (head)
		{
			headMain->render(s, useCompiled);
			headLeftLeg->render(s, useCompiled);
			headRightLeg->render(s, useCompiled);
		}
		else
		{
			footMain->render(s, useCompiled);
			footLeftLeg->render(s, useCompiled);
			footRightLeg->render(s, useCompiled);
		}
	}
};

ResourceLocation BedRenderer::BED_LOCATIONS[17] =
{
	ResourceLocation(TN_ITEM_BED_WHITE),
	ResourceLocation(TN_ITEM_BED_ORANGE),
	ResourceLocation(TN_ITEM_BED_MAGENTA),
	ResourceLocation(TN_ITEM_BED_LIGHT_BLUE),
	ResourceLocation(TN_ITEM_BED_YELLOW),
	ResourceLocation(TN_ITEM_BED_LIME),
	ResourceLocation(TN_ITEM_BED_PINK),
	ResourceLocation(TN_ITEM_BED_GRAY),
	ResourceLocation(TN_ITEM_BED_LIGHT_GRAY),
	ResourceLocation(TN_ITEM_BED_CYAN),
	ResourceLocation(TN_ITEM_BED_PURPLE),
	ResourceLocation(TN_ITEM_BED_BLUE),
	ResourceLocation(TN_ITEM_BED_BROWN),
	ResourceLocation(TN_ITEM_BED_GREEN),
	ResourceLocation(TN_ITEM_BED_RED),
	ResourceLocation(TN_ITEM_BED_BLACK),
	ResourceLocation(TN_ITEM_BED_STRAW)
};

BedRenderer::BedRenderer()
{
	model = new BedModel();
}

BedRenderer::~BedRenderer()
{
	delete model;
}

void BedRenderer::render(shared_ptr<TileEntity> _bed, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled)
{
	shared_ptr<BedTileEntity> bed = dynamic_pointer_cast<BedTileEntity>(_bed);
	if (bed == nullptr) return;

	// a tile entity can outlive its tile for a frame; never draw a bed where there is none
	Tile *tile = bed->getTile();
	if (tile == nullptr || tile->id != Tile::bed_Id) return;

	const int data = bed->getData();
	const int direction = data & BedTile::DIRECTION_MASK;
	const bool head = (data & BedTile::HEAD_PIECE_DATA) != 0;

	int variant = bed->getVariant();
	if (variant < 0 || variant >= BedTileEntity::VARIANT_COUNT) variant = BedTileEntity::DEFAULT_VARIANT;

	if (variant == BedTileEntity::VARIANT_STRAW)
	{
		renderStraw(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), head, direction);
		return;
	}

	// Same transform chain as Java's BedRenderer.renderPiece: lay the model flat (its z becomes down), then turn it to face the
	// way the bed faces. Direction index 0..3 = south, west, north, east, which is exactly 0/90/180/270 degrees of yaw.
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y) + 0.5625f, static_cast<float>(z));
	glRotatef(90.0f, 1, 0, 0);
	glTranslatef(0.5f, 0.5f, 0.5f);
	glRotatef(180.0f + direction * 90.0f, 0, 0, 1);
	glTranslatef(-0.5f, -0.5f, -0.5f);

	bindTexture(&BED_LOCATIONS[variant]);
	model->renderPiece(head, useCompiled);

	glPopMatrix();
}

// ---------------------------------------------------------------------------------------------------------------------------
// Straw bed: the real Straw Bed block models (straw_bed_foot.json / straw_bed_head.json), baked into StrawBedModelData.h. Each half is a
// low mattress (the head half also has the pillow) with loose straw "frills" lying around it, some of which stick out past the edge of
// the block. The models are written for a bed facing south, and the blockstate turns them: south 0, west 90, north 180, east 270 degrees
// clockwise seen from above - exactly the bed's direction index 0..3 times 90.
// Texture: item/bed_straw.png (64x64), the sheet those models were drawn for.
// ---------------------------------------------------------------------------------------------------------------------------
void BedRenderer::renderStraw(float x, float y, float z, bool head, int direction)
{
	glPushMatrix();
	glTranslatef(x, y, z);

	// turn the south-facing model about the middle of the block (clockwise from above = negative rotation about +y in GL)
	glTranslatef(0.5f, 0.0f, 0.5f);
	glRotatef(-90.0f * direction, 0, 1, 0);
	glTranslatef(-0.5f, 0.0f, -0.5f);

	bindTexture(&BED_LOCATIONS[BedTileEntity::VARIANT_STRAW]);

	Tesselator *t = Tesselator::getInstance();
	t->begin();
	t->color(255, 255, 255);

	const StrawBedQuad *quads = head ? kStrawHeadQuads : kStrawFootQuads;
	const int count = head ? kStrawHeadQuadCount : kStrawFootQuadCount;
	const float s = 1.0f / 16.0f;

	for (int i = 0; i < count; ++i)
	{
		const StrawBedQuad &q = quads[i];
		t->normal(q.n[0], q.n[1], q.n[2]);

		// every quad is written twice, once in each winding, so it shows from both sides whether or not back faces are culled
		// (the thin frill planes have only one face in the model and are meant to be seen from above and below)
		for (int pass = 0; pass < 2; ++pass)
		{
			for (int k = 0; k < 4; ++k)
			{
				const int j = pass == 0 ? k : 3 - k;
				t->vertexUV(q.p[j][0] * s, q.p[j][1] * s, q.p[j][2] * s, q.uv[j][0], q.uv[j][1]);
			}
		}
	}

	t->end();

	glPopMatrix();
}
