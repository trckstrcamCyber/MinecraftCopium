#pragma once
#include "TileEntityRenderer.h"

class BedModel;

// neoLegacy - Fancy Beds. Draws every bed (the 16 colors and the straw bed) from its BedTileEntity, one half per tile entity.
//
// Colored beds use the standard 64x64 Java bed entity texture, one per color (item/bed_<color>.png), on the same model Java uses
// (head/foot box 16x16x6 plus two legs per half). Straw beds use the real Straw Bed block models (StrawBedModelData.h) with item/bed_straw.png.
// The terrain atlas is full, so beds are not baked into the chunk mesh any more (BedTile::getRenderShape is the entity shape).
class BedRenderer : public TileEntityRenderer
{
private:
	static ResourceLocation BED_LOCATIONS[17];	// 0-15 colors in wool order, 16 = straw
	BedModel *model;

	void renderStraw(float x, float y, float z, bool head, int direction);

public:
	BedRenderer();
	virtual ~BedRenderer();
	virtual void render(shared_ptr<TileEntity> bed, double x, double y, double z, float a, bool setColor, float alpha = 1.0f, bool useCompiled = true);
};
