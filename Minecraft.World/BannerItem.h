#pragma once
#include "Item.h"
#include "BannerTileEntity.h"

class IconRegister;

class BannerItem : public Item
{
private:
	static const unsigned int COLOR_DESCS[16];

public:
	BannerItem(int id);
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance) override;
	virtual bool useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly = false) override;

	// neoLegacy - patterns. They live in the stack's tag the way Java keeps them: BlockEntityTag > Patterns, a list of { Pattern, Color }.
	static vector<BannerPattern> getPatterns(shared_ptr<ItemInstance> item);
	static void setPatterns(shared_ptr<ItemInstance> item, const vector<BannerPattern> &patterns);
	static void addPattern(shared_ptr<ItemInstance> item, const wstring &code, int color);

	// neoLegacy - banner-editor packet matching: the pattern-index / pattern-color vectors for a
	// banner stack, built from getPatterns(). An unrecognized layer code yields index -1, which can
	// never collide with a valid pattern index, so client and server still agree exactly which
	// stack is being edited. Used by CraftItemPacket + the server craft handler.
	static vector<char> getPatternIndexs(shared_ptr<ItemInstance> item);
	static vector<char> getPatternColors(shared_ptr<ItemInstance> item);

	// The Ominous Banner (white, eight patterns, named). Raid captains carry one and drop it.
	static shared_ptr<ItemInstance> createOminousBanner();
};
