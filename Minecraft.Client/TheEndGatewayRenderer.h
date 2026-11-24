#pragma once

#include "TileEntityRenderer.h"

class TheEndGatewayRenderer : public TileEntityRenderer
{
private:
	static ResourceLocation END_SKY_LOCATION;
	static ResourceLocation END_PORTAL_LOCATION;
	static ResourceLocation BEAM_LOCATION;
	static int RANDOM_SEED;
	static Random RANDOM;

	FloatBuffer *lb;
	FloatBuffer *modelView;
	FloatBuffer *projection;

public:
	TheEndGatewayRenderer();
	virtual void render(shared_ptr<TileEntity> _gateway, double x, double y, double z, float a, bool setColor, float alpha = 1.0f, bool useCompiled = true);

private:
	FloatBuffer *getBuffer(float a, float b, float c, float d);
	static int getPasses(double distanceSqr);
	void renderBeam(double x, double y, double z, float a, double height, float r, float g, float b);
};
