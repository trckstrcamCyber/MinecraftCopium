#include "stdafx.h"
#include "EndGatewayTile.h"
#include "TheEndGatewayTileEntity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.h"

EndGatewayTile::EndGatewayTile(int id, Material *material) : BaseEntityTile(id, material, isSolidRender())
{
	setLightEmission(1.0f);
}

shared_ptr<TileEntity> EndGatewayTile::newTileEntity(Level *level)
{
	return std::make_shared<TheEndGatewayTileEntity>();
}

bool EndGatewayTile::shouldRenderFace(LevelSource *level, int x, int y, int z, int face)
{
	int tile = level->getTile(x, y, z);
	return tile != id && !Tile::isSolidBlockingTile(tile);
}

void EndGatewayTile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
}

bool EndGatewayTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool EndGatewayTile::isCubeShaped()
{
	return false;
}

int EndGatewayTile::getResourceCount(Random *random)
{
	return 0;
}

void EndGatewayTile::animateTick(Level *level, int xt, int yt, int zt, Random *random)
{
	shared_ptr<TheEndGatewayTileEntity> gateway = dynamic_pointer_cast<TheEndGatewayTileEntity>(level->getTileEntity(xt, yt, zt));
	if (gateway == nullptr) return;

	int amount = gateway->getParticleAmount();
	for (int i = 0; i < amount; i++)
	{
		double x = xt + random->nextFloat();
		double y = yt + random->nextFloat();
		double z = zt + random->nextFloat();
		double xa = (random->nextFloat() - 0.5) * 0.5;
		double ya = (random->nextFloat() - 0.5) * 0.5;
		double za = (random->nextFloat() - 0.5) * 0.5;
		int sign = random->nextInt(2) * 2 - 1;
		if (random->nextBoolean())
		{
			z = zt + 0.5 + 0.25 * sign;
			za = random->nextFloat() * 2.0f * sign;
		}
		else
		{
			x = xt + 0.5 + 0.25 * sign;
			xa = random->nextFloat() * 2.0f * sign;
		}
		level->addParticle(eParticleType_ender, x, y, z, xa, ya, za);
	}
}

int EndGatewayTile::getRenderShape()
{
	return SHAPE_INVISIBLE;
}

int EndGatewayTile::cloneTileId(Level *level, int x, int y, int z)
{
	return 0;
}

void EndGatewayTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"portal");
}
