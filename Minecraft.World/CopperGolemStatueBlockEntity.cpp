#include "stdafx.h"
#include "CopperGolemStatueBlockEntity.h"
#include "CopperGolem.h"
#include "CopperGolemStatueBlock.h"
#include "CompoundTag.h"
#include "net.minecraft.world.level.h"
#include "SoundTypes.h"

CopperGolemStatueBlockEntity::CopperGolemStatueBlockEntity()
{
	name = L"";
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
	// facing.toYRot())).
	int data = level->getData(x, y, z);
	int facing = CopperGolemStatueBlock::getFacing(data);
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
}

void CopperGolemStatueBlockEntity::save(CompoundTag *base)
{
	TileEntity::save(base);
	if (hasCustomName()) base->putString(L"CustomName", name);
}

shared_ptr<TileEntity> CopperGolemStatueBlockEntity::clone()
{
	shared_ptr<CopperGolemStatueBlockEntity> result = std::make_shared<CopperGolemStatueBlockEntity>();
	TileEntity::clone(result);
	result->name = name;
	return result;
}
