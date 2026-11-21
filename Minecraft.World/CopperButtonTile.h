#pragma once

#include "ButtonTile.h"

class IconRegister;

class CopperButtonTile : public ButtonTile
{
public:
	CopperButtonTile(int id);

	virtual Icon *getTexture(int face, int data);
	virtual void registerIcons(IconRegister *iconRegister) override;
};
