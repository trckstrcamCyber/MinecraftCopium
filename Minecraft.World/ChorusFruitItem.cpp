#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.h"
#include "ItemInstance.h"
#include "ChorusFruitItem.h"
#include "SoundTypes.h"
#if defined(_WINDOWS64) && defined(MINECRAFT_SERVER_BUILD)
#include "../Minecraft.Server/FourKitBridge.h"
#endif

ChorusFruitItem::ChorusFruitItem(int id, int nutrition, float saturationMod)
	: FoodItem(id, nutrition, saturationMod, false)
{
}

bool ChorusFruitItem::randomTeleport(Level *level, shared_ptr<Player> player, double xx, double yy, double zz)
{
	double xo = player->x;
	double yo = player->y;
	double zo = player->z;

	int xt = Mth::floor(xx);
	int yt = Mth::floor(yy);
	int zt = Mth::floor(zz);
	if (!level->hasChunkAt(xt, yt, zt)) return false;

	bool landed = false;
	while (!landed && yt > 0)
	{
		int t = level->getTile(xt, yt - 1, zt);
		if (t == 0 || !Tile::tiles[t]->material->blocksMotion())
		{
			yy--;
			yt--;
		}
		else
		{
			landed = true;
		}
	}
	if (!landed) return false;

	player->setPos(xx, yy, zz);
	bool ok = level->getCubes(player, player->bb)->empty() && !level->containsAnyLiquid(player->bb);
	player->setPos(xo, yo, zo);
	if (!ok) return false;

#if defined(_WINDOWS64) && defined(MINECRAFT_SERVER_BUILD)
	{
		double outX, outY, outZ;
		bool cancelled = FourKitBridge::FirePlayerTeleport(
			player->entityId,
			xo, yo, zo, player->dimension,
			xx, yy, zz, player->dimension,
			6,
			&outX, &outY, &outZ);
		if (cancelled) return false;
		xx = outX;
		yy = outY;
		zz = outZ;
	}
#endif

	player->teleportTo(xx, yy, zz);
	player->fallDistance = 0;
	level->playSound(xo, yo, zo, eSoundType_MOB_ENDERMEN_PORTAL, 1, 1);
	player->playSound(eSoundType_MOB_ENDERMEN_PORTAL, 1, 1);
	return true;
}

shared_ptr<ItemInstance> ChorusFruitItem::useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	shared_ptr<ItemInstance> result = FoodItem::useTimeDepleted(instance, level, player);

	if (!level->isClientSide)
	{
		for (int attempt = 0; attempt < 16; ++attempt)
		{
			double x = player->x + (player->getRandom()->nextDouble() - 0.5) * 16.0;
			double y = player->y + (player->getRandom()->nextInt(16) - 8);
			if (y < 0.0) y = 0.0;
			if (y > Level::maxBuildHeight - 1) y = Level::maxBuildHeight - 1;
			double z = player->z + (player->getRandom()->nextDouble() - 0.5) * 16.0;

			if (player->isRiding())
			{
				player->ride(nullptr);
			}

			if (randomTeleport(level, player, x, y, z))
			{
				break;
			}
		}
	}

	return result;
}
