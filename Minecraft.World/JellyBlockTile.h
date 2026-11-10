#pragma once
#include "HalfTransparentTile.h"

class Level;
class Entity;
class AABB;

// ---------------------------------------------------------------------------------------------------------------------------
// neoLegacy - Jelly Block. Behaves like Java's honey block:
//   * Sliding: an entity falling down the SIDE of one slides down slowly (-0.05/tick) and takes no fall damage.
//   * Landing on top absorbs 80% of the fall (no bounce); 20% of the damage gets through, like Java.
//   * Standing on top slows movement to 40% and halves the jump height.
//   * Pistons drag it and anything on it or hugging its sides, and it does NOT stick to slime blocks (and slime does not stick to
//     it) - which is what lets redstone builders put two flying-machine halves side by side. See PistonBaseTile.cpp.
//   * Its collision box is 14 x 16 x 14 pixels (narrower than the visible block), so an entity can slide down a jelly block that
//     is flush against a solid wall. The model is still the full cube.
//   * Mobs steer around it when pathfinding (PathFinder.cpp).
// Crafted from 4 Jelly Balls (2x2); one Jelly Block crafts back into 4 Jelly Balls.
// ---------------------------------------------------------------------------------------------------------------------------
class JellyBlockTile : public HalfTransparentTile
{
public:
	// Tuning (Java honey-block values). FALL_DAMAGE_FACTOR is the part of the fall damage still taken when landing on top: 20% like
	// Java's honey block (0 would absorb the whole fall).
	static constexpr float SPEED_FACTOR = 0.4f;
	static constexpr float JUMP_FACTOR = 0.5f;
	static constexpr float FALL_DAMAGE_FACTOR = 0.2f;
	static constexpr float SLIDE_SPEED = -0.05f;

	JellyBlockTile(int id);

	// Drawn exactly like the Slime Block: a translucent outer cube around a smaller inner one (see TileRenderer SHAPE_SLIME)
	virtual int getRenderLayer();
	virtual int getRenderShape();
	virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);

	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual int getPistonPushReaction();
	virtual void fallOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity, float fallDistance);
	virtual void entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity);

	// ---- used by Entity::move and LivingEntity::jumpFromGround ------------------------------------------------------------
	// The jelly block (if any) the entity is standing on or inside.
	static bool isOnJelly(Level *level, Entity *entity);
	// Called once per movement step: slows an entity standing on a jelly block.
	static void applyStandingSlowdown(Level *level, Entity *entity);
	// 1.0 normally, JUMP_FACTOR when standing on a jelly block.
	static float getJumpFactor(Level *level, Entity *entity);

private:
	static bool isSlidingDown(int x, int y, int z, Entity *entity);
};
