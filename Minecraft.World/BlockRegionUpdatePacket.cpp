#include "stdafx.h"
#include <iostream>
#include <exception>
#include "InputOutputStream.h"
#include "net.minecraft.world.level.h"
#include "compression.h"
#include "PacketListener.h"
#include "BlockRegionUpdatePacket.h"
#include "LevelChunk.h"
#include "DataLayer.h"
#include "Dimension.h"


#define BLOCK_REGION_UPDATE_FULLCHUNK 0x01
#define BLOCK_REGION_UPDATE_EXTTILES 0x04	// neoLegacy - a list of extended tile ids (>= 256) follows the block data
#define BLOCK_REGION_UPDATE_ZEROHEIGHT 0x02	// added so we can still send a byte for ys, which really needs the range 0-256

BlockRegionUpdatePacket::~BlockRegionUpdatePacket()
{
	delete [] buffer.data;
}

BlockRegionUpdatePacket::BlockRegionUpdatePacket()
{
	shouldDelay = true;
	x = 0;
	y = 0;
	z = 0;
	xs = 0;
	ys = 0;
	zs = 0;
	bIsFullChunk = false;
}

BlockRegionUpdatePacket::BlockRegionUpdatePacket(int x, int y, int z, int xs, int ys, int zs, Level *level)
{
	
	shouldDelay = true;
	this->x = x;
	this->y = y;
	this->z = z;
	this->xs = xs;
	this->ys = ys;
	this->zs = zs;
	bIsFullChunk = false;
	levelIdx = ( ( level->dimension->id == 0 ) ? 0 : ( (level->dimension->id == -1) ? 1 : 2 ) );

	// 4J - if we are compressing a full chunk, re-order the blocks so that they compress better
	// TODO - we should be using compressed data directly here rather than decompressing first and then recompressing...
	byteArray rawBuffer;

	if( xs == 16 && ys == Level::maxBuildHeight && zs == 16 && ( ( x & 15 ) == 0 ) && ( y == 0 ) && ( ( z & 15 ) == 0 ) )
	{
		bIsFullChunk = true;

		LevelChunk *lc = level->getChunkAt(x,z);
		// neoLegacy - getChunkAt can return null for an unloaded chunk; sending an empty buffer is safe, dereferencing it is not.
		if (lc == nullptr) rawBuffer = byteArray();
		else rawBuffer = lc->getReorderedBlocksAndData(x&0xF, y, z&0xF, xs, this->ys, zs);
	}
	else
	{
		MemSect(50);
		rawBuffer = level->getBlocksAndData(x, y, z, xs, ys, zs, false);
		MemSect(0);
	}

	if(rawBuffer.length == 0)
	{
		size = 0;
		buffer = byteArray();
		app.DebugPrintf("[BRUP-SERVER] *** EMPTY BUFFER for chunk(%d,%d) ys=%d bIsFullChunk=%d\n",
			x>>4, z>>4, this->ys, bIsFullChunk ? 1 : 0);
	}
	else
	{
		// We don't know how this will compress - just make a fixed length buffer to initially decompress into
		// Some small sets of blocks can end up compressing into something bigger than their source
		unsigned char *ucTemp = new unsigned char[(256 * 16 * 16 * 5)/2];
		unsigned int inputSize = (256 * 16 * 16 * 5)/2;
	
		Compression::getCompression()->CompressLZXRLE(ucTemp, &inputSize, rawBuffer.data, (unsigned int) rawBuffer.length);
		//app.DebugPrintf("Chunk (%d,%d) compressed from %d to size %d\n", x>>4, z>>4, rawBuffer.length, inputSize);
		unsigned char *ucTemp2 = new unsigned char[inputSize];
		memcpy(ucTemp2,ucTemp,inputSize);
		delete [] ucTemp;
		buffer = byteArray(ucTemp2,inputSize);
		delete [] rawBuffer.data;
		size = inputSize;
	}

	// neoLegacy - the 8-bit block data above carries a placeholder for tiles with ids >= 256; send the real ids alongside
	level->collectExtTiles(x, y, z, xs, this->ys, zs, extTiles);
}
 
void BlockRegionUpdatePacket::read(DataInputStream *dis) //throws IOException
{
	byte chunkFlags = dis->readByte();
	x = dis->readInt();
	y = dis->readShort();
	z = dis->readInt();
	xs = dis->read() + 1;
	ys = dis->read() + 1;
	zs = dis->read() + 1;

	bIsFullChunk = (chunkFlags & BLOCK_REGION_UPDATE_FULLCHUNK) ? true : false;
	if(chunkFlags & BLOCK_REGION_UPDATE_ZEROHEIGHT)
		ys = 0;

	size = dis->readInt();
	levelIdx = ( size >> 30 ) & 3;
	size &= 0x3fffffff;

	const int MAX_COMPRESSED_CHUNK_SIZE = 5 * 1024 * 1024;
    if (size < 0 || size > MAX_COMPRESSED_CHUNK_SIZE)
    {
        size = 0;
    }

	bool decompressOk = true;	// neoLegacy - false when the payload cannot be trusted; the packet is then dropped below

	if(size == 0)
	{
		buffer = byteArray();
		if(bIsFullChunk)
		{
			app.DebugPrintf("[BRUP-READ] *** ZERO-SIZE full chunk packet at (%d,%d)!\n", x>>4, z>>4);
		}
	}
	else
	{
		byteArray compressedBuffer(size);
		bool success = dis->readFully(compressedBuffer);

		int bufferSize = xs * ys * zs * 5/2;
		// Add the size of the biome data if it's a full chunk
		if(bIsFullChunk) bufferSize += (16*16);
		buffer = byteArray(bufferSize);
		unsigned int outputSize = buffer.length;

		if( success )
		{
			// neoLegacy - a failed decompress used to be ignored: the HRESULT was dropped and the only guard was an
			// assert, compiled out in release, so a corrupt chunk packet was applied to the world verbatim. The raid
			// log shows exactly one such failure on the crashing thread right before the fault, and the line appears
			// in no other log. Drop the packet instead: a missing chunk update is harmless, a corrupt one is not.
			// The trailer below is still read (framing must stay intact); its tiles are discarded with the packet.
			HRESULT hr = Compression::getCompression()->DecompressLZXRLE( buffer.data, &outputSize, compressedBuffer.data, size);
			if (hr != S_OK || buffer.length != outputSize)
			{
				app.DebugPrintf("*** BlockRegionUpdatePacket DECOMPRESS FAILED: dropping corrupt chunk packet at (%d,%d) hr=0x%08X expected=%d got=%d compressedSize=%d\n",
					x>>4, z>>4, (unsigned int)hr, buffer.length, outputSize, size);
				size = 0;
				buffer = byteArray();
				decompressOk = false;
			}
		}
		else
		{
			app.DebugPrintf("Not decompressing packet that wasn't fully read\n");
			decompressOk = false;
		}

		delete [] compressedBuffer.data;
	}

	// neoLegacy - extended tile ids trailer (still read on a corrupt packet so the stream framing stays intact;
	// the tiles are discarded at the end of read() when decompressOk is false)
	extTiles.clear();
	if (chunkFlags & BLOCK_REGION_UPDATE_EXTTILES)
	{
		int count = dis->readInt();
		if (count < 0 || count > 16 * 16 * 256) count = 0;
		for (int i = 0; i < count; i++)
		{
			int rx = dis->read();
			int ry = dis->read();
			int rz = dis->read();
			int tile = dis->readShort() & 0xffff;
			extTiles.push_back(rx);
			extTiles.push_back(ry);
			extTiles.push_back(rz);
			extTiles.push_back(tile);
		}
	}
	if (!decompressOk) extTiles.clear();	// neoLegacy - drop the trailer of a corrupt packet (see above)
}

void BlockRegionUpdatePacket::write(DataOutputStream *dos) // throws IOException
{
	byte chunkFlags = 0;
	if(bIsFullChunk) chunkFlags |= BLOCK_REGION_UPDATE_FULLCHUNK;
	if(ys == 0) chunkFlags |= BLOCK_REGION_UPDATE_ZEROHEIGHT;
	if(!extTiles.empty()) chunkFlags |= BLOCK_REGION_UPDATE_EXTTILES;

	dos->writeByte(chunkFlags);
	dos->writeInt(x);
	dos->writeShort(y);
	dos->writeInt(z);
	dos->write(xs - 1);
	dos->write(ys - 1);
	dos->write(zs - 1);

	int sizeAndLevel = size;
	sizeAndLevel |= ( levelIdx << 30 );
	dos->writeInt(sizeAndLevel);
	dos->write(buffer, 0, size);

	if(!extTiles.empty())
	{
		dos->writeInt(static_cast<int>(extTiles.size() / 4));
		for (size_t i = 0; i + 3 < extTiles.size(); i += 4)
		{
			dos->write(extTiles[i]);
			dos->write(extTiles[i + 1]);
			dos->write(extTiles[i + 2]);
			dos->writeShort(static_cast<short>(extTiles[i + 3]));
		}
	}
}

void BlockRegionUpdatePacket::handle(PacketListener *listener)
{
	listener->handleBlockRegionUpdate(shared_from_this());
}

int BlockRegionUpdatePacket::getEstimatedSize() 
{
	return 17 + size + (extTiles.empty() ? 0 : 4 + static_cast<int>(extTiles.size() / 4) * 5);
}

