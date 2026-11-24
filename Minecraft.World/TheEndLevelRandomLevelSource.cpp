#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.levelgen.h"
#include "net.minecraft.world.level.levelgen.feature.h"
#include "net.minecraft.world.level.levelgen.structure.h"
#include "net.minecraft.world.level.levelgen.synth.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.storage.h"
#include "TheEndLevelRandomLevelSource.h"
#include "TheEndDimension.h"
#include "EndIslandFeature.h"
#include "EndCityFeature.h"
#include "ChorusFlowerTile.h"

TheEndLevelRandomLevelSource::TheEndLevelRandomLevelSource(Level *level, int64_t seed, bool modernEnd)
{
	this->modernEnd = modernEnd;
	m_XZSize = modernEnd ? END_LEVEL_MODERN_WIDTH : END_LEVEL_MIN_WIDTH;
	islandFeature = new EndIslandFeature();
	endCityFeature = modernEnd ? new EndCityFeature(this) : nullptr;

	this->level = level;

	random = new Random(seed);
	pprandom = new Random(seed);	// 4J added
	lperlinNoise1 = new PerlinNoise(random, 16);
	lperlinNoise2 = new PerlinNoise(random, 16);
	perlinNoise1 = new PerlinNoise(random, 8);

	scaleNoise = new PerlinNoise(random, 10);
	depthNoise = new PerlinNoise(random, 16);
}

TheEndLevelRandomLevelSource::~TheEndLevelRandomLevelSource()
{
	delete random;
	delete pprandom;
	delete lperlinNoise1;
	delete lperlinNoise2;
	delete perlinNoise1;
	delete scaleNoise;
	delete depthNoise;
	delete islandFeature;
	delete endCityFeature;
}

void TheEndLevelRandomLevelSource::prepareHeights(int xOffs, int zOffs, byteArray blocks, BiomeArray biomes)
{
	doubleArray buffer;	// 4J - used to be declared with class level scope but tidying up for thread safety reasons

	int xChunks = 16 / CHUNK_WIDTH;

	int xSize = xChunks + 1;
	int ySize = Level::genDepth / CHUNK_HEIGHT + 1;
	int zSize = xChunks + 1;
	buffer = getHeights(buffer, xOffs * xChunks, 0, zOffs * xChunks, xSize, ySize, zSize);

	for (int xc = 0; xc < xChunks; xc++)
	{
		for (int zc = 0; zc < xChunks; zc++)
		{
			for (int yc = 0; yc < Level::genDepth / CHUNK_HEIGHT; yc++)
			{
				double yStep = 1 / static_cast<double>(CHUNK_HEIGHT);
				double s0 = buffer[((xc + 0) * zSize + (zc + 0)) * ySize + (yc + 0)];
				double s1 = buffer[((xc + 0) * zSize + (zc + 1)) * ySize + (yc + 0)];
				double s2 = buffer[((xc + 1) * zSize + (zc + 0)) * ySize + (yc + 0)];
				double s3 = buffer[((xc + 1) * zSize + (zc + 1)) * ySize + (yc + 0)];

				double s0a = (buffer[((xc + 0) * zSize + (zc + 0)) * ySize + (yc + 1)] - s0) * yStep;
				double s1a = (buffer[((xc + 0) * zSize + (zc + 1)) * ySize + (yc + 1)] - s1) * yStep;
				double s2a = (buffer[((xc + 1) * zSize + (zc + 0)) * ySize + (yc + 1)] - s2) * yStep;
				double s3a = (buffer[((xc + 1) * zSize + (zc + 1)) * ySize + (yc + 1)] - s3) * yStep;

				for (int y = 0; y < CHUNK_HEIGHT; y++)
				{
					double xStep = 1 / static_cast<double>(CHUNK_WIDTH);

					double _s0 = s0;
					double _s1 = s1;
					double _s0a = (s2 - s0) * xStep;
					double _s1a = (s3 - s1) * xStep;

					for (int x = 0; x < CHUNK_WIDTH; x++)
					{
						int offs = (x + xc * CHUNK_WIDTH) << Level::genDepthBitsPlusFour | (0 + zc * CHUNK_WIDTH) << Level::genDepthBits | (yc * CHUNK_HEIGHT + y);
						int step = 1 << Level::genDepthBits;
						double zStep = 1 / static_cast<double>(CHUNK_WIDTH);

						double val = _s0;
						double vala = (_s1 - _s0) * zStep;
						for (int z = 0; z < CHUNK_WIDTH; z++)
						{
							int tileId = 0;
							if (val > 0)
							{
								tileId = Tile::end_stone_Id;
							} else {
							}

							blocks[offs] = static_cast<byte>(tileId);
							offs += step;
							val += vala;
						}
						_s0 += _s0a;
						_s1 += _s1a;
					}

					s0 += s0a;
					s1 += s1a;
					s2 += s2a;
					s3 += s3a;
				}
			}
		}
	}
	delete [] buffer.data;

}

void TheEndLevelRandomLevelSource::buildSurfaces(int xOffs, int zOffs, byteArray blocks, BiomeArray biomes)
{
	for (int x = 0; x < 16; x++)
	{
		for (int z = 0; z < 16; z++)
		{
			int runDepth = 1;
			int run = -1;

			byte top = (byte) Tile::end_stone_Id;
			byte material = (byte) Tile::end_stone_Id;

			for (int y = Level::genDepthMinusOne; y >= 0; y--)
			{
				int offs = (z * 16 + x) * Level::genDepth + y;

				int old = blocks[offs];

				if (old == 0)
				{
					run = -1;
				}
				else if (old == Tile::stone_Id)
				{
					if (run == -1)
					{
						if (runDepth <= 0)
						{
							top = 0;
							material = static_cast<byte>(Tile::end_stone_Id);
						}

						run = runDepth;
						if (y >= 0) blocks[offs] = top;
						else blocks[offs] = material;
					}
					else if (run > 0)
					{
						run--;
						blocks[offs] = material;
					}
				}
			}
		}
	}
}

LevelChunk *TheEndLevelRandomLevelSource::create(int x, int z)
{
	return getChunk(x, z);
}

LevelChunk *TheEndLevelRandomLevelSource::getChunk(int xOffs, int zOffs)
{
	random->setSeed(xOffs * 341873128712l + zOffs * 132897987541l);

	BiomeArray biomes;
	// 4J - now allocating this with a physical alloc & bypassing general memory management so that it will get cleanly freed
	unsigned int blocksSize = Level::genDepth * 16 * 16;
	byte *tileData = static_cast<byte *>(XPhysicalAlloc(blocksSize, MAXULONG_PTR, 4096, PAGE_READWRITE));
	XMemSet128(tileData,0,blocksSize);
	byteArray blocks = byteArray(tileData,blocksSize);
	//    byteArray blocks = byteArray(16 * level->depth * 16);

	//    LevelChunk *levelChunk = new LevelChunk(level, blocks, xOffs, zOffs);			// 4J moved below
	level->getBiomeSource()->getBiomeBlock(biomes, xOffs * 16, zOffs * 16, 16, 16, true);

	if (!modernEnd || TheEndDimension::isChunkInArea(xOffs, zOffs))
	{
		prepareHeights(xOffs, zOffs, blocks, biomes);
		buildSurfaces(xOffs, zOffs, blocks, biomes);
	}
	if (endCityFeature != nullptr) endCityFeature->apply(this, level, xOffs, zOffs, blocks);

	// 4J - this now creates compressed block data from the blocks array passed in, so moved it until after the blocks are actually finalised. We also
	// now need to free the passed in blocks as the LevelChunk doesn't use the passed in allocation anymore.
	LevelChunk *levelChunk = new LevelChunk(level, blocks, xOffs, zOffs);
	XPhysicalFree(tileData);

	levelChunk->recalcHeightmap();

	//delete blocks.data; // Don't delete the blocks as the array data is actually owned by the chunk now
	delete biomes.data;

	return levelChunk;
}

doubleArray TheEndLevelRandomLevelSource::getHeights(doubleArray buffer, int x, int y, int z, int xSize, int ySize, int zSize)
{
	if (buffer.data == nullptr)
	{
		buffer = doubleArray(xSize * ySize * zSize);
	}

	double s = 1 * 684.412;
	double hs = 1 * 684.412;

	doubleArray pnr, ar, br, sr, dr, fi, fis;	// 4J - used to be declared with class level scope but moved here for thread safety

	sr = scaleNoise->getRegion(sr, x, z, xSize, zSize, 1.121, 1.121, 0.5);
	dr = depthNoise->getRegion(dr, x, z, xSize, zSize, 200.0, 200.0, 0.5);

	s *= 2;

	pnr = perlinNoise1->getRegion(pnr, x, y, z, xSize, ySize, zSize, s / 80.0, hs / 160.0, s / 80.0);
	ar = lperlinNoise1->getRegion(ar, x, y, z, xSize, ySize, zSize, s, hs, s);
	br = lperlinNoise2->getRegion(br, x, y, z, xSize, ySize, zSize, s, hs, s);

	int p = 0;
	int pp = 0;

	for (int xx = 0; xx < xSize; xx++)
	{
		for (int zz = 0; zz < zSize; zz++)
		{
			double scale = ((sr[pp] + 256.0) / 512);
			if (scale > 1) scale = 1;


			double depth = (dr[pp] / 8000.0);
			if (depth < 0) depth = -depth * 0.3;
			depth = depth * 3.0 - 2.0;

			float xd = ((xx + x) - 0) / 1.0f;
			float zd = ((zz + z) - 0) / 1.0f;
			float doffs = 100 - sqrt(xd * xd + zd * zd) * 8;
			if (doffs > 80) doffs = 80;
			if (doffs < -100) doffs = -100;
			if (modernEnd)
			{
				doffs = getIslandHeightValue(x / 2, z / 2, xx, zz);
				if (doffs > 80) doffs = 80;
				if (doffs < -100) doffs = -100;
			}
			if (depth > 1) depth = 1;
			depth = depth / 8;
			depth = 0;

			if (scale < 0) scale = 0;
			scale = (scale) + 0.5;
			depth = depth * ySize / 16;

			pp++;

			double yCenter = ySize / 2.0;


			for (int yy = 0; yy < ySize; yy++)
			{
				double val = 0;
				double yOffs = (yy - (yCenter)) * 8 / scale;

				if (yOffs < 0) yOffs *= -1;

				double bb = ar[p] / 512;
				double cc = br[p] / 512;

				double v = (pnr[p] / 10 + 1) / 2;
				if (v < 0) val = bb;
				else if (v > 1) val = cc;
				else val = bb + (cc - bb) * v;
				val -= 8;
				val += doffs;

				int r = 2;
				if (yy > ySize / 2 - r)
				{
					double slide = (yy - (ySize / 2 - r)) / (64.0f);
					if (slide < 0) slide = 0;
					if (slide > 1) slide = 1;
					val = val * (1 - slide) + -3000 * slide;
				}
				r = 8;
				if (yy < r)
				{
					double slide = (r - yy) / (r - 1.0f);
					val = val * (1 - slide) + -30 * slide;
				}


				if (modernEnd)
				{
					int worldY = (y + yy) * CHUNK_HEIGHT;
					double pdx = (x + xx) * CHUNK_WIDTH;
					double pdy = worldY - level->seaLevel;
					double pdz = (z + zz) * CHUNK_WIDTH;
					double fade = (19.0 - sqrt(pdx * pdx + pdy * pdy + pdz * pdz)) / 16.0;
					if (fade < 0) fade = 0;
					if (fade > 1) fade = 1;
					val = fade * (level->seaLevel < worldY + 1 ? -50.0 : 50.0) + (1 - fade) * val;
				}
				buffer[p] = val;
				p++;
			}
		}
	}

	delete [] pnr.data;
	delete [] ar.data;
	delete [] br.data;
	delete [] sr.data;
	delete [] dr.data;
	delete [] fi.data;
	delete [] fis.data;

	return buffer;

}

bool TheEndLevelRandomLevelSource::hasChunk(int x, int y)
{
	return true;
}

void TheEndLevelRandomLevelSource::calcWaterDepths(ChunkSource *parent, int xt, int zt)
{
	int xo = xt * 16;
	int zo = zt * 16;
	for (int x = 0; x < 16; x++)
	{
		int y = level->getSeaLevel();
		for (int z = 0; z < 16; z++)
		{
			int xp = xo + x + 7;
			int zp = zo + z + 7;
			int h = level->getHeightmap(xp, zp);
			if (h <= 0)
			{
				if (level->getHeightmap(xp - 1, zp) > 0 || level->getHeightmap(xp + 1, zp) > 0 || level->getHeightmap(xp, zp - 1) > 0 || level->getHeightmap(xp, zp + 1) > 0)
				{
					bool hadWater = false;
					if (hadWater || (level->getTile(xp - 1, y, zp) == Tile::water_Id && level->getData(xp - 1, y, zp) < 7)) hadWater = true;
					if (hadWater || (level->getTile(xp + 1, y, zp) == Tile::water_Id && level->getData(xp + 1, y, zp) < 7)) hadWater = true;
					if (hadWater || (level->getTile(xp, y, zp - 1) == Tile::water_Id && level->getData(xp, y, zp - 1) < 7)) hadWater = true;
					if (hadWater || (level->getTile(xp, y, zp + 1) == Tile::water_Id && level->getData(xp, y, zp + 1) < 7)) hadWater = true;
					if (hadWater)
					{
						for (int x2 = -5; x2 <= 5; x2++)
						{
							for (int z2 = -5; z2 <= 5; z2++)
							{
								int d = (x2 > 0 ? x2 : -x2) + (z2 > 0 ? z2 : -z2);

								if (d <= 5)
								{
									d = 6 - d;
									if (level->getTile(xp + x2, y, zp + z2) == Tile::water_Id)
									{
										int od = level->getData(xp + x2, y, zp + z2);
										if (od < 7 && od < d)
										{
											level->setData(xp + x2, y, zp + z2, d, Tile::UPDATE_CLIENTS);
										}
									}
								}
							}
						}
						if (hadWater)
						{
							level->setTileAndData(xp, y, zp, Tile::water_Id, 7, Tile::UPDATE_CLIENTS);
							for (int y2 = 0; y2 < y; y2++)
							{
								level->setTileAndData(xp, y2, zp, Tile::water_Id, 8, Tile::UPDATE_CLIENTS);
							}
						}
					}
				}
			}
		}
	}

}

void TheEndLevelRandomLevelSource::postProcess(ChunkSource *parent, int xt, int zt)
{
	HeavyTile::instaFall = true;
	int xo = xt * 16;
	int zo = zt * 16;

	// 4J - added. The original java didn't do any setting of the random seed here, and passes the level random to the biome decorator.
	// We'll be running our postProcess in parallel with getChunk etc. so we need to use a separate random - have used the same initialisation code as
	// used in RandomLevelSource::postProcess to make sure this random value is consistent for each world generation.
	pprandom->setSeed(level->getSeed());
	int64_t xScale = pprandom->nextLong() / 2 * 2 + 1;
	int64_t zScale = pprandom->nextLong() / 2 * 2 + 1;
	pprandom->setSeed(((xt * xScale) + (zt * zScale)) ^ level->getSeed());

	if (endCityFeature != nullptr) endCityFeature->postProcess(level, pprandom, xt, zt);

	Biome *biome = level->getBiome(xo + 16, zo + 16);
	biome->decorate(level, pprandom, xo, zo);		// 4J - passing pprandom rather than level->random here to make this consistent with our parallel world generation

	if (modernEnd && outerIslandCheck(xt, zt))
	{
		if (getIslandHeightValue(xt, zt, 1, 1) < -20.0f && pprandom->nextInt(14) == 0)
		{
			islandFeature->place(level, pprandom, xo + pprandom->nextInt(16) + 8, pprandom->nextInt(16) + 55, zo + pprandom->nextInt(16) + 8);
			if (pprandom->nextInt(4) == 0)
			{
				islandFeature->place(level, pprandom, xo + pprandom->nextInt(16) + 8, pprandom->nextInt(16) + 55, zo + pprandom->nextInt(16) + 8);
			}
		}

		if (getIslandHeightValue(xt, zt, 1, 1) > 40.0f)
		{
			int plants = pprandom->nextInt(5);
			for (int i = 0; i < plants; i++)
			{
				int px = xo + pprandom->nextInt(16) + 8;
				int pz = zo + pprandom->nextInt(16) + 8;
				int py = level->getHeightmap(px, pz);
				if (py > 0 && level->isEmptyTile(px, py, pz) && level->getTile(px, py - 1, pz) == Tile::end_stone_Id)
				{
					ChorusFlowerTile::generatePlant(level, px, py, pz, pprandom, 8);
				}
			}
		}
	}

	HeavyTile::instaFall = false;

	app.processSchematics(parent->getChunk(xt,zt));
}

bool TheEndLevelRandomLevelSource::save(bool force, ProgressListener *progressListener)
{
	return true;
}

bool TheEndLevelRandomLevelSource::tick()
{
	return false;
}

bool TheEndLevelRandomLevelSource::shouldSave()
{
	return true;
}

wstring TheEndLevelRandomLevelSource::gatherStats()
{
	return L"RandomLevelSource";
}

vector<Biome::MobSpawnerData *> *TheEndLevelRandomLevelSource::getMobsAt(MobCategory *mobCategory, int x, int y, int z)
{
	Biome *biome = level->getBiome(x, z);
	if (biome == nullptr)
	{
		return nullptr;
	}
	return biome->getMobs(mobCategory);
}

TilePos *TheEndLevelRandomLevelSource::findNearestMapFeature(Level *level, const wstring& featureName, int x, int y, int z)
{
	return nullptr;
}

void TheEndLevelRandomLevelSource::recreateLogicStructuresForChunk(int chunkX, int chunkZ)
{
	if (endCityFeature != nullptr) endCityFeature->apply(this, level, chunkX, chunkZ, byteArray());
}
double TheEndLevelRandomLevelSource::outerIslandProximity(int chunkX, int chunkZ)
{
	int area = TheEndDimension::closestArea(chunkX, chunkZ);
	if (area == 0) return 12.0;

	double xd = chunkX + 0.5 - TheEndDimension::AREA_CHUNK_X[area];
	double zd = chunkZ + 0.5 - TheEndDimension::AREA_CHUNK_Z[area];
	return max(fabs(xd), fabs(zd));
}

bool TheEndLevelRandomLevelSource::outerIslandCheck(int chunkX, int chunkZ)
{
	return outerIslandProximity(chunkX, chunkZ) < 3.0;
}

float TheEndLevelRandomLevelSource::getIslandHeightValue(int chunkX, int chunkZ, int xOffset, int zOffset)
{
	if (TheEndDimension::closestArea(chunkX, chunkZ) == 0)
	{
		float sampleX = chunkX * 2.0f + xOffset;
		float sampleZ = chunkZ * 2.0f + zOffset;
		return 100.0f - sqrt(sampleX * sampleX + sampleZ * sampleZ) * 9.0f;
	}

	int sampleX = chunkX * 2 + xOffset;
	int sampleZ = chunkZ * 2 + zOffset;
	float height = 100.0f - sqrt(static_cast<float>(sampleX * sampleX + sampleZ * sampleZ)) * 8.0f;
	if (height > 80.0f) height = 80.0f;
	if (height < -100.0f) height = -100.0f;

	for (int ox = -4; ox <= 4; ox++)
	{
		for (int oz = -4; oz <= 4; oz++)
		{
			int islandX = chunkX + ox;
			int islandZ = chunkZ + oz;
			if (!outerIslandCheck(islandX, islandZ)) continue;

			float scale = static_cast<float>(static_cast<int>(fabs(static_cast<double>(islandX)) * 3439.0 + fabs(static_cast<double>(islandZ)) * 147.0) % 6 + 18);
			int dx = xOffset - ox * 2;
			int dz = zOffset - oz * 2;
			float islandHeight = 100.0f - sqrt(static_cast<float>(dx * dx + dz * dz)) * scale;
			if (islandHeight > 80.0f) islandHeight = 80.0f;
			if (islandHeight < -100.0f) islandHeight = -100.0f;
			if (islandHeight > height) height = islandHeight;
		}
	}
	return height;
}
