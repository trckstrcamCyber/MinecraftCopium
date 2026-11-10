#include "stdafx.h"
#include "GrindstoneTile.h"
#include "Facing.h"          // changed
#include "IconRegister.h"
#include "Player.h"
#include "net.minecraft.world.level.h"
#include "LivingEntity.h"
#include "Mth.h"

GrindstoneTile::GrindstoneTile(int id)
    : DirectionalTile(id, Material::stone, false)
{
    icon = nullptr;
    sideIcon = roundIcon = pivotIcon = legIcon = nullptr;
    setDestroyTime(2.0f);
    setExplodeable(6.0f);
    setSoundType(Tile::SOUND_STONE);
    // Real grindstones don't block light -- without this the block is as
    // opaque to skylight as solid stone, which (combined with the flat
    // face-shading already in the renderer) can read as "the block is
    // just sitting in its own shadow" from most angles. Matches AnvilTile.
    setLightBlock(0);
    updateDefaultShape();
}

bool GrindstoneTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly)
{
    if (level->isClientSide)
    {
        return true;
    }
    app.DebugPrintf("[GRINDSTONE DIAG] 1/6 GrindstoneTile::use() -- calling startGrindstoneRepairing\n");
    player->startGrindstoneRepairing(x, y, z);
    return true;
}

void GrindstoneTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
    if (by == nullptr) return;

    // 0=south, 1=west, 2=north, 3=east -- the same numbering as the
    // Direction:: constants DirectionalTile stores. Like Java Edition, the
    // grindstone faces the way the player was looking when they placed it;
    // TileRenderer treats Direction::NORTH as the un-rotated base model.
    int dir = Mth::floor(by->yRot * 4 / 360.0f + 0.5f) & 3;
    level->setData(x, y, z, dir, Tile::UPDATE_CLIENTS);
}

bool GrindstoneTile::isCubeShaped()
{
    return false;
}

bool GrindstoneTile::isSolidRender(bool isServerLevel)
{
    return false;
}

int GrindstoneTile::getRenderShape()
{
    return SHAPE_GRINDSTONE;
}

void GrindstoneTile::updateDefaultShape()
{
    // Full model envelope.  The renderer uses the five JSON cuboids.
    setShape(
        2.0f / 16.0f, 0.0f, 2.0f / 16.0f,
        14.0f / 16.0f, 1.0f, 14.0f / 16.0f);
}

Icon *GrindstoneTile::getTexture(int face, int data)
{
    // Fallback texture selection for particles/item rendering.
    // The custom world renderer selects the exact JSON face textures.
    if (face == Facing::NORTH || face == Facing::SOUTH ||
        face == Facing::UP || face == Facing::DOWN)
        return roundIcon;
    return sideIcon;
}

void GrindstoneTile::registerIcons(IconRegister *iconRegister)
{
    sideIcon = iconRegister->registerIcon(L"grindstone_side");
    roundIcon = iconRegister->registerIcon(L"grindstone_round");
    pivotIcon = iconRegister->registerIcon(L"grindstone_pivot");
    legIcon = iconRegister->registerIcon(L"dark_oak_log");

    icon = sideIcon;
}

Icon *GrindstoneTile::getSideTexture() const
{
    return sideIcon;
}

Icon *GrindstoneTile::getRoundTexture() const
{
    return roundIcon;
}

Icon *GrindstoneTile::getPivotTexture() const
{
    return pivotIcon;
}

Icon *GrindstoneTile::getLegTexture() const
{
    return legIcon;
}