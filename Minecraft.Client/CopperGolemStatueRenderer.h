#pragma once

#include "TileEntityRenderer.h"

class CopperGolemModel;

// neoLegacy - Copper Age (Phase 5). Renderer for the copper golem statue
// tile entity (Java CopperGolemStatueRenderer, copper-code.txt :4643).
// Renders the CopperGolemModel in the statue's pose, with the body texture
// for the statue's oxidation state. No eyes pass: a statue's eyes do not
// glow (the living golem's eyes are a separate emissive layer).
// Playtest check: if the Java statue does show eyes, add the eyes pass.
class CopperGolemStatueRenderer : public TileEntityRenderer
{
private:
	CopperGolemModel *statueModel;
	static ResourceLocation STATUE_LOCATION;
	static ResourceLocation STATUE_EXPOSED_LOCATION;
	static ResourceLocation STATUE_WEATHERED_LOCATION;
	static ResourceLocation STATUE_OXIDIZED_LOCATION;

public:
	CopperGolemStatueRenderer();
	~CopperGolemStatueRenderer();

	void render(shared_ptr<TileEntity> statue, double x, double y, double z, float a, bool setColor, float alpha=1.0f, bool useCompiled = true);
};
