#pragma once

class ItemInstance;

// neoLegacy - builds the Enchantment Encyclopedia book.
//
// The book is generated every time it is opened from Enchantment::enchantments, so a newly registered enchantment shows up
// automatically (with a generic "no description yet" entry until you add its text to kDescriptions in EnchantmentEncyclopedia.cpp).
// Facts that can be read from the code are never typed by hand: max level, rarity, table/treasure/curse, what the enchantment
// fits (probed with the virtual canEnchant()) and what it conflicts with (probed with isCompatibleWith()).
//
// The result is a temporary Written Book (id 387) so the stock read-only book reader can display it unchanged.
class EnchantmentEncyclopedia
{
public:
	// A temporary signed book (title, author and "pages") ready to hand to app.LoadWritingBookMenu(..., editable = false).
	static shared_ptr<ItemInstance> createBook();

	// The page texts, lines separated by '\r' (the book reader's line break). Exposed so the layout can be inspected or logged.
	static vector<wstring> buildPages();
};
