#pragma once
#include "TileEntityRenderer.h"

class ChestModel;

class ChestRenderer : public TileEntityRenderer
{
private:
	static ResourceLocation CHEST_LARGE_TRAP_LOCATION;
	//static ResourceLocation CHEST_LARGE_XMAS_LOCATION;
	static ResourceLocation CHEST_LARGE_LOCATION;
	static ResourceLocation CHEST_TRAP_LOCATION;
	//static ResourceLocation CHEST_XMAS_LOCATION;
	static ResourceLocation CHEST_LOCATION;
	// neoLegacy - Copper Age (Phase 3). Copper chest entity textures.
	static ResourceLocation CHEST_COPPER_LOCATION;
	static ResourceLocation CHEST_COPPER_EXPOSED_LOCATION;
	static ResourceLocation CHEST_COPPER_WEATHERED_LOCATION;
	static ResourceLocation CHEST_COPPER_OXIDIZED_LOCATION;

	ChestModel *chestModel;
	ChestModel *largeChestModel;
	boolean xmasTextures;

public:
	ChestRenderer();	
	~ChestRenderer();

	void render(shared_ptr<TileEntity>  _chest, double x, double y, double z, float a, bool setColor, float alpha=1.0f, bool useCompiled = true); // 4J added setColor param
};
