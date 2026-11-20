#include "stdafx.h"
#include "../Minecraft.World/CopperGolem.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "CopperGolemModel.h"
#include "CopperGolemRenderer.h"
#include "ModelPart.h"
#include "TextureAtlas.h"
#include "../Minecraft.World/TileItem.h"

ResourceLocation CopperGolemRenderer::GOLEM_LOCATION = ResourceLocation(TN_MOB_COPPER_GOLEM);
ResourceLocation CopperGolemRenderer::GOLEM_EXPOSED_LOCATION = ResourceLocation(TN_MOB_COPPER_GOLEM_EXPOSED);
ResourceLocation CopperGolemRenderer::GOLEM_WEATHERED_LOCATION = ResourceLocation(TN_MOB_COPPER_GOLEM_WEATHERED);
ResourceLocation CopperGolemRenderer::GOLEM_OXIDIZED_LOCATION = ResourceLocation(TN_MOB_COPPER_GOLEM_OXIDIZED);
ResourceLocation CopperGolemRenderer::GOLEM_EYES_LOCATION = ResourceLocation(TN_MOB_COPPER_GOLEM_EYES);
ResourceLocation CopperGolemRenderer::GOLEM_EXPOSED_EYES_LOCATION = ResourceLocation(TN_MOB_COPPER_GOLEM_EXPOSED_EYES);
ResourceLocation CopperGolemRenderer::GOLEM_WEATHERED_EYES_LOCATION = ResourceLocation(TN_MOB_COPPER_GOLEM_WEATHERED_EYES);
ResourceLocation CopperGolemRenderer::GOLEM_OXIDIZED_EYES_LOCATION = ResourceLocation(TN_MOB_COPPER_GOLEM_OXIDIZED_EYES);

CopperGolemRenderer::CopperGolemRenderer() : MobRenderer(new CopperGolemModel(), 0.5f)
{
	golemModel = static_cast<CopperGolemModel *>(model);
	this->setArmor(model);
}

void CopperGolemRenderer::render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a)
{
	MobRenderer::render(mob, x, y, z, rot, a);
}

ResourceLocation *CopperGolemRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	shared_ptr<CopperGolem> golem = dynamic_pointer_cast<CopperGolem>(mob);
	if (golem == nullptr) return &GOLEM_LOCATION;
	switch (golem->getWeatherState())
	{
	case CopperGolem::WEATHER_STATE_EXPOSED:
		return &GOLEM_EXPOSED_LOCATION;
	case CopperGolem::WEATHER_STATE_WEATHERED:
		return &GOLEM_WEATHERED_LOCATION;
	case CopperGolem::WEATHER_STATE_OXIDIZED:
		return &GOLEM_OXIDIZED_LOCATION;
	default:
		return &GOLEM_LOCATION;
	}
}

int CopperGolemRenderer::prepareArmor(shared_ptr<LivingEntity> _mob, int layer, float a)
{
	// Eyes layer (Java LivingEntityEmissiveLayer): the eyes texture is
	// mostly transparent; rendered additively at full brightness over the
	// body. EndermanRenderer::prepareArmor precedent.
	shared_ptr<CopperGolem> mob = dynamic_pointer_cast<CopperGolem>(_mob);

	if (layer != 0) return -1;

	ResourceLocation *eyes = &GOLEM_EYES_LOCATION;
	if (mob != nullptr)
	{
		switch (mob->getWeatherState())
		{
		case CopperGolem::WEATHER_STATE_EXPOSED:
			eyes = &GOLEM_EXPOSED_EYES_LOCATION;
			break;
		case CopperGolem::WEATHER_STATE_WEATHERED:
			eyes = &GOLEM_WEATHERED_EYES_LOCATION;
			break;
		case CopperGolem::WEATHER_STATE_OXIDIZED:
			eyes = &GOLEM_OXIDIZED_EYES_LOCATION;
			break;
		default:
			break;
		}
	}

	bindTexture(eyes);
	float br = 1;
	glEnable(GL_BLEND);
#ifdef _XBOX
	glDisable(GL_ALPHA_TEST);
#endif
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_LIGHTING);

	if (mob != nullptr && mob->isInvisible())
	{
		glDepthMask(false);
	}
	else
	{
		glDepthMask(true);
	}

	if (SharedConstants::TEXTURE_LIGHTING)
	{
		int col = 0xf0f0;
		int u = col % 65536;
		int v = col / 65536;

		glMultiTexCoord2f(GL_TEXTURE1, u / 1.0f, v / 1.0f);
		glColor4f(1, 1, 1, 1);
	}

	glEnable(GL_LIGHTING);
	glColor4f(1, 1, 1, br);
	return 1;
}

void CopperGolemRenderer::additionalRendering(shared_ptr<LivingEntity> _mob, float a)
{
	shared_ptr<CopperGolem> mob = dynamic_pointer_cast<CopperGolem>(_mob);

	MobRenderer::additionalRendering(mob, a);

	// Antenna block (Java BlockDecorationLayer): render the block item
	// carried on the antenna, if any. The antenna starts empty and
	// nothing fills it in Phase 5, so this path is inert until a fill
	// path is settled — and the transform below is an estimate from the
	// Enderman carried-block precedent (translate to the antenna tip,
	// half scale), NOT visually verified. Tune it in playtest once an
	// antenna item exists.
	shared_ptr<ItemInstance> antenna = (mob != nullptr) ? mob->getAntennaItem() : nullptr;
	if (antenna == nullptr) return;
	if (dynamic_cast<TileItem *>(antenna->getItem()) == nullptr) return;
	Tile *antennaTile = Tile::tiles[antenna->id];
	if (antennaTile == nullptr) return;

	glEnable(GL_RESCALE_NORMAL);
	glPushMatrix();

	// Antenna tip is 1.5 blocks above the feet; a half-size block sits
	// on it (centre ~1.75 blocks up). Block space is corner-based, so
	// offset by half the scaled size to centre it on the antenna.
	float s = 0.5f;
	glTranslatef(-s / 2.0f, 1.5f, -s / 2.0f);
	glScalef(-s, -s, s);

	if (SharedConstants::TEXTURE_LIGHTING)
	{
		int col = mob->getLightColor(a);
		int u = col % 65536;
		int v = col / 65536;
		glMultiTexCoord2f(GL_TEXTURE1, u / 1.0f, v / 1.0f);
		glColor4f(1, 1, 1, 1);
	}

	glColor4f(1, 1, 1, 1);
	bindTexture(&TextureAtlas::LOCATION_BLOCKS);
	tileRenderer->renderTile(antennaTile, antenna->getAuxValue(), 1);
	glPopMatrix();
	glDisable(GL_RESCALE_NORMAL);
}
