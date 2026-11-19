#pragma once

#include "StructureFeature.h"
#include "StructureStart.h"
#include "StructurePiece.h"
#include "Biome.h"
#include "PillagerOutpostData.h"

class VillageFeature;

// neoLegacy - Pillager Outpost, built from the Java structure files (see PillagerOutpostData.h; the data comes from outpost_gen.py).
// One watchtower (the plain tower with the overgrown one laid over it, as in Java) with a scatter of tents, cages, log piles and
// target dummies around it; a chest in the tower; Pillagers spawn inside it; an iron golem sits in one of the cages.
// Not the same as Java: banners on the tower are plain white (banners here have no patterns yet), plain pumpkins are carved ones, there are
// no allays, and the features are picked from 32 pre-assembled layouts instead of a live jigsaw.
class PillagerOutpostFeature : public StructureFeature
{
public:
	static void staticCtor();
	static void loadStatic();								// piece ids
	static vector<Biome *> allowedBiomes;

	// Tuning. Classic-size worlds are small, so these are friendlier than Java's 32 / 8 / one in five.
	static const int SPACING = 24;								// chunks between candidate spots
	static const int MIN_SEPARATION = 8;
	static const int VILLAGE_CLEARANCE_CHUNKS = 10;			// no outpost this close to a village (as in Java)

private:
	vector<Biome::MobSpawnerData *> enemies;
	VillageFeature *villageFeature;

public:
	PillagerOutpostFeature();
	void setVillageFeature(VillageFeature *village) { villageFeature = village; }
	wstring getFeatureName();
	vector<Biome::MobSpawnerData *> *getEnemies();			// Pillagers: used for monster spawning inside an outpost

protected:
	virtual bool isFeatureChunk(int x, int z, bool bIsSuperflat = false);
	StructureStart *createStructureStart(int x, int z);

public:
	class OutpostStart : public StructureStart
	{
	public:
		static StructureStart *Create() { return new OutpostStart(); }
		virtual EStructureStart GetType() { return eStructureStart_PillagerOutpostStart; }

		OutpostStart();
		OutpostStart(Level *level, Random *random, int chunkX, int chunkZ);
	};

	// One block of the outpost: the tower, or one feature. Local coordinates are offsets from the piece's lowest corner; the orientation is
	// left undefined, so the piece places blocks by world position itself.
	class OutpostPiece : public StructurePiece
	{
	public:
		static StructurePiece *Create() { return new OutpostPiece(); }
		virtual EStructurePiece GetType() { return eStructurePiece_PillagerOutpostPiece; }

	private:
		const OutpostTemplate *tmpl;
		bool isTower;
		int heightPosition;
		bool spawnedGolem;

	public:
		OutpostPiece();
		OutpostPiece(const OutpostTemplate *tmpl, bool isTower, int x, int z);

	protected:
		virtual void addAdditonalSaveData(CompoundTag *tag);
		virtual void readAdditonalSaveData(CompoundTag *tag);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);

	private:
		void growFoundation(Level *level, BoundingBox *chunkBB, int wx, int wy, int wz, int tile, int data);
		void fillChest(Level *level, Random *random, int wx, int wy, int wz);
		void spawnGuards(Level *level, Random *random, BoundingBox *chunkBB);
	};
};
