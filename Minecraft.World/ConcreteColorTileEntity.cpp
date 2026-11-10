#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.network.packet.h"
#include "net.minecraft.world.level.h"
#include "ConcreteColorTileEntity.h"
#include "TileEntityDataPacket.h"

ConcreteColorTileEntity::ConcreteColorTileEntity() : TileEntity()
{
	color = 0;
}

void ConcreteColorTileEntity::save(CompoundTag *tag)
{
	TileEntity::save(tag);
	tag->putInt(L"Color", color);
}

void ConcreteColorTileEntity::load(CompoundTag *tag)
{
	TileEntity::load(tag);
	color = tag->getInt(L"Color") & 15;
}

shared_ptr<Packet> ConcreteColorTileEntity::getUpdatePacket()
{
	CompoundTag *tag = new CompoundTag();
	save(tag);
	return std::make_shared<TileEntityDataPacket>(x, y, z, TileEntityDataPacket::TYPE_CONCRETE_COLOR, tag);
}

shared_ptr<TileEntity> ConcreteColorTileEntity::clone()
{
	shared_ptr<ConcreteColorTileEntity> result = std::make_shared<ConcreteColorTileEntity>();
	TileEntity::clone(result);
	result->color = color;
	return result;
}

int ConcreteColorTileEntity::getColorAt(LevelSource *level, int x, int y, int z)
{
	shared_ptr<ConcreteColorTileEntity> cte = dynamic_pointer_cast<ConcreteColorTileEntity>(level->getTileEntity(x, y, z));
	return (cte != nullptr) ? cte->getColor() : -1;
}

void ConcreteColorTileEntity::setColorAt(Level *level, int x, int y, int z, int color)
{
	shared_ptr<ConcreteColorTileEntity> cte = dynamic_pointer_cast<ConcreteColorTileEntity>(level->getTileEntity(x, y, z));
	if (cte != nullptr)
	{
		cte->setColor(color);
		cte->setChanged();
	}
}
