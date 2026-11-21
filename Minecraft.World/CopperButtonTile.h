#pragma once

#include "ButtonTile.h"

class CopperButtonTile : public ButtonTile
{
public:
	CopperButtonTile(int id);

	virtual Icon *getTexture(int face, int data);
};
