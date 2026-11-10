#pragma once

class Textures;
class ResourceLocation;

// ---------------------------------------------------------------------------------------------------------------------------
// neoLegacy - the 3D Mace held in the hand (first and third person). The model is the Blockbench item model Changes/MaceModel/mace.json,
// baked into MaceModelData.h; the texture is item/mace.png. The inventory icon, dropped item and GUI use the flat sprite in items.png.
// Used by ItemInHandRenderer::renderItem, so every mob holding the item draws it the same way.
// ---------------------------------------------------------------------------------------------------------------------------
class MaceModel
{
public:
	static ResourceLocation *getTextureLocation();

	// Draws the model in the same local space the flat item sprites use: the grip sits at the sprite's grip corner and the head points
	// along the sprite's diagonal. "glint" draws the enchantment-shimmer geometry (texture coordinates from position) instead of the skin.
	static void draw(bool glint);
};
