#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "com.mojang.nbt.h"
#include "HtmlString.h"
#include "EnchantmentEncyclopedia.h"

// ---------------------------------------------------------------------------------------------------------------------------
// neoLegacy - Enchantment Encyclopedia (see EnchantmentEncyclopedia.h)
//
// Layout of the book:
//   page 1            intro
//   next pages        contents, grouped by section, each line "p<page> <name>"
//   remaining pages   one page per enchantment (a second "cont." page if an entry does not fit on one)
// ---------------------------------------------------------------------------------------------------------------------------

// ---- layout tuning -------------------------------------------------------------------------------------------------------
// The book reader wraps text by pixel width (its own line counter assumes ~23 characters) and has room for roughly 14 lines.
// These are deliberately a little smaller so nothing is clipped. VERIFY in game and nudge them if pages look cramped or sparse.
static const int ENCY_CHARS_PER_LINE = 20;
static const int ENCY_LINES_PER_PAGE = 13;
// Line break used by the book reader's text field (UIControl_Label turns Enter into '\r').
static const wchar_t ENCY_LINE_BREAK = L'\r';

// ---- effect descriptions -------------------------------------------------------------------------------------------------
// Everything else on an entry (name, max level, rarity, source, what it fits, what it conflicts with) is read from the
// enchantment itself. Only the "what does it do" sentence is typed here, keyed by enchantment id. An enchantment with no
// entry still gets a page; it just shows "No description yet." Keep each one short (about 100 characters).
struct EncyDescriptionEntry
{
	int id;
	const wchar_t *text;
};

static const EncyDescriptionEntry kDescriptions[] =
{
	// armor
	{ 0,  L"Reduces most damage you take." },
	{ 1,  L"Reduces fire and lava damage." },
	{ 2,  L"Reduces fall damage." },
	{ 3,  L"Reduces damage from explosions." },
	{ 4,  L"Reduces damage from arrows and other projectiles." },
	{ 5,  L"Lets you breathe underwater for longer." },
	{ 6,  L"Removes the mining speed penalty while underwater." },
	{ 7,  L"Chance to hurt attackers when you are hit, at the cost of armor durability." },
	{ 8,  L"Move faster while in water." },
	{ 9,  L"Freezes water under your feet into ice as you walk." },
	{ 10, L"Step up 1, 1.5 or 2 blocks without jumping." },
	{ 11, L"Stops you trampling farmland." },
	{ 12, L"Always-on speed boost: +8% movement speed per level." },
	{ 13, L"Below half health you gain speed and take less damage. At III: up to +30% speed and 36% less damage." },
	{ 14, L"The lower your health, the harder you hit: up to +2, +4 or +6 damage near death." },

	// melee
	{ 16, L"Increases melee damage." },
	{ 17, L"Extra damage to undead mobs." },
	{ 18, L"Extra damage to spiders, silverfish and other arthropods." },
	{ 19, L"Knocks enemies back further." },
	{ 20, L"Sets the target on fire." },
	{ 21, L"Mobs drop more loot." },
	{ 22, L"Heals you for 10, 20 or 30% of the melee damage you deal." },
	{ 23, L"Each hit in a combo adds 5, 10 or 15% crit chance; landing a crit resets it." },
	{ 24, L"Each hit in a combo makes the next hit 5, 10 or 15% stronger, up to +200%." },
	{ 25, L"Mace smash attacks deal +0.5 damage per level for every block you fell." },
	{ 26, L"The mace ignores 15% of the target's armor per level, up to 60%." },
	{ 27, L"A mace smash launches you back up (about 7, 11 or 17 blocks) and the fall does not hurt." },

	// tools
	{ 32, L"Mines blocks faster." },
	{ 33, L"Blocks drop themselves instead of their usual items." },
	{ 34, L"Chance to lose no durability when used." },
	{ 35, L"Ores and some blocks drop more items." },
	{ 36, L"Mining an ore breaks its whole connected vein: up to 8, 16 or 32 blocks." },
	{ 37, L"Iron and gold ore drop smelted ingots, with a little bonus XP." },
	{ 38, L"Chopping a log fells the whole connected tree: up to 16, 32 or 64 logs." },

	// bows
	{ 48, L"Arrows deal more damage." },
	{ 49, L"Arrows knock targets back further." },
	{ 50, L"Arrows set targets on fire." },
	{ 51, L"Shoot without using up arrows." },
	{ 52, L"Arrows burst on impact (radius about 1.75, 2.5 or 3.25) without breaking blocks." },

	// fishing
	{ 64, L"Fish bite faster." },
	{ 65, L"More treasure and less junk when fishing." },
	{ 66, L"Less treasure and more junk when fishing: the opposite of Luck of the Sea." },

	// general / utility
	{ 70, L"Repairs the item using the experience you collect." },
	{ 71, L"Jump again in mid-air: one extra jump per level." },
	{ 72, L"Kills have a 1 in 400 chance to drop the mob's spawn egg." },
	{ 73, L"Breaks a 3x3 area of blocks at once." },
	{ 74, L"Gives you night vision while worn." },
	{ 75, L"Turns lava under your feet into magma and protects you from lava damage." },

	// curses
	{ 80, L"In Survival the item cannot be taken off until you die or it breaks." },
	{ 81, L"The item vanishes when you die instead of dropping." },

	// firearms
	{ 82, L"Hold to fire continuously on guns that support it. Wears the gun out 20% faster." },
	{ 83, L"Holds more rounds in the magazine." },
	{ 84, L"Reloads 15% faster (I) or 27% faster (II)." },
	{ 85, L"Tighter bullet spread: 12, 22 or 30% less." },
	{ 86, L"More damage to unarmored targets (+12% or +20%), less to armored ones (-10% or -18%)." },
	{ 87, L"The gun wears out slower: lasts 20, 40 or 65% longer." },
};

static const wchar_t *encyFindDescription(int id)
{
	const int count = static_cast<int>(sizeof(kDescriptions) / sizeof(kDescriptions[0]));
	for (int i = 0; i < count; ++i)
	{
		if (kDescriptions[i].id == id) return kDescriptions[i].text;
	}
	return L"No description yet.";
}

// ---- what an enchantment fits --------------------------------------------------------------------------------------------
// Probed with the virtual Enchantment::canEnchant(), i.e. the rules the anvil, books and /enchant use. (The enchanting table
// itself only checks the category, so it can be a little looser for a few pickaxe-only enchantments - see engine notes.)
enum EncyProbe
{
	ENCY_PROBE_HELMET = 0,
	ENCY_PROBE_CHESTPLATE,
	ENCY_PROBE_LEGGINGS,
	ENCY_PROBE_BOOTS,
	ENCY_PROBE_SWORD,
	ENCY_PROBE_ZORO,
	ENCY_PROBE_AXE,
	ENCY_PROBE_PICKAXE,
	ENCY_PROBE_SHOVEL,
	ENCY_PROBE_HOE,
	ENCY_PROBE_SHEARS,
	ENCY_PROBE_BOW,
	ENCY_PROBE_ROD,
	ENCY_PROBE_FIREARM,
	ENCY_PROBE_MACE,		// neoLegacy - Mace
	ENCY_PROBE_COUNT
};

static int encyProbeItemId(int probe)
{
	switch (probe)
	{
	case ENCY_PROBE_HELMET:		return Item::diamond_helmet_Id;
	case ENCY_PROBE_CHESTPLATE:	return Item::diamond_chestplate_Id;
	case ENCY_PROBE_LEGGINGS:	return Item::diamond_leggings_Id;
	case ENCY_PROBE_BOOTS:		return Item::diamond_boots_Id;
	case ENCY_PROBE_SWORD:		return Item::diamond_sword_Id;
	case ENCY_PROBE_ZORO:		return Item::wado_ichimonji_Id;
	case ENCY_PROBE_AXE:			return Item::diamond_axe_Id;
	case ENCY_PROBE_PICKAXE:		return Item::diamond_pickaxe_Id;
	case ENCY_PROBE_SHOVEL:		return Item::diamond_shovel_Id;
	case ENCY_PROBE_HOE:			return Item::diamond_hoe_Id;
	case ENCY_PROBE_SHEARS:		return Item::shears_Id;
	case ENCY_PROBE_BOW:			return Item::bow_Id;
	case ENCY_PROBE_ROD:			return Item::fishing_rod_Id;
	case ENCY_PROBE_FIREARM:		return Item::mac10_Id;
	case ENCY_PROBE_MACE:		return Item::mace_Id;
	}
	return 0;
}

static void encyProbeFits(Enchantment *e, bool fits[ENCY_PROBE_COUNT])
{
	for (int i = 0; i < ENCY_PROBE_COUNT; ++i)
	{
		fits[i] = false;
		Item *item = Item::items[encyProbeItemId(i)];
		if (item == nullptr) continue;
		shared_ptr<ItemInstance> stack = std::make_shared<ItemInstance>(item, 1, 0);
		fits[i] = e->canEnchant(stack);
	}
}

static wstring encyDescribeFits(const bool fits[ENCY_PROBE_COUNT])
{
	int matched = 0;
	for (int i = 0; i < ENCY_PROBE_COUNT; ++i) if (fits[i]) ++matched;

	if (matched == 0) return L"Special items";
	if (matched == ENCY_PROBE_COUNT) return L"Any durable item";

	vector<wstring> parts;

	if (fits[ENCY_PROBE_HELMET] && fits[ENCY_PROBE_CHESTPLATE] && fits[ENCY_PROBE_LEGGINGS] && fits[ENCY_PROBE_BOOTS])
	{
		parts.push_back(L"All armor");
	}
	else
	{
		if (fits[ENCY_PROBE_HELMET])		parts.push_back(L"Helmet");
		if (fits[ENCY_PROBE_CHESTPLATE])	parts.push_back(L"Chestplate");
		if (fits[ENCY_PROBE_LEGGINGS])	parts.push_back(L"Leggings");
		if (fits[ENCY_PROBE_BOOTS])		parts.push_back(L"Boots");
	}

	// Zoro swords are swords, so when "Sword" fits there is no need to mention them separately.
	if (fits[ENCY_PROBE_SWORD])			parts.push_back(L"Sword");
	else if (fits[ENCY_PROBE_ZORO])		parts.push_back(L"Zoro Sword");

	if (fits[ENCY_PROBE_AXE])			parts.push_back(L"Axe");
	if (fits[ENCY_PROBE_PICKAXE])		parts.push_back(L"Pickaxe");
	if (fits[ENCY_PROBE_SHOVEL])			parts.push_back(L"Shovel");
	if (fits[ENCY_PROBE_HOE])			parts.push_back(L"Hoe");
	if (fits[ENCY_PROBE_SHEARS])			parts.push_back(L"Shears");
	if (fits[ENCY_PROBE_BOW])			parts.push_back(L"Bow");
	if (fits[ENCY_PROBE_ROD])			parts.push_back(L"Fishing Rod");
	if (fits[ENCY_PROBE_FIREARM])		parts.push_back(L"Firearm");
	if (fits[ENCY_PROBE_MACE])			parts.push_back(L"Mace");

	wstring result;
	for (size_t i = 0; i < parts.size(); ++i)
	{
		if (i > 0) result += L", ";
		result += parts[i];
	}
	return result;
}

// ---- sections ------------------------------------------------------------------------------------------------------------
enum EncySection
{
	ENCY_SECTION_ARMOR = 0,
	ENCY_SECTION_MELEE,
	ENCY_SECTION_TOOLS,
	ENCY_SECTION_BOWS,
	ENCY_SECTION_FISHING,
	ENCY_SECTION_FIREARMS,
	ENCY_SECTION_GENERAL,
	ENCY_SECTION_CURSES,
	ENCY_SECTION_OTHER,
	ENCY_SECTION_COUNT
};

static const wchar_t *encySectionTitle(int section)
{
	switch (section)
	{
	case ENCY_SECTION_ARMOR:		return L"Armor";
	case ENCY_SECTION_MELEE:		return L"Melee";
	case ENCY_SECTION_TOOLS:		return L"Tools";
	case ENCY_SECTION_BOWS:		return L"Bows";
	case ENCY_SECTION_FISHING:	return L"Fishing";
	case ENCY_SECTION_FIREARMS:	return L"Firearms";
	case ENCY_SECTION_GENERAL:	return L"General";
	case ENCY_SECTION_CURSES:	return L"Curses";
	}
	return L"Other";
}

// Grouped by what the enchantment actually fits, so a new enchantment lands in the right section without any extra code.
static int encyChooseSection(Enchantment *e, const bool fits[ENCY_PROBE_COUNT])
{
	if (e->isCurse()) return ENCY_SECTION_CURSES;

	int matched = 0;
	for (int i = 0; i < ENCY_PROBE_COUNT; ++i) if (fits[i]) ++matched;
	if (matched >= 10) return ENCY_SECTION_GENERAL;	// fits most gear (Mending, Unbreaking ...)

	if (fits[ENCY_PROBE_FIREARM] && matched == 1) return ENCY_SECTION_FIREARMS;
	if (fits[ENCY_PROBE_HELMET] || fits[ENCY_PROBE_CHESTPLATE] || fits[ENCY_PROBE_LEGGINGS] || fits[ENCY_PROBE_BOOTS]) return ENCY_SECTION_ARMOR;
	if (fits[ENCY_PROBE_SWORD] || fits[ENCY_PROBE_ZORO] || fits[ENCY_PROBE_MACE]) return ENCY_SECTION_MELEE;
	if (fits[ENCY_PROBE_AXE] || fits[ENCY_PROBE_PICKAXE] || fits[ENCY_PROBE_SHOVEL] || fits[ENCY_PROBE_HOE] || fits[ENCY_PROBE_SHEARS]) return ENCY_SECTION_TOOLS;
	if (fits[ENCY_PROBE_BOW]) return ENCY_SECTION_BOWS;
	if (fits[ENCY_PROBE_ROD]) return ENCY_SECTION_FISHING;
	if (fits[ENCY_PROBE_FIREARM]) return ENCY_SECTION_FIREARMS;
	return ENCY_SECTION_OTHER;
}

// ---- text helpers --------------------------------------------------------------------------------------------------------
static wstring encyTrimRight(wstring s)
{
	while (!s.empty() && s[s.size() - 1] == L' ') s.erase(s.size() - 1);
	return s;
}

// The enchantment's display name without a trailing level ("Sharpness", not "Sharpness I"). Uses getFullname() so the custom
// names Cameron's enchantments override there come through unchanged.
static wstring encyBaseName(Enchantment *e)
{
	wstring n = e->getFullname(1).text;
	if (e->getMaxLevel() > 1)
	{
		wstring suffix = wstring(L" ") + e->getLevelString(1);
		if (n.size() >= suffix.size() && n.compare(n.size() - suffix.size(), suffix.size(), suffix) == 0)
		{
			n.erase(n.size() - suffix.size());
		}
	}
	return encyTrimRight(n);
}

// Greedy word wrap into lines of at most ENCY_CHARS_PER_LINE characters. Continuation lines start with `indent`.
static void encyWrapInto(const wstring &text, vector<wstring> &lines, const wstring &indent = L"")
{
	const int width = ENCY_CHARS_PER_LINE;
	const int maxWord = width - static_cast<int>(indent.size());

	wstring current;
	wstring word;
	bool firstLine = true;

	for (size_t i = 0; i <= text.size(); ++i)
	{
		wchar_t c = (i < text.size()) ? text[i] : L' ';
		if (c != L' ')
		{
			word += c;
			continue;
		}
		if (word.empty()) continue;

		// A single word longer than a line is split.
		while (static_cast<int>(word.size()) > maxWord)
		{
			if (!current.empty())
			{
				lines.push_back(current);
				current.clear();
				firstLine = false;
			}
			lines.push_back((firstLine ? wstring() : indent) + word.substr(0, maxWord));
			firstLine = false;
			word.erase(0, maxWord);
		}
		if (word.empty()) continue;

		if (current.empty())
		{
			current = (firstLine ? wstring() : indent) + word;
		}
		else if (static_cast<int>(current.size() + 1 + word.size()) <= width)
		{
			current += L" ";
			current += word;
		}
		else
		{
			lines.push_back(current);
			firstLine = false;
			current = indent + word;
		}
		word.clear();
	}
	if (!current.empty()) lines.push_back(current);
}

static wstring encyJoinLines(const vector<wstring> &lines)
{
	wstring result;
	for (size_t i = 0; i < lines.size(); ++i)
	{
		if (i > 0) result += ENCY_LINE_BREAK;
		result += lines[i];
	}
	return result;
}

static wstring encyRarityLine(Enchantment *e)
{
	int f = e->getFrequency();
	const wchar_t *rarity = L"Very rare";
	if (f >= Enchantment::FREQ_COMMON) rarity = L"Common";
	else if (f >= Enchantment::FREQ_UNCOMMON) rarity = L"Uncommon";
	else if (f >= Enchantment::FREQ_RARE) rarity = L"Rare";

	const wchar_t *source = L"table";
	if (e->isCurse()) source = L"curse";
	else if (e->isTreasureEnchantment()) source = L"treasure";

	return wstring(rarity) + L" (" + source + L")";
}

static void encyCollectConflicts(Enchantment *e, vector<wstring> &names)
{
	for (unsigned int i = 0; i < Enchantment::enchantments.length; ++i)
	{
		Enchantment *other = Enchantment::enchantments[i];
		if (other == nullptr || other == e) continue;
		if (!e->isCompatibleWith(other) || !other->isCompatibleWith(e))
		{
			names.push_back(encyBaseName(other));
		}
	}
}

// ---- entries -------------------------------------------------------------------------------------------------------------
struct EncyEntry
{
	Enchantment *enchantment;
	int section;
	wstring name;
	vector<wstring> pages;	// one page normally, more if the entry is long
	int startPage;			// 1-based page number of the first page of this entry
};

static void encyBuildEntryLines(Enchantment *e, const bool fits[ENCY_PROBE_COUNT], const wstring &name, vector<wstring> &lines)
{
	encyWrapInto(name, lines);

	int maxLevel = e->getMaxLevel();
	if (maxLevel > 1) encyWrapInto(wstring(L"Max level: ") + e->getLevelString(maxLevel), lines);
	else lines.push_back(L"Single level");

	encyWrapInto(encyRarityLine(e), lines);
	encyWrapInto(wstring(L"Fits: ") + encyDescribeFits(fits), lines, L"  ");

	vector<wstring> conflicts;
	encyCollectConflicts(e, conflicts);
	if (!conflicts.empty())
	{
		wstring list;
		const size_t shown = (conflicts.size() > 3) ? 3 : conflicts.size();
		for (size_t i = 0; i < shown; ++i)
		{
			if (i > 0) list += L", ";
			list += conflicts[i];
		}
		if (conflicts.size() > shown) list += L" +" + to_wstring(conflicts.size() - shown) + L" more";
		encyWrapInto(wstring(L"Not with: ") + list, lines, L"  ");
	}

	lines.push_back(L"");
	encyWrapInto(encyFindDescription(e->id), lines);
}

static void encyPaginateEntry(const wstring &name, const vector<wstring> &lines, vector<wstring> &pagesOut)
{
	size_t pos = 0;
	bool first = true;
	while (pos < lines.size())
	{
		vector<wstring> pageLines;
		if (!first)
		{
			encyWrapInto(wstring(L"(") + name + L", cont.)", pageLines);
			// do not start a continuation page with a blank line
			while (pos < lines.size() && lines[pos].empty()) ++pos;
		}
		while (pos < lines.size() && static_cast<int>(pageLines.size()) < ENCY_LINES_PER_PAGE)
		{
			pageLines.push_back(lines[pos]);
			++pos;
		}
		pagesOut.push_back(encyJoinLines(pageLines));
		first = false;
	}
}

// ---- contents ------------------------------------------------------------------------------------------------------------
struct EncyTocLine
{
	wstring text;
	bool header;
};

static void encyBuildTocPages(const vector<EncyEntry> &entries, vector<wstring> &pagesOut)
{
	vector<EncyTocLine> toc;
	for (int section = 0; section < ENCY_SECTION_COUNT; ++section)
	{
		bool any = false;
		for (size_t i = 0; i < entries.size(); ++i)
		{
			if (entries[i].section != section) continue;
			if (!any)
			{
				EncyTocLine h;
				h.text = wstring(L"- ") + encySectionTitle(section) + L" -";
				h.header = true;
				toc.push_back(h);
				any = true;
			}
			vector<wstring> wrapped;
			encyWrapInto(wstring(L"p") + to_wstring(entries[i].startPage) + L" " + entries[i].name, wrapped, L"    ");
			for (size_t w = 0; w < wrapped.size(); ++w)
			{
				EncyTocLine l;
				l.text = wrapped[w];
				l.header = false;
				toc.push_back(l);
			}
		}
	}

	vector<wstring> pageLines;
	for (size_t i = 0; i < toc.size(); ++i)
	{
		const int used = static_cast<int>(pageLines.size());
		// start a new page when full, and never leave a section title alone at the bottom of a page
		const bool needNewPage = (used >= ENCY_LINES_PER_PAGE) || (toc[i].header && used >= ENCY_LINES_PER_PAGE - 1 && used > 0);
		if (needNewPage)
		{
			pagesOut.push_back(encyJoinLines(pageLines));
			pageLines.clear();
		}
		pageLines.push_back(toc[i].text);
	}
	if (!pageLines.empty()) pagesOut.push_back(encyJoinLines(pageLines));
	if (pagesOut.empty()) pagesOut.push_back(L"");
}

// ---- public --------------------------------------------------------------------------------------------------------------
vector<wstring> EnchantmentEncyclopedia::buildPages()
{
	// 1. gather every registered enchantment (array index == id, so this is already in id order)
	vector<EncyEntry> all;
	for (unsigned int i = 0; i < Enchantment::enchantments.length; ++i)
	{
		Enchantment *e = Enchantment::enchantments[i];
		if (e == nullptr) continue;

		bool fits[ENCY_PROBE_COUNT];
		encyProbeFits(e, fits);

		EncyEntry entry;
		entry.enchantment = e;
		entry.section = encyChooseSection(e, fits);
		entry.name = encyBaseName(e);
		entry.startPage = 0;

		vector<wstring> lines;
		encyBuildEntryLines(e, fits, entry.name, lines);
		encyPaginateEntry(entry.name, lines, entry.pages);

		all.push_back(entry);
	}

	// 2. order by section (ids stay in order inside a section)
	vector<EncyEntry> entries;
	for (int section = 0; section < ENCY_SECTION_COUNT; ++section)
	{
		for (size_t i = 0; i < all.size(); ++i)
		{
			if (all[i].section == section) entries.push_back(all[i]);
		}
	}

	// 3. the contents page count depends on the page numbers it prints, so settle it by iterating (converges in 1-2 passes)
	int tocPageCount = 1;
	vector<wstring> tocPages;
	for (int pass = 0; pass < 6; ++pass)
	{
		int page = 1 + tocPageCount + 1;	// intro page + contents pages, then the first entry
		for (size_t i = 0; i < entries.size(); ++i)
		{
			entries[i].startPage = page;
			page += static_cast<int>(entries[i].pages.size());
		}

		tocPages.clear();
		encyBuildTocPages(entries, tocPages);
		if (static_cast<int>(tocPages.size()) == tocPageCount) break;
		tocPageCount = static_cast<int>(tocPages.size());
	}

	// 4. intro page
	vector<wstring> intro;
	intro.push_back(L"Enchantment");
	intro.push_back(L"Encyclopedia");
	intro.push_back(L"");
	encyWrapInto(to_wstring(entries.size()) + L" enchantments", intro);
	intro.push_back(L"");
	encyWrapInto(L"Contents come first, then one entry per enchantment.", intro);

	// 5. assemble
	vector<wstring> pages;
	pages.push_back(encyJoinLines(intro));
	for (size_t i = 0; i < tocPages.size(); ++i) pages.push_back(tocPages[i]);
	for (size_t i = 0; i < entries.size(); ++i)
	{
		for (size_t p = 0; p < entries[i].pages.size(); ++p) pages.push_back(entries[i].pages[p]);
	}
	return pages;
}

shared_ptr<ItemInstance> EnchantmentEncyclopedia::createBook()
{
	// A temporary Written Book (id 387): the stock book reader opens signed books read-only, and the server only accepts book
	// edits for Writable Books, so this can never be saved back over the real Encyclopedia item.
	// (plain new, like the creative menu's ITEM() macro: Item::written_book_Id is an in-class constant, and make_shared would
	// take it by reference and need an out-of-class definition)
	shared_ptr<ItemInstance> book = shared_ptr<ItemInstance>(new ItemInstance(Item::written_book_Id, 1, 0));

	CompoundTag *root = new CompoundTag(L"");
	ListTag<Tag> *pagesTag = new ListTag<Tag>(L"pages");

	vector<wstring> pages = buildPages();
	for (size_t i = 0; i < pages.size(); ++i)
	{
		CompoundTag *page = new CompoundTag(L"");
		page->putString(L"text", pages[i]);
		pagesTag->add(page);
	}

	root->put(L"pages", pagesTag);
	root->putString(L"title", L"Enchantment Encyclopedia");
	root->putString(L"author", L"neoLegacy");
	book->setTag(root);
	return book;
}
