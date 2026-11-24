#include "stdafx.h"
#include "LevelData.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.network.packet.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.chunk.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.phys.h"
#include "TheEndGatewayTileEntity.h"
#include "TheEndDimension.h"
#include "EndGatewayFeature.h"
#include "EndIslandFeature.h"
#include "TileEntityDataPacket.h"
#include "Facing.h"

TheEndGatewayTileEntity::TheEndGatewayTileEntity() : TileEntity()
{
	age = 0;
	teleportCooldown = 0;
	hasExit = false;
	exitX = exitY = exitZ = 0;
	exactTeleport = false;
}

void TheEndGatewayTileEntity::save(CompoundTag *tag)
{
	TileEntity::save(tag);
	tag->putLong(L"Age", age);
	if (hasExit)
	{
		CompoundTag *exit = new CompoundTag();
		exit->putInt(L"X", exitX);
		exit->putInt(L"Y", exitY);
		exit->putInt(L"Z", exitZ);
		tag->putCompound(L"ExitPortal", exit);
	}
	if (exactTeleport) tag->putBoolean(L"ExactTeleport", true);
}

void TheEndGatewayTileEntity::load(CompoundTag *tag)
{
	TileEntity::load(tag);
	age = tag->getLong(L"Age");
	hasExit = tag->contains(L"ExitPortal");
	if (hasExit)
	{
		CompoundTag *exit = tag->getCompound(L"ExitPortal");
		exitX = exit->getInt(L"X");
		exitY = exit->getInt(L"Y");
		exitZ = exit->getInt(L"Z");
	}
	exactTeleport = tag->getBoolean(L"ExactTeleport");
}

double TheEndGatewayTileEntity::getViewDistance()
{
	return 65536.0;
}

bool TheEndGatewayTileEntity::isSpawning()
{
	return age < SPAWN_TIME;
}

bool TheEndGatewayTileEntity::isCoolingDown()
{
	return teleportCooldown > 0;
}

float TheEndGatewayTileEntity::getSpawnPercent(float a)
{
	return Mth::clamp((static_cast<float>(age) + a) / static_cast<float>(SPAWN_TIME), 0.0f, 1.0f);
}

float TheEndGatewayTileEntity::getCooldownPercent(float a)
{
	return 1.0f - Mth::clamp((static_cast<float>(teleportCooldown) - a) / static_cast<float>(COOLDOWN_TIME), 0.0f, 1.0f);
}

void TheEndGatewayTileEntity::tick()
{
	bool spawning = isSpawning();
	bool cooling = isCoolingDown();
	age++;

	if (cooling)
	{
		teleportCooldown--;
	}
	else if (!level->isClientSide)
	{
		vector<shared_ptr<Entity> > *entities = level->getEntities(nullptr, AABB::newTemp(x, y, z, x + 1, y + 1, z + 1));
		if (entities != nullptr)
		{
			for (auto &entity : *entities)
			{
				if (entity->GetType() == eTYPE_ENDER_CRYSTAL || entity->instanceof(eTYPE_ENDERDRAGON) || entity->GetType() == eTYPE_MULTIENTITY_MOB_PART) continue;
				teleportEntity(entity);
				break;
			}
		}

		if (age % ATTENTION_INTERVAL == 0) triggerCooldown();
	}

	if (spawning != isSpawning() || cooling != isCoolingDown()) setChanged();
}

void TheEndGatewayTileEntity::triggerCooldown()
{
	if (level->isClientSide) return;
	teleportCooldown = COOLDOWN_TIME;
	level->tileEvent(x, y, z, Tile::end_gateway_Id, EVENT_COOLDOWN, 0);
	setChanged();
}

bool TheEndGatewayTileEntity::triggerEvent(int b0, int b1)
{
	if (b0 == EVENT_COOLDOWN)
	{
		teleportCooldown = COOLDOWN_TIME;
		return true;
	}
	return TileEntity::triggerEvent(b0, b1);
}

void TheEndGatewayTileEntity::setExitPosition(int x, int y, int z)
{
	hasExit = true;
	exitX = x;
	exitY = y;
	exitZ = z;
}

void TheEndGatewayTileEntity::teleportEntity(shared_ptr<Entity> entity)
{
	if (level->isClientSide || isCoolingDown()) return;

	if (entity->instanceof(eTYPE_THROWNENDERPEARL))
	{
		shared_ptr<LivingEntity> owner = dynamic_pointer_cast<ThrownEnderpearl>(entity)->getOwner();
		if (owner != nullptr)
		{
			teleportEntity(owner);
			entity->remove();
			return;
		}
	}

	teleportCooldown = 100;
	if (!hasExit && level->dimension->id == 1) findExitPortal();

	if (hasExit)
	{
		if (entity->riding != nullptr) entity->ride(nullptr);
		shared_ptr<Entity> rider = entity->rider.lock();
		if (rider != nullptr) rider->ride(nullptr);

		level->getChunkSource()->create(exitX >> 4, exitZ >> 4);

		int tx = exitX, ty = exitY, tz = exitZ;
		if (!exactTeleport)
		{
			findTallestBlock(level, exitX, exitY, exitZ, 5, false, tx, ty, tz);
			ty += 1;
			if ((tx == exitX && tz == exitZ) && (ty == exitY || ty == exitY + 1))
			{
				level->setTileAndUpdate(exitX, exitY + 1, exitZ, Tile::bedrock_Id);
				level->setTileAndUpdate(exitX, exitY + 2, exitZ, Tile::bedrock_Id);
				level->setTileAndUpdate(exitX, exitY + 3, exitZ, 0);
				level->setTileAndUpdate(exitX, exitY + 4, exitZ, 0);
				ty = exitY + 3;
			}
		}

		double px = tx + 0.5, py = ty, pz = tz + 0.5;
		if (entity->instanceof(eTYPE_LIVINGENTITY))
		{
			dynamic_pointer_cast<LivingEntity>(entity)->teleportTo(px, py, pz);
		}
		else
		{
			entity->moveTo(px, py, pz, entity->yRot, entity->xRot);
		}
		entity->fallDistance = 0;
	}

	triggerCooldown();
}

void TheEndGatewayTileEntity::findTallestBlock(Level *level, int x, int y, int z, int radius, bool allowBedrock, int &outX, int &outY, int &outZ)
{
	bool found = false;
	outX = x;
	outY = y;
	outZ = z;
	for (int dx = -radius; dx < radius; dx++)
	{
		for (int dz = -radius; dz < radius; dz++)
		{
			if (dx == 0 && dz == 0 && !allowBedrock) continue;

			for (int yy = Level::maxBuildHeight - 1; yy > (found ? outY : 0); yy--)
			{
				int tile = level->getTile(x + dx, yy, z + dz);
				if (Tile::isSolidBlockingTile(tile) && (allowBedrock || tile != Tile::bedrock_Id))
				{
					outX = x + dx;
					outY = yy;
					outZ = z + dz;
					found = true;
					break;
				}
			}
		}
	}
}

bool TheEndGatewayTileEntity::findValidSpawnInChunk(LevelChunk *chunk, int &outX, int &outY, int &outZ)
{
	bool found = false;
	double bestDistance = 0;
	int top = chunk->getHighestSectionPosition() + 15;
	for (int lx = 1; lx <= 14; lx++)
	{
		for (int lz = 1; lz <= 14; lz++)
		{
			for (int yy = 30; yy <= top && yy < Level::maxBuildHeight - 2; yy++)
			{
				if (chunk->getTile(lx, yy, lz) != Tile::end_stone_Id) continue;
				if (Tile::isSolidBlockingTile(chunk->getTile(lx, yy + 1, lz))) continue;
				if (Tile::isSolidBlockingTile(chunk->getTile(lx, yy + 2, lz))) continue;

				int wx = chunk->x * 16 + lx;
				int wz = chunk->z * 16 + lz;
				double distance = (wx + 0.5) * (wx + 0.5) + (yy + 0.5) * (yy + 0.5) + (wz + 0.5) * (wz + 0.5);
				if (!found || distance < bestDistance)
				{
					found = true;
					bestDistance = distance;
					outX = wx;
					outY = yy;
					outZ = wz;
				}
			}
		}
	}
	return found;
}

void TheEndGatewayTileEntity::findExitPortal()
{
	TheEndDimension *end = dynamic_cast<TheEndDimension *>(level->dimension);
	if (end == nullptr || !end->isModernEnd()) return;

	int portalNumber = level->getData(x, y, z);
	Random random(level->getSeed() + portalNumber);

	TheEndDimension::setWorldSize(level->getLevelData()->getXZSize());
	int area = 1 + portalNumber % (TheEndDimension::AREA_COUNT - 1);
	int xOffset = random.nextInt(2) + 3;
	if (!random.nextBoolean()) xOffset = -xOffset;
	int zOffset = random.nextInt(2) + 3;
	if (!random.nextBoolean()) zOffset = -zOffset;
	int cx = TheEndDimension::AREA_CHUNK_X[area] + xOffset;
	int cz = TheEndDimension::AREA_CHUNK_Z[area] + zOffset;

	LevelChunk *chunk = level->getChunkSource()->create(cx, cz);

	for (int lx = 0; lx < 16; lx++)
	{
		for (int lz = 0; lz < 16; lz++)
		{
			for (int yy = 0; yy < Level::maxBuildHeight; yy++)
			{
				if (chunk->getTile(lx, yy, lz) == Tile::end_gateway_Id)
				{
					setExitPosition(cx * 16 + lx, yy, cz * 16 + lz);
					setChanged();
					return;
				}
			}
		}
	}

	int sx, sy, sz;
	if (!findValidSpawnInChunk(chunk, sx, sy, sz))
	{
		sx = cx * 16 + 8;
		sy = 75;
		sz = cz * 16 + 8;
		Random islandRandom(static_cast<int64_t>(sx) * 31 + sz);
		EndIslandFeature().place(level, &islandRandom, sx, sy, sz);
	}

	int tx, ty, tz;
	findTallestBlock(level, cx * 16 + 8, 0, cz * 16 + 8, 7, true, tx, ty, tz);
	setExitPosition(tx, ty + 10, tz);
	createExitPortal(exitX, exitY, exitZ);
	setChanged();
}

void TheEndGatewayTileEntity::createExitPortal(int px, int py, int pz)
{
	Random random;
	EndGatewayFeature().place(level, &random, px, py, pz);

	shared_ptr<TheEndGatewayTileEntity> gateway = dynamic_pointer_cast<TheEndGatewayTileEntity>(level->getTileEntity(px, py, pz));
	if (gateway != nullptr)
	{
		gateway->setExitPosition(x, y, z);
		gateway->setChanged();
	}
}

bool TheEndGatewayTileEntity::shouldRenderFace(int face)
{
	if (level == nullptr) return true;
	int nx = x + Facing::STEP_X[face];
	int ny = y + Facing::STEP_Y[face];
	int nz = z + Facing::STEP_Z[face];
	int tile = level->getTile(nx, ny, nz);
	return tile != Tile::end_gateway_Id && !Tile::isSolidBlockingTile(tile);
}

int TheEndGatewayTileEntity::getParticleAmount()
{
	int amount = 0;
	for (int face = 0; face < 6; face++)
	{
		if (shouldRenderFace(face)) amount++;
	}
	return amount;
}

shared_ptr<Packet> TheEndGatewayTileEntity::getUpdatePacket()
{
	CompoundTag *tag = new CompoundTag();
	save(tag);
	return std::make_shared<TileEntityDataPacket>(x, y, z, TileEntityDataPacket::TYPE_END_GATEWAY, tag);
}

shared_ptr<TileEntity> TheEndGatewayTileEntity::clone()
{
	shared_ptr<TheEndGatewayTileEntity> result = std::make_shared<TheEndGatewayTileEntity>();
	TileEntity::clone(result);
	result->age = age;
	result->teleportCooldown = teleportCooldown;
	result->hasExit = hasExit;
	result->exitX = exitX;
	result->exitY = exitY;
	result->exitZ = exitZ;
	result->exactTeleport = exactTeleport;
	return result;
}
