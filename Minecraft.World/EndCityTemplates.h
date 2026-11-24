#pragma once

class EndCityTemplates
{
public:
	enum ETemplate
	{
		BASE_FLOOR,
		BASE_ROOF,
		BRIDGE_END,
		BRIDGE_GENTLE_STAIRS,
		BRIDGE_PIECE,
		BRIDGE_STEEP_STAIRS,
		FAT_TOWER_BASE,
		FAT_TOWER_MIDDLE,
		FAT_TOWER_TOP,
		SECOND_FLOOR,
		SECOND_FLOOR_2,
		SECOND_ROOF,
		SHIP,
		THIRD_FLOOR,
		THIRD_FLOOR_B,
		THIRD_FLOOR_C,
		THIRD_ROOF,
		TOWER_BASE,
		TOWER_FLOOR,
		TOWER_PIECE,
		TOWER_TOP,
		COUNT
	};

	enum EExtra
	{
		EXTRA_CHEST_LOOT,
		EXTRA_SENTRY,
		EXTRA_ELYTRA,
		EXTRA_BANNER,
		EXTRA_SKULL,
		EXTRA_HEALING_POTIONS,
	};

	struct Block
	{
		unsigned char x, y, z;
		int id;
		unsigned char data;
	};

	struct Extra
	{
		unsigned char x, y, z;
		unsigned char kind;
		unsigned char arg;
	};

	struct Template
	{
		const wchar_t *name;
		int sizeX, sizeY, sizeZ;
		const Block *blocks;
		int blockCount;
		const Extra *extras;
		int extraCount;
	};

	static const Template TEMPLATES[COUNT];
};
