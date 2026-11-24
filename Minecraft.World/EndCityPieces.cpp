#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.item.h"
#include "EndCityPieces.h"
#include "BoundingBox.h"
#include "Shulker.h"
#include "ItemFrame.h"
#include "WeighedTreasure.h"
#include "LootTableManager.h"
#include "Direction.h"
#include "Facing.h"

static const int TOWER_BRIDGES[4][4] =
{
	{ EndCityPieces::ROT_NONE, 1, -1, 0 },
	{ EndCityPieces::ROT_CLOCKWISE_90, 6, -1, 1 },
	{ EndCityPieces::ROT_COUNTERCLOCKWISE_90, 0, -1, 5 },
	{ EndCityPieces::ROT_CLOCKWISE_180, 5, -1, 6 },
};

static const int FAT_TOWER_BRIDGES[4][4] =
{
	{ EndCityPieces::ROT_NONE, 4, -1, 0 },
	{ EndCityPieces::ROT_CLOCKWISE_90, 12, -1, 4 },
	{ EndCityPieces::ROT_COUNTERCLOCKWISE_90, 0, -1, 8 },
	{ EndCityPieces::ROT_CLOCKWISE_180, 8, -1, 12 },
};

static const int POTION_HEALING_II = 8229;

void EndCityPieces::transform(int rotation, int x, int z, int &outX, int &outZ)
{
	switch (rotation)
	{
	case ROT_CLOCKWISE_90:			outX = -z; outZ = x; break;
	case ROT_CLOCKWISE_180:			outX = -x; outZ = -z; break;
	case ROT_COUNTERCLOCKWISE_90:	outX = z; outZ = -x; break;
	default:						outX = x; outZ = z; break;
	}
}

static const int CLOCKWISE_FACINGS[4] = { Facing::NORTH, Facing::EAST, Facing::SOUTH, Facing::WEST };

static int rotateFacing(int facing, int rotation)
{
	for (int i = 0; i < 4; i++)
	{
		if (CLOCKWISE_FACINGS[i] == facing) return CLOCKWISE_FACINGS[(i + rotation) & 3];
	}
	return facing;
}

static int rotateDirection(int dir, int rotation)
{
	return (dir + rotation) & 3;
}


EndCityPieces::EndCityPiece::EndCityPiece() : StructurePiece(0)
{
	templateType = 0;
	templateX = templateY = templateZ = 0;
	rotation = ROT_NONE;
	overwrite = false;
}

EndCityPieces::EndCityPiece::EndCityPiece(int templateType, int x, int y, int z, int rotation, bool overwrite) : StructurePiece(0)
{
	this->templateType = templateType;
	templateX = x;
	templateY = y;
	templateZ = z;
	this->rotation = rotation;
	this->overwrite = overwrite;
	calculateBoundingBox();
}

void EndCityPieces::EndCityPiece::calculateBoundingBox()
{
	const EndCityTemplates::Template &t = EndCityTemplates::TEMPLATES[templateType];
	bool turned = rotation == ROT_CLOCKWISE_90 || rotation == ROT_COUNTERCLOCKWISE_90;
	int sx = turned ? t.sizeZ : t.sizeX;
	int sz = turned ? t.sizeX : t.sizeZ;

	int ox = 0, oz = 0;
	if (rotation == ROT_CLOCKWISE_90) ox = -sx;
	else if (rotation == ROT_COUNTERCLOCKWISE_90) oz = -sz;
	else if (rotation == ROT_CLOCKWISE_180) { ox = -sx; oz = -sz; }

	delete boundingBox;
	boundingBox = new BoundingBox(templateX + ox, templateY, templateZ + oz, templateX + ox + sx, templateY + t.sizeY - 1, templateZ + oz + sz);
}

void EndCityPieces::EndCityPiece::moveBy(int dx, int dy, int dz)
{
	templateX += dx;
	templateY += dy;
	templateZ += dz;
	boundingBox->move(dx, dy, dz);
}

void EndCityPieces::EndCityPiece::addAdditonalSaveData(CompoundTag *tag)
{
	tag->putInt(L"Template", templateType);
	tag->putInt(L"TX", templateX);
	tag->putInt(L"TY", templateY);
	tag->putInt(L"TZ", templateZ);
	tag->putInt(L"Rot", rotation);
	tag->putBoolean(L"OW", overwrite);
}

void EndCityPieces::EndCityPiece::readAdditonalSaveData(CompoundTag *tag)
{
	templateType = tag->getInt(L"Template");
	templateX = tag->getInt(L"TX");
	templateY = tag->getInt(L"TY");
	templateZ = tag->getInt(L"TZ");
	rotation = tag->getInt(L"Rot");
	overwrite = tag->getBoolean(L"OW");
	calculateBoundingBox();
}

int EndCityPieces::EndCityPiece::rotateData(int id, int data)
{
	if (rotation == ROT_NONE) return data;

	if (id == Tile::purpur_stairs_Id)
	{
		static const int STAIR_TO_FACING[4] = { Facing::EAST, Facing::WEST, Facing::SOUTH, Facing::NORTH };
		int facing = rotateFacing(STAIR_TO_FACING[data & 3], rotation);
		int dir = 0;
		for (int i = 0; i < 4; i++)
		{
			if (STAIR_TO_FACING[i] == facing) dir = i;
		}
		return (data & 4) | dir;
	}
	if (id == Tile::purpur_pillar_Id)
	{
		int axis = data & 0xC;
		if ((rotation & 1) && (axis == 4 || axis == 8)) axis ^= 0xC;
		return (data & ~0xC) | axis;
	}
	if (id == Tile::end_rod_Id || id == Tile::ladder_Id || id == Tile::chest_Id || id == Tile::ender_chest_Id || id == 177 || id == Tile::skull_Id)
	{
		int facing = data & 7;
		if (facing >= 2 && facing <= 5) return (data & ~7) | rotateFacing(facing, rotation);
	}
	return data;
}

bool EndCityPieces::EndCityPiece::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	const EndCityTemplates::Template &t = EndCityTemplates::TEMPLATES[templateType];

	for (int i = 0; i < t.blockCount; i++)
	{
		const EndCityTemplates::Block &b = t.blocks[i];
		int id = b.id;
		int data = b.data;

		if (id == 0 && !overwrite) continue;

		int tx, tz;
		transform(rotation, b.x, b.z, tx, tz);
		int wx = templateX + tx;
		int wy = templateY + b.y;
		int wz = templateZ + tz;
		if (!chunkBB->isInside(wx, wy, wz)) continue;

		data = rotateData(id, data);

		bool wallBanner = id == 177;
		if (wallBanner) id = Tile::standing_banner_Id;

		level->setTileAndData(wx, wy, wz, id, data, Tile::UPDATE_CLIENTS);

		if (wallBanner)
		{
			shared_ptr<BannerTileEntity> banner = dynamic_pointer_cast<BannerTileEntity>(level->getTileEntity(wx, wy, wz));
			if (banner != nullptr) banner->setIsWall(true);
		}
	}

	for (int i = 0; i < t.extraCount; i++)
	{
		handleExtra(level, random, chunkBB, t.extras[i]);
	}
	return true;
}

void EndCityPieces::EndCityPiece::handleExtra(Level *level, Random *random, BoundingBox *chunkBB, const EndCityTemplates::Extra &extra)
{
	int tx, tz;
	transform(rotation, extra.x, extra.z, tx, tz);
	int x = templateX + tx;
	int y = templateY + extra.y;
	int z = templateZ + tz;
	if (!chunkBB->isInside(x, y, z)) return;

	switch (extra.kind)
	{
	case EndCityTemplates::EXTRA_CHEST_LOOT:
		{
			shared_ptr<ChestTileEntity> chest = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(x, y - 1, z));
			if (chest != nullptr)
			{
				WeighedTreasure::addChestItems(random,
					LootTableManager::Get().ResolveDrops("chests/end_city_treasure", false, 0,
						[random](int bound) { return random->nextInt(bound); }),
					chest);
			}
		}
		break;

	case EndCityTemplates::EXTRA_SENTRY:
		{
			shared_ptr<Shulker> shulker = std::make_shared<Shulker>(level);
			shulker->moveTo(x + 0.5, y, z + 0.5, 0, 0);
			shulker->setAttachPosition(x, y, z);
			level->addEntity(shulker);
		}
		break;

	case EndCityTemplates::EXTRA_ELYTRA:
		{
			int dir = rotateDirection(Direction::SOUTH, rotation);
			int wallX = x - Direction::STEP_X[dir];
			int wallZ = z - Direction::STEP_Z[dir];
			shared_ptr<ItemFrame> frame = std::make_shared<ItemFrame>(level, wallX, y, wallZ, dir);
			frame->setItem(std::make_shared<ItemInstance>(Item::elytra));
			level->addEntity(frame);
		}
		break;

	case EndCityTemplates::EXTRA_BANNER:
		{
			shared_ptr<BannerTileEntity> banner = dynamic_pointer_cast<BannerTileEntity>(level->getTileEntity(x, y, z));
			if (banner != nullptr)
			{
				banner->setBaseColor(extra.arg);
				banner->clearPatterns();
				banner->addPattern(L"tt", 0);
				banner->addPattern(L"bt", 0);
			}
		}
		break;

	case EndCityTemplates::EXTRA_SKULL:
		{
			shared_ptr<SkullTileEntity> skull = dynamic_pointer_cast<SkullTileEntity>(level->getTileEntity(x, y, z));
			if (skull != nullptr) skull->setSkullType(extra.arg, L"");
		}
		break;

	case EndCityTemplates::EXTRA_HEALING_POTIONS:
		{
			shared_ptr<BrewingStandTileEntity> stand = dynamic_pointer_cast<BrewingStandTileEntity>(level->getTileEntity(x, y, z));
			if (stand != nullptr)
			{
				stand->setItem(0, std::make_shared<ItemInstance>(Item::potion, 1, POTION_HEALING_II));
				stand->setItem(2, std::make_shared<ItemInstance>(Item::potion, 1, POTION_HEALING_II));
			}
		}
		break;

	default:
		break;
	}
}


EndCityPieces::Builder::Builder(list<StructurePiece *> &pieces, Random *random) : pieces(pieces)
{
	this->random = random;
	for (int i = 0; i < 8; i++) houseGenerated[i] = false;
	shipGenerated = false;
}

EndCityPieces::EndCityPiece *EndCityPieces::Builder::add(list<StructurePiece *> &into, EndCityPiece *piece)
{
	into.push_back(piece);
	return piece;
}

EndCityPieces::EndCityPiece *EndCityPieces::Builder::addPiece(EndCityPiece *previous, int ox, int oy, int oz, int templateType, int rotation, bool overwrite)
{
	EndCityPiece *piece = new EndCityPiece(templateType, previous->templateX, previous->templateY, previous->templateZ, rotation, overwrite);
	int tx, tz;
	transform(previous->rotation, ox, oz, tx, tz);
	piece->moveBy(tx, oy, tz);
	return piece;
}

void EndCityPieces::Builder::startHouseTower(int x, int y, int z, int rotation)
{
	EndCityPiece *piece = add(pieces, new EndCityPiece(EndCityTemplates::BASE_FLOOR, x, y, z, rotation, true));
	piece = add(pieces, addPiece(piece, -1, 0, -1, EndCityTemplates::SECOND_FLOOR, rotation, false));
	piece = add(pieces, addPiece(piece, -1, 4, -1, EndCityTemplates::THIRD_FLOOR, rotation, false));
	piece = add(pieces, addPiece(piece, -1, 8, -1, EndCityTemplates::THIRD_ROOF, rotation, true));
	recursiveChildren(SECTION_TOWER, 1, piece, 0, 0, 0, pieces);
}

bool EndCityPieces::Builder::recursiveChildren(ESection section, int genDepth, EndCityPiece *previous, int ox, int oy, int oz, list<StructurePiece *> &into)
{
	if (genDepth > 8) return false;

	list<StructurePiece *> children;
	if (!generate(section, genDepth, previous, ox, oy, oz, children))
	{
		for (auto &child : children) delete child;
		return false;
	}

	int tag = random->nextInt();
	for (auto &child : children)
	{
		static_cast<EndCityPiece *>(child)->setGenDepth(tag);
		StructurePiece *collision = StructurePiece::findCollisionPiece(&into, child->getBoundingBox());
		if (collision != nullptr && collision->getGenDepth() != previous->getGenDepth())
		{
			for (auto &c : children) delete c;
			return false;
		}
	}

	into.insert(into.end(), children.begin(), children.end());
	return true;
}

bool EndCityPieces::Builder::generate(ESection section, int genDepth, EndCityPiece *previous, int ox, int oy, int oz, list<StructurePiece *> &out)
{
	switch (section)
	{
	case SECTION_HOUSE:			return generateHouse(genDepth, previous, ox, oy, oz, out);
	case SECTION_TOWER:			return generateTower(genDepth, previous, out);
	case SECTION_TOWER_BRIDGE:	return generateTowerBridge(genDepth, previous, out);
	case SECTION_FAT_TOWER:		return generateFatTower(genDepth, previous, out);
	}
	return false;
}

bool EndCityPieces::Builder::generateHouse(int genDepth, EndCityPiece *previous, int ox, int oy, int oz, list<StructurePiece *> &out)
{
	if (genDepth > 8) return false;

	int rotation = previous->rotation;
	EndCityPiece *piece = add(out, addPiece(previous, ox, oy, oz, EndCityTemplates::BASE_FLOOR, rotation, true));
	int roof = random->nextInt(3);
	if (!houseGenerated[genDepth])
	{
		houseGenerated[genDepth] = true;
		roof = 2;
	}

	if (roof == 0)
	{
		add(out, addPiece(piece, -1, 4, -1, EndCityTemplates::BASE_ROOF, rotation, true));
	}
	else if (roof == 1)
	{
		piece = add(out, addPiece(piece, -1, 0, -1, EndCityTemplates::SECOND_FLOOR_2, rotation, false));
		piece = add(out, addPiece(piece, -1, 8, -1, EndCityTemplates::SECOND_ROOF, rotation, false));
		recursiveChildren(SECTION_TOWER, genDepth + 1, piece, 0, 0, 0, out);
	}
	else
	{
		piece = add(out, addPiece(piece, -1, 0, -1, EndCityTemplates::SECOND_FLOOR_2, rotation, false));
		piece = add(out, addPiece(piece, -1, 4, -1, EndCityTemplates::THIRD_FLOOR_C, rotation, false));
		piece = add(out, addPiece(piece, -1, 8, -1, EndCityTemplates::THIRD_ROOF, rotation, true));
		recursiveChildren(SECTION_TOWER, genDepth + 1, piece, 0, 0, 0, out);
	}
	return true;
}

bool EndCityPieces::Builder::generateTower(int genDepth, EndCityPiece *previous, list<StructurePiece *> &out)
{
	int rotation = previous->rotation;
	EndCityPiece *piece = add(out, addPiece(previous, 4, -3, 4, EndCityTemplates::TOWER_BASE, rotation, true));
	piece = add(out, addPiece(piece, 0, 7, 0, EndCityTemplates::TOWER_PIECE, rotation, true));
	EndCityPiece *bridgePiece = random->nextInt(3) == 0 ? piece : nullptr;
	int extraFloors = random->nextInt(3);
	for (int i = 0; i < extraFloors + 1; i++)
	{
		piece = add(out, addPiece(piece, 0, 4, 0, EndCityTemplates::TOWER_PIECE, rotation, true));
		if (i < extraFloors && random->nextInt(10) > 2) bridgePiece = piece;
	}
	if (genDepth == 1 && bridgePiece == nullptr) bridgePiece = piece;

	if (bridgePiece == nullptr)
	{
		if (genDepth == 7)
		{
			add(out, addPiece(piece, -1, 4, -1, EndCityTemplates::TOWER_TOP, rotation, true));
			return true;
		}
		return recursiveChildren(SECTION_FAT_TOWER, genDepth + 1, piece, 0, 0, 0, out);
	}

	double bridgeWeight = 3 - genDepth > 0 ? static_cast<double>(3 - genDepth) : 0.0;
	double remaining = 4.0;
	for (int i = 0; i < 4; i++)
	{
		if (random->nextFloat() < bridgeWeight / remaining || random->nextInt(10) > 2)
		{
			EndCityPiece *bridge = add(out, addPiece(bridgePiece, TOWER_BRIDGES[i][1], TOWER_BRIDGES[i][2], TOWER_BRIDGES[i][3],
				EndCityTemplates::BRIDGE_END, rotate(rotation, TOWER_BRIDGES[i][0]), true));
			if (recursiveChildren(SECTION_TOWER_BRIDGE, genDepth + 1, bridge, 0, 0, 0, out)) bridgeWeight -= 1.0;
		}
		remaining -= 1.0;
	}
	add(out, addPiece(piece, -1, 4, -1, EndCityTemplates::TOWER_TOP, rotation, true));
	return true;
}

bool EndCityPieces::Builder::generateTowerBridge(int genDepth, EndCityPiece *previous, list<StructurePiece *> &out)
{
	int rotation = previous->rotation;
	int count = random->nextInt(4) + 1;
	EndCityPiece *piece = add(out, addPiece(previous, 0, 0, -4, EndCityTemplates::BRIDGE_PIECE, rotation, true));
	piece->setGenDepth(-1);
	int y = 0;
	for (int i = 0; i < count; i++)
	{
		if (random->nextBoolean())
		{
			piece = add(out, addPiece(piece, 0, y, -4, EndCityTemplates::BRIDGE_PIECE, rotation, true));
			y = 0;
		}
		else
		{
			if (random->nextBoolean())
				piece = add(out, addPiece(piece, 0, y, -4, EndCityTemplates::BRIDGE_STEEP_STAIRS, rotation, true));
			else
				piece = add(out, addPiece(piece, 0, y, -8, EndCityTemplates::BRIDGE_GENTLE_STAIRS, rotation, true));
			y = 4;
		}
	}

	if (!shipGenerated)
	{
		add(out, addPiece(piece, random->nextInt(8) - 8, y, -48, EndCityTemplates::SHIP, rotation, true));
		shipGenerated = true;
	}
	else if (!recursiveChildren(SECTION_HOUSE, genDepth + 1, piece, -3, y + 1, -11, out))
	{
		return false;
	}

	piece = add(out, addPiece(piece, 4, y, 0, EndCityTemplates::BRIDGE_END, rotate(rotation, ROT_CLOCKWISE_180), true));
	piece->setGenDepth(-1);
	return true;
}

bool EndCityPieces::Builder::generateFatTower(int genDepth, EndCityPiece *previous, list<StructurePiece *> &out)
{
	int rotation = previous->rotation;
	EndCityPiece *piece = add(out, addPiece(previous, -3, 4, -3, EndCityTemplates::FAT_TOWER_BASE, rotation, true));
	piece = add(out, addPiece(piece, 0, 4, 0, EndCityTemplates::FAT_TOWER_MIDDLE, rotation, true));
	for (int floor = 0; floor < 2; floor++)
	{
		piece = add(out, addPiece(piece, 0, 8, 0, EndCityTemplates::FAT_TOWER_MIDDLE, rotation, true));
		for (int i = 0; i < 4; i++)
		{
			if (random->nextInt(10) > 2)
			{
				EndCityPiece *bridge = add(out, addPiece(piece, FAT_TOWER_BRIDGES[i][1], FAT_TOWER_BRIDGES[i][2], FAT_TOWER_BRIDGES[i][3],
					EndCityTemplates::BRIDGE_END, rotate(rotation, FAT_TOWER_BRIDGES[i][0]), true));
				recursiveChildren(SECTION_TOWER_BRIDGE, genDepth + 1, bridge, 0, 0, 0, out);
			}
		}
	}
	add(out, addPiece(piece, -2, 8, -2, EndCityTemplates::FAT_TOWER_TOP, rotation, true));
	return true;
}
