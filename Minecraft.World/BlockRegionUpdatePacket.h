#pragma once
using namespace std;

#include "Packet.h"
#include <vector>

class Level;

class BlockRegionUpdatePacket : public Packet, public enable_shared_from_this<BlockRegionUpdatePacket>
{
public:
    int x, y, z;
    int xs, ys, zs;
    byteArray buffer;
	int levelIdx;
	bool bIsFullChunk; // 4J Added
	std::vector<int> extTiles;	// neoLegacy - tiles with ids >= 256 in this region, as region-relative (x, y, z, tile) quads

private:
	int size;

public:
	BlockRegionUpdatePacket();
	~BlockRegionUpdatePacket();
	BlockRegionUpdatePacket(int x, int y, int z, int xs, int ys, int zs, Level *level);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return std::make_shared<BlockRegionUpdatePacket>(); }
	virtual int getId() { return 51; }
};
