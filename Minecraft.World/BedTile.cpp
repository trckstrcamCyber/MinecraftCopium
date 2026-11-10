#include "stdafx.h"
#include "Dimension.h"
#include "net.minecraft.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"
#include "BedTile.h"
#include "BedTileEntity.h"

int BedTile::HEAD_DIRECTION_OFFSETS[4][2] =
{
	{ 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 0 }
};

BedTile::BedTile(int id) : DirectionalTile(id, Material::cloth, isSolidRender())
{
	setShape();

	iconEnd = nullptr;
	iconSide = nullptr;
	iconTop = nullptr;

	_isEntityTile = true;	// neoLegacy - makes LevelChunk::getTileEntity create the BedTileEntity on demand (village beds, older placements)
	m_dropVariant = BedTileEntity::DEFAULT_VARIANT;
}

shared_ptr<TileEntity> BedTile::newTileEntity(Level *level)
{
	return std::make_shared<BedTileEntity>();
}

void BedTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	DirectionalTile::onRemove(level, x, y, z, id, data);
	level->removeTileEntity(x, y, z);
}

int BedTile::cloneTileData(Level *level, int x, int y, int z)
{
	int variant = BedTileEntity::getVariantAt(level, x, y, z);
	return (variant >= 0) ? variant : BedTileEntity::DEFAULT_VARIANT;
}

void BedTile::removeBothHalves(Level *level, int x, int y, int z)
{
	int data = level->getData(x, y, z);
	int direction = getDirection(data);
	int footX = x, footZ = z, headX = x, headZ = z;
	if (isHeadPiece(data))
	{
		footX -= HEAD_DIRECTION_OFFSETS[direction][0];
		footZ -= HEAD_DIRECTION_OFFSETS[direction][1];
	}
	else
	{
		headX += HEAD_DIRECTION_OFFSETS[direction][0];
		headZ += HEAD_DIRECTION_OFFSETS[direction][1];
	}
	// Foot first: removing the head first would make the foot drop a bed item as it falls apart (neighborChanged).
	if (level->getTile(footX, y, footZ) == Tile::bed_Id) level->removeTile(footX, y, footZ);
	if (level->getTile(headX, y, headZ) == Tile::bed_Id) level->removeTile(headX, y, headZ);
}

void BedTile::createBlockStateDefinition()
{
	if (!m_blockStateDefinition)
		m_blockStateDefinition = new BlockStateDefinition(this);
}

int BedTile::defaultBlockState()
{
	return 0;
}

int BedTile::convertBlockStateToLegacyData(BlockState *state)
{
	return state ? (state->value & 0xF) : 0;
}

Tile::BlockState BedTile::getBlockState(int data)
{
	return Tile::BlockState(data & 0xF);
}

Tile::BlockState BedTile::getBlockState(LevelSource *level, int x, int y, int z)
{
	return Tile::BlockState(level->getData(x, y, z) & 0xF);
}

// 4J Added override
void BedTile::updateDefaultShape()
{
	setShape();
}

// 4J-PB - Adding a TestUse for tooltip display
bool BedTile::TestUse(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	//if (level->isClientSide) return true;

	int data = level->getData(x, y, z);

	if (!BedTile::isHeadPiece(data))
	{
		// fetch head piece instead
		int direction = getDirection(data);
		x += HEAD_DIRECTION_OFFSETS[direction][0];
		z += HEAD_DIRECTION_OFFSETS[direction][1];
		if (level->getTile(x, y, z) != id)
		{
			return true;
		}
		data = level->getData(x, y, z);
	}

	if (!level->dimension->mayRespawn())
	{
		return false;
	}
	if (BedTile::isOccupied(data))
	{
		return false;
	}

	Player::BedSleepingResult result = player->startSleepInBed(x, y, z, true); // true to just test the start sleep
	if (result == Player::OK)
	{
		return true;
	}

	return false;
}

bool BedTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if( soundOnly) return false;
	if (level->isClientSide) return true;

	int data = level->getData(x, y, z);

	if (!isHeadPiece(data))
	{
		// fetch head piece instead
		int direction = getDirection(data);
		x += HEAD_DIRECTION_OFFSETS[direction][0];
		z += HEAD_DIRECTION_OFFSETS[direction][1];
		if (level->getTile(x, y, z) != id)
		{
			return true;
		}
		data = level->getData(x, y, z);
	}

	if (!level->dimension->mayRespawn() || level->getBiome(x, z) == Biome::hell)
	{
		// neoLegacy - a straw bed simply breaks (no explosion, no drops) where a normal bed would blow up
		if (BedTileEntity::getVariantAt(level, x, y, z) == BedTileEntity::VARIANT_STRAW)
		{
			removeBothHalves(level, x, y, z);
			return true;
		}

		double xc = x + 0.5;
		double yc = y + 0.5;
		double zc = z + 0.5;
		level->removeTile(x, y, z);
		int direction = getDirection(data);
		x += HEAD_DIRECTION_OFFSETS[direction][0];
		z += HEAD_DIRECTION_OFFSETS[direction][1];
		if (level->getTile(x, y, z) == id)
		{
			level->removeTile(x, y, z);
			xc = (xc + x + 0.5) / 2;
			yc = (yc + y + 0.5) / 2;
			zc = (zc + z + 0.5) / 2;
		}
		level->explode(nullptr, x + 0.5f, y + 0.5f, z + 0.5f, 5, true, true);
		return true;
	}

	if (isOccupied(data))
	{
		shared_ptr<Player> sleepingPlayer = nullptr;
		for ( auto& p : level->players )
		{
			if (p->isSleeping())
			{
				Pos pos = p->bedPosition;
				if (pos.x == x && pos.y == y && pos.z == z)
				{
					sleepingPlayer = p;
				}
			}
		}

		if (sleepingPlayer == nullptr)
		{
			setOccupied(level, x, y, z, false);
		}
		else
		{
			player->displayClientMessage(IDS_TILE_BED_OCCUPIED );

			return true;
		}
	}

	Player::BedSleepingResult result = player->startSleepInBed(x, y, z);
	if (result == Player::OK)
	{
		setOccupied(level, x, y, z, true);
		// 4J-PB added
		// are there multiple players in the same world as us?
		if(level->AllPlayersAreSleeping()==false)
		{
			player->displayClientMessage(IDS_TILE_BED_PLAYERSLEEP);
		}       
		return true;
	}

	if (result == Player::NOT_POSSIBLE_NOW)
	{
		player->displayClientMessage(IDS_TILE_BED_NO_SLEEP);
	}
	else if (result == Player::NOT_SAFE)
	{
		player->displayClientMessage(IDS_TILE_BED_NOTSAFE);
	}

	return true;
}

Icon *BedTile::getTexture(int face, int data)
{
	if (face == Facing::DOWN)
	{
		return Tile::wood->getTexture(face);
	}

	int direction = getDirection(data);
	int tileFacing = Direction::RELATIVE_DIRECTION_FACING[direction][face];

	int part = isHeadPiece(data) ? PART_HEAD : PART_FOOT;

	if ((part == PART_HEAD && tileFacing == Facing::NORTH) || (part == PART_FOOT && tileFacing == Facing::SOUTH))
	{
		return iconEnd[part];
	}
	if (tileFacing == Facing::EAST || tileFacing == Facing::WEST)
	{
		return iconSide[part];
	}
	return iconTop[part];
}

void BedTile::registerIcons(IconRegister *iconRegister)
{
	iconTop = new Icon *[2];
	iconTop[0] = iconRegister->registerIcon(L"bed_feet_top");
	iconTop[1] = iconRegister->registerIcon(L"bed_head_top");

	iconEnd = new Icon *[2];
	iconEnd[0] = iconRegister->registerIcon(L"bed_feet_end");
	iconEnd[1] = iconRegister->registerIcon(L"bed_head_end");

	iconSide = new Icon *[2];
	iconSide[0] = iconRegister->registerIcon(L"bed_feet_side");
	iconSide[1] = iconRegister->registerIcon(L"bed_head_side");
}

int BedTile::getRenderShape()
{
	// neoLegacy - Fancy Beds: every bed (all 16 colors + straw) is drawn by BedRenderer from its BedTileEntity, not baked into the
	// terrain atlas (which has no room for ~100 bed faces). The terrain icons above are still used for break particles.
	return Tile::SHAPE_ENTITYTILE_ANIMATED;
}

bool BedTile::isCubeShaped()
{
	return false;
}

bool BedTile::isSolidRender(bool isServerLevel)
{
	return false;
}

void BedTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	setShape();
}

void BedTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	int data = level->getData(x, y, z);
	int direction = getDirection(data);

	if (isHeadPiece(data))
	{
		if (level->getTile(x - HEAD_DIRECTION_OFFSETS[direction][0], y, z - HEAD_DIRECTION_OFFSETS[direction][1]) != id)
		{
			level->removeTile(x, y, z);
		}
	} else
	{
		if (level->getTile(x + HEAD_DIRECTION_OFFSETS[direction][0], y, z + HEAD_DIRECTION_OFFSETS[direction][1]) != id)
		{
			int variant = BedTileEntity::getVariantAt(level, x, y, z);	// neoLegacy - read before the tile entity goes away
			if (variant >= 0) m_dropVariant = variant;
			level->removeTile(x, y, z);
			if (!level->isClientSide)
			{
				Tile::spawnResources(level, x, y, z, data, 0);		// 4J - had to add Tile:: here for C++ since this class doesn't have this overloaded method itself
			}
		}
	}
}

int BedTile::getResource(int data, Random *random, int playerBonusLevel)
{
	if (isHeadPiece(data))
	{
		return 0;
	}
	return Item::bed->id;
}

void BedTile::setShape()
{
	Tile::setShape(0, 0, 0, 1, 9 / 16.0f, 1);
}

bool BedTile::isHeadPiece(int data)
{
	return (data & HEAD_PIECE_DATA) != 0;
}

bool BedTile::isOccupied(int data)
{
	return (data & OCCUPIED_DATA) != 0;
}

void BedTile::setOccupied(Level *level, int x, int y, int z, bool occupied)
{
	int data = level->getData(x, y, z);
	if (occupied)
	{
		data = data | OCCUPIED_DATA;
	} else
	{
		data = data & ~OCCUPIED_DATA;
	}
	level->setData(x, y, z, data, Tile::UPDATE_NONE);
}

Pos *BedTile::findStandUpPosition(Level *level, int x, int y, int z, int skipCount)
{
	int data = level->getData(x, y, z);
	int direction = DirectionalTile::getDirection(data);

	// try to find a clear location near the bed
	for (int step = 0; step <= 1; step++)
	{
		int startX = x - HEAD_DIRECTION_OFFSETS[direction][0] * step - 1;
		int startZ = z - HEAD_DIRECTION_OFFSETS[direction][1] * step - 1;
		int endX = startX + 2;
		int endZ = startZ + 2;

		for (int standX = startX; standX <= endX; standX++)
		{
			for (int standZ = startZ; standZ <= endZ; standZ++)
			{
				// 4J Stu - Changed to check isSolidBlockingTile rather than isEmpty for the blocks that we wish to place the player
				// This allows the player to spawn in blocks with snow, grass etc
				if (level->isTopSolidBlocking(standX, y - 1, standZ) &&
					!level->getMaterial(standX, y, standZ)->isSolidBlocking() &&
					!level->getMaterial(standX, y + 1, standZ)->isSolidBlocking() )
				{
					if (skipCount > 0) {
						skipCount--;
						continue;
					}
					return new Pos(standX, y, standZ);
				}
			}
		}
	}

	return nullptr;
}

void BedTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus)
{
	if (isHeadPiece(data)) return;
	if (level->isClientSide) return;

	// neoLegacy - drop the bed that was actually there (color / straw). The tile entity is usually already gone by now, so fall back to
	// what playerWillDestroy / neighborChanged captured.
	int variant = BedTileEntity::getVariantAt(level, x, y, z);
	if (variant < 0) variant = m_dropVariant;

	int count = getResourceCountForLootBonus(0, level->random);
	for (int i = 0; i < count; i++)
	{
		if (level->random->nextFloat() > odds) continue;
		popResource(level, x, y, z, std::make_shared<ItemInstance>(Item::bed_Id, 1, variant));
	}
	m_dropVariant = BedTileEntity::DEFAULT_VARIANT;
}

int BedTile::getPistonPushReaction()
{
	return Material::PUSH_DESTROY;
}

int BedTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::bed_Id;
}

void BedTile::playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player)
{
	// neoLegacy - remember which bed this is: the tile entity is removed before spawnResources() runs
	int variant = BedTileEntity::getVariantAt(level, x, y, z);
	m_dropVariant = (variant >= 0) ? variant : BedTileEntity::DEFAULT_VARIANT;

	if (player->abilities.instabuild)
	{
		if (isHeadPiece(data))
		{
			int direction = getDirection(data);
			x -= HEAD_DIRECTION_OFFSETS[direction][0];
			z -= HEAD_DIRECTION_OFFSETS[direction][1];
			if (level->getTile(x, y, z) == id)
			{
				level->removeTile(x, y, z);
			}
		}
	}
}