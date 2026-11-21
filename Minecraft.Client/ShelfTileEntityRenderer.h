#pragma once

#include "TileEntityRenderer.h"

// neoLegacy - Copper Age (Round 5). Renderer for the copper shelf tile
// entity (Java ShelfBlockEntityRenderer). The shelf stores up to 3
// ItemInstances (one per slot) but Phase 6 registered no renderer, so
// TileEntityRenderDispatcher::getRenderer() returned nullptr and stored
// items were invisible in-world. This renderer draws one item per
// non-empty slot, standing on the shelf front, facing outward — the
// ItemFrame pattern (temp ItemEntity + EntityRenderDispatcher::render
// with bItemFrame = true), positioned to match ShelfTile::use()'s slot
// mapping (thirds along the front face: clickX for N/S, clickZ for
// E/W). It changes no storage, facing, or shape logic.
class ShelfTileEntityRenderer : public TileEntityRenderer
{
public:
	ShelfTileEntityRenderer();
	~ShelfTileEntityRenderer();

	void render(shared_ptr<TileEntity> shelf, double x, double y, double z, float a, bool setColor, float alpha=1.0f, bool useCompiled = true);
};
