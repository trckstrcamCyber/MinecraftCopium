#pragma once

#include "StructureFeature.h"
#include "StructureStart.h"
#include "StructurePiece.h"
#include "Biome.h"
#include "IllagerStructureData.h"

class VillageFeature;

// neoLegacy - the Illager Invasion structures, built from the mod's structure files (see IllagerStructureData.h; the data comes from tools/illager_struct_gen.py).
//   Sorcerer Hut   - a small hut in dark forests: a Sorcerer, a Vindicator and a chest of paper, books and Unusual Dust.
//   Illusioner Tower - a tall tower (3 designs) in dark forests, swamps and taigas: Provokers, Vindicators, an Archivist, a Basher, an Illusioner, three chests.
//   Illager Fort   - a wooden fort in taigas and snowy lands: Vindicators, Inquisitors, Provokers and four chests.
// One feature hosts all three: each has its own spacing and chance, so a given chunk starts at most one of them. /locate illager finds the nearest of any kind.
// Not the same as Java: the mod's Firecaller Hut and Labyrinth (live jigsaw assemblies) are not here, and the mod's allays and captive villagers are
// left out. The baked cells cannot hold the 12-bit extended ids, so the generator substituted a hanging lantern -> glowstone and a chain -> iron
// bars; applyIronOverlay (IllagerStructureFeature.cpp) replaces those with the real iron lantern (2331) and iron chain (2330) at runtime, after the
// grid is placed. Candles stay torches (there is no candle block in this build) and so do the standing lanterns, which bake to the same torch cell.
class IllagerStructureFeature : public StructureFeature
{
public:
	static void staticCtor();
	static void loadStatic();							// piece ids

	enum Kind { KIND_HUT = 0, KIND_TOWER = 1, KIND_FORT = 2, KIND_COUNT = 3 };
	static vector<Biome *> allowedBiomes[KIND_COUNT];

	// Tuning (chunks). Classic-size worlds are small, so these are friendlier than the mod's.
	static const int SPACING[KIND_COUNT];				// between candidate spots
	static const int MIN_SEPARATION[KIND_COUNT];
	static const int ONE_IN[KIND_COUNT];				// one candidate in N is used
	static const int VILLAGE_CLEARANCE_CHUNKS = 5;

private:
	vector<Biome::MobSpawnerData *> enemies;
	VillageFeature *villageFeature;

	int kindAt(int x, int z);

public:
	IllagerStructureFeature();
	void setVillageFeature(VillageFeature *village) { villageFeature = village; }
	wstring getFeatureName();
	vector<Biome::MobSpawnerData *> *getEnemies();

protected:
	virtual bool isFeatureChunk(int x, int z, bool bIsSuperflat = false);
	StructureStart *createStructureStart(int x, int z);

public:
	class IllagerStart : public StructureStart
	{
	public:
		static StructureStart *Create() { return new IllagerStart(); }
		virtual EStructureStart GetType() { return eStructureStart_IllagerStructureStart; }

		IllagerStart();
		IllagerStart(Level *level, Random *random, int chunkX, int chunkZ, int kind);
	};

	// the whole building is one piece; local coordinates are offsets from its lowest corner
	class IllagerPiece : public StructurePiece
	{
	public:
		static StructurePiece *Create() { return new IllagerPiece(); }
		virtual EStructurePiece GetType() { return eStructurePiece_IllagerStructurePiece; }

	private:
		const IllagerStructTemplate *tmpl;
		int heightPosition;

	public:
		IllagerPiece();
		IllagerPiece(const IllagerStructTemplate *tmpl, int x, int z);

	protected:
		virtual void addAdditonalSaveData(CompoundTag *tag);
		virtual void readAdditonalSaveData(CompoundTag *tag);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);

	private:
		void growFoundation(Level *level, BoundingBox *chunkBB, int wx, int wy, int wz, int tile, int data);
		void fillChest(Level *level, Random *random, int wx, int wy, int wz, int lootType);
		void spawnMobs(Level *level, Random *random, BoundingBox *chunkBB);
		void applyIronOverlay(Level *level, BoundingBox *chunkBB);
	};
};
