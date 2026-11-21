#pragma once
using namespace std;

#include "Packet.h"

class CompoundTag;

class TileEntityDataPacket : public Packet, public enable_shared_from_this<TileEntityDataPacket>
{
public:
	static const int TYPE_MOB_SPAWNER = 1;
	static const int TYPE_ADV_COMMAND = 2;
	static const int TYPE_BEACON = 3;
	static const int TYPE_SKULL = 4;
	static const int TYPE_BANNER = 5;
	static const int TYPE_CONCRETE_COLOR = 6;	// neoLegacy - concrete stairs/slabs color
	static const int TYPE_BED = 7;				// neoLegacy - Fancy Beds (which bed: color / straw)
	static const int TYPE_FLOWER_POT = 8;
	static const int TYPE_SHELF = 9;			// neoLegacy - Copper Age (shelf contents + facing)
	static const int TYPE_COPPER_OXIDATION = 10;	// neoLegacy - Copper Age (oxidation workaround: door/trapdoor/stairs oxidation)
	static const int TYPE_COPPER_CHEST = 11;		// neoLegacy - Copper Age (oxidation workaround: chest oxidation)
	static const int TYPE_COPPER_STATUE = 12;		// neoLegacy - Copper Age (oxidation workaround: statue pose + facing)

	int x, y, z;
	int type;
	CompoundTag *tag;

private:
	void _init();

public:
	TileEntityDataPacket();
	~TileEntityDataPacket();
	TileEntityDataPacket(int x, int y, int z, int type, CompoundTag *tag);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<TileEntityDataPacket>(); }
	virtual int getId() { return 132; }
};