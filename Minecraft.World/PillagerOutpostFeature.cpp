#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.level.levelgen.structure.h"
#include "BannerTileEntity.h"
#include "BannerItem.h"
#include "VillagerGolem.h"
#include "WeighedTreasure.h"
#include "PillagerOutpostFeature.h"

vector<Biome *> PillagerOutpostFeature::allowedBiomes;

void PillagerOutpostFeature::staticCtor()
{
	allowedBiomes.push_back(Biome::plains);
	allowedBiomes.push_back(Biome::sunflowersPlains);
	allowedBiomes.push_back(Biome::desert);
	allowedBiomes.push_back(Biome::desertHills);
	allowedBiomes.push_back(Biome::savanna);
	allowedBiomes.push_back(Biome::iceFlats);
	allowedBiomes.push_back(Biome::taiga);
	allowedBiomes.push_back(Biome::taigaHills);
	allowedBiomes.push_back(Biome::coldTaiga);
}

void PillagerOutpostFeature::loadStatic()
{
	StructureFeatureIO::setPieceId(eStructurePiece_PillagerOutpostPiece, OutpostPiece::Create, L"POP");
}

PillagerOutpostFeature::PillagerOutpostFeature()
{
	villageFeature = nullptr;
	enemies.push_back(new Biome::MobSpawnerData(eTYPE_PILLAGER, 1, 1, 1));
}

wstring PillagerOutpostFeature::getFeatureName()
{
	return L"Outpost";	// LargeFeature::OUTPOST, for /locate
}

vector<Biome::MobSpawnerData *> *PillagerOutpostFeature::getEnemies()
{
	return &enemies;
}

// One candidate chunk per SPACING x SPACING cells (the same scheme as temples), then one in three of those, only in the allowed biomes,
// and never within VILLAGE_CLEARANCE_CHUNKS of a village.
bool PillagerOutpostFeature::isFeatureChunk(int x, int z, bool bIsSuperflat)
{
	int xx = x;
	int zz = z;
	if (x < 0) x -= SPACING - 1;
	if (z < 0) z -= SPACING - 1;

	int xCenterFeatureChunk = x / SPACING;
	int zCenterFeatureChunk = z / SPACING;
	Random *r = level->getRandomFor(xCenterFeatureChunk, zCenterFeatureChunk, 165745296);
	xCenterFeatureChunk *= SPACING;
	zCenterFeatureChunk *= SPACING;
	xCenterFeatureChunk += r->nextInt(SPACING - MIN_SEPARATION);
	zCenterFeatureChunk += r->nextInt(SPACING - MIN_SEPARATION);
	x = xx;
	z = zz;

	if (x != xCenterFeatureChunk || z != zCenterFeatureChunk) return false;

	Random *chance = level->getRandomFor(x, z, 10387320);
	if (chance->nextInt(3) != 0) return false;

	Biome *biome = level->getBiomeSource()->getBiome(x * 16 + 8, z * 16 + 8);
	bool biomeOk = false;
	for (const auto &a : allowedBiomes)
	{
		if (biome == a) { biomeOk = true; break; }
	}
	if (!biomeOk) return false;

	if (villageFeature != nullptr)
	{
		for (int dx = -VILLAGE_CLEARANCE_CHUNKS; dx <= VILLAGE_CLEARANCE_CHUNKS; dx++)
		{
			for (int dz = -VILLAGE_CLEARANCE_CHUNKS; dz <= VILLAGE_CLEARANCE_CHUNKS; dz++)
			{
				if (villageFeature->isVillageChunk(x + dx, z + dz)) return false;
			}
		}
	}
	return true;
}

StructureStart *PillagerOutpostFeature::createStructureStart(int x, int z)
{
	return new OutpostStart(level, random, x, z);
}

// ---------------------------------------------------------------------------------------------------------------------------------
// Start
// ---------------------------------------------------------------------------------------------------------------------------------
PillagerOutpostFeature::OutpostStart::OutpostStart()
{
	// for reflection
}

// The start chunk's corner is the corner of the Java base plate; the tower and the features hang off it at the offsets baked into the layout.
PillagerOutpostFeature::OutpostStart::OutpostStart(Level *level, Random *random, int chunkX, int chunkZ) : StructureStart(chunkX, chunkZ)
{
	const OutpostLayout &layout = OUTPOST_LAYOUTS[random->nextInt(OUTPOST_LAYOUT_COUNT)];
	int ox = chunkX * 16;
	int oz = chunkZ * 16;

	pieces.push_back(new OutpostPiece(&OUTPOST_TOWER[layout.towerRotation], true, ox + layout.towerX, oz + layout.towerZ));
	for (int i = 0; i < layout.placementCount; i++)
	{
		const OutpostPlacement &p = layout.placements[i];
		if (p.type < 0) continue;
		pieces.push_back(new OutpostPiece(&OUTPOST_FEATURE[p.type][p.rotation], false, ox + p.x, oz + p.z));
	}

	calculateBoundingBox();

	// the pieces are lifted onto the terrain when they are generated, so this box must not clip them vertically (monster spawning looks at it)
	boundingBox->y0 = 0;
	boundingBox->y1 = Level::maxBuildHeight - 1;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// Piece
// ---------------------------------------------------------------------------------------------------------------------------------
PillagerOutpostFeature::OutpostPiece::OutpostPiece()
{
	tmpl = nullptr;
	isTower = false;
	heightPosition = -1;
	spawnedGolem = false;
	// for reflection
}

PillagerOutpostFeature::OutpostPiece::OutpostPiece(const OutpostTemplate *tmpl, bool isTower, int x, int z) : StructurePiece(0)
{
	this->tmpl = tmpl;
	this->isTower = isTower;
	heightPosition = -1;
	spawnedGolem = false;

	orientation = Direction::UNDEFINED;
	boundingBox = new BoundingBox(x, 64, z, x + tmpl->sx - 1, 64 + tmpl->sy - 1, z + tmpl->sz - 1);
}

void PillagerOutpostFeature::OutpostPiece::addAdditonalSaveData(CompoundTag *tag)
{
	// structures are rebuilt from the world seed rather than saved here
}

void PillagerOutpostFeature::OutpostPiece::readAdditonalSaveData(CompoundTag *tag)
{
}

// A column of stone or log under a block that is left hanging over a slope, like Java's terrain adaptation.
void PillagerOutpostFeature::OutpostPiece::growFoundation(Level *level, BoundingBox *chunkBB, int wx, int wy, int wz, int tile, int data)
{
	int fillTile = (tile == Tile::log2_Id || tile == Tile::cobblestone_Id || tile == Tile::mossy_cobblestone_Id) ? tile : Tile::cobblestone_Id;
	int fillData = (tile == Tile::log2_Id) ? (data & 3) : 0;	// an upright log

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

// Java's pillager_outpost loot, as far as it is known here: 0-1 crossbow, 2-3 of wheat / potato / carrot, 1-3 dark oak logs, and 1-3 of
// experience bottle / string / arrow / iron ingot / enchanted book. The book carries one random enchantment at a random level and turns up
// on about 11% of the rolls of that last pool (22 of 202 in the weights below).
void PillagerOutpostFeature::OutpostPiece::fillChest(Level *level, Random *random, int wx, int wy, int wz)
{
	static WeighedTreasure *crossbowLoot[1] = { new WeighedTreasure(Item::crossbow_Id, 0, 1, 1, 1) };
	static WeighedTreasure *stapleLoot[3] = {
		new WeighedTreasure(Item::wheat_Id, 0, 3, 5, 7),
		new WeighedTreasure(Item::potato_Id, 0, 2, 3, 5),
		new WeighedTreasure(Item::carrot_Id, 0, 3, 5, 5)
	};
	static WeighedTreasure *logLoot[1] = { new WeighedTreasure(Tile::log2_Id, 1, 2, 3, 1) };
	static WeighedTreasure *miscLoot[4] = {
		new WeighedTreasure(Item::experience_bottle_Id, 0, 1, 1, 70),
		new WeighedTreasure(Item::string_Id, 0, 1, 6, 40),
		new WeighedTreasure(Item::arrow_Id, 0, 2, 7, 40),
		new WeighedTreasure(Item::iron_ingot_Id, 0, 1, 3, 30)
	};

	shared_ptr<ChestTileEntity> chest = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(wx, wy, wz));
	if (chest == nullptr) return;

	WeighedTreasure::addChestItems(random, WeighedTreasureArray(crossbowLoot, 1), chest, random->nextInt(2));
	WeighedTreasure::addChestItems(random, WeighedTreasureArray(stapleLoot, 3), chest, 2 + random->nextInt(2));
	WeighedTreasure::addChestItems(random, WeighedTreasureArray(logLoot, 1), chest, 1 + random->nextInt(3));
	// a fresh book for every chest, so each gets its own random enchantment and level
	WeighedTreasureArray misc = WeighedTreasure::addToTreasure(WeighedTreasureArray(miscLoot, 4), Item::enchanted_book->createForRandomTreasure(random, 1, 1, 22));
	WeighedTreasure::addChestItems(random, misc, chest, 1 + random->nextInt(3));
}

// Pillagers already standing around when you arrive (Java), so the loot has to be fought for. Called for every chunk a piece touches, and the
// pillagers stand in the part of the surroundings that is in that chunk: the tower touches 2-4 chunks and puts 1-2 in each, a feature is
// guarded one time in three per chunk. They never despawn.
void PillagerOutpostFeature::OutpostPiece::spawnGuards(Level *level, Random *random, BoundingBox *chunkBB)
{
	int count = isTower ? 1 + random->nextInt(2) : (random->nextInt(3) == 0 ? 1 : 0);

	for (int n = 0; n < count; n++)
	{
		for (int attempt = 0; attempt < 12; attempt++)
		{
			int x = boundingBox->x0 - 3 + random->nextInt(boundingBox->getXSpan() + 6);
			int z = boundingBox->z0 - 3 + random->nextInt(boundingBox->getZSpan() + 6);
			if (!chunkBB->isInside(x, 64, z)) continue;

			// beside the build, not inside it
			if (x >= boundingBox->x0 && x <= boundingBox->x1 && z >= boundingBox->z0 && z <= boundingBox->z1) continue;

			int y = level->getTopSolidBlock(x, z);
			if (y <= 1 || y > Level::maxBuildHeight - 3) continue;
			if (level->getMaterial(x, y - 1, z)->isLiquid()) continue;

			shared_ptr<Pillager> pillager = std::make_shared<Pillager>(level);
			pillager->moveTo(x + 0.5, y, z + 0.5, random->nextFloat() * 360, 0);
			pillager->finalizeMobSpawn(nullptr);	// gives it its crossbow
			pillager->setPersistenceRequired();
			level->addEntity(pillager);
			break;
		}
	}
}

bool PillagerOutpostFeature::OutpostPiece::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
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
					fillChest(level, random, wx, wy, wz);
				}
				else if (tile == Tile::wall_banner_Id)
				{
					shared_ptr<BannerTileEntity> banner = dynamic_pointer_cast<BannerTileEntity>(level->getTileEntity(wx, wy, wz));
					if (banner != nullptr)
					{
						// the outpost's banners are Ominous Banners
						static const vector<BannerPattern> ominous = BannerItem::getPatterns(BannerItem::createOminousBanner());
						banner->setBaseColor(15);	// white
						for (const BannerPattern &bp : ominous) banner->addPattern(bp.pattern, bp.color);
					}
				}

				if (y == 0) growFoundation(level, chunkBB, wx, wy, wz, tile, data);
			}
		}
	}

	spawnGuards(level, random, chunkBB);

	// the iron golem in the cage (in the one chunk that holds its spot, so it is only spawned once)
	if (tmpl->golem != nullptr && !spawnedGolem)
	{
		double gx = ox + tmpl->golem[0] / 10.0;
		double gy = oy + tmpl->golem[1] / 10.0;
		double gz = oz + tmpl->golem[2] / 10.0;
		if (chunkBB->isInside(Mth::floor(gx), Mth::floor(gy), Mth::floor(gz)))
		{
			spawnedGolem = true;
			shared_ptr<VillagerGolem> golem = std::make_shared<VillagerGolem>(level);
			golem->moveTo(gx, gy, gz, random->nextFloat() * 360, 0);
			golem->setPersistenceRequired();
			level->addEntity(golem);
		}
	}

	return true;
}
