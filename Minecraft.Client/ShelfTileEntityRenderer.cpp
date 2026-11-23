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
#include "../Minecraft.World/Icon.h"
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

	// Shelf body (TileRenderer::tesselateShelfInWorld, base faces
	// north): back panel z 13-16, bottom board z 11-13 (y 0-4), top
	// board z 11-13 (y 12-16), overlay plane at z 13 (y 4-12). Open
	// cubby: y 4-12 (board top 0.25), z 11-13 (0.6875-0.8125) — 0.125
	// deep. Front face plane is at 11/16 (NORTH/WEST) or 5/16
	// (SOUTH/EAST) of the block.
	// SHELF FIX (2026-11-23, seating — round 2): the staged Part 3
	// cut (OUT_BLOCK 0.17 -> 0.0) was NOT enough. Screenshots
	// 2026-11-23 08:47/08:48 show the lantern floating in front of
	// the shelf, and the head geometry explains it two ways:
	//  (a) HEIGHT. A block item's rendered bottom is NOT at ITEM_Y.
	//      ItemRenderer::render (frame path) translates the temp
	//      entity by a constant +0.1 bob (age 0, bobOffs 0, a = 0, so
	//      sin(0)*0.1+0.1 = 0.1 every frame), then the frame path
	//      adds glTranslatef(0, 0.05, 0) inside the 1.25x scale
	//      (+0.0625), then renderTile centres the model with
	//      glTranslatef(-0.5, -0.5, -0.5) at 0.3125 scale (-0.15625).
	//      Bottom = ITEM_Y + 0.1 + 0.0625 - 0.15625 = ITEM_Y + 0.00625.
	//      With ITEM_Y 0.4 the bottom sat at 0.406 — 0.156 (2.5 px)
	//      ABOVE the board top (0.25): the item hovered mid-cubby.
	//      The same arithmetic holds for the lantern (inventory model
	//      y 0..9/16, base at 0) and for flat items (billboard quad
	//      y -0.25..+0.75, FLAT_SCALE 0.6, 1/1.95 frame scale:
	//      bottom = ITEM_Y + 0.1 - 0.0923 = ITEM_Y + 0.0077 — the
	//      same bottom as block items, so one knob seats both).
	//      ITEM_Y 0.24 puts the bottom at 0.246, sunk 0.004 into the
	//      board top (exact seating would be 0.24375; 0.24 dodges any
	//      coplanar shimmer). Cube top 0.559 clears the top board
	//      (0.75); lantern spans 0.246..0.422; flat top 0.555.
	//  (b) DEPTH. OUT_BLOCK 0.0 centred the cube on the face plane
	//      (front proud 0.156). Java holds items INSIDE the block,
	//      slightly in front of the back panel. OUT_BLOCK -0.0625
	//      centres the 0.3125 cube in the cubby mouth (z 0.75):
	//      front proud only 0.094, back sunk 0.094 into the back
	//      panel (hidden); the overlay plane (z 0.8125) slices the
	//      cube's rear third and still reads as a divider frame. The
	//      lantern (z half-width 0.0586) then sits 0.691..0.809 —
	//      entirely inside the cubby, front essentially flush.
	// OUT_FLAT stays 0.02 (flat items are planes; a plane has no
	// depth to recess and 0.02 proud is invisible). Scope: this file
	// only — ItemRenderer is untouched, so no other tile's item
	// rendering changes.
	const float FRONT_FAR = 11.0f / 16.0f;
	const float FRONT_NEAR = 5.0f / 16.0f;
	const float OUT_FLAT = 0.02f;
	const float OUT_BLOCK = -0.0625f;
	// Item origin height. See the seating derivation above: the
	// rendered bottom is ITEM_Y + 0.00625 (block items) /
	// ITEM_Y + 0.0077 (flat items), so 0.24 seats the bottom on
	// the board top (0.25). The +0.1 bob is constant (fresh temp
	// entity, age 0, bobOffs 0, a = 0), NOT an animation.
	const float ITEM_Y = 0.24f;
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
		// Java parity: slot 0 is the player's LEFT slot. For NORTH and
		// EAST facings the slot order is mirrored (slot 0 would sit at
		// low x / low z, the player's right), so mirror it here to
		// match ShelfTile::use().
		float along = (static_cast<float>(slot) + 0.5f) / 3.0f;
		if (facing == Facing::NORTH || facing == Facing::EAST)
		{
			along = 1.0f - along;
		}
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
		// SHELF FIX (2026-11-22, renderer aliasing): the temp entity
		// MUST hold a CLONE of the stored stack, not the stack itself.
		// ItemEntity::setItem stores the shared_ptr as-is (and
		// getItemInstance returns it directly), so setting count = 1
		// on the temp entity mutated the stored stack every frame —
		// a stack of 64 came back as 1 the next time the shelf was
		// used. ItemInstance::clone copies id, count, aux AND the
		// tag, so the clone renders identically and the stored stack
		// is never touched.
		shared_ptr<ItemEntity> itemEntity = std::make_shared<ItemEntity>(level, 0, 0, 0, ItemInstance::clone(item));
		itemEntity->getItem()->count = 1;
		itemEntity->bobOffs = 0;
		EntityRenderDispatcher::instance->render(itemEntity, 0, 0, 0, 0, 0, true);

		glPopMatrix();
	}

	if (setColor) glColor4f(1, 1, 1, 1);
}
