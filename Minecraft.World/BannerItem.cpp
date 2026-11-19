#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "ItemInstance.h"
#include "BannerItem.h"
#include "BannerTileEntity.h"
#include "Mth.h"
#include "com.mojang.nbt.h"
#include "BannerPatterns.h"

const unsigned int BannerItem::COLOR_DESCS[16] =
{
	IDS_TILE_BANNER_BLACK,
	IDS_TILE_BANNER_RED,
	IDS_TILE_BANNER_GREEN,
	IDS_TILE_BANNER_BROWN,
	IDS_TILE_BANNER_BLUE,
	IDS_TILE_BANNER_PURPLE,
	IDS_TILE_BANNER_CYAN,
	IDS_TILE_BANNER_SILVER,
	IDS_TILE_BANNER_GRAY,
	IDS_TILE_BANNER_PINK,
	IDS_TILE_BANNER_LIME,
	IDS_TILE_BANNER_YELLOW,
	IDS_TILE_BANNER_LIGHT_BLUE,
	IDS_TILE_BANNER_MAGENTA,
	IDS_TILE_BANNER_ORANGE,
	IDS_TILE_BANNER_WHITE,
};

BannerItem::BannerItem(int id) : Item(id)
{
	maxStackSize = 16;
	setStackedByData(true);
	setMaxDamage(0);
}

unsigned int BannerItem::getDescriptionId(shared_ptr<ItemInstance> instance)
{
	int color = instance->getAuxValue() & 15;
	return COLOR_DESCS[color];
}

bool BannerItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level,
	int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	if (face == 0) return false;
	if (!level->getMaterial(x, y, z)->isSolid()) return false;

	if (face == 1) y++;
	if (face == 2) z--;
	if (face == 3) z++;
	if (face == 4) x--;
	if (face == 5) x++;

	if (!player->mayUseItemAt(x, y, z, face, instance)) return false;

	if (!bTestUseOnOnly)
	{
		if (face == 1)
		{
			int rot = Mth::floor(((player->yRot + 180.0f) * 16.0f) / 360.0f + 0.5f) & 15;
			level->setTileAndData(x, y, z, Tile::standing_banner_Id, rot, Tile::UPDATE_CLIENTS);
		}
		else
		{
			level->setTileAndData(x, y, z, Tile::wall_banner_Id, face, Tile::UPDATE_CLIENTS);
		}

		shared_ptr<BannerTileEntity> bte = dynamic_pointer_cast<BannerTileEntity>(level->getTileEntity(x, y, z));
		if (bte != nullptr)
		{
			int color = instance->getAuxValue() & 15;
			bte->setBaseColor(color);
			vector<BannerPattern> patterns = getPatterns(instance);	// the placed banner keeps its patterns
			for (const BannerPattern &p : patterns) bte->addPattern(p.pattern, p.color);
			bte->setChanged();
		}

		instance->count--;
	}
	return true;
}

// ---- patterns ---------------------------------------------------------------------------------------------------------------

vector<BannerPattern> BannerItem::getPatterns(shared_ptr<ItemInstance> item)
{
	vector<BannerPattern> out;
	if (item == nullptr || !item->hasTag()) return out;

	CompoundTag *tag = item->getTag();
	if (!tag->contains(L"BlockEntityTag")) return out;
	CompoundTag *entityTag = tag->getCompound(L"BlockEntityTag");
	if (entityTag == nullptr || !entityTag->contains(L"Patterns")) return out;

	ListTag<CompoundTag> *list = static_cast<ListTag<CompoundTag> *>(static_cast<void *>(entityTag->getList(L"Patterns")));
	if (list == nullptr) return out;

	for (int i = 0; i < list->size() && i < BannerPatterns::MAX_LAYERS; i++)
	{
		CompoundTag *entry = list->get(i);
		BannerPattern bp;
		bp.pattern = entry->getString(L"Pattern");
		bp.color = entry->getInt(L"Color");
		out.push_back(bp);
	}
	return out;
}

void BannerItem::setPatterns(shared_ptr<ItemInstance> item, const vector<BannerPattern> &patterns)
{
	if (item == nullptr || patterns.empty()) return;

	if (!item->hasTag()) item->setTag(new CompoundTag());

	CompoundTag *entityTag = new CompoundTag();
	entityTag->putInt(L"Base", item->getAuxValue() & 15);
	ListTag<CompoundTag> *list = new ListTag<CompoundTag>();
	for (const BannerPattern &p : patterns)
	{
		CompoundTag *entry = new CompoundTag();
		entry->putString(L"Pattern", p.pattern);
		entry->putInt(L"Color", p.color);
		list->add(entry);
	}
	entityTag->put(L"Patterns", list);
	item->getTag()->putCompound(L"BlockEntityTag", entityTag);
}

void BannerItem::addPattern(shared_ptr<ItemInstance> item, const wstring &code, int color)
{
	vector<BannerPattern> patterns = getPatterns(item);
	BannerPattern bp;
	bp.pattern = code;
	bp.color = color & 15;
	patterns.push_back(bp);
	setPatterns(item, patterns);
}

vector<char> BannerItem::getPatternIndexs(shared_ptr<ItemInstance> item)
{
	vector<char> out;
	for (const BannerPattern &bp : getPatterns(item))
	{
		out.push_back(static_cast<char>(BannerPatterns::indexOf(bp.pattern)));
	}
	return out;
}

vector<char> BannerItem::getPatternColors(shared_ptr<ItemInstance> item)
{
	vector<char> out;
	for (const BannerPattern &bp : getPatterns(item))
	{
		out.push_back(static_cast<char>(bp.color));
	}
	return out;
}

// Java: a white banner with rhombus (cyan), bottom stripe (light grey), centre stripe (grey), border (light grey), middle stripe (black),
// half horizontal (light grey), circle (light grey) and border (black). Colours here are dye numbers: 0 black, 6 cyan, 7 light grey, 8 grey.
shared_ptr<ItemInstance> BannerItem::createOminousBanner()
{
	shared_ptr<ItemInstance> banner = std::make_shared<ItemInstance>(Tile::standing_banner_Id, 1, 15);

	static const struct { const wchar_t *code; int color; } layers[8] = {
		{ L"mr", 6 }, { L"bs", 7 }, { L"cs", 8 }, { L"bo", 7 }, { L"ms", 0 }, { L"hh", 7 }, { L"mc", 7 }, { L"bo", 0 }
	};
	vector<BannerPattern> patterns;
	for (int i = 0; i < 8; i++)
	{
		BannerPattern bp;
		bp.pattern = layers[i].code;
		bp.color = layers[i].color;
		patterns.push_back(bp);
	}
	setPatterns(banner, patterns);
	banner->setHoverName(L"Ominous Banner");
	return banner;
}
