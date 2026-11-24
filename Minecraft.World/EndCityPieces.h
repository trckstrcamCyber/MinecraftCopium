#pragma once
#include "StructurePiece.h"
#include "EndCityTemplates.h"

class EndCityPieces
{
public:
	enum ERotation
	{
		ROT_NONE,
		ROT_CLOCKWISE_90,
		ROT_CLOCKWISE_180,
		ROT_COUNTERCLOCKWISE_90,
	};

	static int rotate(int rotation, int by) { return (rotation + by) & 3; }
	static void transform(int rotation, int x, int z, int &outX, int &outZ);

	class EndCityPiece : public StructurePiece
	{
	public:
		virtual EStructurePiece GetType() { return eStructurePiece_EndCityPiece; }
		static StructurePiece *Create() { return new EndCityPiece(); }

		int templateType;
		int templateX, templateY, templateZ;
		int rotation;
		bool overwrite;

		EndCityPiece();
		EndCityPiece(int templateType, int x, int y, int z, int rotation, bool overwrite);

		void moveBy(int dx, int dy, int dz);
		void setGenDepth(int depth) { genDepth = depth; }

		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);

	protected:
		virtual void addAdditonalSaveData(CompoundTag *tag);
		virtual void readAdditonalSaveData(CompoundTag *tag);

	private:
		void calculateBoundingBox();
		void handleExtra(Level *level, Random *random, BoundingBox *chunkBB, const EndCityTemplates::Extra &extra);
		int rotateData(int id, int data);
	};

	class Builder
	{
	public:
		Builder(list<StructurePiece *> &pieces, Random *random);
		void startHouseTower(int x, int y, int z, int rotation);

	private:
		enum ESection { SECTION_HOUSE, SECTION_TOWER, SECTION_TOWER_BRIDGE, SECTION_FAT_TOWER };

		list<StructurePiece *> &pieces;
		Random *random;
		bool houseGenerated[8];
		bool shipGenerated;

		EndCityPiece *add(list<StructurePiece *> &into, EndCityPiece *piece);
		EndCityPiece *addPiece(EndCityPiece *previous, int ox, int oy, int oz, int templateType, int rotation, bool overwrite);
		bool recursiveChildren(ESection section, int genDepth, EndCityPiece *previous, int ox, int oy, int oz, list<StructurePiece *> &into);
		bool generate(ESection section, int genDepth, EndCityPiece *previous, int ox, int oy, int oz, list<StructurePiece *> &out);

		bool generateHouse(int genDepth, EndCityPiece *previous, int ox, int oy, int oz, list<StructurePiece *> &out);
		bool generateTower(int genDepth, EndCityPiece *previous, list<StructurePiece *> &out);
		bool generateTowerBridge(int genDepth, EndCityPiece *previous, list<StructurePiece *> &out);
		bool generateFatTower(int genDepth, EndCityPiece *previous, list<StructurePiece *> &out);
	};
};
