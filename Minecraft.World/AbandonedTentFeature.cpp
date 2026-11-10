#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.item.h"
#include "WeighedTreasure.h"
#include "CompoundTag.h"
#include "ListTag.h"
#include "StringTag.h"
#include <vector>
#include <utility>
#include "AbandonedTentFeature.h"
#include "LuckyStructureConfig.h"
#include "WorldGenSafetyChecks.h"

AbandonedTentFeature::AbandonedTentFeature() : Feature(false)
{
}

static WeighedTreasureArray earlyGameTentTreasure()
{
	static WeighedTreasureArray table;
	static bool initialized = false;
	if (!initialized)
	{
		table = WeighedTreasureArray(7);
		table[0] = new WeighedTreasure(Item::bread_Id, 0, 1, 3, 10);
		table[1] = new WeighedTreasure(Item::wooden_sword_Id, 0, 1, 1, 4);
		table[2] = new WeighedTreasure(Item::wooden_pickaxe_Id, 0, 1, 1, 4);
		table[3] = new WeighedTreasure(Item::wooden_axe_Id, 0, 1, 1, 3);
		table[4] = new WeighedTreasure(Tile::torch_Id, 0, 2, 5, 8);
		table[5] = new WeighedTreasure(Item::leather_Id, 0, 1, 2, 5);
		table[6] = new WeighedTreasure(Item::stick_Id, 0, 1, 3, 6);
		initialized = true;
	}
	return table;
}

static shared_ptr<ItemInstance> makeTravelersNote(Random *random)
{
	static const wchar_t *lines[] = {
		L"If you're reading this, I made it past the ridge. Heading north at first light.",
		L"Lost the trail near here. Left what I could spare - take it, whoever finds this.",
		L"Three nights out and still no sign of the others. The woods are quieter than I'd like.",
		L"Good water two days east of here. Bad memories the same direction. Your choice.",
		L"Packed light, traveled far. Should have packed a second pair of boots.",
	};
	int count = 5;

	shared_ptr<ItemInstance> note = std::make_shared<ItemInstance>(Item::paper, 1);
	note->setHoverName(L"Traveler's Note");

	if (!note->hasTag()) note->setTag(new CompoundTag());
	CompoundTag *tag = note->getTag();
	if (!tag->contains(L"display")) tag->putCompound(L"display", new CompoundTag());
	CompoundTag *display = tag->getCompound(L"display");

	ListTag<StringTag> *lore = new ListTag<StringTag>(L"Lore");
	lore->add(new StringTag(L"", lines[random->nextInt(count)]));
	display->put(L"Lore", lore);

	return note;
}

void AbandonedTentFeature::fillChest(Level *level, Random *random, int x, int y, int z)
{
	shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
	shared_ptr<ChestTileEntity> chest = dynamic_pointer_cast<ChestTileEntity>(te);
	if (chest == nullptr) return;

	WeighedTreasure::addChestItems(random, earlyGameTentTreasure(), chest, 2 + random->nextInt(2));

	if (random->nextInt(LuckyStructureConfig::tentTravelersNoteChance) == 0)
	{
		for (unsigned int slot = 0; slot < chest->getContainerSize(); slot++)
		{
			if (chest->getItem(slot) == nullptr)
			{
				chest->setItem(slot, makeTravelersNote(random));
				break;
			}
		}
	}
}

void AbandonedTentFeature::buildTent(Level *level, Random *random, int cx, int groundY, int cz, int size, int facing)
{
	int xMin = cx - (size / 2);
	int xMax = xMin + size - 1;
	int zMin = cz - (size / 2);
	int zMax = zMin + size - 1;
	int midX = (xMin + xMax) / 2;
	int midZ = (zMin + zMax) / 2;

	int entranceX = midX, entranceZ = midZ;
	switch (facing)
	{
	case 0: entranceX = xMax; entranceZ = midZ; break; // opens +X
	case 1: entranceX = xMin; entranceZ = midZ; break; // opens -X
	case 2: entranceX = midX; entranceZ = zMax; break; // opens +Z
	default: entranceX = midX; entranceZ = zMin; break; // opens -Z
	}

	int wallY = groundY + 1;
	int peakY = groundY + 2;

	int woolColor = random->nextInt(2) == 0 ? DyePowderItem::BROWN : DyePowderItem::SILVER;

	std::vector<std::pair<int, int>> interiorCells;

	for (int x = xMin; x <= xMax; x++)
	{
		for (int z = zMin; z <= zMax; z++)
		{
			bool isCorner = (x == xMin || x == xMax) && (z == zMin || z == zMax);
			bool isPerimeter = (x == xMin || x == xMax || z == zMin || z == zMax);
			bool isEntrance = (x == entranceX && z == entranceZ);

			if (isEntrance)
			{
				placeBlock(level, x, wallY, z, Tile::air_Id, 0);
				continue;
			}

			if (isCorner)
			{
				// Support pole - visually reads as a tent pole at each corner.
				placeBlock(level, x, wallY, z, Tile::fence_Id, 0);
			}
			else if (isPerimeter)
			{
				placeBlock(level, x, wallY, z, Tile::wool_Id, woolColor);
			}
			else
			{
				interiorCells.push_back(std::make_pair(x, z));
			}
		}
	}

	// Peaked "canvas" cap over the middle of the tent.
	placeBlock(level, midX, peakY, midZ, Tile::wool_Id, woolColor);

	if (!interiorCells.empty())
	{
		auto chestPos = interiorCells[0];
		placeBlock(level, chestPos.first, groundY, chestPos.second, Tile::chest_Id, 0);
		fillChest(level, random, chestPos.first, groundY, chestPos.second);

		if (interiorCells.size() > 1)
		{
			auto torchPos = interiorCells[1];
			placeBlock(level, torchPos.first, groundY, torchPos.second, Tile::torch_Id, 5);
		}
	}

	// Hay bale just outside the entrance, if the spot's still clear -
	// purely decorative, so we skip it quietly if it isn't.
	int hayX = entranceX, hayZ = entranceZ;
	switch (facing)
	{
	case 0: hayX += 1; break;
	case 1: hayX -= 1; break;
	case 2: hayZ += 1; break;
	default: hayZ -= 1; break;
	}
	if (random->nextInt(2) == 0 && level->getTile(hayX, groundY, hayZ) == Tile::air_Id)
	{
		int hayGroundTile = level->getTile(hayX, groundY - 1, hayZ);
		if (WorldGenSafetyChecks::isNaturalGroundTile(hayGroundTile))
		{
			placeBlock(level, hayX, groundY, hayZ, Tile::hay_block_Id, 0);
		}
	}
}

bool AbandonedTentFeature::place(Level *level, Random *random, int x, int y, int z)
{
	if (!LuckyStructureConfig::tentSpawnEnabled) return false;
	if (random->nextInt(LuckyStructureConfig::tentChance) != 0) return false;

	Biome *biome = level->getBiome(x, z);
	if (!LuckyStructureConfig::isBiomeIn(biome, LuckyStructureConfig::travelerCampBiomes())) return false;

	int size = random->nextInt(2) == 0 ? 3 : 4;
	int half = size / 2;

	int groundY;
	if (!WorldGenSafetyChecks::isSafeOutdoorSurface(level, x, z, groundY)) return false;

	// Check the whole footprint (plus a one-block buffer) is still
	// undisturbed natural terrain before committing to building here.
	if (!WorldGenSafetyChecks::isFootprintNatural(level, x - half - 1, z - half - 1, x - half + size, z - half + size, groundY))
	{
		return false;
	}

	int facing = random->nextInt(4);
	buildTent(level, random, x, groundY, z, size, facing);
	return true;
}
