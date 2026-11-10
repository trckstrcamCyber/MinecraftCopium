#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "Mth.h"
#include "JellyBlockTile.h"

// ---------------------------------------------------------------------------------------------------------------------------
// neoLegacy - Jelly Block (see JellyBlockTile.h)
// ---------------------------------------------------------------------------------------------------------------------------

JellyBlockTile::JellyBlockTile(int id) : HalfTransparentTile(id, L"jelly_block", Material::clay, false)
{
	friction = 0.6f;
}

int JellyBlockTile::getRenderLayer()
{
	return 2;
}

int JellyBlockTile::getRenderShape()
{
	return Tile::SHAPE_SLIME;
}

bool JellyBlockTile::shouldRenderFace(LevelSource *level, int x, int y, int z, int face)
{
	return true;
}

AABB *JellyBlockTile::getAABB(Level *level, int x, int y, int z)
{
	// 14 x 16 x 14 pixels: one pixel narrower on every horizontal side than the visible cube
	const double inset = 1.0 / 16.0;
	return AABB::newTemp(x + inset, y, z + inset, x + 1.0 - inset, y + 1.0, z + 1.0 - inset);
}

int JellyBlockTile::getPistonPushReaction()
{
	return Material::PUSH_SLIME;	// pulled by sticky pistons like a slime block
}

void JellyBlockTile::fallOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity, float fallDistance)
{
	HalfTransparentTile::fallOn(level, x, y, z, entity, fallDistance);
	if (entity == nullptr) return;

	// No bounce. LivingEntity::checkFallDamage calls this just before it turns fallDistance into damage.
	entity->fallDistance *= FALL_DAMAGE_FACTOR;
}

// ---- sliding down the sides ------------------------------------------------------------------------------------------------
bool JellyBlockTile::isSlidingDown(int x, int y, int z, Entity *entity)
{
	if (entity->onGround) return false;							// standing on something
	if (entity->bb->y0 > y + 1.0 - 1e-7) return false;			// at or above the top of the block
	if (entity->yd >= -0.08) return false;						// only when actually falling

	// the entity must be hugging the SIDE of the block: its centre is farther from the block centre than the 14 pixel collision face
	double d0 = fabs(entity->x - 0.5 - x);
	double d1 = fabs(entity->z - 0.5 - z);
	double d2 = 0.4375 + entity->bbWidth / 2.0;
	return d0 + 1e-7 > d2 || d1 + 1e-7 > d2;
}

void JellyBlockTile::entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
	if (entity == nullptr) return;
	if (!isSlidingDown(x, y, z, entity.get())) return;

	// slide down slowly, keeping the direction of any horizontal movement
	if (entity->yd < -0.13)
	{
		double factor = SLIDE_SPEED / entity->yd;
		entity->xd *= factor;
		entity->zd *= factor;
	}
	entity->yd = SLIDE_SPEED;
	entity->fallDistance = 0.0f;
}

// ---- standing on top -------------------------------------------------------------------------------------------------------
static bool isJellyAt(Level *level, int x, int y, int z)
{
	if (Tile::jellyBlock == nullptr) return false;
	return level->getTile(x, y, z) == Tile::jellyBlock->id;
}

bool JellyBlockTile::isOnJelly(Level *level, Entity *entity)
{
	if (level == nullptr || entity == nullptr) return false;
	int x = Mth::floor(entity->x);
	int z = Mth::floor(entity->z);
	// the block the feet are in, then the block just under them (Java checks 0.5 below the entity's block position)
	return isJellyAt(level, x, Mth::floor(entity->bb->y0 + 0.001), z) || isJellyAt(level, x, Mth::floor(entity->bb->y0 - 0.02), z);
}

void JellyBlockTile::applyStandingSlowdown(Level *level, Entity *entity)
{
	if (entity == nullptr || !entity->onGround) return;
	if (!isOnJelly(level, entity)) return;
	entity->xd *= SPEED_FACTOR;
	entity->zd *= SPEED_FACTOR;
}

float JellyBlockTile::getJumpFactor(Level *level, Entity *entity)
{
	return isOnJelly(level, entity) ? JUMP_FACTOR : 1.0f;
}
