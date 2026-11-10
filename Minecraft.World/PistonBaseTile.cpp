#include "stdafx.h"
#include "PistonBaseTile.h"
#include "PistonMovingPiece.h"
#include "PistonPieceEntity.h"
#include "PistonExtensionTile.h"
#include "ObserverTile.h"
#include "Facing.h"
#include "net.minecraft.world.level.h"
#include "../Minecraft.Client/Minecraft.h"
#include "../Minecraft.Client/MultiPlayerLevel.h"
#include "net.minecraft.world.h"
#include "LevelChunk.h"
#include "Dimension.h"
#include <algorithm>
#include <deque>
#include <climits>
#if defined(_WINDOWS64) && defined(MINECRAFT_SERVER_BUILD)
#include "../Minecraft.Server/FourKitBridge.h"
#endif

const wstring PistonBaseTile::EDGE_TEX = L"piston_side";
const wstring PistonBaseTile::PLATFORM_TEX = L"piston_top";
const wstring PistonBaseTile::PLATFORM_STICKY_TEX = L"piston_top_sticky";
const wstring PistonBaseTile::BACK_TEX = L"piston_bottom";
const wstring PistonBaseTile::INSIDE_TEX = L"piston_inner_top";

const float PistonBaseTile::PLATFORM_THICKNESS = 4.0f;

DWORD PistonBaseTile::tlsIdx = TlsAlloc();

// 4J - NOTE - this ignoreUpdate stuff has been removed from the java version, but I'm not currently sure how the java version does without it... there must be
// some other mechanism that we don't have that stops the event from one piston being processed, from causing neighbours to have extra events created for them.
// For us, that means that if we create a piston next to another one, then one of them gets two events to createPush, the second of which fails, leaving the
// piston in a bad (simultaneously extended & not extended) state.
// 4J - ignoreUpdate is a static in java, implementing as TLS here to make thread safe

//I removed the code for ignoreUpdate so the above comment no longer applies ^.^

PistonBaseTile::PistonBaseTile(int id, bool isSticky) : Tile(id, Material::piston, isSolidRender() )
{
	this->isSticky = isSticky;
	setSoundType(SOUND_STONE);
	setDestroyTime(0.5f);

	iconInside = nullptr;
	iconBack = nullptr;
	iconPlatform = nullptr;
}

void PistonBaseTile::createBlockStateDefinition()
{
	if (!m_blockStateDefinition)
		m_blockStateDefinition = new BlockStateDefinition(this);
}

int PistonBaseTile::defaultBlockState()
{
	return 0;
}

int PistonBaseTile::convertBlockStateToLegacyData(BlockState *state)
{
	return state ? (state->value & 0xF) : 0;
}

Tile::BlockState PistonBaseTile::getBlockState(int data)
{
	return Tile::BlockState(data & 0xF);
}

Tile::BlockState PistonBaseTile::getBlockState(LevelSource *level, int x, int y, int z)
{
	return Tile::BlockState(level->getData(x, y, z) & 0xF);
}

Icon *PistonBaseTile::getPlatformTexture()
{
	return iconPlatform;
}

void PistonBaseTile::updateShape(float x0, float y0, float z0, float x1, float y1, float z1)
{
	setShape(x0, y0, z0, x1, y1, z1);
}

Icon *PistonBaseTile::getTexture(int face, int data)
{
	int facing = getFacing(data);

	if (facing > 5)
	{
		return iconPlatform;
	}

	if (face == facing)
	{
		// sorry about this mess...
		// when the piston is extended, either normally
		// or because a piston arm animation, the top
		// texture is the furnace bottom
		ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
		if (isExtended(data) || tls->xx0 > 0 || tls->yy0 > 0 || tls->zz0 > 0 || tls->xx1 < 1 || tls->yy1 < 1 || tls->zz1 < 1)
		{
			return iconInside;
		}
		return iconPlatform;
	}
	if (face == Facing::OPPOSITE_FACING[facing])
	{
		return iconBack;
	}

	return icon;
}

Icon *PistonBaseTile::getTexture(const wstring &name)
{
	if (name.compare(EDGE_TEX) == 0) return Tile::pistonBase->icon;
	if (name.compare(PLATFORM_TEX) == 0) return Tile::pistonBase->iconPlatform;
	if (name.compare(PLATFORM_STICKY_TEX) == 0) return Tile::sticky_piston->iconPlatform;
	if (name.compare(INSIDE_TEX) == 0) return Tile::pistonBase->iconInside;

	return nullptr;
}

//@Override
void PistonBaseTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(EDGE_TEX);
	iconPlatform = iconRegister->registerIcon(isSticky ? PLATFORM_STICKY_TEX : PLATFORM_TEX);
	iconInside = iconRegister->registerIcon(INSIDE_TEX);
	iconBack = iconRegister->registerIcon(BACK_TEX);
}

int PistonBaseTile::getRenderShape()
{
	return SHAPE_PISTON_BASE;
}

bool PistonBaseTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool PistonBaseTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	return false;
}

void PistonBaseTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	int targetData = getNewFacing(level, x, y, z, dynamic_pointer_cast<Player>(by) );
	level->setData(x, y, z, targetData, Tile::UPDATE_CLIENTS);
	if (!level->isClientSide)
	{
		checkIfExtend(level, x, y, z);
	}
}

void PistonBaseTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (!level->isClientSide)
	{
		checkIfExtend(level, x, y, z);
	}
}

void PistonBaseTile::onPlace(Level *level, int x, int y, int z)
{
	if (!level->isClientSide && level->getTileEntity(x, y, z) == nullptr)
	{
		checkIfExtend(level, x, y, z);
	}
}

// forward declaration - the resolver lives further down
static bool resolvePistonStructure(Level *level, int pistonX, int pistonY, int pistonZ, int direction, bool extending,
	std::vector<BlockPos3> &toPush, std::vector<BlockPos3> &toDestroy);

void PistonBaseTile::checkIfExtend(Level *level, int x, int y, int z)
{
	int data = level->getData(x, y, z);
	int facing = getFacing(data);

	if (facing == UNDEFINED_FACING)
	{
		return;
	}
	bool extend = getNeighborSignal(level, x, y, z, facing);

	if (extend && !isExtended(data))
	{
		std::vector<BlockPos3> toPush, toDestroy;
		if (resolvePistonStructure(level, x, y, z, facing, true, toPush, toDestroy))
		{
			level->tileEvent(x, y, z, id, TRIGGER_EXTEND, facing);
		}
	}
	else if (!extend && isExtended(data))
	{
		// neoLegacy - Java: if the block this piston pushed is still on its way (still a moving piece), a retract
		// request becomes a DROP (event 2): the piston lets go instead of pulling it back. That is what makes the
		// 1-tick-pulse observer flying machines work.
		int px = x + Facing::STEP_X[facing] * 2;
		int py = y + Facing::STEP_Y[facing] * 2;
		int pz = z + Facing::STEP_Z[facing] * 2;
		int event = TRIGGER_CONTRACT;
		if (level->getTile(px, py, pz) == Tile::piston_extension_Id && (level->getData(px, py, pz) & 7) == facing)
		{
			shared_ptr<TileEntity> te = level->getTileEntity(px, py, pz);
			shared_ptr<PistonPieceEntity> ppe = dynamic_pointer_cast<PistonPieceEntity>(te);
			if (ppe != nullptr && ppe->isExtending()
				&& (ppe->getProgress(0.0f) < 0.5f || level->getGameTime() == ppe->getLastTicked() || level->isHandlingTick()))
			{
				event = TRIGGER_DROP;
			}
		}
		level->tileEvent(x, y, z, id, event, facing);
	}
}

/**
* This method checks neighbor signals for this block and the block above,
* and directly beneath. However, it avoids checking blocks that would be
* pushed by this block.
* 
* @param level
* @param x
* @param y
* @param z
* @return
*/
bool PistonBaseTile::getNeighborSignal(Level *level, int x, int y, int z, int facing)
{
	// check adjacent neighbors, but not in push direction
	if (facing != Facing::DOWN && level->hasSignal(x, y - 1, z, Facing::DOWN)) return true;
	if (facing != Facing::UP && level->hasSignal(x, y + 1, z, Facing::UP)) return true;
	if (facing != Facing::NORTH && level->hasSignal(x, y, z - 1, Facing::NORTH)) return true;
	if (facing != Facing::SOUTH && level->hasSignal(x, y, z + 1, Facing::SOUTH)) return true;
	if (facing != Facing::EAST && level->hasSignal(x + 1, y, z, Facing::EAST)) return true;
	if (facing != Facing::WEST && level->hasSignal(x - 1, y, z, Facing::WEST)) return true;

	// check signals above
	if (level->hasSignal(x, y, z, 0)) return true;
	if (level->hasSignal(x, y + 2, z, 1)) return true;
	if (level->hasSignal(x, y + 1, z - 1, 2)) return true;
	if (level->hasSignal(x, y + 1, z + 1, 3)) return true;
	if (level->hasSignal(x - 1, y + 1, z, 4)) return true;
	if (level->hasSignal(x + 1, y + 1, z, 5)) return true;

	return false;
}

bool PistonBaseTile::triggerEvent(Level *level, int x, int y, int z, int param1, int facing)
{
	if (!level->isClientSide)
	{
		bool extend = getNeighborSignal(level, x, y, z, facing);

		if (extend && (param1 == TRIGGER_CONTRACT || param1 == TRIGGER_DROP))
		{
			level->setData(x, y, z, facing | EXTENDED_BIT, Tile::UPDATE_CLIENTS);
			return false;
		}
		else if (!extend && param1 == TRIGGER_EXTEND)
		{
			return false;
		}
	}

	if (param1 == TRIGGER_EXTEND)
	{
#if defined(_WINDOWS64) && defined(MINECRAFT_SERVER_BUILD)
		{
			int pushLength = 0;
			int cx = x + Facing::STEP_X[facing];
			int cy = y + Facing::STEP_Y[facing];
			int cz = z + Facing::STEP_Z[facing];
			for (int i = 0; i < MAX_PUSH_DEPTH + 1; i++)
			{
				int block = level->getTile(cx, cy, cz);
				if (block == 0) break;
				if (!isPushable(block, level, cx, cy, cz, true)) break;
				if (Tile::tiles[block]->getPistonPushReaction() == Material::PUSH_DESTROY) { pushLength++; break; }
				pushLength++;
				if (i == MAX_PUSH_DEPTH) break;
				cx += Facing::STEP_X[facing];
				cy += Facing::STEP_Y[facing];
				cz += Facing::STEP_Z[facing];
			}
			if (FourKitBridge::FirePistonExtend(level->dimension->id, x, y, z, facing, pushLength))
			{
				return false;
			}
		}
#endif
		PIXBeginNamedEvent(0,"Create push\n");
		if (!moveBlocks(level, x, y, z, facing, true))
		{
			PIXEndNamedEvent();
			return false;
		}
		// 4J - it is critical that this setData sends data to the client, so the force flag is set (see original note:
		// otherwise the client can restore the old, retracted, state of the base while the arm is extended)
		level->setData(x, y, z, facing | EXTENDED_BIT, Tile::UPDATE_ALL | Tile::UPDATE_MOVE_BY_PISTON, true);
		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_TILE_PISTON_OUT, 0.5f, level->random->nextFloat() * 0.25f + 0.6f);
		PIXEndNamedEvent();
	}
	else if (param1 == TRIGGER_CONTRACT || param1 == TRIGGER_DROP)
	{
#if defined(_WINDOWS64) && defined(MINECRAFT_SERVER_BUILD)
		if (FourKitBridge::FirePistonRetract(level->dimension->id, x, y, z, facing))
		{
			level->setData(x, y, z, facing | EXTENDED_BIT, UPDATE_CLIENTS);
			return false;
		}
#endif
		PIXBeginNamedEvent(0,"Contract phase A\n");
		const int armX = x + Facing::STEP_X[facing];
		const int armY = y + Facing::STEP_Y[facing];
		const int armZ = z + Facing::STEP_Z[facing];

		// a head that is still extending is finished first (a source piece finishes as air - see PistonPieceEntity)
		shared_ptr<TileEntity> prevTileEntity = level->getTileEntity(armX, armY, armZ);
		if (prevTileEntity != nullptr && dynamic_pointer_cast<PistonPieceEntity>(prevTileEntity) != nullptr)
		{
			dynamic_pointer_cast<PistonPieceEntity>(prevTileEntity)->finalTick();
		}

		// the piston base becomes a retracting "source" moving piece (Java setBlock(..., 276) = no neighbour/shape updates)
		stopSharingIfServer(level, x, y, z);	// 4J added
		level->setTileAndData(x, y, z, Tile::piston_extension_Id, facing, Tile::UPDATE_NONE | Tile::UPDATE_KNOWN_SHAPE);
		level->setTileEntity(x, y, z, PistonMovingPiece::newMovingPieceEntity(id, facing, facing, false, true));
		level->updateNeighborsAt(x, y, z, Tile::piston_extension_Id);
		ObserverTile::notifyWatchers(level, x, y, z);	// movingPistonState.updateNeighbourShapes(level, pos, 2)
		PIXEndNamedEvent();

		if (isSticky)
		{
			PIXBeginNamedEvent(0,"Contract sticky\n");
			const int twoX = x + Facing::STEP_X[facing] * 2;
			const int twoY = y + Facing::STEP_Y[facing] * 2;
			const int twoZ = z + Facing::STEP_Z[facing] * 2;
			const int twoTile = level->getTile(twoX, twoY, twoZ);
			bool pistonPiece = false;

			if (twoTile == Tile::piston_extension_Id)
			{
				// the block two steps away is a moving piece THIS piston pushed and that is still travelling
				shared_ptr<PistonPieceEntity> ppe = dynamic_pointer_cast<PistonPieceEntity>(level->getTileEntity(twoX, twoY, twoZ));
				if (ppe != nullptr && ppe->getFacing() == facing && ppe->isExtending())
				{
					ppe->finalTick();
					pistonPiece = true;
				}
			}

			if (!pistonPiece)
			{
				bool canPull = false;
				if (param1 == TRIGGER_CONTRACT && twoTile != 0
					&& isPushable(level, twoX, twoY, twoZ, Facing::OPPOSITE_FACING[facing], false, facing))
				{
					int reaction = Tile::tiles[twoTile]->getPistonPushReaction();
					canPull = reaction == Material::PUSH_NORMAL || reaction == Material::PUSH_SLIME
						|| twoTile == Tile::piston_Id || twoTile == Tile::sticky_piston_Id;
				}

				if (!canPull)
				{
					stopSharingIfServer(level, armX, armY, armZ);
					level->removeTile(armX, armY, armZ);
				}
				else
				{
					moveBlocks(level, x, y, z, facing, false);
				}
			}
			PIXEndNamedEvent();
		}
		else
		{
			stopSharingIfServer(level, armX, armY, armZ);	// 4J added
			level->removeTile(armX, armY, armZ);
		}

		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_TILE_PISTON_IN, 0.5f, level->random->nextFloat() * 0.15f + 0.6f);
	}

	return true;
}

void PistonBaseTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	int data = (forceData == -1 ) ? level->getData(x, y, z) : forceData;

	if (isExtended(data))
	{
		const float thickness = PLATFORM_THICKNESS / 16.0f;
		switch (getFacing(data))
		{
		case Facing::DOWN:
			setShape(0, thickness, 0, 1, 1, 1);
			break;
		case Facing::UP:
			setShape(0, 0, 0, 1, 1 - thickness, 1);
			break;
		case Facing::NORTH:
			setShape(0, 0, thickness, 1, 1, 1);
			break;
		case Facing::SOUTH:
			setShape(0, 0, 0, 1, 1, 1 - thickness);
			break;
		case Facing::WEST:
			setShape(thickness, 0, 0, 1, 1, 1);
			break;
		case Facing::EAST:
			setShape(0, 0, 0, 1 - thickness, 1, 1);
			break;
		}
	}
	else
	{
		setShape(0, 0, 0, 1, 1, 1);
	}
}

void PistonBaseTile::updateDefaultShape()
{
	setShape(0, 0, 0, 1, 1, 1);
}

void PistonBaseTile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
	setShape(0, 0, 0, 1, 1, 1);
	Tile::addAABBs(level, x, y, z, box, boxes, source);
}

AABB *PistonBaseTile::getAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	return Tile::getAABB(level, x, y, z);
}

bool PistonBaseTile::isCubeShaped()
{
	return false;
}

int PistonBaseTile::getFacing(int data)
{
	return data & 0x7;
}

bool PistonBaseTile::isExtended(int data)
{
	return (data & EXTENDED_BIT) != 0;
}

int PistonBaseTile::getNewFacing(Level *level, int x, int y, int z, shared_ptr<LivingEntity> player)
{
	if (Mth::abs(static_cast<float>(player->x) - x) < 2 && Mth::abs(static_cast<float>(player->z) - z) < 2) 
	{
		// If the player is above the block, the slot is on the top
		double py = player->y + 1.82 - player->heightOffset;
		if (py - y > 2)
		{
			return Facing::UP;
		}
		// If the player is below the block, the slot is on the bottom
		if (y - py > 0)
		{
			return Facing::DOWN;
		}
	}
	// The slot is on the side
	int i = Mth::floor(player->yRot * 4.0f / 360.0f + 0.5) & 0x3;
	if (i == 0) return Facing::NORTH;
	if (i == 1) return Facing::EAST;
	if (i == 2) return Facing::SOUTH;
	if (i == 3) return Facing::WEST;
	return 0;
}

bool PistonBaseTile::isPushable(int block, Level *level, int cx, int cy, int cz, bool allowDestroyable)
{
	Tile *tile = Tile::tiles[block];
	if (tile == nullptr) return false; // tu31 tutorial world fix

	// special case for obsidian
	if (block == Tile::obsidian_Id)
	{
		return false;
	}

	if (block == Tile::piston_Id || block == Tile::sticky_piston_Id)
	{
		// special case for piston bases
		if (isExtended(level->getData(cx, cy, cz)))
		{
			return false;
		}
	}
	else
	{
		if (tile->getDestroySpeed(level, cx, cy, cz) == Tile::INDESTRUCTIBLE_DESTROY_TIME)
		{
			return false;
		}

		if (tile->getPistonPushReaction() == Material::PUSH_BLOCK)
		{
			return false;
		}

		if (tile->getPistonPushReaction() == Material::PUSH_DESTROY)
		{
			if(!allowDestroyable)
			{
				return false;
			}
			return true;
		}
	}

	if( tile->isEntityTile() )	// 4J - java uses instanceof EntityTile here
	{
		// may not push tile entities
		return false;
	}

	return true;
}

// neoLegacy - Java PistonBaseBlock.isPushable(): note the block-entity rule. Observers are plain blocks now, so they can be moved.
bool PistonBaseTile::isPushable(Level *level, int x, int y, int z, int direction, bool allowDestroyable, int connectionDirection)
{
	if (y < 0 || y >= Level::maxBuildHeight) return false;
	int minXZ = -(level->dimension->getXZSize() * 16) / 2;
	int maxXZ = (level->dimension->getXZSize() * 16) / 2 - 1;
	if (x <= minXZ || x >= maxXZ || z <= minXZ || z >= maxXZ) return false;

	int block = level->getTile(x, y, z);
	if (block == 0) return true;
	if (direction == Facing::DOWN && y == 0) return false;
	if (direction == Facing::UP && y == Level::maxBuildHeight - 1) return false;
	return isPushable(block, level, x, y, z, allowDestroyable);
}

namespace
{
	// Java PistonStructureResolver, ported 1:1 (12 block limit, slime branching, collision reordering)
	struct PistonResolver
	{
		Level *level;
		BlockPos3 pistonPos;
		int pistonDir;
		bool extending;
		BlockPos3 startPos;
		int pushDir;
		std::vector<BlockPos3> toPush;
		std::vector<BlockPos3> toDestroy;

		PistonResolver(Level *l, int px, int py, int pz, int direction, bool ext)
			: level(l), pistonPos{px, py, pz}, pistonDir(direction), extending(ext)
		{
			if (extending)
			{
				pushDir = direction;
				startPos = rel(pistonPos, direction, 1);
			}
			else
			{
				pushDir = Facing::OPPOSITE_FACING[direction];
				startPos = rel(pistonPos, direction, 2);
			}
		}

		static BlockPos3 rel(const BlockPos3 &p, int dir, int n)
		{
			return BlockPos3{p.x + Facing::STEP_X[dir] * n, p.y + Facing::STEP_Y[dir] * n, p.z + Facing::STEP_Z[dir] * n};
		}
		int tileAt(const BlockPos3 &p) { return level->getTile(p.x, p.y, p.z); }
		static bool isSlime(int tile) { return tile != 0 && Tile::slimeBlock != nullptr && tile == Tile::slimeBlock->id; }
		static bool isJelly(int tile) { return tile != 0 && Tile::jellyBlock != nullptr && tile == Tile::jellyBlock->id; }	// neoLegacy - behaves like Java's honey block
		static bool isSticky(int tile) { return isSlime(tile) || isJelly(tile); }
		static bool canStickToEachOther(int a, int b)
		{
			// Java: honey and slime blocks never stick to each other, which is what lets two flying-machine halves sit side by side
			if (isJelly(a) && isSlime(b)) return false;
			if (isSlime(a) && isJelly(b)) return false;
			return isSticky(a) || isSticky(b);
		}
		int indexOf(const BlockPos3 &p)
		{
			for (size_t i = 0; i < toPush.size(); i++) if (toPush[i] == p) return static_cast<int>(i);
			return -1;
		}
		bool pushable(const BlockPos3 &p, int direction, bool allowDestroy, int connection)
		{
			return PistonBaseTile::isPushable(level, p.x, p.y, p.z, direction, allowDestroy, connection);
		}
		static bool isDestroyed(int tile) { return tile != 0 && Tile::tiles[tile] != nullptr && Tile::tiles[tile]->getPistonPushReaction() == Material::PUSH_DESTROY; }

		bool resolve()
		{
			toPush.clear();
			toDestroy.clear();
			int nextTile = tileAt(startPos);
			if (!pushable(startPos, pushDir, false, pistonDir))
			{
				if (extending && isDestroyed(nextTile))
				{
					toDestroy.push_back(startPos);
					return true;
				}
				return false;
			}
			if (!addBlockLine(startPos, pushDir)) return false;
			for (size_t i = 0; i < toPush.size(); i++)
			{
				if (isSticky(tileAt(toPush[i])) && !addBranchingBlocks(toPush[i])) return false;
			}
			return true;
		}

		bool addBlockLine(const BlockPos3 &start, int direction)
		{
			int nextTile = tileAt(start);
			if (nextTile == 0) return true;
			if (!pushable(start, pushDir, false, direction)) return true;
			if (start == pistonPos) return true;
			if (indexOf(start) >= 0) return true;

			int blockCount = 1;
			if (blockCount + static_cast<int>(toPush.size()) > PistonBaseTile::MAX_PUSH_DEPTH) return false;

			while (isSticky(nextTile))
			{
				BlockPos3 pos = rel(start, Facing::OPPOSITE_FACING[pushDir], blockCount);
				int previousTile = nextTile;
				nextTile = tileAt(pos);
				if (nextTile == 0
					|| !canStickToEachOther(previousTile, nextTile)
					|| !pushable(pos, pushDir, false, Facing::OPPOSITE_FACING[pushDir])
					|| pos == pistonPos)
				{
					break;
				}
				if (++blockCount + static_cast<int>(toPush.size()) > PistonBaseTile::MAX_PUSH_DEPTH) return false;
			}

			int blocksAdded = 0;
			for (int i = blockCount - 1; i >= 0; i--)
			{
				toPush.push_back(rel(start, Facing::OPPOSITE_FACING[pushDir], i));
				blocksAdded++;
			}

			int i = 1;
			while (true)
			{
				BlockPos3 pos = rel(start, pushDir, i);
				int collisionPos = indexOf(pos);
				if (collisionPos > -1)
				{
					reorderListAtCollision(blocksAdded, collisionPos);
					for (int j = 0; j <= collisionPos + blocksAdded; j++)
					{
						BlockPos3 blockPos = toPush[j];
						if (isSticky(tileAt(blockPos)) && !addBranchingBlocks(blockPos)) return false;
					}
					return true;
				}

				nextTile = tileAt(pos);
				if (nextTile == 0) return true;
				if (!pushable(pos, pushDir, true, pushDir) || pos == pistonPos) return false;
				if (isDestroyed(nextTile))
				{
					toDestroy.push_back(pos);
					return true;
				}
				if (static_cast<int>(toPush.size()) >= PistonBaseTile::MAX_PUSH_DEPTH) return false;
				toPush.push_back(pos);
				blocksAdded++;
				i++;
			}
		}

		void reorderListAtCollision(int blocksAdded, int collisionPos)
		{
			std::vector<BlockPos3> head(toPush.begin(), toPush.begin() + collisionPos);
			std::vector<BlockPos3> lastLineAdded(toPush.end() - blocksAdded, toPush.end());
			std::vector<BlockPos3> collisionToLine(toPush.begin() + collisionPos, toPush.end() - blocksAdded);
			toPush.clear();
			toPush.insert(toPush.end(), head.begin(), head.end());
			toPush.insert(toPush.end(), lastLineAdded.begin(), lastLineAdded.end());
			toPush.insert(toPush.end(), collisionToLine.begin(), collisionToLine.end());
		}

		bool addBranchingBlocks(const BlockPos3 &fromPos)
		{
			int fromTile = tileAt(fromPos);
			for (int direction = 0; direction < 6; direction++)
			{
				// only the directions on the other axes - the push axis is handled by addBlockLine
				if ((direction >> 1) != (pushDir >> 1))
				{
					BlockPos3 neighbourPos = rel(fromPos, direction, 1);
					int neighbourTile = tileAt(neighbourPos);
					if (canStickToEachOther(neighbourTile, fromTile) && !addBlockLine(neighbourPos, direction)) return false;
				}
			}
			return true;
		}
	};
}

static bool resolvePistonStructure(Level *level, int pistonX, int pistonY, int pistonZ, int direction, bool extending,
	std::vector<BlockPos3> &toPush, std::vector<BlockPos3> &toDestroy)
{
	PistonResolver resolver(level, pistonX, pistonY, pistonZ, direction, extending);
	if (!resolver.resolve()) return false;
	toPush = resolver.toPush;
	toDestroy = resolver.toDestroy;
	return true;
}

// Java PistonBaseBlock.moveBlocks()
bool PistonBaseTile::moveBlocks(Level *level, int x, int y, int z, int direction, bool extending)
{
	const int armX = x + Facing::STEP_X[direction];
	const int armY = y + Facing::STEP_Y[direction];
	const int armZ = z + Facing::STEP_Z[direction];

	if (!extending && level->getTile(armX, armY, armZ) == Tile::piston_head_Id)
	{
		stopSharingIfServer(level, armX, armY, armZ);
		level->setTileAndData(armX, armY, armZ, 0, 0, Tile::UPDATE_NONE | Tile::UPDATE_KNOWN_SHAPE);
	}

	std::vector<BlockPos3> toPush, toDestroy;
	if (!resolvePistonStructure(level, x, y, z, direction, extending, toPush, toDestroy)) return false;

	struct SavedBlock { int tile; int data; };
	std::vector<SavedBlock> pushed(toPush.size());
	std::vector<BlockPos3> deleteAfterMove = toPush;
	for (size_t i = 0; i < toPush.size(); i++)
	{
		pushed[i].tile = level->getTile(toPush[i].x, toPush[i].y, toPush[i].z);
		pushed[i].data = level->getData(toPush[i].x, toPush[i].y, toPush[i].z);
	}

	std::vector<int> destroyedTiles;
	const int pushDirection = extending ? direction : Facing::OPPOSITE_FACING[direction];

	// blocks that get popped (flowers, torches...) drop their items and become air (flags 18: no shape updates)
	for (int i = static_cast<int>(toDestroy.size()) - 1; i >= 0; i--)
	{
		const BlockPos3 &pos = toDestroy[i];
		int tile = level->getTile(pos.x, pos.y, pos.z);
		if (tile > 0)
		{
			Tile::tiles[tile]->spawnResources(level, pos.x, pos.y, pos.z, level->getData(pos.x, pos.y, pos.z), 0);
			stopSharingIfServer(level, pos.x, pos.y, pos.z);
			level->setTileAndData(pos.x, pos.y, pos.z, 0, 0, Tile::UPDATE_CLIENTS | Tile::UPDATE_KNOWN_SHAPE);
		}
		destroyedTiles.push_back(tile);
	}

	// every pushed block becomes a moving piece one step further along (flags 324: shape updates happen -> observers see it)
	for (int i = static_cast<int>(toPush.size()) - 1; i >= 0; i--)
	{
		BlockPos3 dest = toPush[i];
		dest.x += Facing::STEP_X[pushDirection];
		dest.y += Facing::STEP_Y[pushDirection];
		dest.z += Facing::STEP_Z[pushDirection];

		for (size_t k = 0; k < deleteAfterMove.size(); k++)
		{
			if (deleteAfterMove[k] == dest) { deleteAfterMove.erase(deleteAfterMove.begin() + k); break; }
		}

		stopSharingIfServer(level, dest.x, dest.y, dest.z);
		level->setTileAndData(dest.x, dest.y, dest.z, Tile::piston_extension_Id, direction, Tile::UPDATE_NONE | Tile::UPDATE_MOVE_BY_PISTON);
		level->setTileEntity(dest.x, dest.y, dest.z,
			PistonMovingPiece::newMovingPieceEntity(pushed[i].tile, pushed[i].data, direction, extending, false));
	}

	if (extending)
	{
		const int armData = direction | (isSticky ? PistonExtensionTile::STICKY_BIT : 0);
		for (size_t k = 0; k < deleteAfterMove.size(); k++)
		{
			BlockPos3 a{armX, armY, armZ};
			if (deleteAfterMove[k] == a) { deleteAfterMove.erase(deleteAfterMove.begin() + k); break; }
		}
		stopSharingIfServer(level, armX, armY, armZ);
		level->setTileAndData(armX, armY, armZ, Tile::piston_extension_Id, armData, Tile::UPDATE_NONE | Tile::UPDATE_MOVE_BY_PISTON);
		level->setTileEntity(armX, armY, armZ,
			PistonMovingPiece::newMovingPieceEntity(Tile::piston_head_Id, armData, direction, true, false));
	}

	// positions that were vacated and not refilled turn to air (flags 82: quiet)...
	for (const BlockPos3 &p : deleteAfterMove)
	{
		stopSharingIfServer(level, p.x, p.y, p.z);
		level->setTileAndData(p.x, p.y, p.z, 0, 0, Tile::UPDATE_CLIENTS | Tile::UPDATE_KNOWN_SHAPE | Tile::UPDATE_MOVE_BY_PISTON);
	}
	// ...and then their neighbours get the shape update the quiet change skipped (this is what wakes observers watching them)
	for (const BlockPos3 &p : deleteAfterMove)
	{
		ObserverTile::notifyWatchers(level, p.x, p.y, p.z);
	}

	// finally the usual neighbour updates, in the same order as Java
	for (size_t i = 0; i < toDestroy.size(); i++)
	{
		int idx = static_cast<int>(toDestroy.size()) - 1 - static_cast<int>(i);	// destroyedTiles was filled in reverse
		const BlockPos3 &pos = toDestroy[idx];
		level->updateNeighborsAt(pos.x, pos.y, pos.z, destroyedTiles[i]);
	}
	for (int i = static_cast<int>(toPush.size()) - 1; i >= 0; i--)
	{
		level->updateNeighborsAt(toPush[i].x, toPush[i].y, toPush[i].z, pushed[i].tile);
	}
	if (extending)
	{
		level->updateNeighborsAt(armX, armY, armZ, Tile::piston_head_Id);
	}

	return true;
}

void PistonBaseTile::stopSharingIfServer(Level *level, int x, int y, int z)
{
	if( !level->isClientSide )
	{ 
		MultiPlayerLevel *clientLevel = Minecraft::GetInstance()->getLevel(level->dimension->id);
		if( clientLevel )
		{
			LevelChunk *lc = clientLevel->getChunkAt( x, z );
			lc->stopSharingTilesAndData();
		}
	}
}
