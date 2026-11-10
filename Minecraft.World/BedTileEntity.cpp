#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.network.packet.h"
#include "net.minecraft.world.level.h"
#include "BedTileEntity.h"
#include "TileEntityDataPacket.h"

BedTileEntity::BedTileEntity() : TileEntity()
{
	variant = DEFAULT_VARIANT;
}

void BedTileEntity::save(CompoundTag *tag)
{
	TileEntity::save(tag);
	tag->putInt(L"Variant", variant);
}

void BedTileEntity::load(CompoundTag *tag)
{
	TileEntity::load(tag);
	setVariant(tag->getInt(L"Variant"));
}

shared_ptr<Packet> BedTileEntity::getUpdatePacket()
{
	CompoundTag *tag = new CompoundTag();
	save(tag);
	return std::make_shared<TileEntityDataPacket>(x, y, z, TileEntityDataPacket::TYPE_BED, tag);
}

shared_ptr<TileEntity> BedTileEntity::clone()
{
	shared_ptr<BedTileEntity> result = std::make_shared<BedTileEntity>();
	TileEntity::clone(result);
	result->variant = variant;
	return result;
}

int BedTileEntity::getVariantAt(LevelSource *level, int x, int y, int z)
{
	shared_ptr<BedTileEntity> bed = dynamic_pointer_cast<BedTileEntity>(level->getTileEntity(x, y, z));
	return (bed != nullptr) ? bed->getVariant() : -1;
}

void BedTileEntity::setVariantAt(Level *level, int x, int y, int z, int variant)
{
	shared_ptr<BedTileEntity> bed = dynamic_pointer_cast<BedTileEntity>(level->getTileEntity(x, y, z));
	if (bed != nullptr)
	{
		bed->setVariant(variant);
		bed->setChanged();
	}
}
