#include "stdafx.h"
#include "CopperGolemStatueBlock.h"
#include "CopperGolemStatueBlockEntity.h"
#include "HatchetItem.h"
#include "IconRegister.h"
#include "net.minecraft.world.level.h"
#include "SoundTypes.h"

int CopperGolemStatueBlock::makeData(int oxidation, int pose, int facing)
{
	return (oxidation & 3) | ((pose & 3) << 3) | ((facing & 3) << 5);
}

int CopperGolemStatueBlock::getOxidation(int data)
{
	return data & 3;
}

int CopperGolemStatueBlock::getPose(int data)
{
	return (data >> 3) & 3;
}

int CopperGolemStatueBlock::getFacing(int data)
{
	return (data >> 5) & 3;
}

float CopperGolemStatueBlock::facingToYaw(int facing)
{
	// Minecraft yaw: 0 = south, 90 = west, 180 = north, 270 = east.
	switch (facing & 3)
	{
	case FACING_NORTH:
		return 180.0f;
	case FACING_EAST:
		return 270.0f;
	case FACING_WEST:
		return 90.0f;
	default:
		return 0.0f;
	}
}

CopperGolemStatueBlock::CopperGolemStatueBlock(int id) : BaseEntityTile(id, Material::metal)
{
	for (int i = 0; i < 4; i++) stateIcons[i] = nullptr;
}

Icon *CopperGolemStatueBlock::getTexture(int face, int data)
{
	// Waxed (bit 2) reuses the unwaxed texture for the same oxidation state.
	return stateIcons[data & 3];
}

Icon *CopperGolemStatueBlock::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	return stateIcons[level->getData(x, y, z) & 3];
}

void CopperGolemStatueBlock::registerIcons(IconRegister *iconRegister)
{
	// CopperTile pattern (4 oxidation icons from the base name), with the
	// IconRegister definition included directly — CopperTile.cpp itself
	// was missing this include and failed to compile (Phase 3 fix
	// aa7b5fc6); do not repeat that here.
	static const wchar_t *prefixes[4] = { L"", L"exposed_", L"weathered_", L"oxidized_" };
	for (int i = 0; i < 4; i++)
	{
		wstring name = wstring(prefixes[i]) + L"copper_golem_statue";
		stateIcons[i] = iconRegister->registerIcon(name);
	}
	// Keep the base Tile::icon on the unaffected state so anything that reads
	// the icon field directly (particles, pick-block) gets a valid icon.
	icon = stateIcons[0];
}

shared_ptr<TileEntity> CopperGolemStatueBlock::newTileEntity(Level *level)
{
	return std::make_shared<CopperGolemStatueBlockEntity>();
}

void CopperGolemStatueBlock::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	// Java Block.column(10, 0, 14): a 10/16-wide, 14/16-tall column.
	setShape(3 / 16.0f, 0, 3 / 16.0f, 13 / 16.0f, 14 / 16.0f, 13 / 16.0f);
}

void CopperGolemStatueBlock::updateDefaultShape()
{
	setShape(3 / 16.0f, 0, 3 / 16.0f, 13 / 16.0f, 14 / 16.0f, 13 / 16.0f);
}

AABB *CopperGolemStatueBlock::getTileAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z, -1, shared_ptr<TileEntity>());
	return Tile::getTileAABB(level, x, y, z);
}

AABB *CopperGolemStatueBlock::getAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z, -1, shared_ptr<TileEntity>());
	return Tile::getAABB(level, x, y, z);
}

void CopperGolemStatueBlock::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	// The statue has no item form, so nothing places it by hand — only
	// turnToStatue (via setTileAndData, which does not call setPlacedBy)
	// creates one. Implemented defensively: facing from the placer's yaw,
	// pose STANDING, oxidation bits preserved.
	int data = level->getData(x, y, z);
	int facing = (Mth::floor(by->yRot * 4.0f / 360.0f + 0.5f)) & 3;
	level->setData(x, y, z, makeData(getOxidation(data), POSE_STANDING, facing), Tile::UPDATE_ALL);
}

bool CopperGolemStatueBlock::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/)
{
	if (soundOnly)
	{
		// DoorTile pattern: just play the sound, change nothing.
		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_COPPER_STATUE_BECOME_STATUE, 1.0f, 1.0f);
		return false;
	}

	// Axe on a statue: PASS (Java updatePose returns PASS for axes — the
	// axe's wax-scrape / de-oxidize applies to golems and to waxed blocks
	// via the Phase 4 recipes, not to a statue's pose).
	shared_ptr<ItemInstance> item = player->inventory->getSelected();
	if (item != nullptr && dynamic_cast<HatchetItem *>(item->getItem()) != nullptr)
	{
		return false;
	}

	// Otherwise: cycle the pose (STANDING -> SITTING -> RUNNING -> STAR ->
	// STANDING; Java getNextPose wraps to ZERO on out-of-bounds) and play
	// the become-statue sound.
	int data = level->getData(x, y, z);
	int nextPose = (getPose(data) + 1) & 3;
	int newData = makeData(getOxidation(data), nextPose, getFacing(data));
	level->setData(x, y, z, newData, Tile::UPDATE_ALL);
	level->setTilesDirty(x, y, z, x, y, z);
	level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_COPPER_STATUE_BECOME_STATUE, 1.0f, 1.0f);
	return true;
}

bool CopperGolemStatueBlock::hasAnalogOutputSignal()
{
	return true;
}

int CopperGolemStatueBlock::getAnalogOutputSignal(Level *level, int x, int y, int z, int dir)
{
	// Java: comparator output = pose ordinal + 1.
	return getPose(level->getData(x, y, z)) + 1;
}

int CopperGolemStatueBlock::getResourceCount(Random *random)
{
	// Conservative statue-trigger decision: breaking a statue drops
	// nothing and does NOT spawn a golem. (The block entity's removeStatue
	// can build the golem back, but no trigger is wired — open decision
	// for Cameron.)
	return 0;
}
