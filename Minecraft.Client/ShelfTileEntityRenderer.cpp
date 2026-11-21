#include "stdafx.h"
#include "ShelfTileEntityRenderer.h"
#include "EntityRenderDispatcher.h"
#include "TileRenderer.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/Level.h"
#include "../Minecraft.World/net.minecraft.world.entity.item.h"
#include "../Minecraft.World/net.minecraft.world.item.h"
#include "../Minecraft.World/ShelfTileEntity.h"
#include "../Minecraft.World/ItemInstance.h"
#include "../Minecraft.World/Facing.h"

ShelfTileEntityRenderer::ShelfTileEntityRenderer() : TileEntityRenderer()
{
}

ShelfTileEntityRenderer::~ShelfTileEntityRenderer()
{
}

void ShelfTileEntityRenderer::render(shared_ptr<TileEntity> _shelf, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled)
{
	shared_ptr<ShelfTileEntity> shelf = dynamic_pointer_cast<ShelfTileEntity>(_shelf);
	if (shelf == nullptr || shelf->isEmpty()) return;
	// A level-less shelf has nowhere to draw from (no ItemEntity can be
	// built without a Level); in-world shelves always have one. The
	// shelf has no held/inventory TE form — its item form renders via
	// the TileRenderer cuboid path, not through this renderer.
	Level *level = shelf->getLevel();
	if (level == nullptr) return;

	if (setColor) glColor4f(1, 1, 1, alpha);

	// The facing lives in the TE (round-5 shelf fix, a36f028e); anything
	// outside the four horizontal facings (e.g. a pre-fix shelf whose
	// NBT has no "Facing" key) reads NORTH, the same default
	// ShelfTile::updateShape() uses.
	int facing = shelf->getFacing();
	if (facing != Facing::NORTH && facing != Facing::SOUTH && facing != Facing::WEST && facing != Facing::EAST)
	{
		facing = Facing::NORTH;
	}

	// Yaw that turns the item's front (the billboard's +Z face) to point
	// along the shelf's outward normal. Verified against
	// ItemFrameRenderer::drawItem (glRotatef(180 + yRot)) with
	// HangingEntity::setDir (yRot = dir * 90, flipped to the opposite
	// for NORTH/SOUTH; Direction NORTH = 2, SOUTH = 0, WEST = 1,
	// EAST = 3): NORTH frame -> 180, SOUTH -> 0, WEST -> 270, EAST -> 90.
	float yaw = 180.0f;
	if (facing == Facing::SOUTH) yaw = 0.0f;
	else if (facing == Facing::WEST) yaw = 270.0f;
	else if (facing == Facing::EAST) yaw = 90.0f;

	// The shelf body is a 5/16 slab: front face at 11/16 (NORTH/WEST
	// sides) or 5/16 (SOUTH/EAST sides) of the block. Items sit just
	// outside that face. A flat item is a plane at the origin, so it
	// needs only a hair of clearance; a block item renders as a 0.3125
	// cube centred on the origin (ItemRenderer's frame path: 1.25 *
	// 1/4), so its centre must stand half its width plus a gap outside
	// the face or its back would sink into the slab.
	const float FRONT_FAR = 11.0f / 16.0f;
	const float FRONT_NEAR = 5.0f / 16.0f;
	const float OUT_FLAT = 0.02f;
	const float OUT_BLOCK = 0.17f;
	// Item origin height. ItemRenderer adds a +0.1 bob to the temp
	// entity (age 0, bobOffs 0), so 0.4 puts the visual centre at
	// about the block's mid-height.
	const float ITEM_Y = 0.4f;
	// Flat items render 1/1.95 (~0.513) wide in the item-frame path —
	// wider than a 1/3 slot — so shrink them to slot width. Block
	// items are already 0.3125 wide and need no shrink.
	const float FLAT_SCALE = 0.6f;

	for (unsigned int slot = 0; slot < ShelfTileEntity::SLOT_COUNT; slot++)
	{
		shared_ptr<ItemInstance> item = shelf->getItem(slot);
		if (item == nullptr) continue;

		// Same block-item test as ItemRenderer::render: a terrain icon
		// on a renderable tile (and not the barrier). Everything else
		// takes the flat billboard path.
		Tile *tile = Tile::tiles[item->id];
		bool isBlock = (item->getIconType() == Icon::TYPE_TERRAIN && tile != nullptr && TileRenderer::canRender(tile->getRenderShape()) && item->id != Tile::barrier_Id);

		// Slot centre along the front face — the same thirds
		// ShelfTile::use() maps clicks to (clickX for N/S, clickZ for
		// E/W), so the drawn item sits in the slot it is taken from.
		float along = (static_cast<float>(slot) + 0.5f) / 3.0f;
		float out = isBlock ? OUT_BLOCK : OUT_FLAT;
		float px = 0.0f, pz = 0.0f;
		switch (facing)
		{
		case Facing::SOUTH: px = along; pz = FRONT_NEAR + out; break;
		case Facing::WEST:  px = FRONT_FAR - out; pz = along; break;
		case Facing::EAST:  px = FRONT_NEAR + out; pz = along; break;
		default:            px = along; pz = FRONT_FAR - out; break; // NORTH
		}

		glPushMatrix();
		glTranslatef(static_cast<float>(x) + px, static_cast<float>(y) + ITEM_Y, static_cast<float>(z) + pz);
		glRotatef(yaw, 0, 1, 0);
		if (!isBlock)
		{
			glScalef(FLAT_SCALE, FLAT_SCALE, FLAT_SCALE);
		}

		// ItemFrame pattern: a throwaway ItemEntity (count forced to 1
		// so only one copy draws, bobOffs 0 so it does not spin),
		// rendered through the entity dispatcher with bItemFrame = true
		// — that sets ItemRenderer::m_bItemFrame, so flat items keep
		// the frame orientation instead of billboarding to the camera.
		shared_ptr<ItemEntity> itemEntity = std::make_shared<ItemEntity>(level, 0, 0, 0, item);
		itemEntity->getItem()->count = 1;
		itemEntity->bobOffs = 0;
		EntityRenderDispatcher::instance->render(itemEntity, 0, 0, 0, 0, 0, true);

		glPopMatrix();
	}

	if (setColor) glColor4f(1, 1, 1, 1);
}
