#include "stdafx.h"
#include "ArcadeMachineTile.h"
#include "ItemInstance.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.material.h"
#include "Facing.h"
#include "IconRegister.h"
#include "Level.h"
#include "PistonBaseTile.h"
#include "Player.h"

ArcadeMachineTile::ArcadeMachineTile(int id, ArcadeGameType gameType, int unpoweredId, int poweredId, bool poweredVariant)
    : Tile(id, Material::stone, true), gameType(gameType), unpoweredId(unpoweredId), poweredId(poweredId), poweredVariant(poweredVariant)
{
    frontOffIcon = nullptr;
    frontOnIcon = nullptr;
    backIcon = nullptr;
    sideIcon = nullptr;
    topIcon = nullptr;
    setDestroyTime(3.5f);
    setExplodeable(6.0f);
    setSoundType(Tile::SOUND_STONE);
}

void ArcadeMachineTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
    int facing = Facing::getOpposite(PistonBaseTile::getNewFacing(level, x, y, z, by));
    level->setData(x, y, z, facing & FACING_MASK, Tile::UPDATE_ALL);
    syncPowerState(level, x, y, z);
}

Icon *ArcadeMachineTile::getTexture(int face, int data)
{
    int facing = getFacing(data);
    int opposite = Facing::getOpposite(facing);
    if (face == facing) return poweredVariant ? frontOnIcon : frontOffIcon;
    if (face == opposite) return backIcon;
    bool horizontal = (facing != Facing::UP && facing != Facing::DOWN);
    if (horizontal && (face == Facing::UP || face == Facing::DOWN)) return topIcon;
    return sideIcon;
}

void ArcadeMachineTile::registerIcons(IconRegister *iconRegister)
{
    frontOffIcon = iconRegister->registerIcon(L"game_front_off");
    if (gameType == ARCADE_GAME_MINE_SNAKE) frontOnIcon = iconRegister->registerIcon(L"gameMS_front_on");
    else if (gameType == ARCADE_GAME_FLAPPY_CRAFT) frontOnIcon = iconRegister->registerIcon(L"gameFC_front_on");
    else frontOnIcon = iconRegister->registerIcon(L"gameCM_front_on");
    backIcon = iconRegister->registerIcon(L"observer_back");
    sideIcon = iconRegister->registerIcon(L"observer_side");
    topIcon = iconRegister->registerIcon(L"observer_side");
    icon = sideIcon;
}

void ArcadeMachineTile::syncPowerState(Level *level, int x, int y, int z)
{
    if (level == nullptr) return;
    int existing = level->getTile(x, y, z);
    if (existing != unpoweredId && existing != poweredId) return;
    bool powered = level->hasNeighborSignal(x, y, z);
    int wanted = powered ? poweredId : unpoweredId;
    if (existing != wanted)
    {
        // Preserve orientation while swapping only the block ID. UPDATE_ALL refreshes
        // lighting, neighbor notifications, and the powered front texture.
        level->setTileAndData(x, y, z, wanted, level->getData(x, y, z) & FACING_MASK, Tile::UPDATE_ALL);
    }
}

void ArcadeMachineTile::onPlace(Level *level, int x, int y, int z)
{
    syncPowerState(level, x, y, z);
}

void ArcadeMachineTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
    syncPowerState(level, x, y, z);
}

bool ArcadeMachineTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly)
{
    if (soundOnly) return false;
    if (player == nullptr) return false;
    // Player's default implementation is a no-op on server-side Player objects;
    // the real game session is created only by the local client player.
    return player->startArcadeGame(x, y, z, static_cast<int>(gameType));
}

int ArcadeMachineTile::getResource(int data, Random *random, int playerBonusLevel)
{
    // Both visual/redstone states always drop the single craftable cabinet item.
    return unpoweredId;
}

int ArcadeMachineTile::cloneTileId(Level *level, int x, int y, int z)
{
    return unpoweredId;
}

wstring ArcadeMachineTile::getTileItemIconName()
{
    // Use each game's powered face for the single inventory item so players can
    // distinguish the three cabinets before placing them.
    if (gameType == ARCADE_GAME_MINE_SNAKE) return L"gameMS_front_on";
    if (gameType == ARCADE_GAME_FLAPPY_CRAFT) return L"gameFC_front_on";
    return L"gameCM_front_on";
}
