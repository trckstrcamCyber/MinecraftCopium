#pragma once
using namespace std;

#include "Item.h"

class Player;
class Material;
class Level;

class DoorItem : public Item 
{
private:
	Material *material;
	wstring doorType;

public:
	DoorItem(int id, Material *material, const wstring& doorType);

	virtual bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
	// neoLegacy - Copper Age variants (2026-11-23): the copper door's item
	// name varies by oxidation/waxed state (aux 0-7); every other door
	// keeps its single name.
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);
	// neoLegacy - Copper Age (Phase 4): place takes the item aux so a
	// copper door item keeps its oxidation/waxed state when placed (the
	// aux is encoded into the upper half's bits 5-7; 0 for every other door).
	static void place(Level *level, int x, int y, int z, int dir, Tile *tile, int aux = 0);
};
 