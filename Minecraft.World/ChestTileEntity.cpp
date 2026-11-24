#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.level.h"
#include "TileEntity.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.inventory.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "ChestTileEntity.h"
#include "ContainerOpenPacket.h"
#include "SoundTypes.h"
#include "net.minecraft.network.packet.h"
#include "TileEntityDataPacket.h"

int ChestTileEntity::getContainerType()
{
	if (isBonusChest)	return ContainerOpenPacket::BONUS_CHEST;
	else				return ContainerOpenPacket::CONTAINER;
}

void ChestTileEntity::_init(bool isBonusChest)
{
	items = new ItemInstanceArray(9 * 3); // == getContainerSize(); load() also allocates 27 (was 9 * 4)

	hasCheckedNeighbors = false;
	this->isBonusChest = isBonusChest;

	openness = 0.0f;
	oOpenness = 0.0f;
	openCount = 0;
	tickInterval = 0;
	oxidation = 0;

	type = -1;
	name = L"";
}

ChestTileEntity::ChestTileEntity(bool isBonusChest/* = false*/) : TileEntity()
{
	_init(isBonusChest);
}

ChestTileEntity::ChestTileEntity(int type, bool isBonusChest/* = false*/) : TileEntity()
{
	_init(isBonusChest);

	this->type = type;
}

ChestTileEntity::~ChestTileEntity()
{
	delete[] items->data;
	delete items;
}

unsigned int ChestTileEntity::getContainerSize()
{
	return 9 * 3;
}

shared_ptr<ItemInstance> ChestTileEntity::getItem(unsigned int slot)
{
	return items->data[slot];
}

shared_ptr<ItemInstance> ChestTileEntity::removeItem(unsigned int slot, int count)
{
	if (items->data[slot] != nullptr)
	{
		if (items->data[slot]->count <= count)
		{
			shared_ptr<ItemInstance> item = items->data[slot];
			items->data[slot] = nullptr;
			setChanged();
			// 4J Stu - Fix for duplication glitch
			if(item->count <= 0) return nullptr;
			return item;
		}
		else
		{
			shared_ptr<ItemInstance> i = items->data[slot]->remove(count);
			if (items->data[slot]->count == 0) items->data[slot] = nullptr;
			setChanged();
			// 4J Stu - Fix for duplication glitch
			if(i->count <= 0) return nullptr;
			return i;
		}
	}
	return nullptr;
}

shared_ptr<ItemInstance> ChestTileEntity::removeItemNoUpdate(int slot)
{
	if (items->data[slot] != nullptr)
	{
		shared_ptr<ItemInstance> item = items->data[slot];
		items->data[slot] = nullptr;
		return item;
	}
	return nullptr;
}

void ChestTileEntity::setItem(unsigned int slot, shared_ptr<ItemInstance> item)
{
	items->data[slot] = item;
	if (item != nullptr && item->count > getMaxStackSize()) item->count = getMaxStackSize();
	this->setChanged();
}

wstring ChestTileEntity::getName()
{
	return hasCustomName() ? name : app.GetString(IDS_TILE_CHEST);
}

wstring ChestTileEntity::getCustomName()
{
	return hasCustomName() ? name : L"";
}

bool ChestTileEntity::hasCustomName()
{
	return !name.empty();
}

void ChestTileEntity::setCustomName(const wstring &name)
{
	this->name = name;
}

// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). The
// oxidation field lives in this tile entity, not in the block data
// nibble (see the header).
int ChestTileEntity::getOxidation()
{
	return oxidation;
}

void ChestTileEntity::setOxidation(int oxidation)
{
	this->oxidation = oxidation & 7;
	setChanged();
}

int ChestTileEntity::getOxidationStage()
{
	return oxidation & 3;
}

void ChestTileEntity::load(CompoundTag *base)
{
	TileEntity::load(base);
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). Old
	// saves have no "Oxidation" key — default to unaffected (0).
	if (base->contains(L"Oxidation"))
	{
		oxidation = base->getByte(L"Oxidation") & 7;
	}
	ListTag<CompoundTag> *inventoryList = (ListTag<CompoundTag> *) base->getList(L"Items");
	if( items )
	{
		delete [] items->data;
		delete items;
	}
	items = new ItemInstanceArray(getContainerSize());
	if (base->contains(L"CustomName")) name = base->getString(L"CustomName");
	// A tag without an "Items" list (hand-edited / truncated save):
	// getList() returns null — skip the item loop, items stay empty.
	// (ShelfTileEntity::load() returns early here; this keeps loading
	// so the bonus flag below is still read.)
	int inventorySize = inventoryList != nullptr ? inventoryList->size() : 0;
	for (int i = 0; i < inventorySize; i++)
	{
		CompoundTag *tag = inventoryList->get(i);
		unsigned int slot = tag->getByte(L"Slot") & 0xff;
		if (slot < items->length)
		{
			shared_ptr<ItemInstance> loadedItem = ItemInstance::fromTag(tag);
			if (loadedItem == nullptr)
			{
				Tag *idTag = tag->get(L"id");
				int idType = idTag != nullptr ? idTag->getId() : -1;
				
				/*
				if (idType == Tag::TAG_String)
				{
					app.DebugPrintf("[ChestTileEntity] Missing chest item at %d,%d,%d slot=%u idType=%d idStr=%ls count=%d damage=%d\n", x, y, z, slot, idType, tag->getString(L"id").c_str(), tag->getByte(L"Count"), tag->getShort(L"Damage"));
				}
				else if (idType == Tag::TAG_Int)
				{
					app.DebugPrintf("[ChestTileEntity] Missing chest item at %d,%d,%d slot=%u idType=%d id=%d count=%d damage=%d\n", x, y, z, slot, idType, tag->getInt(L"id"), tag->getByte(L"Count"), tag->getShort(L"Damage"));
				}
				else
				{
					app.DebugPrintf("[ChestTileEntity] Missing chest item at %d,%d,%d slot=%u idType=%d id=%d count=%d damage=%d\n", x, y, z, slot, idType, tag->getShort(L"id"), tag->getByte(L"Count"), tag->getShort(L"Damage"));
				}
				*/
			}
			(*items)[slot] = loadedItem;
		}
	}
	isBonusChest = base->getBoolean(L"bonus");
}

void ChestTileEntity::save(CompoundTag *base)
{
	TileEntity::save(base);
	ListTag<CompoundTag> *listTag = new ListTag<CompoundTag>;

	for (unsigned int i = 0; i < items->length; i++)
	{
		if (items->data[i] != nullptr)
		{
			CompoundTag *tag = new CompoundTag();
			tag->putByte(L"Slot", static_cast<byte>(i));
			items->data[i]->save(tag);
			listTag->add(tag);
		}
	}
	base->put(L"Items", listTag);
	if (hasCustomName()) base->putString(L"CustomName", name);
	base->putBoolean(L"bonus", isBonusChest);
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20).
	base->putByte(L"Oxidation", static_cast<byte>(oxidation));
}

int ChestTileEntity::getMaxStackSize() const
{
	return Container::LARGE_MAX_STACK_SIZE;
}

bool ChestTileEntity::stillValid(shared_ptr<Player> player)
{
	if (level->getTileEntity(x, y, z) != shared_from_this() ) return false;
	if (player->distanceToSqr(x + 0.5, y + 0.5, z + 0.5) > 8 * 8) return false;
	return true;
}

void ChestTileEntity::setChanged()
{
	TileEntity::setChanged();
}

void ChestTileEntity::clearCache()
{
	TileEntity::clearCache();
	hasCheckedNeighbors = false;
}

void ChestTileEntity::heyImYourNeighbor(shared_ptr<ChestTileEntity> neighbor, int from)
{
	if (neighbor->isRemoved())
	{
		hasCheckedNeighbors = false;
	}
	else if (hasCheckedNeighbors)
	{
		switch (from)
		{
		case Direction::NORTH:
			if (n.lock() != neighbor) hasCheckedNeighbors = false;
			break;
		case Direction::SOUTH:
			if (s.lock() != neighbor) hasCheckedNeighbors = false;
			break;
		case Direction::EAST:
			if (e.lock() != neighbor) hasCheckedNeighbors = false;
			break;
		case Direction::WEST:
			if (w.lock() != neighbor) hasCheckedNeighbors = false;
			break;
		}
	}
}

void ChestTileEntity::checkNeighbors()
{
	if (hasCheckedNeighbors) return;

	hasCheckedNeighbors = true;
	n = weak_ptr<ChestTileEntity>();
	e = weak_ptr<ChestTileEntity>();
	w = weak_ptr<ChestTileEntity>();
	s = weak_ptr<ChestTileEntity>();

	if (isSameChest(x - 1, y, z))
	{
		w = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(x - 1, y, z));
	}
	if (isSameChest(x + 1, y, z))
	{
		e = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(x + 1, y, z));
	}
	if (isSameChest(x, y, z - 1))
	{
		n = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(x, y, z - 1));
	}
	if (isSameChest(x, y, z + 1))
	{
		s = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(x, y, z + 1));
	}

	shared_ptr<ChestTileEntity> cteThis = dynamic_pointer_cast<ChestTileEntity>(shared_from_this());
	if (n.lock() != nullptr) n.lock()->heyImYourNeighbor(cteThis, Direction::SOUTH);
	if (s.lock() != nullptr) s.lock()->heyImYourNeighbor(cteThis, Direction::NORTH);
	if (e.lock() != nullptr) e.lock()->heyImYourNeighbor(cteThis, Direction::WEST);
	if (w.lock() != nullptr) w.lock()->heyImYourNeighbor(cteThis, Direction::EAST);
}

bool ChestTileEntity::isSameChest(int x, int y, int z)
{
	Tile *tile = Tile::tiles[level->getTile(x, y, z)];
	if (tile == nullptr || !(dynamic_cast<ChestTile *>(tile) != nullptr)) return false;
	return static_cast<ChestTile *>(tile)->type == getType();
}

// neoLegacy - Copper Age (Phase 6). The copper chest's oxidation lives
// in this tile entity (see oxidation), NOT in data bits 3-5 — the 4-bit
// nibble truncated it. Stage = field & 3, so waxed chests (field 4-7)
// sound like their unwaxed stage. UNAFFECTED (0) and EXPOSED (1) share
// the regular set, as with the golem sounds.
int ChestTileEntity::copperChestSound(bool opening)
{
	int stage = oxidation & 3;
	if (stage >= 3) return opening ? eSoundType_COPPER_CHEST_OXIDIZED_OPEN : eSoundType_COPPER_CHEST_OXIDIZED_CLOSE;
	if (stage == 2) return opening ? eSoundType_COPPER_CHEST_WEATHERED_OPEN : eSoundType_COPPER_CHEST_WEATHERED_CLOSE;
	return opening ? eSoundType_COPPER_CHEST_OPEN : eSoundType_COPPER_CHEST_CLOSE;
}
void ChestTileEntity::tick()
{
	TileEntity::tick();
	checkNeighbors();

	++tickInterval;
	if (!level->isClientSide && openCount != 0 && (tickInterval + x + y + z) % (SharedConstants::TICKS_PER_SECOND * 10) == 0)
	{
		//            level.tileEvent(x, y, z, Tile.chest.id, ChestTile.EVENT_SET_OPEN_COUNT, openCount);

		openCount = 0;

		float range = 5;
		vector<shared_ptr<Entity> > *players = level->getEntitiesOfClass(typeid(Player), AABB::newTemp(x - range, y - range, z - range, x + 1 + range, y + 1 + range, z + 1 + range));
		if ( players )
		{
			for (auto& it : *players)
			{
				shared_ptr<Player> player = dynamic_pointer_cast<Player>(it);

				ContainerMenu* containerMenu = dynamic_cast<ContainerMenu*>(player->containerMenu);
				if (containerMenu)
				{
					shared_ptr<Container> container = containerMenu->getContainer();
					shared_ptr<Container> thisContainer = dynamic_pointer_cast<Container>(shared_from_this());
					shared_ptr<CompoundContainer> compoundContainer = dynamic_pointer_cast<CompoundContainer>(container);
					if ((container == thisContainer) || (compoundContainer != nullptr && compoundContainer->contains(thisContainer)))
					{
						openCount++;
					}
				}
			}
			delete players;
		}
	}

	oOpenness = openness;

	float speed = 0.10f;
	if (openCount > 0 && openness == 0)
	{
		if (n.lock() == nullptr && w.lock() == nullptr)
		{
			double xc = x + 0.5;
			double zc = z + 0.5;
			if (s.lock() != nullptr) zc += 0.5;
			if (e.lock() != nullptr) xc += 0.5;

			// 4J-PB - Seems the chest open volume is much louder than other sounds from user reports. We'll tone it down a bit
			// neoLegacy - Copper Age (Phase 3): the copper chest has its own open sound.
			level->playSound(xc, y + 0.5, zc, (getType() == ChestTile::TYPE_COPPER) ? copperChestSound(true) : eSoundType_RANDOM_CHEST_OPEN, 0.2f, level->random->nextFloat() * 0.1f + 0.9f);
		}
	}
	if ((openCount == 0 && openness > 0) || (openCount > 0 && openness < 1))
	{
		float oldOpen = openness;
		if (openCount > 0) openness += speed;
		else openness -= speed;
		if (openness > 1)
		{
			openness = 1;
		}
		float lim = 0.5f;
		if (openness < lim && oldOpen >= lim)
		{
			// Fix for #64546 - Customer Encountered: TU7: Chests placed by the Player are closing too fast.
			//openness = 0;
			if (n.lock() == nullptr && w.lock() == nullptr)
			{
				double xc = x + 0.5;
				double zc = z + 0.5;
				if (s.lock() != nullptr) zc += 0.5;
				if (e.lock() != nullptr) xc += 0.5;

				// 4J-PB - Seems the chest open volume is much louder than other sounds from user reports. We'll tone it down a bit
				// neoLegacy - Copper Age (Phase 3): the copper chest has its own close sound.
				level->playSound(xc, y + 0.5, zc, (getType() == ChestTile::TYPE_COPPER) ? copperChestSound(false) : eSoundType_RANDOM_CHEST_CLOSE, 0.2f, level->random->nextFloat() * 0.1f + 0.9f);
			}
		}
		if (openness < 0)
		{
			openness = 0;
		}
	}

}

bool ChestTileEntity::triggerEvent(int b0, int b1)
{
	if (b0 == ChestTile::EVENT_SET_OPEN_COUNT)
	{
		openCount = b1;
		return true;
	}
	return TileEntity::triggerEvent(b0, b1);
}

void ChestTileEntity::startOpen()
{
	if (openCount < 0)
	{
		openCount = 0;
	}
	openCount++;
	level->tileEvent(x, y, z, getTile()->id, ChestTile::EVENT_SET_OPEN_COUNT, openCount);
	level->updateNeighborsAt(x, y, z, getTile()->id);
	level->updateNeighborsAt(x, y - 1, z, getTile()->id);
}

void ChestTileEntity::stopOpen()
{
	if (getTile() == nullptr || !( dynamic_cast<ChestTile *>( getTile() ) != nullptr)) return;
	openCount--;
	level->tileEvent(x, y, z, getTile()->id, ChestTile::EVENT_SET_OPEN_COUNT, openCount);
	level->updateNeighborsAt(x, y, z, getTile()->id);
	level->updateNeighborsAt(x, y - 1, z, getTile()->id);
}

bool ChestTileEntity::canPlaceItem(int slot, shared_ptr<ItemInstance> item)
{
	return true;
}

void ChestTileEntity::setRemoved()
{
	TileEntity::setRemoved();
	clearCache();
	checkNeighbors();
}

int ChestTileEntity::getType()
{
	if (type == -1)
	{
		if (level != nullptr && dynamic_cast<ChestTile *>( getTile() ) != nullptr)
		{
			type = static_cast<ChestTile *>(getTile())->type;
		}
		else
		{
			return ChestTile::TYPE_BASIC;
		}
	}

	return type;
}

// 4J Added
shared_ptr<TileEntity> ChestTileEntity::clone()
{
	shared_ptr<ChestTileEntity> result = std::make_shared<ChestTileEntity>();
	TileEntity::clone(result);
	// neoLegacy - Copper Age (oxidation workaround, 2026-11-20).
	result->oxidation = oxidation;

	for (unsigned int i = 0; i < items->length; i++)
	{
		if (items->data[i] != nullptr)
		{
			result->items->data[i] = ItemInstance::clone(items->data[i]);
		}
	}
	return result;
}

// neoLegacy - Copper Age (oxidation workaround, 2026-11-20). Live-sync
// the oxidation to the client. Minimal tag: Oxidation only — do NOT call
// save() here, which would send the whole inventory.
shared_ptr<Packet> ChestTileEntity::getUpdatePacket()
{
	CompoundTag *tag = new CompoundTag();
	tag->putByte(L"Oxidation", static_cast<byte>(oxidation));
	return std::make_shared<TileEntityDataPacket>(x, y, z, TileEntityDataPacket::TYPE_COPPER_CHEST, tag);
}

void ChestTileEntity::applyUpdate(CompoundTag *tag)
{
	// Client-side only: apply the oxidation from a TileEntityDataPacket
	// without touching anything else. Do NOT call setChanged() here —
	// the client must not mark its chunk unsaved.
	if (tag != nullptr && tag->contains(L"Oxidation"))
	{
		oxidation = tag->getByte(L"Oxidation") & 7;
	}
}