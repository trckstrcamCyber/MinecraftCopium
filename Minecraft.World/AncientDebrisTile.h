#pragma once

#include "OreTile.h"

class IconRegister;

// neoLegacy - Netherite tier (2026-11-26): ancient debris, a Nether ore with
// a different top texture (like a log). Drops itself (OreTile::getResource
// returns the block id), so the furnace recipe turns it into scrap.
class AncientDebrisTile : public OreTile
{
public:
    explicit AncientDebrisTile(int id);

    virtual Icon* getTexture(int face, int data) override;
    virtual void registerIcons(IconRegister* iconRegister) override;

private:
    Icon* icons[2]; // 0 = side, 1 = top
};
