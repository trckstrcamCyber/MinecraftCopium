#pragma once

class ShapelessRecipy : public Recipy 
{
private:
	_eGroupType group;
	const ItemInstance *result;
	vector<ItemInstance *> *ingredients;

public:
	ShapelessRecipy(ItemInstance *result, vector<ItemInstance *> *ingredients, _eGroupType egroup=Recipy::eGroupType_Decoration);
	virtual ~ShapelessRecipy() override;

	virtual const ItemInstance *getResultItem();
	virtual const int getGroup();
	virtual bool matches(shared_ptr<CraftingContainer> craftSlots, Level *level);
	virtual shared_ptr<ItemInstance> assemble(shared_ptr<CraftingContainer> craftSlots);
	virtual int size();

	// neoLegacy - true if any ingredient is the given item/tile id (used by Recipes::isComplexOnly
	// to tell the Complex Table's redstone -> red dye recipe apart from the rose -> red dye one).
	bool hasIngredient(int itemId) const;

	// 4J-PB - to return the items required to make a recipe
	virtual bool reqs(int iRecipe);
	virtual void reqs(INGREDIENTS_REQUIRED *pIngReq); 

	virtual void writeToStream(DataOutputStream* dos);
	static ShapelessRecipy* readFromStream(DataInputStream* dos);
};
