#include "stdafx.h"
#include "CopperGolemStatueBlockEntity.h"
#include "CopperGolem.h"
#include "CopperGolemStatueBlock.h"
#include "CompoundTag.h"
#include "TileEntityDataPacket.h"
#include "net.minecraft.world.level.h"
#include "SoundTypes.h"

CopperGolemStatueBlockEntity::CopperGolemStatueBlockEntity()
{
	name = L"";
	pose = CopperGolemStatueBlock::POSE_STANDING;
	facing = CopperGolemStatueBlock::FACING_NORTH;
}

int CopperGolemStatueBlockEntity::getPose()
{
	return pose;
}

void CopperGolemStatueBlockEntity::setPose(int pose)
{
	this->pose = pose & 3;
	setChanged();
}

int CopperGolemStatueBlockEntity::getFacing()
{
	return facing;
}

void CopperGolemStatueBlockEntity::setFacing(int facing)
{
	this->facing = facing & 3;
	setChanged();
}

void CopperGolemStatueBlockEntity::createStatue(shared_ptr<CopperGolem> golem)
{
	if (golem != nullptr && golem->hasCustomName())
	{
		setCustomName(golem->getCustomName());
	}
	setChanged();
}

shared_ptr<CopperGolem> CopperGolemStatueBlockEntity::removeStatue()
{
	if (level == nullptr) return nullptr;

	shared_ptr<CopperGolem> golem = std::make_shared<CopperGolem>(level);
	if (golem == nullptr) return nullptr;

	// Custom name carries over from the statue (Java removeStatue).
	if (hasCustomName())
	{
		golem->setCustomName(name);
	}

	// Stand at the centre of the statue's block, facing the statue's
	// facing (Java initCopperGolem: snapTo(atCenterOf(blockPos),
	// facing.toYRot())). The facing lives on this TE now, not in the
	// nibble (oxidation workaround).
	golem->moveTo(x + 0.5, y, z + 0.5, CopperGolemStatueBlock::facingToYaw(facing), 0.0f);
	golem->playSound(eSoundType_MOB_COPPER_GOLEM_SPAWN, 1.0f, 1.0f);
	return golem;
}

wstring CopperGolemStatueBlockEntity::getCustomName()
{
	return name;
}

bool CopperGolemStatueBlockEntity::hasCustomName()
{
	return !name.empty();
}

void CopperGolemStatueBlockEntity::setCustomName(const wstring &name)
{
	this->name = name;
}

void CopperGolemStatueBlockEntity::load(CompoundTag *base)
{
	TileEntity::load(base);
	if (base->contains(L"CustomName")) name = base->getString(L"CustomName");
	// Old saves have no "Pose"/"Facing" keys — keep the STANDING/NORTH
	// defaults rather than failing. (The pose/facing an old statue had
	// in-session is already gone: the nibble was truncated to 15 when it
	// was written, so it was never on disk.)
	if (base->contains(L"Pose")) pose = base->getByte(L"Pose") & 3;
	if (base->contains(L"Facing")) facing = base->getByte(L"Facing") & 3;
}

void CopperGolemStatueBlockEntity::save(CompoundTag *base)
{
	TileEntity::save(base);
	if (hasCustomName()) base->putString(L"CustomName", name);
	base->putByte(L"Pose", static_cast<byte>(pose));
	base->putByte(L"Facing", static_cast<byte>(facing));
}

shared_ptr<TileEntity> CopperGolemStatueBlockEntity::clone()
{
	shared_ptr<CopperGolemStatueBlockEntity> result = std::make_shared<CopperGolemStatueBlockEntity>();
	TileEntity::clone(result);
	result->name = name;
	result->pose = pose;
	result->facing = facing;
	return result;
}

shared_ptr<Packet> CopperGolemStatueBlockEntity::getUpdatePacket()
{
	// Minimal tag: pose + facing only. Oxidation lives in the nibble and
	// the custom name is not part of the live sync.
	CompoundTag *tag = new CompoundTag();
	tag->putByte(L"Pose", static_cast<byte>(pose));
	tag->putByte(L"Facing", static_cast<byte>(facing));
	return std::make_shared<TileEntityDataPacket>(x, y, z, TileEntityDataPacket::TYPE_COPPER_STATUE, tag);
}

void CopperGolemStatueBlockEntity::applyUpdate(CompoundTag *tag)
{
	// Client-side only: apply pose/facing from a TileEntityDataPacket
	// without touching anything else. Do NOT call setChanged() here —
	// the client must not mark its chunk unsaved or re-read the nibble.
	if (tag != nullptr)
	{
		if (tag->contains(L"Pose")) pose = tag->getByte(L"Pose") & 3;
		if (tag->contains(L"Facing")) facing = tag->getByte(L"Facing") & 3;
	}
}
