#pragma once

#include "DirectionalTile.h"

// Lightweight LCE grindstone tile.  The custom model renderer in
// Minecraft.Client/TileRenderer.cpp consumes the five texture regions
// exposed by this class.
class GrindstoneTile : public DirectionalTile
{
private:
    Icon *sideIcon;
    Icon *roundIcon;
    Icon *pivotIcon;
    Icon *legIcon;

public:
    GrindstoneTile(int id);

    virtual bool isCubeShaped() override;
    virtual bool isSolidRender(bool isServerLevel = false) override;
    virtual void updateDefaultShape() override;
    virtual int getRenderShape() override;

    virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false) override;

    // DirectionalTile keeps the facing in data bits 0-1; without this the
    // block was always placed with data 0, whichever way the player looked.
    virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance) override;

    virtual Icon *getTexture(int face, int data) override;
    virtual void registerIcons(IconRegister *iconRegister) override;

    Icon *getSideTexture() const;
    Icon *getRoundTexture() const;
    Icon *getPivotTexture() const;
    Icon *getLegTexture() const;
};
