#pragma once
#include "HalfSlabTile.h"

class ChunkRebuildData;

class PurpurSlabTile : public HalfSlabTile
{
	friend class ChunkRebuildData;
public:
	PurpurSlabTile(int id);

	virtual int isFullSize() = 0;

	virtual Icon        *getTexture(int face, int data) override;
	virtual void         registerIcons(IconRegister *iconRegister) override;
	virtual int          getResource(int data, Random *random, int playerBonusLevel) override;
	virtual unsigned int getDescriptionId(int iData = -1) override;
	virtual int          getAuxName(int auxValue) override;

protected:
	virtual shared_ptr<ItemInstance> getSilkTouchItemInstance(int data) override;
};

class HalfPurpurSlabTile : public PurpurSlabTile
{
public:
	HalfPurpurSlabTile(int id) : PurpurSlabTile(id) { DerivedInit(); }
	virtual int isFullSize() override { return 0; }
};

class FullPurpurSlabTile : public PurpurSlabTile
{
public:
	FullPurpurSlabTile(int id) : PurpurSlabTile(id) { DerivedInit(); }
	virtual int isFullSize() override { return 1; }
};
