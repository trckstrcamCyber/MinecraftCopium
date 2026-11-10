#pragma once
#include "StructurePiece.h"

class BiomeSource;

class VillagePieces
{

private:
	static const int MAX_DEPTH = 50;
	static const int BASE_ROAD_DEPTH = 3;
	// the dungeon starts at 64 and traverses downwards to this point
	static const int LOWEST_Y_POSITION = 10;

public:
	static const int SIZE_SMALL = 0;
	static const int SIZE_BIG = 1;
	static const int SIZE_BIGGEST = 2;

	// 4J - added to replace use of Class<? extends VillagePiece> within this class
	enum EPieceClass
	{
		EPieceClass_SimpleHouse,
		EPieceClass_SmallTemple,
		EPieceClass_BookHouse,
		EPieceClass_SmallHut,
		EPieceClass_PigHouse,
		EPieceClass_DoubleFarmland,
		EPieceClass_Farmland,
		EPieceClass_Smithy,
		EPieceClass_TwoRoomHouse,
		EPieceClass_BlacksmithHouse,
		EPieceClass_ModernHouse
	};

	// Which material/biome palette a village instance uses. Plains is the
	// original default look; Desert is the original sandstone reskin.
	// Savanna/Taiga/Snowy are new - see VillagePiece::biomeBlock/biomeData.
	enum EVillageStyle
	{
		EVillageStyle_Plains = 0,
		EVillageStyle_Desert = 1,
		EVillageStyle_Savanna = 2,
		EVillageStyle_Taiga = 3,
		EVillageStyle_Snowy = 4
	};

	static void loadStatic();

	class PieceWeight
	{
	public:
		EPieceClass pieceClass;		// 4J - EPieceClass was Class<? extends VillagePiece>
		const int weight;
		int placeCount;
		int maxPlaceCount;

		PieceWeight(EPieceClass pieceClass, int weight, int maxPlaceCount);	// 4J - EPieceClass was Class<? extends VillagePiece>
		bool doPlace(int depth);
		bool isValid();
	};

	static list<PieceWeight *> *createPieceSet(Random *random, int villageSize);		// 4J - was ArrayList

	class StartPiece;
private:
	class VillagePiece;
	static int updatePieceWeight(list<PieceWeight *> *currentPieces);		// 4J = was array list
	static VillagePiece *findAndCreatePieceFactory(StartPiece *startPiece, PieceWeight *piece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth);
	static VillagePiece *generatePieceFromSmallDoor(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth);
	static StructurePiece *generateAndAddPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth);
	static StructurePiece *generateAndAddRoadPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth);


	/**
	*
	*
	*/
private:
	class VillagePiece : public StructurePiece
	{
	protected:
		int heightPosition;
	private:
		int spawnedVillagerCount;
		int villageStyle;	// one of EVillageStyle - was a plain "isDesertVillage" bool
	protected:
		StartPiece *startPiece;

		VillagePiece();
		VillagePiece(StartPiece *startPiece, int genDepth);
		virtual void addAdditonalSaveData(CompoundTag *tag);
		virtual void readAdditonalSaveData(CompoundTag *tag);
		StructurePiece *generateHouseNorthernLeft(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int yOff, int zOff);
		StructurePiece *generateHouseNorthernRight(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int yOff, int zOff);
		int getAverageGroundHeight(Level *level, BoundingBox *chunkBB);
		static bool isOkBox(BoundingBox *box, StartPiece *startRoom); // 4J added startRoom param
		void spawnVillagers(Level *level, BoundingBox *chunkBB, int x, int y, int z, int count);
		virtual int getVillagerProfession(int villagerNumber);
		int getVillageStyle() const { return villageStyle; }	// neoLegacy
		virtual int biomeBlock(int tile, int data);
		virtual int biomeData(int tile, int data);
		virtual void placeBlock(Level *level, int block, int data, int x, int y, int z, BoundingBox *chunkBB);
		virtual void generateBox(Level *level, BoundingBox *chunkBB, int x0, int y0, int z0, int x1, int y1, int z1, int edgeTile, int fillTile, bool skipAir);
		virtual void fillColumnDown(Level *level, int block, int data, int x, int startY, int z, BoundingBox *chunkBB);

		// Places a two-block bed (foot + head) so villages have somewhere to
		// sleep, like modern Minecraft. x/y/z is the foot position, in this
		// piece's own local (unrotated, Direction::NORTH-reference) coordinate
		// frame - same convention as every other directional block placed in
		// this file (see the door/stairs/ladder calls). localDirection is a
		// Direction:: value (SOUTH/WEST/NORTH/EAST) telling the helper which
		// way the head lies relative to the foot in that same local frame;
		// getOrientationData() rotates it to match however this piece
		// instance actually landed in the world.
		void placeBed(Level *level, BoundingBox *chunkBB, int x, int y, int z, int localDirection);
	};

	/**
	*
	*
	*/
public:
	class Well : public VillagePiece
	{
	public:
		static StructurePiece *Create() { return new Well(); }
		virtual EStructurePiece GetType() { return eStructurePiece_Well; }

	private:
		static const int width = 6;
		static const int height = 15;
		static const int depth = 6;

	public:
		Well();
		Well(StartPiece *startPiece, int genDepth, Random *random, int west, int north);
		Well(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction);
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);

	private:
		bool camelSpawned;	// neoLegacy - desert villages get exactly one camel, placed by the well

	protected:
		virtual void addAdditonalSaveData(CompoundTag *tag);
		virtual void readAdditonalSaveData(CompoundTag *tag);
	};

public:
	class StartPiece : public Well
	{
	public:
		virtual EStructurePiece GetType() { return eStructurePiece_VillageStartPiece; }

	public:
		// these fields are only used in generation step and aren't serialized :{
		BiomeSource *biomeSource;
		int villageStyle;	// one of EVillageStyle - was a plain "isDesertVillage" bool

		int villageSize;
		bool isLibraryAdded;
		PieceWeight *previousPiece;
		list<PieceWeight *> *pieceSet;
		Level *m_level;

		// these queues are used so that the addChildren calls are called in a random order
		vector<StructurePiece *> pendingHouses;
		vector<StructurePiece *> pendingRoads;

		StartPiece();
		StartPiece(BiomeSource *biomeSource, int genDepth, Random *random, int west, int north, list<PieceWeight *> *pieceSet, int villageSize, Level *level); // 4J Added level param
		virtual ~StartPiece();

		BiomeSource *getBiomeSource();

	};

public:
	class VillageRoadPiece : public VillagePiece
	{
	protected:
		VillageRoadPiece() {}
		VillageRoadPiece(StartPiece *startPiece, int genDepth) : VillagePiece(startPiece, genDepth) {}
	};

	/**
	*
	*
	*/
public:
	class StraightRoad : public VillageRoadPiece
	{
	public:
		static StructurePiece *Create() { return new StraightRoad(); }
		virtual EStructurePiece GetType() { return eStructurePiece_StraightRoad; }

	private:
		static const int width = 3;
		int length;
	public:
		StraightRoad();
		StraightRoad(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction);

	protected:
		void addAdditonalSaveData(CompoundTag *tag);
		void readAdditonalSaveData(CompoundTag *tag);

	public:
		virtual void addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random);
		static BoundingBox *findPieceBox(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

	/**
	*
	*
	*/
public:
	class SimpleHouse : public VillagePiece
	{
	public:
		static StructurePiece *Create() { return new SimpleHouse(); }
		virtual EStructurePiece GetType() { return eStructurePiece_SimpleHouse; }

	private:
		static const int width = 5;
		static const int height = 6;
		static const int depth = 5;

	private:
		bool hasTerrace;

	public:
		SimpleHouse();
		SimpleHouse(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction);

	protected:
		void addAdditonalSaveData(CompoundTag *tag);
		void readAdditonalSaveData(CompoundTag *tag);

	public:
		static SimpleHouse *createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

public:
	class SmallTemple : public VillagePiece
	{
	public:
		static StructurePiece *Create() { return new SmallTemple(); }
		virtual EStructurePiece GetType() { return eStructurePiece_SmallTemple; }

	private:
		static const int width = 5;
		static const int height = 12;
		static const int depth = 9;

		int heightPosition;

	public:
		SmallTemple();
		SmallTemple(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction);

		static SmallTemple *createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
		virtual int getVillagerProfession(int villagerNumber);
	};

public:
	class BookHouse : public VillagePiece
	{
	public:
		static StructurePiece *Create() { return new BookHouse(); }
		virtual EStructurePiece GetType() { return eStructurePiece_BookHouse; }

	private:
		static const int width = 9;
		static const int height = 9;
		static const int depth = 6;

		int heightPosition;

	public:
		BookHouse();
		BookHouse(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction);

		static BookHouse *createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
		virtual int getVillagerProfession(int villagerNumber);
	};

public:
	class SmallHut : public VillagePiece
	{
	public:
		static StructurePiece *Create() { return new SmallHut(); }
		virtual EStructurePiece GetType() { return eStructurePiece_SmallHut; }


	private:
		static const int width = 4;
		static const int height = 6;
		static const int depth = 5;

		bool lowCeiling;
		int tablePlacement;

	public:
		SmallHut();
		SmallHut(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction);

	protected:
		virtual void addAdditonalSaveData(CompoundTag *tag);
		virtual void readAdditonalSaveData(CompoundTag *tag);

	public:
		static SmallHut *createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

public:
	class PigHouse : public VillagePiece
	{
	public:
		static StructurePiece *Create() { return new PigHouse(); }
		virtual EStructurePiece GetType() { return eStructurePiece_PigHouse; }


	private:
		static const int width = 9;
		static const int height = 7;
		static const int depth = 11;

	public:
		PigHouse();
		PigHouse(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction);
		static PigHouse *createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
		virtual int getVillagerProfession(int villagerNumber);
	};

public:
	class TwoRoomHouse : public VillagePiece
	{
	public:
		static StructurePiece *Create() { return new TwoRoomHouse(); }
		virtual EStructurePiece GetType() { return eStructurePiece_TwoRoomHouse; }

	private:
		static const int width = 9;
		static const int height = 7;
		static const int depth = 12;

		int heightPosition;

	public:
		TwoRoomHouse();
		TwoRoomHouse(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction);
		static TwoRoomHouse *createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual  bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

public:
	class Smithy : public VillagePiece
	{
	public:
		static StructurePiece *Create() { return new Smithy(); }
		virtual EStructurePiece GetType() { return eStructurePiece_Smithy; }


	private:
		static const int width = 10;
		static const int height = 6;
		static const int depth = 7;

		bool hasPlacedChest;

		static WeighedTreasureArray treasureItems;

	public:
		static void staticCtor();

		Smithy();
		Smithy(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction);
		static Smithy *createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);

	protected:
		void addAdditonalSaveData(CompoundTag *tag);
		void readAdditonalSaveData(CompoundTag *tag);

	public:
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
		virtual int getVillagerProfession(int villagerNumber);
	};

public:
	class Farmland : public VillagePiece
	{
	public:
		static StructurePiece *Create() { return new Farmland(); }
		virtual EStructurePiece GetType() { return eStructurePiece_Farmland; }


	private:
		static const int width = 7;
		static const int height = 4;
		static const int depth = 9;

		int cropsA;
		int cropsB;

		int selectCrops(Random *random);

	public:
		Farmland();
		Farmland(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction);

	protected:
		virtual void addAdditonalSaveData(CompoundTag *tag);
		virtual void readAdditonalSaveData(CompoundTag *tag);

	public:
		static Farmland *createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

public:
	class DoubleFarmland : public VillagePiece
	{
	public:
		static StructurePiece *Create() { return new DoubleFarmland(); }
		virtual EStructurePiece GetType() { return eStructurePiece_DoubleFarmland; }

	private:
		static const int width = 13;
		static const int height = 4;
		static const int depth = 9;

		int heightPosition;

		int cropsA;
		int cropsB;
		int cropsC;
		int cropsD;

		int selectCrops(Random *random);

	public:
		DoubleFarmland();
		DoubleFarmland(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction);

	protected:
		virtual void addAdditonalSaveData(CompoundTag *tag);
		virtual void readAdditonalSaveData(CompoundTag *tag);

	public:
		static DoubleFarmland *createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

public:
	class LightPost : public VillagePiece
	{
	public:
		static StructurePiece *Create() { return new LightPost(); }
		virtual EStructurePiece GetType() { return eStructurePiece_LightPost; }

	private:
		static const int width = 3;
		static const int height = 4;
		static const int depth = 2;

		int heightPosition;

	public:
		LightPost();
		LightPost(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *box, int direction);
		static BoundingBox *findPieceBox(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};

	/**
	* A blacksmith building matching modern Java's actual weaponsmith layout:
	* an enclosed room (chest + door) beside an open porch forge (lava,
	* furnaces, grindstone) - no anvil, that's not part of the real building.
	* Cobblestone/log/planks, same as Smithy - this reskins by village style
	* exactly like every other piece (see biomeBlock/biomeData on the base
	* VillagePiece class).
	*/
public:
	class BlacksmithHouse : public VillagePiece
	{
	public:
		static StructurePiece *Create() { return new BlacksmithHouse(); }
		virtual EStructurePiece GetType() { return eStructurePiece_BlacksmithHouse; }

	private:
		static const int width = 10;
		static const int height = 9;
		static const int depth = 7;

		bool hasPlacedChest;

		static WeighedTreasureArray treasureItems;

	public:
		static void staticCtor();

		BlacksmithHouse();
		BlacksmithHouse(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction);
		static BlacksmithHouse *createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);

	protected:
		void addAdditonalSaveData(CompoundTag *tag);
		void readAdditonalSaveData(CompoundTag *tag);

	public:
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
		virtual int getVillagerProfession(int villagerNumber);
	};

	/**
	* A larger dwelling in the style of a modern (post-Village & Pillage)
	* Java plains house - same log/plank/cobblestone palette as every other
	* house (reskins by village style normally), just a roomier layout with
	* more windows and a hay-bale-and-fence-gate yard, approximating the
	* newer house look with the older block set this fork actually has.
	*/
public:
	class ModernHouse : public VillagePiece
	{
	public:
		static StructurePiece *Create() { return new ModernHouse(); }
		virtual EStructurePiece GetType() { return eStructurePiece_ModernHouse; }

	private:
		static const int width = 9;
		static const int height = 7;
		static const int depth = 12;

	public:
		ModernHouse();
		ModernHouse(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction);
		static ModernHouse *createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth);
		virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
	};
};
