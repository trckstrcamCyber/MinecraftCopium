#pragma once

#include "Tile.h"
#include "ArcadeGameTypes.h"

// A solid, six-way-facing arcade cabinet. The unpowered and powered states use
// separate Tile IDs so light emission is block-state-correct in the TU43 renderer.
class ArcadeMachineTile : public Tile
{
public:
    static const int FACING_MASK = 0x7;

    ArcadeMachineTile(int id, ArcadeGameType gameType, int unpoweredId, int poweredId, bool poweredVariant);

    virtual bool isCubeShaped() override { return true; }
    virtual bool isSolidRender(bool isServerLevel = false) override { return true; }
    virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance) override;
    virtual Icon *getTexture(int face, int data) override;
    virtual void registerIcons(IconRegister *iconRegister) override;
    virtual void onPlace(Level *level, int x, int y, int z) override;
    virtual void neighborChanged(Level *level, int x, int y, int z, int type) override;
    virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false) override;
    virtual int getResource(int data, Random *random, int playerBonusLevel) override;
    virtual int cloneTileId(Level *level, int x, int y, int z) override;
    virtual wstring getTileItemIconName() override;

    static int getFacing(int data) { return data & FACING_MASK; }
    ArcadeGameType getGameType() const { return gameType; }
    bool isPoweredVariant() const { return poweredVariant; }

private:
    void syncPowerState(Level *level, int x, int y, int z);
    ArcadeGameType gameType;
    int unpoweredId;
    int poweredId;
    bool poweredVariant;
    Icon *frontOffIcon;
    Icon *frontOnIcon;
    Icon *backIcon;
    Icon *sideIcon;
    Icon *topIcon;
};
