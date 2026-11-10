#include "stdafx.h"
#include "MaceModel.h"
#include "MaceModelData.h"
#include "Tesselator.h"
#include "Textures.h"
#include "ResourceLocation.h"

static ResourceLocation MACE_MODEL_LOCATION = ResourceLocation(TN_ITEM_MACE);

ResourceLocation *MaceModel::getTextureLocation()
{
	return &MACE_MODEL_LOCATION;
}

// One face as a quad, corners in the order top-left, top-right, bottom-right, bottom-left with the matching texture coordinates. It is
// emitted twice with opposite winding so it shows whether or not back-face culling is on.
static void maceQuad(Tesselator *t, const float c[4][3], const float uv[4][2], bool glint)
{
	for (int pass = 0; pass < 2; ++pass)
	{
		for (int i = 0; i < 4; ++i)
		{
			int k = pass == 0 ? i : 3 - i;
			float u = uv[k][0];
			float v = uv[k][1];
			if (glint)
			{
				// shimmer: texture coordinates follow the position on the model
				u = (c[k][0] + c[k][2]) / 16.0f;
				v = c[k][1] / 16.0f;
			}
			t->vertexUV(c[k][0], c[k][1], c[k][2], u, v);
		}
	}
}

void MaceModel::draw(bool glint)
{
	// model space (1/16 block, grip end of the handle at x=8,y=-7,z=8) -> sprite space: the grip goes to the sprite's grip corner
	// (15/16, 1/16), the long axis is turned onto the sprite diagonal, one model unit is 1/16 of a block
	glPushMatrix();
	glTranslatef(15 / 16.0f, 1 / 16.0f, 0);
	glRotatef(45, 0, 0, 1);
	glScalef(1 / 16.0f, 1 / 16.0f, 1 / 16.0f);
	glTranslatef(-8, 7, -8);

	Tesselator *t = Tesselator::getInstance();
	t->begin();

	// face order in MaceModelBox::uv: north, east, south, west, up, down; mild per-face shading gives the box shapes some depth
	static const float shade[6] = { 0.9f, 0.82f, 0.9f, 0.82f, 1.0f, 0.7f };

	for (int b = 0; b < kMaceModelBoxCount; ++b)
	{
		const MaceModelBox &box = kMaceModelBoxes[b];
		const float fx = box.from[0], fy = box.from[1], fz = box.from[2];
		const float tx = box.to[0], ty = box.to[1], tz = box.to[2];

		for (int face = 0; face < 6; ++face)
		{
			const float *f = &box.uv[face * 4];	// u1, v1, u2, v2
			const float uv[4][2] = { { f[0], f[1] }, { f[2], f[1] }, { f[2], f[3] }, { f[0], f[3] } };
			float c[4][3];

			switch (face)
			{
			case 0:	// north (-z)
				{ const float q[4][3] = { { tx, ty, fz }, { fx, ty, fz }, { fx, fy, fz }, { tx, fy, fz } }; memcpy(c, q, sizeof(c)); t->normal(0, 0, -1); }
				break;
			case 1:	// east (+x)
				{ const float q[4][3] = { { tx, ty, tz }, { tx, ty, fz }, { tx, fy, fz }, { tx, fy, tz } }; memcpy(c, q, sizeof(c)); t->normal(1, 0, 0); }
				break;
			case 2:	// south (+z)
				{ const float q[4][3] = { { fx, ty, tz }, { tx, ty, tz }, { tx, fy, tz }, { fx, fy, tz } }; memcpy(c, q, sizeof(c)); t->normal(0, 0, 1); }
				break;
			case 3:	// west (-x)
				{ const float q[4][3] = { { fx, ty, fz }, { fx, ty, tz }, { fx, fy, tz }, { fx, fy, fz } }; memcpy(c, q, sizeof(c)); t->normal(-1, 0, 0); }
				break;
			case 4:	// up (+y)
				{ const float q[4][3] = { { fx, ty, fz }, { tx, ty, fz }, { tx, ty, tz }, { fx, ty, tz } }; memcpy(c, q, sizeof(c)); t->normal(0, 1, 0); }
				break;
			default:	// down (-y)
				{ const float q[4][3] = { { fx, fy, tz }, { tx, fy, tz }, { tx, fy, fz }, { fx, fy, fz } }; memcpy(c, q, sizeof(c)); t->normal(0, -1, 0); }
				break;
			}

			if (!glint) t->color(shade[face], shade[face], shade[face]);
			maceQuad(t, c, uv, glint);
		}
	}

	t->end();
	glPopMatrix();
}
