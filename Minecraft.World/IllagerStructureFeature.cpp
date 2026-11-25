#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.level.levelgen.structure.h"
#include "BannerTileEntity.h"
#include "EnchantmentHelper.h"
#include "WeighedTreasure.h"
#include "IllagerStructureFeature.h"

vector<Biome *> IllagerStructureFeature::allowedBiomes[IllagerStructureFeature::KIND_COUNT];

// hut, tower, fort
const int IllagerStructureFeature::SPACING[KIND_COUNT]			= { 20, 30, 36 };
const int IllagerStructureFeature::MIN_SEPARATION[KIND_COUNT]	= { 8, 10, 12 };
const int IllagerStructureFeature::ONE_IN[KIND_COUNT]			= { 2, 2, 3 };
static const int SALT[IllagerStructureFeature::KIND_COUNT]		= { 14357617, 14357618, 14357619 };
static const int CHANCE_SALT[IllagerStructureFeature::KIND_COUNT]	= { 10387401, 10387402, 10387403 };

void IllagerStructureFeature::staticCtor()
{
	// Sorcerer Hut: dark forests
	allowedBiomes[KIND_HUT].push_back(Biome::roofedForest);
	allowedBiomes[KIND_HUT].push_back(Biome::roofedForestM);

	// Illusioner Tower: dark forests, swamps, taigas and old-growth taigas
	allowedBiomes[KIND_TOWER].push_back(Biome::roofedForest);
	allowedBiomes[KIND_TOWER].push_back(Biome::roofedForestM);
	allowedBiomes[KIND_TOWER].push_back(Biome::swampland);
	allowedBiomes[KIND_TOWER].push_back(Biome::swamplandM);
	allowedBiomes[KIND_TOWER].push_back(Biome::taiga);
	allowedBiomes[KIND_TOWER].push_back(Biome::taigaHills);
	allowedBiomes[KIND_TOWER].push_back(Biome::taigaM);
	allowedBiomes[KIND_TOWER].push_back(Biome::megaTaiga);
	allowedBiomes[KIND_TOWER].push_back(Biome::megaTaigaHills);

	// Illager Fort: taigas and snowy lands
	allowedBiomes[KIND_FORT].push_back(Biome::taiga);
	allowedBiomes[KIND_FORT].push_back(Biome::taigaHills);
	allowedBiomes[KIND_FORT].push_back(Biome::taigaM);
	allowedBiomes[KIND_FORT].push_back(Biome::coldTaiga);
	allowedBiomes[KIND_FORT].push_back(Biome::coldTaigaHills);
	allowedBiomes[KIND_FORT].push_back(Biome::coldTaigaM);
	allowedBiomes[KIND_FORT].push_back(Biome::iceFlats);
}

void IllagerStructureFeature::loadStatic()
{
	StructureFeatureIO::setPieceId(eStructurePiece_IllagerStructurePiece, IllagerPiece::Create, L"ISP");
}

IllagerStructureFeature::IllagerStructureFeature()
{
	villageFeature = nullptr;
	// monsters that spawn on their own inside the structures
	enemies.push_back(new Biome::MobSpawnerData(eTYPE_VINDICATOR, 3, 1, 2));
	enemies.push_back(new Biome::MobSpawnerData(eTYPE_PROVOKER, 1, 1, 1));
}

wstring IllagerStructureFeature::getFeatureName()
{
	return L"IllagerStructure";	// LargeFeature::ILLAGER_STRUCTURE, for /locate
}

vector<Biome::MobSpawnerData *> *IllagerStructureFeature::getEnemies()
{
	return &enemies;
}

// Which structure, if any, starts in this chunk. Each kind has one candidate chunk per SPACING x SPACING cells (the scheme temples and outposts use), one in ONE_IN of
// those are used, only in the kind's biomes, and not close to a village. The first kind that fits wins.
int IllagerStructureFeature::kindAt(int x, int z)
{
	for (int kind = 0; kind < KIND_COUNT; kind++)
	{
		const int spacing = SPACING[kind];
		int cx = x;
		int cz = z;
		if (cx < 0) cx -= spacing - 1;
		if (cz < 0) cz -= spacing - 1;

		int xCenter = cx / spacing;
		int zCenter = cz / spacing;
		Random *r = level->getRandomFor(xCenter, zCenter, SALT[kind]);
		xCenter *= spacing;
		zCenter *= spacing;
		xCenter += r->nextInt(spacing - MIN_SEPARATION[kind]);
		zCenter += r->nextInt(spacing - MIN_SEPARATION[kind]);

		if (x != xCenter || z != zCenter) continue;

		Random *chance = level->getRandomFor(x, z, CHANCE_SALT[kind]);
		if (chance->nextInt(ONE_IN[kind]) != 0) continue;

		Biome *biome = level->getBiomeSource()->getBiome(x * 16 + 8, z * 16 + 8);
		bool biomeOk = false;
		for (const auto &a : allowedBiomes[kind])
		{
			if (biome == a) { biomeOk = true; break; }
		}
		if (!biomeOk) continue;

		if (villageFeature != nullptr)
		{
			bool nearVillage = false;
			for (int dx = -VILLAGE_CLEARANCE_CHUNKS; dx <= VILLAGE_CLEARANCE_CHUNKS && !nearVillage; dx++)
				for (int dz = -VILLAGE_CLEARANCE_CHUNKS; dz <= VILLAGE_CLEARANCE_CHUNKS; dz++)
					if (villageFeature->isVillageChunk(x + dx, z + dz)) { nearVillage = true; break; }
			if (nearVillage) continue;
		}
		return kind;
	}
	return -1;
}

bool IllagerStructureFeature::isFeatureChunk(int x, int z, bool bIsSuperflat)
{
	return kindAt(x, z) >= 0;
}

StructureStart *IllagerStructureFeature::createStructureStart(int x, int z)
{
	return new IllagerStart(level, random, x, z, kindAt(x, z));
}

// ---------------------------------------------------------------------------------------------------------------------------------
// Start
// ---------------------------------------------------------------------------------------------------------------------------------
IllagerStructureFeature::IllagerStart::IllagerStart()
{
	// for reflection
}

// one piece, centred on the start chunk, in a random rotation (and, for the tower, one of its three designs)
IllagerStructureFeature::IllagerStart::IllagerStart(Level *level, Random *random, int chunkX, int chunkZ, int kind) : StructureStart(chunkX, chunkZ)
{
	int variant = 0;
	if (kind == KIND_TOWER) variant = 1 + random->nextInt(3);
	else if (kind == KIND_FORT) variant = 4;
	int rotation = random->nextInt(4);

	const IllagerStructTemplate *t = &ILLAGER_STRUCT[variant][rotation];
	int ox = chunkX * 16 + (16 - t->sx) / 2;
	int oz = chunkZ * 16 + (16 - t->sz) / 2;
	pieces.push_back(new IllagerPiece(t, ox, oz));

	calculateBoundingBox();

	// the piece is lifted onto the terrain when it is generated, so this box must not clip it vertically (monster spawning looks at it)
	boundingBox->y0 = 0;
	boundingBox->y1 = Level::maxBuildHeight - 1;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// Piece
// ---------------------------------------------------------------------------------------------------------------------------------
IllagerStructureFeature::IllagerPiece::IllagerPiece()
{
	tmpl = nullptr;
	heightPosition = -1;
	// for reflection
}

IllagerStructureFeature::IllagerPiece::IllagerPiece(const IllagerStructTemplate *tmpl, int x, int z) : StructurePiece(0)
{
	this->tmpl = tmpl;
	heightPosition = -1;

	orientation = Direction::UNDEFINED;
	boundingBox = new BoundingBox(x, 64, z, x + tmpl->sx - 1, 64 + tmpl->sy - 1, z + tmpl->sz - 1);
}

void IllagerStructureFeature::IllagerPiece::addAdditonalSaveData(CompoundTag *tag)
{
	// structures are rebuilt from the world seed rather than saved here
}

void IllagerStructureFeature::IllagerPiece::readAdditonalSaveData(CompoundTag *tag)
{
}

// A column of cobblestone (or the log) under a block left hanging over a slope, like Java's terrain adaptation.
void IllagerStructureFeature::IllagerPiece::growFoundation(Level *level, BoundingBox *chunkBB, int wx, int wy, int wz, int tile, int data)
{
	int fillTile = (tile == Tile::log_Id || tile == Tile::log2_Id || tile == Tile::cobblestone_Id || tile == Tile::mossy_cobblestone_Id) ? tile : Tile::cobblestone_Id;
	int fillData = (tile == Tile::log_Id || tile == Tile::log2_Id) ? (data & 3) : 0;	// an upright log

	for (int depth = 1; depth <= 10; depth++)
	{
		int y = wy - depth;
		if (y <= 1) break;
		if (!chunkBB->isInside(wx, y, wz)) break;

		int below = level->getTile(wx, y, wz);
		if (below != 0)
		{
			Tile *t = Tile::tiles[below];
			if (t == nullptr) break;
			if (!t->material->isLiquid() && !t->material->isReplaceable()) break;
		}
		placeBlock(level, fillTile, fillData, wx, y, wz, chunkBB);
	}
}

// The mod's chest loot (baked from its loot tables): every entry is rolled once with its own chance and put in a random free slot.
void IllagerStructureFeature::IllagerPiece::fillChest(Level *level, Random *random, int wx, int wy, int wz, int lootType)
{
	shared_ptr<ChestTileEntity> chest = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(wx, wy, wz));
	if (chest == nullptr) return;
	if (lootType < 0 || lootType >= ILLAGER_LOOT_TYPES) lootType = 0;

	const IllagerLootEntry *entries = ILLAGER_LOOT[lootType];
	int count = ILLAGER_LOOT_COUNT[lootType];
	int size = chest->getContainerSize();

	for (int i = 0; i < count; i++)
	{
		const IllagerLootEntry &e = entries[i];
		if (random->nextInt(100) >= e.chancePercent) continue;
		int n = e.minCount + (e.maxCount > e.minCount ? random->nextInt(e.maxCount - e.minCount + 1) : 0);
		if (n <= 0) continue;

		shared_ptr<ItemInstance> stack;
		if (e.item == -1)
		{
			// an enchanted book with one random enchantment
			WeighedTreasure *book = Item::enchanted_book->createForRandomTreasure(random, 1, 1, 1);
			stack = book->getItem()->copy();
			delete book;
		}
		else
		{
			stack = std::make_shared<ItemInstance>(e.item, n, e.aux);
			if (e.enchant) stack = EnchantmentHelper::enchantItem(random, stack, 20 + random->nextInt(11));
		}

		for (int attempt = 0; attempt < 30; attempt++)
		{
			int slot = random->nextInt(size);
			if (chest->getItem(slot) == nullptr) { chest->setItem(slot, stack); break; }
		}
	}
}

// The inhabitants (from the mod's jigsaw mob markers): spawned in the chunk that holds their spot, and never despawning.
void IllagerStructureFeature::IllagerPiece::spawnMobs(Level *level, Random *random, BoundingBox *chunkBB)
{
	const int ox = boundingBox->x0;
	const int oy = boundingBox->y0;
	const int oz = boundingBox->z0;

	for (int i = 0; i < tmpl->spawnCount; i++)
	{
		int type = tmpl->spawns[i * 4];
		double x = ox + tmpl->spawns[i * 4 + 1] / 10.0;
		double y = oy + tmpl->spawns[i * 4 + 2] / 10.0;
		double z = oz + tmpl->spawns[i * 4 + 3] / 10.0;
		if (!chunkBB->isInside(Mth::floor(x), Mth::floor(y), Mth::floor(z))) continue;

		shared_ptr<Mob> mob;
		switch (type)
		{
		case 0: mob = std::make_shared<Sorcerer>(level); break;
		case 1: mob = std::make_shared<Vindicator>(level); break;
		case 2: mob = std::make_shared<Provoker>(level); break;
		case 3: mob = std::make_shared<Archivist>(level); break;
		case 4: mob = std::make_shared<Basher>(level); break;
		case 5: mob = std::make_shared<Illusioner>(level); break;
		case 6: mob = std::make_shared<Inquisitor>(level); break;
		case 11: mob = std::make_shared<Alchemist>(level); break;
		default: break;	// villagers, evokers... the structures here do not use them
		}
		if (mob == nullptr) continue;

		mob->moveTo(x, y, z, random->nextFloat() * 360, 0);
		mob->finalizeMobSpawn(nullptr);
		mob->setPersistenceRequired();
		level->addEntity(mob);
	}
}

bool IllagerStructureFeature::IllagerPiece::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (tmpl == nullptr) return false;

	// the first time a chunk of the piece is generated, settle on a ground level (the average over this piece's columns in that chunk)
	if (heightPosition < 0)
	{
		int total = 0;
		int count = 0;
		for (int z = boundingBox->z0; z <= boundingBox->z1; z++)
		{
			for (int x = boundingBox->x0; x <= boundingBox->x1; x++)
			{
				if (chunkBB->isInside(x, 64, z))
				{
					total += max(level->getTopSolidBlock(x, z), level->dimension->getSpawnYPosition());
					count++;
				}
			}
		}
		if (count == 0) return false;
		heightPosition = total / count;
		boundingBox->move(0, heightPosition - boundingBox->y0, 0);
	}

	const int sx = tmpl->sx;
	const int sy = tmpl->sy;
	const int sz = tmpl->sz;
	const int ox = boundingBox->x0;
	const int oy = boundingBox->y0;
	const int oz = boundingBox->z0;

	// unpack the blocks into a grid so that every other cell of the volume can be cleared to air, as in Java
	vector<unsigned short> grid((size_t)sx * sy * sz, 0);
	for (int i = 0; i < tmpl->cellCount; i++)
	{
		unsigned int c = tmpl->cells[i];
		int x = (c >> 25) & 127;
		int y = (c >> 19) & 63;
		int z = (c >> 12) & 127;
		int tile = (c >> 4) & 255;
		int data = c & 15;
		if (x >= sx || y >= sy || z >= sz) continue;
		grid[((size_t)y * sz + z) * sx + x] = (unsigned short)(((tile << 4) | data) + 1);
	}

	for (int y = 0; y < sy; y++)
	{
		for (int z = 0; z < sz; z++)
		{
			for (int x = 0; x < sx; x++)
			{
				int wx = ox + x;
				int wy = oy + y;
				int wz = oz + z;
				if (!chunkBB->isInside(wx, wy, wz)) continue;

				unsigned short v = grid[((size_t)y * sz + z) * sx + x];
				if (v == 0)
				{
					if (level->getTile(wx, wy, wz) != 0) placeBlock(level, 0, 0, wx, wy, wz, chunkBB);
					continue;
				}

				int tile = (v - 1) >> 4;
				int data = (v - 1) & 15;
				placeBlock(level, tile, data, wx, wy, wz, chunkBB);

				if (tile == Tile::chest_Id)
				{
					int lootType = 0;
					for (int c = 0; c < tmpl->chestCount; c++)
					{
						if (tmpl->chests[c * 4] == x && tmpl->chests[c * 4 + 1] == y && tmpl->chests[c * 4 + 2] == z) { lootType = tmpl->chests[c * 4 + 3]; break; }
					}
					fillChest(level, random, wx, wy, wz, lootType);
				}
				else if (tile == Tile::wall_banner_Id)
				{
					shared_ptr<BannerTileEntity> banner = dynamic_pointer_cast<BannerTileEntity>(level->getTileEntity(wx, wy, wz));
					if (banner != nullptr) banner->setBaseColor(15);	// white
				}

				if (y == 0) growFoundation(level, chunkBB, wx, wy, wz, tile, data);
			}
		}
	}

	spawnMobs(level, random, chunkBB);
	return true;
}
