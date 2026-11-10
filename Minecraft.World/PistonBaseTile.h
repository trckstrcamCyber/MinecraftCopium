#pragma once
#include "Tile.h"
#include <vector>

struct BlockPos3
{
	int x, y, z;
	bool operator==(const BlockPos3 &o) const { return x == o.x && y == o.y && z == o.z; }
};

class PistonBaseTile : public Tile
{
public:
	static const int EXTENDED_BIT = 8;
	static const int UNDEFINED_FACING = 7;

	static const float PLATFORM_THICKNESS;
	static const int MAX_PUSH_DEPTH = 12;
	static const int TRIGGER_EXTEND = 0;
	static const int TRIGGER_CONTRACT = 1;
	static const int TRIGGER_DROP = 2;		// neoLegacy - Java: retract WITHOUT pulling the block back

	static const wstring EDGE_TEX;
	static const wstring PLATFORM_TEX;
	static const wstring PLATFORM_STICKY_TEX;
	static const wstring BACK_TEX;
	static const wstring INSIDE_TEX;

private:
	bool isSticky;

	Icon *iconInside;
	Icon *iconBack;
	Icon *iconPlatform;

	static DWORD tlsIdx;
	//code removed so the above comment no longer applies

public:
	PistonBaseTile(int id, bool isSticky);
	virtual void createBlockStateDefinition() override;
	virtual int defaultBlockState() override;
	virtual int convertBlockStateToLegacyData(BlockState *state) override;
	virtual Tile::BlockState getBlockState(int data);
	virtual Tile::BlockState getBlockState(LevelSource *level, int x, int y, int z) override;

	Icon *getPlatformTexture();
	virtual void updateShape(float x0, float y0, float z0, float x1, float y1, float z1);

	virtual Icon *getTexture(int face, int data);
	static Icon *getTexture(const wstring &name);
	void registerIcons(IconRegister *iconRegister);

	virtual int getRenderShape();
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual void onPlace(Level *level, int x, int y, int z);

private:
	void checkIfExtend(Level *level, int x, int y, int z);
	bool getNeighborSignal(Level *level, int x, int y, int z, int facing);

public:
	virtual bool triggerEvent(Level *level, int x, int y, int z, int param1, int facing);
	virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
	virtual void updateDefaultShape();
	virtual void addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source);
	virtual AABB *getAABB(Level *level, int x, int y, int z);
	virtual bool isCubeShaped();

	static int getFacing(int data);
	static bool isExtended(int data);
	static int getNewFacing(Level *level, int x, int y, int z, shared_ptr<LivingEntity> player);
	// neoLegacy - Java PistonBaseBlock.isPushable(state, level, pos, direction, allowDestroyable, connectionDirection)
	static bool isPushable(Level *level, int x, int y, int z, int direction, bool allowDestroyable, int connectionDirection);
private:
	static bool isPushable(int block, Level *level, int cx, int cy, int cz, bool allowDestroyable);
	static void stopSharingIfServer(Level *level, int x, int y, int z);			// 4J added

	// neoLegacy - Java PistonBaseBlock.moveBlocks(level, pistonPos, direction, extending)
	bool moveBlocks(Level *level, int x, int y, int z, int direction, bool extending);
};
