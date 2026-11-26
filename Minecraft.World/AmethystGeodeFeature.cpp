#include "stdafx.h"
#include <cmath>
#include "AmethystGeodeFeature.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "Dimension.h"
#include "AmethystTile.h"
#include "BlackStoneTile.h"

namespace
{
    const int GEODE_CHUNK_CHANCE = 24; // 1 in 24 chunks
    const int RADIUS_X = 6;
    const int RADIUS_Y = 5;
    const int RADIUS_Z = 6;
    const float OUTER_LAYER = 0.78f;
    const float CALCITE_LAYER = 0.58f;
    const float AMETHYST_LAYER = 0.43f;

    const int DIRECTIONS[6][3] =
    {
        { 1, 0, 0 }, { -1, 0, 0 },
        { 0, 1, 0 }, { 0, -1, 0 },
        { 0, 0, 1 }, { 0, 0, -1 }
    };

    bool IsNaturalHostRock(int tile)
    {
        return tile == Tile::stone_Id || tile == Tile::dirt_Id || tile == Tile::gravel_Id;
    }

    bool InBuildHeight(int y)
    {
        // This fork's overworld generator is 128 blocks tall and starts at Y=0.
        return y >= 1 && y < 127;
    }

    float NormalizedDistance(int dx, int dy, int dz)
    {
        const float nx = static_cast<float>(dx * dx) / static_cast<float>(RADIUS_X * RADIUS_X);
        const float ny = static_cast<float>(dy * dy) / static_cast<float>(RADIUS_Y * RADIUS_Y);
        const float nz = static_cast<float>(dz * dz) / static_cast<float>(RADIUS_Z * RADIUS_Z);
        return std::sqrt(nx + ny + nz);
    }

    bool IsCrackCell(bool cracked, int dx, int dy, int dz)
    {
        // A slim slit on one side exposes the hollow interior. The 5% of
        // geodes which fail the crack roll remain fully enclosed.
        return cracked && dx >= 2 && std::abs(dy) <= 1 && std::abs(dz) <= 1;
    }
}

AmethystGeodeFeature::AmethystGeodeFeature()
    : Feature(false)
{
}

bool AmethystGeodeFeature::place(Level* level, Random* random, int chunkX, int, int chunkZ)
{
    if (level == nullptr || random == nullptr || level->dimension == nullptr)
        return false;

    // Geodes are an Overworld-only feature (dimension ID 0).
    if (level->dimension->id != 0)
        return false;

    if (random->nextInt(GEODE_CHUNK_CHANCE) != 0)
        return false;

    // The source game uses a 128-block-deep world with no negative Y. Keep
    // geodes low in the Overworld while leaving room for the complete shell.
    const int centerX = chunkX + random->nextInt(16) + 8;
    const int centerZ = chunkZ + random->nextInt(16) + 8;
    const int centerY = 10 + random->nextInt(15); // Y 10..24 (outer shell ~5 blocks)
    const bool cracked = random->nextInt(100) < 95;

    // Reject attempts crossing the bottom/top build limits.
    if (!InBuildHeight(centerY - RADIUS_Y - 1) ||
        !InBuildHeight(centerY + RADIUS_Y + 1))
        return false;

    // Preflight the volume. Existing caves are allowed, but liquid, bedrock,
    // and a mostly-empty/mostly-foreign volume stop the feature from chewing
    // through oceans, structures, or the bedrock floor.
    int hostRockCount = 0;
    int liquidCount = 0;
    for (int dx = -RADIUS_X; dx <= RADIUS_X; ++dx)
    {
        for (int dy = -RADIUS_Y; dy <= RADIUS_Y; ++dy)
        {
            for (int dz = -RADIUS_Z; dz <= RADIUS_Z; ++dz)
            {
                const int tile = level->getTile(centerX + dx, centerY + dy, centerZ + dz);
                if (IsNaturalHostRock(tile))
                    ++hostRockCount;
                if (tile == Tile::water_Id || tile == Tile::flowing_water_Id ||
                    tile == Tile::lava_Id || tile == Tile::flowing_lava_Id)
                    ++liquidCount;
            }
        }
    }

    if (hostRockCount < 300 || liquidCount > 0)
        return false;

    // Build three ellipsoidal layers and a hollow center. A small per-cell
    // perturbation gives the shell a less-perfect, natural outline.
    for (int dx = -RADIUS_X; dx <= RADIUS_X; ++dx)
    {
        for (int dy = -RADIUS_Y; dy <= RADIUS_Y; ++dy)
        {
            for (int dz = -RADIUS_Z; dz <= RADIUS_Z; ++dz)
            {
                const int x = centerX + dx;
                const int y = centerY + dy;
                const int z = centerZ + dz;
                if (!InBuildHeight(y))
                    continue;

                const float distance = NormalizedDistance(dx, dy, dz);
                const float irregularity = (static_cast<float>(random->nextInt(5)) - 2.0f) * 0.025f;
                if (distance > 1.0f + irregularity)
                    continue;

                // Carve the crack through the shell so it genuinely exposes
                // the hollow interior instead of leaving the original stone in place.
                if (IsCrackCell(cracked, dx, dy, dz))
                {
                    const int crackTile = level->getTile(x, y, z);
                    if (IsNaturalHostRock(crackTile))
                        placeBlock(level, x, y, z, Tile::air_Id);
                    continue;
                }

                const int existing = level->getTile(x, y, z);
                if (!IsNaturalHostRock(existing))
                    continue;

                if (distance >= OUTER_LAYER + irregularity)
                {
                    placeBlock(level, x, y, z, Tile::blackstone_block_Id,
                        BlackStoneTile::SMOOTH_BASALT);
                }
                else if (distance >= CALCITE_LAYER + irregularity)
                {
                    placeBlock(level, x, y, z, Tile::calcite_block_Id);
                }
                else if (distance >= AMETHYST_LAYER + irregularity)
                {
                    const int variant = (random->nextInt(12) == 0)
                        ? AmethystTile::BUDDING_AMETHYST
                        : AmethystTile::AMETHYST_BLOCK;
                    placeBlock(level, x, y, z, Tile::amethyst_stone_Id, variant);
                }
                else
                {
                    // Hollow interior.
                    placeBlock(level, x, y, z, Tile::air_Id);
                }
            }
        }
    }

    // Seed a few small buds on the inner-facing sides of budding blocks.
    // The budding tile's random tick handles later growth to full clusters.
    int placedBuds = 0;
    for (int dx = -RADIUS_X; dx <= RADIUS_X && placedBuds < 12; ++dx)
    {
        for (int dy = -RADIUS_Y; dy <= RADIUS_Y && placedBuds < 12; ++dy)
        {
            for (int dz = -RADIUS_Z; dz <= RADIUS_Z && placedBuds < 12; ++dz)
            {
                const int x = centerX + dx;
                const int y = centerY + dy;
                const int z = centerZ + dz;
                if (level->getTile(x, y, z) != Tile::amethyst_stone_Id ||
                    level->getData(x, y, z) != AmethystTile::BUDDING_AMETHYST)
                    continue;

                for (int i = 0; i < 6; ++i)
                {
                    const int nx = x + DIRECTIONS[i][0];
                    const int ny = y + DIRECTIONS[i][1];
                    const int nz = z + DIRECTIONS[i][2];
                    const int rx = nx - centerX;
                    const int ry = ny - centerY;
                    const int rz = nz - centerZ;
                    if (NormalizedDistance(rx, ry, rz) >= AMETHYST_LAYER - 0.04f)
                        continue;
                    if (level->getTile(nx, ny, nz) != Tile::air_Id)
                        continue;
                    if (random->nextInt(3) == 0)
                    {
                        placeBlock(level, nx, ny, nz, Tile::amethyst_crystal_Id,
                            AmethystCrystalTile::SMALL);
                        ++placedBuds;
                        break;
                    }
                }
            }
        }
    }

    return true;
}
