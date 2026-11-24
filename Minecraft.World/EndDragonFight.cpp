#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.storage.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.boss.enderdragon.h"
#include "net.minecraft.world.phys.h"
#include "EndDragonFight.h"
#include "TheEndDimension.h"
#include "TheEndBiomeDecorator.h"
#include "EndGatewayFeature.h"
#include "SpikeFeature.h"
#include "TheEndPortal.h"
#include "SoundTypes.h"
#include "LevelEvent.h"

EndDragonFight::EndDragonFight(Level *level)
{
	this->level = level;
	respawnStage = eRespawn_None;
	respawnTime = 0;
}

EndDragonFight *EndDragonFight::get(Level *level)
{
	if (level == nullptr || level->isClientSide) return nullptr;
	TheEndDimension *end = dynamic_cast<TheEndDimension *>(level->dimension);
	if (end == nullptr || !end->isModernEnd()) return nullptr;
	return end->getDragonFight();
}

int EndDragonFight::getPortalY()
{
	return level->seaLevel;
}

void EndDragonFight::spawnExitPortal(Level *level, bool active, bool placeEgg)
{
	int x = 0, z = 0;
	int y = level->seaLevel;
	int r = 4;

	TheEndPortal::allowAnywhere(true);
	for (int yy = y - 1; yy <= y + 32; yy++)
	{
		for (int xx = x - r; xx <= x + r; xx++)
		{
			for (int zz = z - r; zz <= z + r; zz++)
			{
				double xd = xx - x;
				double zd = zz - z;
				double d = sqrt(xd * xd + zd * zd);
				if (d > r - 0.5) continue;

				if (yy < y)
				{
					if (d <= r - 1 - 0.5) level->setTileAndUpdate(xx, yy, zz, Tile::bedrock_Id);
				}
				else if (yy > y)
				{
					if (level->getTile(xx, yy, zz) != Tile::dragon_egg_Id) level->setTileAndUpdate(xx, yy, zz, 0);
				}
				else if (d > r - 1 - 0.5)
				{
					level->setTileAndUpdate(xx, yy, zz, Tile::bedrock_Id);
				}
				else
				{
					level->setTileAndUpdate(xx, yy, zz, active ? Tile::end_portal_Id : 0);
				}
			}
		}
	}

	level->setTileAndUpdate(x, y + 0, z, Tile::bedrock_Id);
	level->setTileAndUpdate(x, y + 1, z, Tile::bedrock_Id);
	level->setTileAndUpdate(x, y + 2, z, Tile::bedrock_Id);
	level->setTileAndUpdate(x - 1, y + 2, z, Tile::torch_Id);
	level->setTileAndUpdate(x + 1, y + 2, z, Tile::torch_Id);
	level->setTileAndUpdate(x, y + 2, z - 1, Tile::torch_Id);
	level->setTileAndUpdate(x, y + 2, z + 1, Tile::torch_Id);
	level->setTileAndUpdate(x, y + 3, z, Tile::bedrock_Id);
	if (placeEgg) level->setTileAndUpdate(x, y + 4, z, Tile::dragon_egg_Id);

	for (int yy = y - 5; yy < y - 1; yy++)
	{
		for (int xx = x - (r - 1); xx <= x + (r - 1); xx++)
		{
			for (int zz = z - (r - 1); zz <= z + (r - 1); zz++)
			{
				if (level->isEmptyTile(xx, yy, zz)) level->setTileAndUpdate(xx, yy, zz, Tile::end_stone_Id);
			}
		}
	}
	TheEndPortal::allowAnywhere(false);
}

void EndDragonFight::onDragonKilled()
{
	LevelData *data = level->getLevelData();
	spawnExitPortal(level, true, !data->isEndPreviouslyKilled());

	if (!data->isEndGatewaySpawned())
	{
		Random random;
		EndGatewayFeature().place(level, &random, GATEWAY_X, GATEWAY_Y, GATEWAY_Z);
		level->levelEvent(LevelEvent::ANIMATION_END_GATEWAY_SPAWN, GATEWAY_X, GATEWAY_Y, GATEWAY_Z, 0);
		data->setEndGatewaySpawned(true);
	}

	data->setEndPreviouslyKilled(true);
	data->setEndDragonKilled(true);
}

void EndDragonFight::findPortalCrystals(vector<shared_ptr<EnderCrystal> > &result)
{
	result.clear();
	int y = getPortalY() + 1;
	static const int SIDES[4][2] = { { 2, 0 }, { -2, 0 }, { 0, 2 }, { 0, -2 } };
	for (int i = 0; i < 4; i++)
	{
		int bx = SIDES[i][0];
		int bz = SIDES[i][1];
		double x0 = bx;
		double z0 = bz;
		vector<shared_ptr<Entity> > *entities = level->getEntitiesOfClass(typeid(EnderCrystal), AABB::newTemp(x0, y, z0, x0 + 1, y + 1, z0 + 1));
		bool found = false;
		if (entities != nullptr)
		{
			for (auto &e : *entities)
			{
				shared_ptr<EnderCrystal> crystal = dynamic_pointer_cast<EnderCrystal>(e);
				if (crystal != nullptr && !crystal->removed)
				{
					result.push_back(crystal);
					found = true;
				}
			}
			delete entities;
		}
		if (!found)
		{
			result.clear();
			return;
		}
	}
}

void EndDragonFight::tryRespawn()
{
	LevelData *data = level->getLevelData();
	if (!data->isEndDragonKilled() || respawnStage != eRespawn_None) return;

	vector<shared_ptr<EnderCrystal> > crystals;
	findPortalCrystals(crystals);
	if (crystals.size() < 4) return;

	respawnCrystals = crystals;
	spawnExitPortal(level, false, false);
	setRespawnStage(eRespawn_Start);
}

void EndDragonFight::onCrystalDestroyed(shared_ptr<EnderCrystal> crystal)
{
	if (respawnStage == eRespawn_None) return;

	bool tracked = false;
	for (auto &c : respawnCrystals)
	{
		if (c == crystal) tracked = true;
	}
	if (!tracked) return;

	for (auto &c : respawnCrystals)
	{
		if (c != crystal) c->clearBeamTarget();
	}
	respawnCrystals.clear();
	setRespawnStage(eRespawn_None);
	resetSpikeCrystals();
	spawnExitPortal(level, true, false);
}

void EndDragonFight::setRespawnStage(RespawnStage stage)
{
	respawnStage = stage;
	respawnTime = 0;
}

void EndDragonFight::tick()
{
	if (respawnStage != eRespawn_None)
	{
		tickRespawn();
		respawnTime++;
	}
}

void EndDragonFight::pointCrystalsAt(int x, int y, int z)
{
	for (auto &c : respawnCrystals)
	{
		if (!c->removed) c->setBeamTarget(x, y, z);
	}
}

void EndDragonFight::growl()
{
	level->levelEvent(LevelEvent::ANIMATION_DRAGON_SUMMON_ROAR, 0, 128, 0, 0);
}

void EndDragonFight::tickRespawn()
{
	switch (respawnStage)
	{
	case eRespawn_Start:
		pointCrystalsAt(0, 128, 0);
		setRespawnStage(eRespawn_PreparingPillars);
		break;

	case eRespawn_PreparingPillars:
		if (respawnTime < 100)
		{
			if (respawnTime == 0 || respawnTime == 50 || respawnTime == 51 || respawnTime == 52 || respawnTime >= 95) growl();
		}
		else
		{
			setRespawnStage(eRespawn_SummoningPillars);
		}
		break;

	case eRespawn_SummoningPillars:
		{
			int index = respawnTime / 40;
			bool target = respawnTime % 40 == 0;
			bool rebuild = respawnTime % 40 == 39;
			if (index < 8)
			{
				const TheEndBiomeDecorator::SPIKE &spike = TheEndBiomeDecorator::SpikeValA[index];
				if (target)
				{
					int top = level->getTopSolidBlock(spike.x, spike.z);
					pointCrystalsAt(spike.x, top + 1, spike.z);
				}
				else if (rebuild)
				{
					rebuildSpike(index);
				}
			}
			else if (target)
			{
				setRespawnStage(eRespawn_SummoningDragon);
			}
		}
		break;

	case eRespawn_SummoningDragon:
		if (respawnTime < 100)
		{
			if (respawnTime == 0) pointCrystalsAt(0, 128, 0);
			else if (respawnTime < 5 || respawnTime >= 80) growl();
		}
		else
		{
			resetSpikeCrystals();
			for (auto &c : respawnCrystals)
			{
				if (c->removed) continue;
				c->clearBeamTarget();
				level->explode(c, c->x, c->y, c->z, 6.0f, false);
				c->remove();
			}
			respawnCrystals.clear();
			setRespawnStage(eRespawn_None);
			createNewDragon();
			level->getLevelData()->setEndDragonKilled(false);
			spawnExitPortal(level, false, false);
		}
		break;

	default:
		break;
	}
}

void EndDragonFight::rebuildSpike(int index)
{
	const TheEndBiomeDecorator::SPIKE &spike = TheEndBiomeDecorator::SpikeValA[index];

	int ground = getPortalY() - 10;
	for (int yy = Level::maxBuildHeight - 1; yy > 0; yy--)
	{
		if (level->getTile(spike.x, yy, spike.z) == Tile::end_stone_Id)
		{
			ground = yy;
			break;
		}
	}
	int r = spike.radius + 2;
	for (int yy = ground + 1; yy < Level::maxBuildHeight; yy++)
	{
		for (int xx = spike.x - r; xx <= spike.x + r; xx++)
		{
			for (int zz = spike.z - r; zz <= spike.z + r; zz++)
			{
				int tile = level->getTile(xx, yy, zz);
				if (tile == Tile::obsidian_Id || tile == Tile::bedrock_Id || tile == Tile::iron_bars_Id || tile == Tile::fire_Id)
				{
					level->setTileAndUpdate(xx, yy, zz, 0);
				}
			}
		}
	}

	vector<shared_ptr<Entity> > *old = level->getEntitiesOfClass(typeid(EnderCrystal), AABB::newTemp(spike.x - r, ground, spike.z - r, spike.x + r + 1, Level::maxBuildHeight, spike.z + r + 1));
	if (old != nullptr)
	{
		for (auto &e : *old)
		{
			bool summoning = false;
			for (auto &c : respawnCrystals)
			{
				if (c == e) summoning = true;
			}
			if (!summoning) e->remove();
		}
		delete old;
	}

	int base = TheEndBiomeDecorator::SPIKE_BASE_Y;
	int top = base + 12 + index * 3;
	level->explode(nullptr, spike.x + 0.5, top, spike.z + 0.5, 5.0f, false);

	Random random;
	SpikeFeature feature(Tile::end_stone_Id);
	feature.placeWithIndex(level, &random, spike.x, base, spike.z, index, spike.radius);

	vector<shared_ptr<Entity> > *placed = level->getEntitiesOfClass(typeid(EnderCrystal), AABB::newTemp(spike.x - 1, ground, spike.z - 1, spike.x + 2, Level::maxBuildHeight, spike.z + 2));
	if (placed != nullptr)
	{
		for (auto &e : *placed)
		{
			shared_ptr<EnderCrystal> crystal = dynamic_pointer_cast<EnderCrystal>(e);
			if (crystal == nullptr) continue;
			crystal->setInvulnerable(true);
			crystal->setBeamTarget(0, 128, 0);
		}
		delete placed;
	}
}

void EndDragonFight::resetSpikeCrystals()
{
	for (int i = 0; i < 8; i++)
	{
		const TheEndBiomeDecorator::SPIKE &spike = TheEndBiomeDecorator::SpikeValA[i];
		vector<shared_ptr<Entity> > *entities = level->getEntitiesOfClass(typeid(EnderCrystal), AABB::newTemp(spike.x - 1, 0, spike.z - 1, spike.x + 2, Level::maxBuildHeight, spike.z + 2));
		if (entities == nullptr) continue;
		for (auto &e : *entities)
		{
			shared_ptr<EnderCrystal> crystal = dynamic_pointer_cast<EnderCrystal>(e);
			if (crystal == nullptr) continue;
			crystal->setInvulnerable(false);
			crystal->clearBeamTarget();
		}
		delete entities;
	}
}

void EndDragonFight::createNewDragon()
{
	shared_ptr<EnderDragon> dragon = std::make_shared<EnderDragon>(level);
	dragon->AddParts();
	dragon->moveTo(0, 128, 0, level->random->nextFloat() * 360, 0);
	level->addEntity(dragon);
}
