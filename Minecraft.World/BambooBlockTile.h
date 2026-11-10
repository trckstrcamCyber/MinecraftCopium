#pragma once
#include "RotatedPillarTile.h"

class IconRegister;

// neoLegacy - Block of Bamboo and Block of Stripped Bamboo. Rotates when placed, exactly like the logs and the hay block
// (RotatedPillarTile: the placed face picks the axis, the "top" texture is drawn on the two ends).
class BambooBlockTile : public RotatedPillarTile
{
private:
	wstring sideName;
	wstring topName;
	Icon *sideIcon;

public:
	BambooBlockTile(int id, const wstring &sideName, const wstring &topName);

protected:
	virtual Icon *getTypeTexture(int type);

public:
	virtual void registerIcons(IconRegister *iconRegister);
};
