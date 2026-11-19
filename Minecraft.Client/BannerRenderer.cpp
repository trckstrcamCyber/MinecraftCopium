#include "stdafx.h"
#include "BannerRenderer.h"
#include "BannerModel.h"
#include "ModelPart.h"
#include "TileEntityRenderDispatcher.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "../Minecraft.World/DyePowderItem.h"
#include "../Minecraft.World/BannerTileEntity.h"
#include "../Minecraft.World/BannerPatterns.h"
#include "../Minecraft.World/Entity.h"
#include "../Minecraft.World/Level.h"
#include "Lighting.h"
#include <cmath>

BannerRenderer *BannerRenderer::instance = nullptr;
ResourceLocation BannerRenderer::BANNER_LOCATION = ResourceLocation(TN_MOB_BANNER_BANNER_BASE);

BannerRenderer::BannerRenderer()
{
	bannerModel = new BannerModel();
}

void BannerRenderer::init(TileEntityRenderDispatcher *dispatcher)
{
	TileEntityRenderer::init(dispatcher);
	instance = this;
}

static void applyBannerColor(int baseColor, float alpha, float brightness = 1.0f)
{
	if (baseColor >= 0 && baseColor < 16)
	{
		int rgb = DyePowderItem::COLOR_RGB[baseColor];
		float r = ((rgb >> 16) & 0xFF) / 255.0f * brightness;
		float g = ((rgb >>  8) & 0xFF) / 255.0f * brightness;
		float b = ( rgb        & 0xFF) / 255.0f * brightness;
		glColor4f(r, g, b, alpha);
	}
	else
	{
		glColor4f(brightness, brightness, brightness, alpha);
	}
}

// The picture of each pattern (a white mask, see mob/banner_layers), in the order of BannerPatterns' table.
static ResourceLocation *patternLocation(int index)
{
	static ResourceLocation locations[BANNER_PATTERN_COUNT] = {
		ResourceLocation(TN_MOB_BANNER_SQUARE_BOTTOM_LEFT),
		ResourceLocation(TN_MOB_BANNER_SQUARE_BOTTOM_RIGHT),
		ResourceLocation(TN_MOB_BANNER_SQUARE_TOP_LEFT),
		ResourceLocation(TN_MOB_BANNER_SQUARE_TOP_RIGHT),
		ResourceLocation(TN_MOB_BANNER_STRIPE_BOTTOM),
		ResourceLocation(TN_MOB_BANNER_STRIPE_TOP),
		ResourceLocation(TN_MOB_BANNER_STRIPE_LEFT),
		ResourceLocation(TN_MOB_BANNER_STRIPE_RIGHT),
		ResourceLocation(TN_MOB_BANNER_STRIPE_CENTER),
		ResourceLocation(TN_MOB_BANNER_STRIPE_MIDDLE),
		ResourceLocation(TN_MOB_BANNER_STRIPE_DOWNRIGHT),
		ResourceLocation(TN_MOB_BANNER_STRIPE_DOWNLEFT),
		ResourceLocation(TN_MOB_BANNER_SMALL_STRIPES),
		ResourceLocation(TN_MOB_BANNER_CROSS),
		ResourceLocation(TN_MOB_BANNER_STRAIGHT_CROSS),
		ResourceLocation(TN_MOB_BANNER_TRIANGLE_BOTTOM),
		ResourceLocation(TN_MOB_BANNER_TRIANGLE_TOP),
		ResourceLocation(TN_MOB_BANNER_TRIANGLES_BOTTOM),
		ResourceLocation(TN_MOB_BANNER_TRIANGLES_TOP),
		ResourceLocation(TN_MOB_BANNER_DIAGONAL_LEFT),
		ResourceLocation(TN_MOB_BANNER_DIAGONAL_RIGHT),
		ResourceLocation(TN_MOB_BANNER_DIAGONAL_UP_LEFT),
		ResourceLocation(TN_MOB_BANNER_DIAGONAL_UP_RIGHT),
		ResourceLocation(TN_MOB_BANNER_CIRCLE),
		ResourceLocation(TN_MOB_BANNER_RHOMBUS),
		ResourceLocation(TN_MOB_BANNER_HALF_VERTICAL),
		ResourceLocation(TN_MOB_BANNER_HALF_HORIZONTAL),
		ResourceLocation(TN_MOB_BANNER_HALF_VERTICAL_RIGHT),
		ResourceLocation(TN_MOB_BANNER_HALF_HORIZONTAL_BOTTOM),
		ResourceLocation(TN_MOB_BANNER_BORDER),
		ResourceLocation(TN_MOB_BANNER_CURLY_BORDER),
		ResourceLocation(TN_MOB_BANNER_CREEPER),
		ResourceLocation(TN_MOB_BANNER_GRADIENT),
		ResourceLocation(TN_MOB_BANNER_GRADIENT_UP),
		ResourceLocation(TN_MOB_BANNER_BRICKS),
		ResourceLocation(TN_MOB_BANNER_SKULL),
		ResourceLocation(TN_MOB_BANNER_FLOWER),
		ResourceLocation(TN_MOB_BANNER_MOJANG)
	};
	return &locations[index];
}

void BannerRenderer::renderModelColoured(int baseColor, float alpha, bool useCompiled, float brightness, const vector<BannerPattern> *patterns)
{
	bindTexture(&BANNER_LOCATION);

	applyBannerColor(baseColor, alpha, brightness);
	bannerModel->bannerSlate->render(1.0f / 16.0f, false);

	// neoLegacy - the patterns, one coloured mask over the cloth after another, each blended over what is already there
	if (patterns != nullptr && !patterns->empty())
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPolygonOffset(-0.3f, -0.3f);	// the layers lie exactly on the cloth: a z-bias (as the chest lid has) keeps them from flickering

		for (size_t i = 0; i < patterns->size() && i < (size_t)BannerPatterns::MAX_LAYERS; i++)
		{
			int index = BannerPatterns::indexOf((*patterns)[i].pattern);
			if (index < 0) continue;

			bindTexture(patternLocation(index));
			applyBannerColor((*patterns)[i].color, alpha, brightness);
			bannerModel->bannerSlate->render(1.0f / 16.0f, false);
		}

		glPolygonOffset(0.0f, 0.0f);
		if (alpha >= 1.0f) glDisable(GL_BLEND);
		bindTexture(&BANNER_LOCATION);
	}

	glColor4f(brightness, brightness, brightness, alpha);
	bannerModel->bannerStand->render(1.0f / 16.0f, useCompiled);
	bannerModel->bannerTop->render(1.0f / 16.0f, useCompiled);
}

void BannerRenderer::render(shared_ptr<TileEntity> _banner, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled)
{
	shared_ptr<BannerTileEntity> banner = dynamic_pointer_cast<BannerTileEntity>(_banner);
	if (!banner) return;

	Tile *tile = banner->getTile();
	if (tile != Tile::standing_banner && tile != Tile::wall_banner) return;
	const float f = 0.6666667f;

	glDisable(GL_LIGHTING);
	glPushMatrix();

	if (tile == Tile::standing_banner)
	{
		glTranslatef((float)x + 0.5f, (float)y + 0.75f * f, (float)z + 0.5f);
		float rot = banner->getData() * 360.0f / 16.0f;
		glRotatef(-rot, 0.0f, 1.0f, 0.0f);
		bannerModel->bannerStand->visible = true;
	}
	else
	{
		glTranslatef((float)x + 0.5f, (float)y - 0.25f * f, (float)z + 0.5f);
		int face = banner->getData();
		float rot = 0.0f;
		if (face == 2) rot = 180.0f;
		if (face == 4) rot =  90.0f;
		if (face == 5) rot = -90.0f;
		glRotatef(-rot, 0.0f, 1.0f, 0.0f);
		glTranslatef(0.0f, -0.3125f, -0.4375f);
		bannerModel->bannerStand->visible = false;
	}

	bannerModel->bannerSlate->y = -32.0f;

	float f3 = (float)(banner->x * 7 + banner->y * 9 + banner->z * 13)
	         + (float)banner->level->getGameTime() + a;
	bannerModel->bannerSlate->xRot = (-0.0125f + 0.01f * cosf(f3 * 3.14159265f * 0.02f)) * 3.14159265f;

	glScalef(f, -f, -f);
	renderModelColoured(banner->getBaseColor(), alpha, useCompiled, 1.0f, &banner->getPatterns());

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glPopMatrix();
	glEnable(GL_LIGHTING);
}

void BannerRenderer::renderBannerForGui(float x, float y, float scaleX, float scaleY, float alpha, int baseColor, const vector<BannerPattern> *patterns)
{
	glPushMatrix();
	glEnable(GL_COLOR_MATERIAL);

	glTranslatef(x, y, 0.0f);
	glScalef(16.0f * scaleX, 16.0f * scaleY, 1.0f);

	glTranslatef(0.5f, 0.72f, 0.0f);

	glRotatef(-25.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(199.0f, 0.0f, 1.0f, 0.0f);
	glScalef(-1.0f, 1.0f, 1.0f);

	const float s = 0.33f;
	glScalef(s, s, s);

	bannerModel->bannerSlate->y   = -32.0f;
	bannerModel->bannerSlate->xRot = 0.0f;
	bannerModel->bannerStand->visible = true;

	Lighting::setAmbient(0.6f);
	renderModelColoured(baseColor, alpha, false, 1.0f, patterns);
	Lighting::setAmbient(0.4f);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_COLOR_MATERIAL);
	glPopMatrix();
}

void BannerRenderer::renderBannerForHand(float alpha, int baseColor, const vector<BannerPattern> *patterns)
{
	glDisable(GL_LIGHTING);
	glPushMatrix();

	bannerModel->bannerSlate->y   = -32.0f;
	bannerModel->bannerSlate->xRot = 0.0f;
	bannerModel->bannerStand->visible = true;

	renderModelColoured(baseColor, alpha, false, 1.0f, patterns);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glPopMatrix();
	glEnable(GL_LIGHTING);
}
