#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.boss.enderdragon.h"
#include "net.minecraft.world.phys.h"
#include "EndCrystalItem.h"
#include "EndDragonFight.h"

EndCrystalItem::EndCrystalItem(int id) : Item(id)
{
}

static bool isClearForCrystal(Level *level, int x, int y, int z)
{
	int t = level->getTile(x, y, z);
	return t == 0 || (Tile::tiles[t] != nullptr && Tile::tiles[t]->material->isReplaceable());
}

bool EndCrystalItem::useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	int t = level->getTile(x, y, z);
	if (t != Tile::obsidian_Id && t != Tile::bedrock_Id) return false;
	if (!player->mayUseItemAt(x, y + 1, z, face, itemInstance)) return false;
	if (!isClearForCrystal(level, x, y + 1, z) || !isClearForCrystal(level, x, y + 2, z)) return false;

	vector<shared_ptr<Entity> > *entities = level->getEntities(nullptr, AABB::newTemp(x, y + 1, z, x + 1.0, y + 3.0, z + 1.0));
	bool blocked = entities != nullptr && !entities->empty();
	if (blocked) return false;

	if (bTestUseOnOnly) return true;

	if (!level->isClientSide)
	{
		shared_ptr<EnderCrystal> crystal = std::make_shared<EnderCrystal>(level, x + 0.5, y + 1.0, z + 0.5);
		crystal->setShowBottom(false);
		level->addEntity(crystal);
		EndDragonFight *fight = EndDragonFight::get(level);
		if (fight != nullptr) fight->tryRespawn();
	}

	if (!player->abilities.instabuild) itemInstance->count--;
	return true;
}

bool EndCrystalItem::isFoil(shared_ptr<ItemInstance> itemInstance)
{
	return true;
}
