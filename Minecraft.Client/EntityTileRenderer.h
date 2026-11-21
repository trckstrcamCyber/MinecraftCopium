#pragma once

class ChestTileEntity;
class EnderChestTileEntity;
class CopperGolemStatueBlockEntity;
class Tile;

class EntityTileRenderer
 {
 public:
	 static EntityTileRenderer *instance;

 private:
	 shared_ptr<ChestTileEntity> chest;
	 shared_ptr<ChestTileEntity> trappedChest;
	 shared_ptr<EnderChestTileEntity> enderChest;
	 // neoLegacy - Copper Age (Round 5): held-item TEs for the copper
	 // chest (TYPE_COPPER) and the golem statue, so they render as
	 // themselves instead of falling through to the vanilla chest.
	 shared_ptr<ChestTileEntity> copperChest;
	 shared_ptr<CopperGolemStatueBlockEntity> statue;

 public:
	 EntityTileRenderer();
	 void render(Tile *tile, int data, float brightness, float alpha, bool setColor = true, bool useCompiled = true);		// 4J - added setColor parameter and alpha for chest in the crafting menu, and added useCompiled
};
