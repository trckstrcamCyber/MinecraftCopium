#include "stdafx.h"
#include "PyreheartSanctumFeature.h"
#include "ChunkPos.h"
#include "BoundingBox.h"
#include "Direction.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"

PyreheartSanctumFeature::PyreheartSanctumFeature() : StructureFeature(), isSpotSelected(false), sanctumChunk(nullptr) {}
PyreheartSanctumFeature::~PyreheartSanctumFeature() { delete sanctumChunk; }
wstring PyreheartSanctumFeature::getFeatureName() { return L"PyreheartSanctum"; }

bool PyreheartSanctumFeature::isFeatureChunk(int x, int z, bool bIsSuperflat)
{
    if (!isSpotSelected)
    {
        random->setSeed(level->getSeed() ^ 0x48454C4C46495245LL);
        // Keep the 35x35 structure inside even the smallest Legacy Nether border.
        // The existing fortress uses the positive quadrant, so reserve the opposite quadrant.
        sanctumChunk = new ChunkPos(-6 + random->nextInt(3), -6 + random->nextInt(3));
        isSpotSelected = true;
    }
    return sanctumChunk != nullptr && x == sanctumChunk->x && z == sanctumChunk->z;
}

StructureStart *PyreheartSanctumFeature::createStructureStart(int x, int z)
{
    return new SanctumStart(level, random, x, z);
}
PyreheartSanctumFeature::SanctumStart::SanctumStart() : StructureStart() {}
PyreheartSanctumFeature::SanctumStart::SanctumStart(Level *level, Random *random, int chunkX, int chunkZ) : StructureStart(chunkX, chunkZ)
{
    pieces.push_back(new SanctumPiece(chunkX, chunkZ));
    calculateBoundingBox();
}
PyreheartSanctumFeature::SanctumPiece::SanctumPiece() : StructurePiece(0) {}
PyreheartSanctumFeature::SanctumPiece::SanctumPiece(int chunkX, int chunkZ) : StructurePiece(0)
{
    int centerX = (chunkX << 4) + 8, centerZ = (chunkZ << 4) + 8;
    boundingBox = new BoundingBox(centerX - 17, 53, centerZ - 17, centerX + 17, 70, centerZ + 17);
    orientation = Direction::UNDEFINED;
}

bool PyreheartSanctumFeature::SanctumPiece::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
    if (level == nullptr || chunkBB == nullptr) return true;
    int x0 = boundingBox->x0, z0 = boundingBox->z0, cx = x0 + 17, cz = z0 + 17, floorY = 55;
    for (int x = x0; x <= x0 + 34; ++x)
        for (int z = z0; z <= z0 + 34; ++z)
        {
            for (int y = floorY + 1; y <= 69; ++y) placeBlock(level, 0, 0, x, y, z, chunkBB);
            int dx = x - cx, dz = z - cz;
            bool ring = abs(dx) == 17 || abs(dz) == 17;
            int floor = Tile::blackstone_block_Id;
            if (ring) floor = Tile::crimson_nether_bricks_Id;
            else if ((x + z) % 11 == 0) floor = Tile::magma_block_Id;
            else if ((x * 3 + z) % 7 == 0) floor = Tile::warped_nether_bricks_Id;
            else if ((x + 2 * z) % 5 == 0) floor = Tile::cracked_crimson_nether_bricks_Id;
            placeBlock(level, floor, 0, x, floorY, z, chunkBB);
            if (ring)
            {
                bool pier = ((abs(dx) == 17) && dz % 4 == 0) || ((abs(dz) == 17) && dx % 4 == 0);
                for (int y = floorY + 1; y <= floorY + (pier ? 10 : 6); ++y)
                    placeBlock(level, pier ? Tile::blackstone_block_Id : Tile::crimson_nether_bricks_Id, 0, x, y, z, chunkBB);
                placeBlock(level, Tile::chiseled_crimson_nether_bricks_Id, 0, x, floorY + 7, z, chunkBB);
            }
        }
    // The inner 23x23 square is edged with warped Nether brick detailing.
    for (int x = cx - 11; x <= cx + 11; ++x)
        for (int z = cz - 11; z <= cz + 11; ++z)
            if (abs(x - cx) == 11 || abs(z - cz) == 11)
                placeBlock(level, Tile::chiseled_warped_nether_bricks_Id, 0, x, floorY + 1, z, chunkBB);
    // Four cardinal pillars: red core + iron chain + Soul Lantern. The core is the sigil/anchor.
    const int px[4] = {cx, cx, cx - 11, cx + 11};
    const int pz[4] = {cz - 11, cz + 11, cz, cz};
    for (int i = 0; i < 4; ++i)
    {
        for (int y = floorY + 1; y <= floorY + 8; ++y)
        {
            int block = y % 3 == 0 ? Tile::warped_nether_bricks_Id : Tile::crimson_nether_bricks_Id;
            placeBlock(level, block, 0, px[i], y, pz[i], chunkBB);
            placeBlock(level, Tile::blackstone_block_Id, 0, px[i] + 1, y, pz[i], chunkBB);
            placeBlock(level, Tile::blackstone_block_Id, 0, px[i] - 1, y, pz[i], chunkBB);
            placeBlock(level, Tile::blackstone_block_Id, 0, px[i], y, pz[i] + 1, chunkBB);
            placeBlock(level, Tile::blackstone_block_Id, 0, px[i], y, pz[i] - 1, chunkBB);
        }
        placeBlock(level, Tile::hellfire_anchor_Id, 0, px[i], floorY + 9, pz[i], chunkBB);
        // Lantern hangs below the chain; the cap block gives the chain support.
        placeBlock(level, Tile::soul_lantern_Id, 0, px[i], floorY + 10, pz[i], chunkBB);
        placeBlock(level, Tile::iron_chain_Id, 0, px[i], floorY + 11, pz[i], chunkBB);
        placeBlock(level, Tile::blackstone_block_Id, 0, px[i], floorY + 12, pz[i], chunkBB);
    }
    // Entrance corridor with flanking walls.
    for (int z = cz + 17; z >= cz + 12; --z)
        for (int x = cx - 3; x <= cx + 3; ++x)
        {
            placeBlock(level, Tile::crimson_nether_bricks_Id, 0, x, floorY, z, chunkBB);
            if (x == cx - 3 || x == cx + 3)
                for (int y = floorY + 1; y <= floorY + 5; ++y) placeBlock(level, Tile::blackstone_block_Id, 0, x, y, z, chunkBB);
        }
    // Four corner lamps and the central altar plinth.
    for (int sx = -1; sx <= 1; sx += 2)
        for (int sz = -1; sz <= 1; sz += 2)
        {
            int lx = cx + sx * 14, lz = cz + sz * 14;
            placeBlock(level, Tile::soul_lantern_Id, 0, lx, floorY + 2, lz, chunkBB);
            placeBlock(level, Tile::iron_chain_Id, 0, lx, floorY + 3, lz, chunkBB);
            placeBlock(level, Tile::blackstone_block_Id, 0, lx, floorY + 4, lz, chunkBB);
        }
    for (int x = cx - 2; x <= cx + 2; ++x)
        for (int z = cz - 2; z <= cz + 2; ++z) placeBlock(level, Tile::blackstone_block_Id, 0, x, floorY + 1, z, chunkBB);
    placeBlock(level, Tile::pyreheart_altar_Id, 0, cx, floorY + 2, cz, chunkBB);
    placeBlock(level, Tile::chiseled_crimson_nether_bricks_Id, 0, cx - 1, floorY + 1, cz, chunkBB);
    placeBlock(level, Tile::chiseled_crimson_nether_bricks_Id, 0, cx + 1, floorY + 1, cz, chunkBB);
    placeBlock(level, Tile::chiseled_crimson_nether_bricks_Id, 0, cx, floorY + 1, cz - 1, chunkBB);
    placeBlock(level, Tile::chiseled_crimson_nether_bricks_Id, 0, cx, floorY + 1, cz + 1, chunkBB);
    for (int x = cx - 5; x <= cx + 5; ++x)
        for (int z = cz - 5; z <= cz + 5; ++z)
            if (abs(x - cx) == 5 || abs(z - cz) == 5) placeBlock(level, Tile::magma_block_Id, 0, x, floorY + 1, z, chunkBB);
    return true;
}
