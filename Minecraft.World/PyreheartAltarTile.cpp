#include "stdafx.h"
#include "PyreheartAltarTile.h"
#include "HellFireBoss.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"

bool PyreheartAltarTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player,
    int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly)
{
    if (level == nullptr || player == nullptr || soundOnly) return false;
    shared_ptr<ItemInstance> held = player->getSelectedItem();
    if (held == nullptr || held->id != Item::flint_and_steel_Id) return false;
    if (level->isClientSide) return true;
    if (level->countInstanceOfInRange(eTYPE_HELLFIRE, true, 64, x, y, z) > 0) return true;
    int anchors = 0;
    for (int bx = x - 18; bx <= x + 18; ++bx)
        for (int bz = z - 18; bz <= z + 18; ++bz)
            for (int by = y - 8; by <= y + 14; ++by)
                if (level->getTile(bx, by, bz) == Tile::hellfire_anchor_Id) ++anchors;
    if (anchors < 4) return true;
    shared_ptr<HellFireBoss> boss = std::make_shared<HellFireBoss>(level);
    boss->setPos(x + 0.5, y + 1.0, z + 0.5);
    level->addEntity(boss);
    held->hurtAndBreak(1, player);
    return true;
}
