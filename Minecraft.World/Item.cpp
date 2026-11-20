#include "stdafx.h"
#include "IceWandItem.h"
#include "OminousBottleItem.h"
#include "CrossbowItem.h"

#include "net.minecraft.locale.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "BambooItem.h"
#include "net.minecraft.world.item.alchemy.h"
#include "net.minecraft.world.food.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.stats.h"
#include "MapItem.h"
#include "WrittenBook.h"
#include "WritingBookItem.h"
#include "Item.h"
#include "HangingEntityItem.h"
#include "HtmlString.h"
#include "ElytraItem.h"
#include "Mac10Item.h"
#include "MaceItem.h"
#include "EnchantmentEncyclopediaItem.h"
#include "RoseGoldFoodItem.h"	// neoLegacy - Copper Age (Phase 6)

typedef Item::Tier _Tier;

//const UUID Item::BASE_ATTACK_DAMAGE_UUID = UUID::fromString(L"CB3F55D3-645C-4F38-A497-9C13A33DB5CF");

wstring Item::ICON_DESCRIPTION_PREFIX = L"item.";

const _Tier *_Tier::WOOD = new _Tier(0, 59, 2, 0, 15); //
const _Tier *_Tier::STONE = new _Tier(1, 131, 4, 1, 5); //
const _Tier *_Tier::IRON = new _Tier(2, 250, 6, 2, 14); //
const _Tier *_Tier::DIAMOND = new _Tier(3, 1561, 8, 3, 10); //
const _Tier *_Tier::GOLD = new _Tier(0, 32, 12, 0, 22);
const _Tier *_Tier::ZORO = new _Tier(4, 2031, 9, 5, 18); // neoLegacy - Zoro sword tier
const _Tier *_Tier::JDMON = new _Tier(5, 2500, 10, 4, 30); // neoLegacy - JDemon sword tier (damage 4: one above diamond)
const _Tier *_Tier::CMON = new _Tier(5, 2500, 10, 4, 30); // neoLegacy - CMonster sword tier (damage 4: one above diamond)
const _Tier *_Tier::COPPER = new _Tier(1, 190, 5, 1, 13); // neoLegacy - Copper Age (Phase 2)
const _Tier *_Tier::ROSE_GOLD = new _Tier(2, 466, 10, 2, 45); // neoLegacy - Copper Age (Phase 2): level 2 = mines what iron can

Random *Item::random = new Random();

ItemArray Item::items = ItemArray( ITEM_NUM_COUNT );

Item *Item::iron_shovel = nullptr;
Item *Item::iron_pickaxe = nullptr;
Item *Item::iron_axe = nullptr;
Item *Item::flint_and_steel = nullptr;
Item *Item::apple = nullptr;
BowItem *Item::bow = nullptr;
Item *Item::arrow = nullptr;
Item *Item::coal = nullptr;
Item *Item::diamond = nullptr;
Item *Item::iron_ingot = nullptr;
Item *Item::gold_ingot = nullptr;
Item *Item::copper_ingot = nullptr;
Item *Item::copper_nugget = nullptr;
Item *Item::raw_copper = nullptr;
Item *Item::raw_rose_gold = nullptr;
Item *Item::rose_gold_ingot = nullptr;
Item *Item::rose_gold_nugget = nullptr;
Item *Item::iron_sword = nullptr;

Item *Item::wooden_sword = nullptr;
Item *Item::wooden_shovel = nullptr;
Item *Item::wooden_pickaxe = nullptr;
Item *Item::wooden_axe = nullptr;

Item *Item::stone_sword = nullptr;
Item *Item::stone_shovel = nullptr;
Item *Item::stone_pickaxe = nullptr;
Item *Item::stone_axe = nullptr;

Item *Item::diamond_sword = nullptr;

// neoLegacy - Zoro swords
Item *Item::wado_ichimonji = nullptr;
Item *Item::sandai_kitetsu = nullptr;
Item *Item::enma = nullptr;
Item *Item::shusui = nullptr;
Item *Item::jdemon_sword = nullptr;	// neoLegacy - JDemon sword
Item *Item::cmonster_sword = nullptr;	// neoLegacy - CMonster sword
Item *Item::cursed_hilt = nullptr;
Item *Item::mac10 = nullptr;
Item *Item::mace = nullptr;		// neoLegacy - Mace
Item *Item::jelly_ball = nullptr;	// neoLegacy - Jelly Ball
Item *Item::ice_ball = nullptr;	// neoLegacy - Ice Ball
Item *Item::ice_wand = nullptr;	// neoLegacy - Ice Wand
Item *Item::crossbow = nullptr;	// neoLegacy
Item *Item::crossbow_loaded = nullptr;
Item *Item::crossbow_loaded_firework = nullptr;
Item *Item::ominous_bottle = nullptr;
Item *Item::bullet_9mm = nullptr;
Item *Item::m16 = nullptr;
Item *Item::bullet_556 = nullptr;
Item *Item::flintlock_copper = nullptr;
Item *Item::flintlock_iron = nullptr;
Item *Item::flintlock_gold = nullptr;
Item *Item::enchantment_encyclopedia = nullptr;
Item *Item::bamboo = nullptr;
Item *Item::bamboo_door = nullptr;
Item *Item::diamond_shovel = nullptr;
Item *Item::diamond_pickaxe = nullptr;
Item *Item::diamond_axe = nullptr;

Item *Item::stick = nullptr;
Item *Item::bowl = nullptr;
Item *Item::mushroom_stew = nullptr;

Item *Item::golden_sword = nullptr;
Item *Item::golden_shovel = nullptr;
Item *Item::golden_pickaxe = nullptr;
Item *Item::golden_axe = nullptr;

Item *Item::copper_sword = nullptr;
Item *Item::copper_shovel = nullptr;
Item *Item::copper_pickaxe = nullptr;
Item *Item::copper_axe = nullptr;

Item *Item::rose_gold_sword = nullptr;
Item *Item::rose_gold_shovel = nullptr;
Item *Item::rose_gold_pickaxe = nullptr;
Item *Item::rose_gold_axe = nullptr;

Item *Item::string = nullptr;
Item *Item::feather = nullptr;
Item *Item::gunpowder = nullptr;

Item *Item::wooden_hoe = nullptr;
Item *Item::stone_hoe = nullptr;
Item *Item::iron_hoe = nullptr;
Item *Item::diamond_hoe = nullptr;
Item *Item::golden_hoe = nullptr;

Item *Item::copper_hoe = nullptr;
Item *Item::rose_gold_hoe = nullptr;

Item *Item::wheat_seeds = nullptr;
Item *Item::wheat = nullptr;
Item *Item::bread = nullptr;

ArmorItem *Item::leather_helmet = nullptr;
ArmorItem *Item::leather_chestplate = nullptr;
ArmorItem *Item::leather_leggings = nullptr;
ArmorItem *Item::leather_boots = nullptr;

ArmorItem *Item::chainmail_helmet = nullptr;
ArmorItem *Item::chainmail_chestplate = nullptr;
ArmorItem *Item::chainmail_leggings = nullptr;
ArmorItem *Item::chainmail_boots = nullptr;

ArmorItem *Item::iron_helmet = nullptr;
ArmorItem *Item::iron_chestplate = nullptr;
ArmorItem *Item::iron_leggings = nullptr;
ArmorItem *Item::iron_boots = nullptr;

ArmorItem *Item::diamond_helmet = nullptr;
ArmorItem *Item::diamond_chestplate = nullptr;
ArmorItem *Item::diamond_leggings = nullptr;
ArmorItem *Item::diamond_boots = nullptr;

ArmorItem *Item::golden_helmet = nullptr;
ArmorItem *Item::golden_chestplate = nullptr;
ArmorItem *Item::golden_leggings = nullptr;
ArmorItem *Item::golden_boots = nullptr;

ArmorItem *Item::copper_helmet = nullptr;
ArmorItem *Item::copper_chestplate = nullptr;
ArmorItem *Item::copper_leggings = nullptr;
ArmorItem *Item::copper_boots = nullptr;

ArmorItem *Item::rose_gold_helmet = nullptr;
ArmorItem *Item::rose_gold_chestplate = nullptr;
ArmorItem *Item::rose_gold_leggings = nullptr;
ArmorItem *Item::rose_gold_boots = nullptr;

Item *Item::flint = nullptr;
Item *Item::raw_porkchop = nullptr;
Item *Item::cooked_porkchop = nullptr;
Item *Item::painting = nullptr;

Item *Item::golden_apple = nullptr;

Item *Item::sign = nullptr;
Item *Item::wooden_door = nullptr;
Item *Item::copper_door = nullptr;	// neoLegacy - Copper Age (Phase 3)
Item *Item::rose_gold_door = nullptr;	// neoLegacy - Copper Age (Phase 3)
Item *Item::honeycomb = nullptr;	// neoLegacy - Copper Age (Phase 4)
Item *Item::rose_gilded_beetroot = nullptr;	// neoLegacy - Copper Age (Phase 6)
Item *Item::rose_gilded_mushroom = nullptr;	// neoLegacy - Copper Age (Phase 6)
Item *Item::rose_gilded_cookie = nullptr;	// neoLegacy - Copper Age (Phase 6)
Item *Item::rose_full_jelly = nullptr;	// neoLegacy - Copper Age (Phase 6)
Item *Item::rose_half_jelly = nullptr;	// neoLegacy - Copper Age (Phase 6)
Item *Item::rose_mushroom_omelette = nullptr;	// neoLegacy - Copper Age (Phase 6)
Item *Item::rose_mushroom_partial = nullptr;	// neoLegacy - Copper Age (Phase 6)
Item *Item::rose_mushroom_crumbs = nullptr;	// neoLegacy - Copper Age (Phase 6)
Item *Item::rosey_beetroot_stew = nullptr;	// neoLegacy - Copper Age (Phase 6)

Item *Item::bucket = nullptr;
Item *Item::water_bucket = nullptr;
Item *Item::lava_bucket = nullptr;

Item *Item::minecart = nullptr;
Item *Item::saddle = nullptr;
Item *Item::iron_door = nullptr;
Item *Item::redstone = nullptr;
Item *Item::snowball = nullptr;

Item *Item::boat = nullptr;

Item *Item::leather = nullptr;
Item *Item::milk_bucket = nullptr;
Item *Item::brick = nullptr;
Item *Item::clay = nullptr;
Item *Item::reeds = nullptr;
Item *Item::paper = nullptr;
Item *Item::book = nullptr;
Item *Item::slime_ball = nullptr;
Item *Item::chest_minecart = nullptr;
Item *Item::furnace_minecart = nullptr;
Item *Item::egg = nullptr;
Item *Item::compass = nullptr;
FishingRodItem *Item::fishing_rod = nullptr;
Item *Item::clock = nullptr;
Item *Item::glowstone_dust = nullptr;
Item *Item::raw_fish = nullptr;
Item *Item::cooked_fish = nullptr;

Item *Item::dye = nullptr;
Item *Item::bone = nullptr;
Item *Item::sugar = nullptr;
Item *Item::cake = nullptr;

Item *Item::bed = nullptr;

Item *Item::repeater = nullptr;
Item *Item::cookie = nullptr;

MapItem *Item::map = nullptr;

Item *Item::record_01 = nullptr;
Item *Item::record_02 = nullptr;
Item *Item::record_03 = nullptr;
Item *Item::record_04 = nullptr;
Item *Item::record_05 = nullptr;
Item *Item::record_06 = nullptr;
Item *Item::record_07 = nullptr;
Item *Item::record_08 = nullptr;
Item *Item::record_09 = nullptr;
Item *Item::record_10 = nullptr;
Item *Item::record_11 = nullptr;
Item *Item::record_12 = nullptr;

ShearsItem *Item::shears = nullptr;

Item *Item::melon = nullptr;

Item *Item::seeds_pumpkin = nullptr;
Item *Item::seeds_melon = nullptr;

Item *Item::raw_beef = nullptr;
Item *Item::cooked_beef = nullptr;
Item *Item::raw_chicken = nullptr;
Item *Item::cooked_chicken = nullptr;
Item *Item::rotten_flesh = nullptr;

Item *Item::ender_pearl = nullptr;

Item *Item::blaze_rod = nullptr;
Item *Item::ghast_tear = nullptr;
Item *Item::gold_nugget = nullptr;
Item *Item::netherwart_seeds = nullptr;
PotionItem *Item::potion = nullptr;
Item *Item::glassBottle = nullptr;
Item *Item::spider_eye = nullptr;
Item *Item::fermented_spider_eye = nullptr;
Item *Item::blaze_powder = nullptr;
Item *Item::magma_cream = nullptr;
Item *Item::brewing_stand = nullptr;
Item *Item::cauldron = nullptr;
Item *Item::eye_of_ender = nullptr;
Item *Item::speckled_melon = nullptr;

Item *Item::spawn_egg = nullptr;

Item *Item::experience_bottle = nullptr;

// TU9
Item *Item::fireball = nullptr;
Item *Item::frame = nullptr;

Item *Item::skull = nullptr;


// TU14
Item *Item::writable_book = nullptr;
Item *Item::written_book = nullptr;

Item *Item::emerald = nullptr;

Item *Item::flower_pot = nullptr;

Item *Item::carrots = nullptr;
Item *Item::potato = nullptr;
Item *Item::baked_potato = nullptr;
Item *Item::poisonous_potato = nullptr;

EmptyMapItem *Item::empty_map = nullptr;

Item *Item::golden_carrot = nullptr;

Item *Item::carrot_on_a_stick = nullptr;
Item *Item::nether_star = nullptr;
Item *Item::pumpkin_pie = nullptr;
Item *Item::fireworks = nullptr;
Item *Item::firework_charge = nullptr;

EnchantedBookItem *Item::enchanted_book = nullptr;

Item *Item::comparator = nullptr;
Item *Item::netherbrick = nullptr;
Item *Item::nether_quartz = nullptr;
Item *Item::tnt_minecart = nullptr;
Item *Item::hopper_minecart = nullptr;
Item *Item::diamond_minecart = nullptr;

Item *Item::iron_horse_armor = nullptr;
Item *Item::golden_horse_armor = nullptr;
Item *Item::diamond_horse_armor = nullptr;
Item *Item::copper_horse_armor = nullptr;
Item *Item::lead = nullptr;
Item *Item::name_tag = nullptr;

Item* Item::spruce_door = nullptr;
Item* Item::birch_door = nullptr;
Item* Item::jungle_door = nullptr;
Item* Item::acacia_door = nullptr;
Item* Item::dark_oak_door = nullptr;

//TU31
Item* Item::raw_mutton = nullptr;
Item* Item::cooked_mutton = nullptr;
Item* Item::raw_rabbit = nullptr;
Item* Item::cooked_rabbit = nullptr;
Item* Item::rabbit_foot = nullptr;
Item* Item::rabbit_hide = nullptr;
Item* Item::armor_stand = nullptr;
Item* Item::rabbit_stew = nullptr;
Item* Item::prismarine_crystal = nullptr;
Item* Item::prismarine_shard = nullptr;

Item* Item::elytra = nullptr;

Item* Item::beetroot = nullptr;
Item* Item::beetroot_seeds = nullptr;
Item* Item::beetroot_soup = nullptr;


void Item::staticCtor()
{
	Item::wooden_sword		= ( new WeaponItem(12, _Tier::WOOD) )		->setBaseItemTypeAndMaterial(eBaseItemType_sword,	eMaterial_wood)		->setIconName(L"swordWood")->setDescriptionId(IDS_ITEM_SWORD_WOOD)->setUseDescriptionId(IDS_DESC_SWORD);
	Item::stone_sword		= ( new WeaponItem(16, _Tier::STONE) )		->setBaseItemTypeAndMaterial(eBaseItemType_sword,	eMaterial_stone)	->setIconName(L"swordStone")->setDescriptionId(IDS_ITEM_SWORD_STONE)->setUseDescriptionId(IDS_DESC_SWORD);
	Item::iron_sword		= ( new WeaponItem(11, _Tier::IRON) )		->setBaseItemTypeAndMaterial(eBaseItemType_sword,	eMaterial_iron)		->setIconName(L"swordIron")->setDescriptionId(IDS_ITEM_SWORD_IRON)->setUseDescriptionId(IDS_DESC_SWORD);
	Item::diamond_sword		= ( new WeaponItem(20, _Tier::DIAMOND) )	->setBaseItemTypeAndMaterial(eBaseItemType_sword,	eMaterial_diamond)	->setIconName(L"swordDiamond")->setDescriptionId(IDS_ITEM_SWORD_DIAMOND)->setUseDescriptionId(IDS_DESC_SWORD);
	Item::golden_sword		= ( new WeaponItem(27, _Tier::GOLD) )		->setBaseItemTypeAndMaterial(eBaseItemType_sword,	eMaterial_gold)		->setIconName(L"swordGold")->setDescriptionId(IDS_ITEM_SWORD_GOLD)->setUseDescriptionId(IDS_DESC_SWORD);
	Item::copper_sword		= ( new WeaponItem(copper_sword_Id - 256, _Tier::COPPER) )	->setBaseItemTypeAndMaterial(eBaseItemType_sword,	eMaterial_copper)	->setIconName(L"copper_sword")->setDescriptionId(IDS_ITEM_SWORD_COPPER)->setUseDescriptionId(IDS_DESC_SWORD);
	Item::rose_gold_sword	= ( new WeaponItem(rose_gold_sword_Id - 256, _Tier::ROSE_GOLD) )	->setBaseItemTypeAndMaterial(eBaseItemType_sword,	eMaterial_rose_gold)	->setIconName(L"rose_gold_sword")->setDescriptionId(IDS_ITEM_SWORD_ROSE_GOLD)->setUseDescriptionId(IDS_DESC_SWORD);

	// neoLegacy - Zoro swords.  They use diamond-tier attack damage for balance;
	// their custom combat feel comes from timing, reach, angle and animation.
	Item::wado_ichimonji	= (new WeaponItem(wado_ichimonji_Id - 256, _Tier::DIAMOND, WeaponItem::ZORO_WADO_ICHIMONJI))
		->setBaseItemTypeAndMaterial(eBaseItemType_sword, eMaterial_diamond)
		->setIconName(L"swordWadoIchimonji")->setDescriptionId(IDS_ITEM_SWORD_WADO_ICHIMONJI)->setUseDescriptionId(IDS_DESC_SWORD)
		->setMaxDamage(2031);
	Item::sandai_kitetsu	= (new WeaponItem(sandai_kitetsu_Id - 256, _Tier::DIAMOND, WeaponItem::ZORO_SANDAI_KITETSU))
		->setBaseItemTypeAndMaterial(eBaseItemType_sword, eMaterial_diamond)
		->setIconName(L"swordSandaiKitetsu")->setDescriptionId(IDS_ITEM_SWORD_SANDAI_KITETSU)->setUseDescriptionId(IDS_DESC_SWORD)
		->setMaxDamage(2031);
	Item::enma	= (new WeaponItem(enma_Id - 256, _Tier::DIAMOND, WeaponItem::ZORO_ENMA))
		->setBaseItemTypeAndMaterial(eBaseItemType_sword, eMaterial_diamond)
		->setIconName(L"swordEmma")->setDescriptionId(IDS_ITEM_SWORD_EMMA)->setUseDescriptionId(IDS_DESC_SWORD)
		->setMaxDamage(2200);
	Item::shusui	= (new WeaponItem(shusui_Id - 256, _Tier::DIAMOND, WeaponItem::ZORO_SHUSUI))
		->setBaseItemTypeAndMaterial(eBaseItemType_sword, eMaterial_diamond)
		->setIconName(L"swordWadoShusui")->setDescriptionId(IDS_ITEM_SWORD_SHUSUI)->setUseDescriptionId(IDS_DESC_SWORD)
		->setMaxDamage(2100);

	// neoLegacy - JDemon / CMonster swords. Both sit one tier above diamond
	// (tier damage 4 vs diamond's 3); their identity is their on-hit and
	// on-use effects, not their stats.
	Item::jdemon_sword	= (new JDemonSword(jdemon_sword_Id - 256, _Tier::JDMON))
		->setBaseItemTypeAndMaterial(eBaseItemType_sword, eMaterial_iron)
		->setIconName(L"swordJDemon")->setDescriptionId(IDS_ITEM_SWORD_JDEMON)->setUseDescriptionId(IDS_DESC_SWORD);
	Item::cmonster_sword	= (new CMonsterSword(cmonster_sword_Id - 256, _Tier::CMON))
		->setBaseItemTypeAndMaterial(eBaseItemType_sword, eMaterial_iron)
		->setIconName(L"swordCMonster")->setDescriptionId(IDS_ITEM_SWORD_CMONSTER)->setUseDescriptionId(IDS_DESC_SWORD);

	// neoLegacy - Cursed Hilt: the shared core of every Zoro sword recipe (Complex Table only)
	Item::cursed_hilt	= (new Item(cursed_hilt_Id - 256))
		->setIconName(L"cursedHilt")->setDescriptionId(IDS_ITEM_CURSED_HILT)->setUseDescriptionId(IDS_DESC_CURSED_HILT);

	// neoLegacy - firearms. Every gun is a Mac10Item with its own GunKind (numbers live in Mac10Item.cpp). The magazine is stored in ItemInstance NBT.
	Item::mac10 = (new Mac10Item(mac10_Id - 256, GUN_MAC10))
		->setIconName(L"mac10")->setDescriptionId(IDS_ITEM_MAC10)->setUseDescriptionId(IDS_DESC_MAC10)
		->setMaxStackSize(1)->setMaxDamage(650);
	Item::bullet_9mm = (new Item(bullet_9mm_Id - 256))
		->setIconName(L"9mm_bullet")->setDescriptionId(IDS_ITEM_9MM_BULLET)->setUseDescriptionId(IDS_DESC_9MM_BULLET)
		->setMaxStackSize(64);
	Item::m16 = (new Mac10Item(m16_Id - 256, GUN_M16))
		->setIconName(L"m16")->setDescriptionId(IDS_ITEM_M16)->setUseDescriptionId(IDS_DESC_M16)
		->setMaxStackSize(1)->setMaxDamage(800);
	Item::bullet_556 = (new Item(bullet_556_Id - 256))
		->setIconName(L"556_bullet")->setDescriptionId(IDS_ITEM_556_BULLET)->setUseDescriptionId(IDS_DESC_556_BULLET)
		->setMaxStackSize(64);
	Item::flintlock_copper = (new Mac10Item(flintlock_copper_Id - 256, GUN_FLINTLOCK_COPPER))
		->setIconName(L"flintlock_copper")->setDescriptionId(IDS_ITEM_FLINTLOCK_COPPER)->setUseDescriptionId(IDS_DESC_FLINTLOCK_COPPER)
		->setMaxStackSize(1)->setMaxDamage(180);
	Item::flintlock_iron = (new Mac10Item(flintlock_iron_Id - 256, GUN_FLINTLOCK_IRON))
		->setIconName(L"flintlock_iron")->setDescriptionId(IDS_ITEM_FLINTLOCK_IRON)->setUseDescriptionId(IDS_DESC_FLINTLOCK_IRON)
		->setMaxStackSize(1)->setMaxDamage(320);
	Item::flintlock_gold = (new Mac10Item(flintlock_gold_Id - 256, GUN_FLINTLOCK_GOLD))
		->setIconName(L"flintlock_gold")->setDescriptionId(IDS_ITEM_FLINTLOCK_GOLD)->setUseDescriptionId(IDS_DESC_FLINTLOCK_GOLD)
		->setMaxStackSize(1)->setMaxDamage(90);

	// neoLegacy - Enchantment Encyclopedia: a reference book that opens a read-only book generated from the enchantment registry
	// (EnchantmentEncyclopedia.cpp). Icon is items.png row 31, col 10 (registered in PreStitchedTextureMap.cpp).
	Item::enchantment_encyclopedia = (new EnchantmentEncyclopediaItem(enchantment_encyclopedia_Id - 256))
		->setIconName(L"enchantment_encyclopedia")->setDescriptionId(IDS_ITEM_ENCHANTMENT_ENCYCLOPEDIA)->setUseDescriptionId(IDS_DESC_ENCHANTMENT_ENCYCLOPEDIA)
		->setMaxStackSize(1);

	// neoLegacy - Mace: 5 attack damage, 500 durability, smash attacks (see MaceItem.h). Icon: items.png row 31, col 11.
	Item::mace = (new MaceItem(mace_Id - 256))
		->setBaseItemTypeAndMaterial(eBaseItemType_sword, eMaterial_iron)	// listed with the swords in the Complex Table menu
		->setIconName(L"mace")->setDescriptionId(IDS_ITEM_MACE)->setUseDescriptionId(IDS_DESC_MACE);

	// neoLegacy - Jelly Ball: sticky snack dropped by the Jelly Slime. Fills you up like a cookie (2 hunger, poor saturation).
	// Icon: items.png row 31, col 12.
	Item::jelly_ball = (new FoodItem(jelly_ball_Id - 256, 2, FoodConstants::FOOD_SATURATION_POOR, false))
		->setBaseItemTypeAndMaterial(eBaseItemType_treasure, eMaterial_slime)	// same crafting-menu column as the Slime Ball
		->setIconName(L"jelly_ball")->setDescriptionId(IDS_ITEM_JELLY_BALL)->setUseDescriptionId(IDS_DESC_JELLY_BALL);

	// neoLegacy - Ice Ball: dropped by Frostbites, crafted from Ice, and the ingredient that brews a Potion of Freezing.
	// Icon: items.png row 31, col 13.
	Item::ice_ball = (new SnowballItem(ice_ball_Id - 256, true))
		->setIconName(L"ice_ball")->setDescriptionId(IDS_ITEM_ICE_BALL)->setUseDescriptionId(IDS_DESC_ICE_BALL)->setPotionBrewingFormula(PotionBrewing::MOD_ICEBALL);

	// neoLegacy - Ice Wand (from the FrozenFiend mod): right-click a creature to drop an Ice Cube on it. Icon: items.png row 31, col 14.
	Item::ice_wand = (new IceWandItem(ice_wand_Id - 256))->handEquipped()
		->setIconName(L"ice_wand")->setMaxDamage(64)->setDescriptionId(IDS_ITEM_ICE_WAND)->setUseDescriptionId(IDS_DESC_ICE_WAND);

	// neoLegacy - Crossbow: two ids, empty and loaded (the loaded one is only ever made by drawing the empty one). Icons: items.png (31,15) and (30,9).
	// In the crafting menu the crossbow shares the bow's column (base type bow), right below the bow and above the arrow.
	Item::crossbow = (new CrossbowItem(crossbow_Id - 256, false))->handEquipped()
		->setBaseItemTypeAndMaterial(eBaseItemType_bow, eMaterial_crossbow)
		->setIconName(L"crossbow_standby")->setMaxDamage(326)->setDescriptionId(IDS_ITEM_CROSSBOW)->setUseDescriptionId(IDS_DESC_CROSSBOW);
	Item::crossbow_loaded = (new CrossbowItem(crossbow_loaded_Id - 256, true))->handEquipped()
		->setIconName(L"crossbow_arrow")->setMaxDamage(326)->setDescriptionId(IDS_ITEM_CROSSBOW)->setUseDescriptionId(IDS_DESC_CROSSBOW);
	Item::ominous_bottle = (new OminousBottleItem(ominous_bottle_Id - 256))
		->setIconName(L"ominous_bottle")->setDescriptionId(IDS_ITEM_OMINOUS_BOTTLE)->setUseDescriptionId(IDS_DESC_OMINOUS_BOTTLE);
	Item::crossbow_loaded_firework = (new CrossbowItem(crossbow_loaded_firework_Id - 256, true, true))->handEquipped()
		->setIconName(L"crossbow_firework")->setMaxDamage(326)->setDescriptionId(IDS_ITEM_CROSSBOW)->setUseDescriptionId(IDS_DESC_CROSSBOW);

	Item::wooden_shovel		= ( new ShovelItem(13, _Tier::WOOD) )		->setBaseItemTypeAndMaterial(eBaseItemType_shovel,	eMaterial_wood)		->setIconName(L"shovelWood")->setDescriptionId(IDS_ITEM_SHOVEL_WOOD)->setUseDescriptionId(IDS_DESC_SHOVEL);
	Item::stone_shovel		= ( new ShovelItem(17, _Tier::STONE) )		->setBaseItemTypeAndMaterial(eBaseItemType_shovel,	eMaterial_stone)	->setIconName(L"shovelStone")->setDescriptionId(IDS_ITEM_SHOVEL_STONE)->setUseDescriptionId(IDS_DESC_SHOVEL);
	Item::iron_shovel		= ( new ShovelItem(0, _Tier::IRON) )		->setBaseItemTypeAndMaterial(eBaseItemType_shovel,	eMaterial_iron)		->setIconName(L"shovelIron")->setDescriptionId(IDS_ITEM_SHOVEL_IRON)->setUseDescriptionId(IDS_DESC_SHOVEL);
	Item::diamond_shovel	= ( new ShovelItem(21, _Tier::DIAMOND) )	->setBaseItemTypeAndMaterial(eBaseItemType_shovel,	eMaterial_diamond)	->setIconName(L"shovelDiamond")->setDescriptionId(IDS_ITEM_SHOVEL_DIAMOND)->setUseDescriptionId(IDS_DESC_SHOVEL);
	Item::golden_shovel		= ( new ShovelItem(28, _Tier::GOLD) )		->setBaseItemTypeAndMaterial(eBaseItemType_shovel,	eMaterial_gold)		->setIconName(L"shovelGold")->setDescriptionId(IDS_ITEM_SHOVEL_GOLD)->setUseDescriptionId(IDS_DESC_SHOVEL);
	Item::copper_shovel		= ( new ShovelItem(copper_shovel_Id - 256, _Tier::COPPER) )	->setBaseItemTypeAndMaterial(eBaseItemType_shovel,	eMaterial_copper)	->setIconName(L"copper_shovel")->setDescriptionId(IDS_ITEM_SHOVEL_COPPER)->setUseDescriptionId(IDS_DESC_SHOVEL);
	Item::rose_gold_shovel	= ( new ShovelItem(rose_gold_shovel_Id - 256, _Tier::ROSE_GOLD) )	->setBaseItemTypeAndMaterial(eBaseItemType_shovel,	eMaterial_rose_gold)	->setIconName(L"rose_gold_shovel")->setDescriptionId(IDS_ITEM_SHOVEL_ROSE_GOLD)->setUseDescriptionId(IDS_DESC_SHOVEL);

	Item::wooden_pickaxe		= ( new PickaxeItem(14, _Tier::WOOD) )		->setBaseItemTypeAndMaterial(eBaseItemType_pickaxe,	eMaterial_wood)		->setIconName(L"pickaxeWood")->setDescriptionId(IDS_ITEM_PICKAXE_WOOD)->setUseDescriptionId(IDS_DESC_PICKAXE);
	Item::stone_pickaxe		= ( new PickaxeItem(18, _Tier::STONE) )		->setBaseItemTypeAndMaterial(eBaseItemType_pickaxe,	eMaterial_stone)	->setIconName(L"pickaxeStone")->setDescriptionId(IDS_ITEM_PICKAXE_STONE)->setUseDescriptionId(IDS_DESC_PICKAXE);
	Item::iron_pickaxe		= ( new PickaxeItem(1, _Tier::IRON) )		->setBaseItemTypeAndMaterial(eBaseItemType_pickaxe,	eMaterial_iron)		->setIconName(L"pickaxeIron")->setDescriptionId(IDS_ITEM_PICKAXE_IRON)->setUseDescriptionId(IDS_DESC_PICKAXE);
	Item::diamond_pickaxe	= ( new PickaxeItem(22, _Tier::DIAMOND) )	->setBaseItemTypeAndMaterial(eBaseItemType_pickaxe,	eMaterial_diamond)	->setIconName(L"pickaxeDiamond")->setDescriptionId(IDS_ITEM_PICKAXE_DIAMOND)->setUseDescriptionId(IDS_DESC_PICKAXE);
	Item::golden_pickaxe		= ( new PickaxeItem(29, _Tier::GOLD) )		->setBaseItemTypeAndMaterial(eBaseItemType_pickaxe,	eMaterial_gold)		->setIconName(L"pickaxeGold")->setDescriptionId(IDS_ITEM_PICKAXE_GOLD)->setUseDescriptionId(IDS_DESC_PICKAXE);
	Item::copper_pickaxe		= ( new PickaxeItem(copper_pickaxe_Id - 256, _Tier::COPPER) )	->setBaseItemTypeAndMaterial(eBaseItemType_pickaxe,	eMaterial_copper)	->setIconName(L"copper_pickaxe")->setDescriptionId(IDS_ITEM_PICKAXE_COPPER)->setUseDescriptionId(IDS_DESC_PICKAXE);
	Item::rose_gold_pickaxe	= ( new PickaxeItem(rose_gold_pickaxe_Id - 256, _Tier::ROSE_GOLD) )	->setBaseItemTypeAndMaterial(eBaseItemType_pickaxe,	eMaterial_rose_gold)	->setIconName(L"rose_gold_pickaxe")->setDescriptionId(IDS_ITEM_PICKAXE_ROSE_GOLD)->setUseDescriptionId(IDS_DESC_PICKAXE);

	Item::wooden_axe		= ( new HatchetItem(15, _Tier::WOOD) )		->setBaseItemTypeAndMaterial(eBaseItemType_hatchet,	eMaterial_wood)		->setIconName(L"hatchetWood")->setDescriptionId(IDS_ITEM_HATCHET_WOOD)->setUseDescriptionId(IDS_DESC_HATCHET);
	Item::stone_axe		= ( new HatchetItem(19, _Tier::STONE) )		->setBaseItemTypeAndMaterial(eBaseItemType_hatchet,	eMaterial_stone)	->setIconName(L"hatchetStone")->setDescriptionId(IDS_ITEM_HATCHET_STONE)->setUseDescriptionId(IDS_DESC_HATCHET);
	Item::iron_axe		= ( new HatchetItem(2, _Tier::IRON) )		->setBaseItemTypeAndMaterial(eBaseItemType_hatchet,	eMaterial_iron)		->setIconName(L"hatchetIron")->setDescriptionId(IDS_ITEM_HATCHET_IRON)->setUseDescriptionId(IDS_DESC_HATCHET);
	Item::diamond_axe	= ( new HatchetItem(23, _Tier::DIAMOND) )	->setBaseItemTypeAndMaterial(eBaseItemType_hatchet,	eMaterial_diamond)	->setIconName(L"hatchetDiamond")->setDescriptionId(IDS_ITEM_HATCHET_DIAMOND)->setUseDescriptionId(IDS_DESC_HATCHET);
	Item::golden_axe		= ( new HatchetItem(30, _Tier::GOLD) )		->setBaseItemTypeAndMaterial(eBaseItemType_hatchet,	eMaterial_gold)		->setIconName(L"hatchetGold")->setDescriptionId(IDS_ITEM_HATCHET_GOLD)->setUseDescriptionId(IDS_DESC_HATCHET);
	Item::copper_axe		= ( new HatchetItem(copper_axe_Id - 256, _Tier::COPPER) )	->setBaseItemTypeAndMaterial(eBaseItemType_hatchet,	eMaterial_copper)	->setIconName(L"copper_axe")->setDescriptionId(IDS_ITEM_HATCHET_COPPER)->setUseDescriptionId(IDS_DESC_HATCHET);
	Item::rose_gold_axe	= ( new HatchetItem(rose_gold_axe_Id - 256, _Tier::ROSE_GOLD) )	->setBaseItemTypeAndMaterial(eBaseItemType_hatchet,	eMaterial_rose_gold)	->setIconName(L"rose_gold_axe")->setDescriptionId(IDS_ITEM_HATCHET_ROSE_GOLD)->setUseDescriptionId(IDS_DESC_HATCHET);

	Item::wooden_hoe			= ( new HoeItem(34, _Tier::WOOD) )			->setBaseItemTypeAndMaterial(eBaseItemType_hoe,	eMaterial_wood)		->setIconName(L"hoeWood")->setDescriptionId(IDS_ITEM_HOE_WOOD)->setUseDescriptionId(IDS_DESC_HOE);
	Item::stone_hoe			= ( new HoeItem(35, _Tier::STONE) )			->setBaseItemTypeAndMaterial(eBaseItemType_hoe,	eMaterial_stone)	->setIconName(L"hoeStone")->setDescriptionId(IDS_ITEM_HOE_STONE)->setUseDescriptionId(IDS_DESC_HOE);
	Item::iron_hoe			= ( new HoeItem(36, _Tier::IRON) )			->setBaseItemTypeAndMaterial(eBaseItemType_hoe,	eMaterial_iron)		->setIconName(L"hoeIron")->setDescriptionId(IDS_ITEM_HOE_IRON)->setUseDescriptionId(IDS_DESC_HOE);
	Item::diamond_hoe		= ( new HoeItem(37, _Tier::DIAMOND) )		->setBaseItemTypeAndMaterial(eBaseItemType_hoe,	eMaterial_diamond)	->setIconName(L"hoeDiamond")->setDescriptionId(IDS_ITEM_HOE_DIAMOND)->setUseDescriptionId(IDS_DESC_HOE);
	Item::golden_hoe			= ( new HoeItem(38, _Tier::GOLD) )			->setBaseItemTypeAndMaterial(eBaseItemType_hoe,	eMaterial_gold)		->setIconName(L"hoeGold")->setDescriptionId(IDS_ITEM_HOE_GOLD)->setUseDescriptionId(IDS_DESC_HOE);
	Item::copper_hoe			= ( new HoeItem(copper_hoe_Id - 256, _Tier::COPPER) )		->setBaseItemTypeAndMaterial(eBaseItemType_hoe,	eMaterial_copper)	->setIconName(L"copper_hoe")->setDescriptionId(IDS_ITEM_HOE_COPPER)->setUseDescriptionId(IDS_DESC_HOE);
	Item::rose_gold_hoe		= ( new HoeItem(rose_gold_hoe_Id - 256, _Tier::ROSE_GOLD) )	->setBaseItemTypeAndMaterial(eBaseItemType_hoe,	eMaterial_rose_gold)	->setIconName(L"rose_gold_hoe")->setDescriptionId(IDS_ITEM_HOE_ROSE_GOLD)->setUseDescriptionId(IDS_DESC_HOE);

	Item::wooden_door			= ( new DoorItem(68, Material::wood, L"doorWood"))->setBaseItemTypeAndMaterial(eBaseItemType_door, eMaterial_wood)->setIconName(L"doorWood")->setDescriptionId(IDS_ITEM_DOOR_WOOD)->setUseDescriptionId(IDS_DESC_DOOR_WOOD);
	Item::iron_door			= ( new DoorItem(74, Material::metal, L"doorIron"))->setBaseItemTypeAndMaterial(eBaseItemType_door, eMaterial_iron)->setIconName(L"doorIron")->setDescriptionId(IDS_ITEM_DOOR_IRON)->setUseDescriptionId(IDS_DESC_DOOR_IRON);

	Item::leather_helmet		= static_cast<ArmorItem *>((new ArmorItem(42, ArmorItem::ArmorMaterial::CLOTH, 0, ArmorItem::SLOT_HEAD))->setBaseItemTypeAndMaterial(eBaseItemType_helmet, eMaterial_cloth)->setIconName(L"helmetCloth")->setDescriptionId(IDS_ITEM_HELMET_CLOTH)->setUseDescriptionId(IDS_DESC_HELMET_LEATHER));
	Item::iron_helmet		= static_cast<ArmorItem *>((new ArmorItem(50, ArmorItem::ArmorMaterial::IRON, 2, ArmorItem::SLOT_HEAD))->setBaseItemTypeAndMaterial(eBaseItemType_helmet, eMaterial_iron)->setIconName(L"helmetIron")->setDescriptionId(IDS_ITEM_HELMET_IRON)->setUseDescriptionId(IDS_DESC_HELMET_IRON));
	Item::diamond_helmet	= static_cast<ArmorItem *>((new ArmorItem(54, ArmorItem::ArmorMaterial::DIAMOND, 3, ArmorItem::SLOT_HEAD))->setBaseItemTypeAndMaterial(eBaseItemType_helmet, eMaterial_diamond)->setIconName(L"helmetDiamond")->setDescriptionId(IDS_ITEM_HELMET_DIAMOND)->setUseDescriptionId(IDS_DESC_HELMET_DIAMOND));
	Item::golden_helmet		= static_cast<ArmorItem *>((new ArmorItem(58, ArmorItem::ArmorMaterial::GOLD, 4, ArmorItem::SLOT_HEAD))->setBaseItemTypeAndMaterial(eBaseItemType_helmet, eMaterial_gold)->setIconName(L"helmetGold")->setDescriptionId(IDS_ITEM_HELMET_GOLD)->setUseDescriptionId(IDS_DESC_HELMET_GOLD));
	Item::copper_helmet		= static_cast<ArmorItem *>((new ArmorItem(copper_helmet_Id - 256, ArmorItem::ArmorMaterial::COPPER, 5, ArmorItem::SLOT_HEAD))->setBaseItemTypeAndMaterial(eBaseItemType_helmet, eMaterial_copper)->setIconName(L"copper_helmet")->setDescriptionId(IDS_ITEM_HELMET_COPPER)->setUseDescriptionId(IDS_DESC_HELMET_COPPER));
	Item::rose_gold_helmet	= static_cast<ArmorItem *>((new ArmorItem(rose_gold_helmet_Id - 256, ArmorItem::ArmorMaterial::ROSE_GOLD, 6, ArmorItem::SLOT_HEAD))->setBaseItemTypeAndMaterial(eBaseItemType_helmet, eMaterial_rose_gold)->setIconName(L"rose_gold_helmet")->setDescriptionId(IDS_ITEM_HELMET_ROSE_GOLD)->setUseDescriptionId(IDS_DESC_HELMET_ROSE_GOLD));

	Item::leather_chestplate	= static_cast<ArmorItem *>((new ArmorItem(43, ArmorItem::ArmorMaterial::CLOTH, 0, ArmorItem::SLOT_TORSO))->setBaseItemTypeAndMaterial(eBaseItemType_chestplate, eMaterial_cloth)->setIconName(L"chestplateCloth")->setDescriptionId(IDS_ITEM_CHESTPLATE_CLOTH)->setUseDescriptionId(IDS_DESC_CHESTPLATE_LEATHER));
	Item::iron_chestplate		= static_cast<ArmorItem *>((new ArmorItem(51, ArmorItem::ArmorMaterial::IRON, 2, ArmorItem::SLOT_TORSO))->setBaseItemTypeAndMaterial(eBaseItemType_chestplate, eMaterial_iron)->setIconName(L"chestplateIron")->setDescriptionId(IDS_ITEM_CHESTPLATE_IRON)->setUseDescriptionId(IDS_DESC_CHESTPLATE_IRON));
	Item::diamond_chestplate	= static_cast<ArmorItem *>((new ArmorItem(55, ArmorItem::ArmorMaterial::DIAMOND, 3, ArmorItem::SLOT_TORSO))->setBaseItemTypeAndMaterial(eBaseItemType_chestplate, eMaterial_diamond)->setIconName(L"chestplateDiamond")->setDescriptionId(IDS_ITEM_CHESTPLATE_DIAMOND)->setUseDescriptionId(IDS_DESC_CHESTPLATE_DIAMOND));
	Item::golden_chestplate		= static_cast<ArmorItem *>((new ArmorItem(59, ArmorItem::ArmorMaterial::GOLD, 4, ArmorItem::SLOT_TORSO))->setBaseItemTypeAndMaterial(eBaseItemType_chestplate, eMaterial_gold)->setIconName(L"chestplateGold")->setDescriptionId(IDS_ITEM_CHESTPLATE_GOLD)->setUseDescriptionId(IDS_DESC_CHESTPLATE_GOLD));
	Item::copper_chestplate	= static_cast<ArmorItem *>((new ArmorItem(copper_chestplate_Id - 256, ArmorItem::ArmorMaterial::COPPER, 5, ArmorItem::SLOT_TORSO))->setBaseItemTypeAndMaterial(eBaseItemType_chestplate, eMaterial_copper)->setIconName(L"copper_chestplate")->setDescriptionId(IDS_ITEM_CHESTPLATE_COPPER)->setUseDescriptionId(IDS_DESC_CHESTPLATE_COPPER));
	Item::rose_gold_chestplate	= static_cast<ArmorItem *>((new ArmorItem(rose_gold_chestplate_Id - 256, ArmorItem::ArmorMaterial::ROSE_GOLD, 6, ArmorItem::SLOT_TORSO))->setBaseItemTypeAndMaterial(eBaseItemType_chestplate, eMaterial_rose_gold)->setIconName(L"rose_gold_chestplate")->setDescriptionId(IDS_ITEM_CHESTPLATE_ROSE_GOLD)->setUseDescriptionId(IDS_DESC_CHESTPLATE_ROSE_GOLD));

	Item::leather_leggings	= static_cast<ArmorItem *>((new ArmorItem(44, ArmorItem::ArmorMaterial::CLOTH, 0, ArmorItem::SLOT_LEGS))->setBaseItemTypeAndMaterial(eBaseItemType_leggings, eMaterial_cloth)->setIconName(L"leggingsCloth")->setDescriptionId(IDS_ITEM_LEGGINGS_CLOTH)->setUseDescriptionId(IDS_DESC_LEGGINGS_LEATHER));
	Item::iron_leggings		= static_cast<ArmorItem *>((new ArmorItem(52, ArmorItem::ArmorMaterial::IRON, 2, ArmorItem::SLOT_LEGS))->setBaseItemTypeAndMaterial(eBaseItemType_leggings, eMaterial_iron)->setIconName(L"leggingsIron")->setDescriptionId(IDS_ITEM_LEGGINGS_IRON)->setUseDescriptionId(IDS_DESC_LEGGINGS_IRON));
	Item::diamond_leggings	= static_cast<ArmorItem *>((new ArmorItem(56, ArmorItem::ArmorMaterial::DIAMOND, 3, ArmorItem::SLOT_LEGS))->setBaseItemTypeAndMaterial(eBaseItemType_leggings, eMaterial_diamond)->setIconName(L"leggingsDiamond")->setDescriptionId(IDS_ITEM_LEGGINGS_DIAMOND)->setUseDescriptionId(IDS_DESC_LEGGINGS_DIAMOND));
	Item::golden_leggings		= static_cast<ArmorItem *>((new ArmorItem(60, ArmorItem::ArmorMaterial::GOLD, 4, ArmorItem::SLOT_LEGS))->setBaseItemTypeAndMaterial(eBaseItemType_leggings, eMaterial_gold)->setIconName(L"leggingsGold")->setDescriptionId(IDS_ITEM_LEGGINGS_GOLD)->setUseDescriptionId(IDS_DESC_LEGGINGS_GOLD));
	Item::copper_leggings	= static_cast<ArmorItem *>((new ArmorItem(copper_leggings_Id - 256, ArmorItem::ArmorMaterial::COPPER, 5, ArmorItem::SLOT_LEGS))->setBaseItemTypeAndMaterial(eBaseItemType_leggings, eMaterial_copper)->setIconName(L"copper_leggings")->setDescriptionId(IDS_ITEM_LEGGINGS_COPPER)->setUseDescriptionId(IDS_DESC_LEGGINGS_COPPER));
	Item::rose_gold_leggings	= static_cast<ArmorItem *>((new ArmorItem(rose_gold_leggings_Id - 256, ArmorItem::ArmorMaterial::ROSE_GOLD, 6, ArmorItem::SLOT_LEGS))->setBaseItemTypeAndMaterial(eBaseItemType_leggings, eMaterial_rose_gold)->setIconName(L"rose_gold_leggings")->setDescriptionId(IDS_ITEM_LEGGINGS_ROSE_GOLD)->setUseDescriptionId(IDS_DESC_LEGGINGS_ROSE_GOLD));

	Item::chainmail_helmet		= static_cast<ArmorItem *>((new ArmorItem(46, ArmorItem::ArmorMaterial::CHAIN, 1, ArmorItem::SLOT_HEAD))->setBaseItemTypeAndMaterial(eBaseItemType_helmet, eMaterial_chain)->setIconName(L"helmetChain")->setDescriptionId(IDS_ITEM_HELMET_CHAIN)->setUseDescriptionId(IDS_DESC_HELMET_CHAIN));
	Item::chainmail_chestplate	= static_cast<ArmorItem *>((new ArmorItem(47, ArmorItem::ArmorMaterial::CHAIN, 1, ArmorItem::SLOT_TORSO))->setBaseItemTypeAndMaterial(eBaseItemType_chestplate, eMaterial_chain)->setIconName(L"chestplateChain")->setDescriptionId(IDS_ITEM_CHESTPLATE_CHAIN)->setUseDescriptionId(IDS_DESC_CHESTPLATE_CHAIN));
	Item::chainmail_leggings	= static_cast<ArmorItem *>((new ArmorItem(48, ArmorItem::ArmorMaterial::CHAIN, 1, ArmorItem::SLOT_LEGS))->setBaseItemTypeAndMaterial(eBaseItemType_leggings, eMaterial_chain)->setIconName(L"leggingsChain")->setDescriptionId(IDS_ITEM_LEGGINGS_CHAIN)->setUseDescriptionId(IDS_DESC_LEGGINGS_CHAIN));
	Item::chainmail_boots		= static_cast<ArmorItem *>((new ArmorItem(49, ArmorItem::ArmorMaterial::CHAIN, 1, ArmorItem::SLOT_FEET))->setBaseItemTypeAndMaterial(eBaseItemType_boots, eMaterial_chain)->setIconName(L"bootsChain")->setDescriptionId(IDS_ITEM_BOOTS_CHAIN)->setUseDescriptionId(IDS_DESC_BOOTS_CHAIN));

	Item::leather_boots		= static_cast<ArmorItem *>((new ArmorItem(45, ArmorItem::ArmorMaterial::CLOTH, 0, ArmorItem::SLOT_FEET))->setBaseItemTypeAndMaterial(eBaseItemType_boots, eMaterial_cloth)->setIconName(L"bootsCloth")->setDescriptionId(IDS_ITEM_BOOTS_CLOTH)->setUseDescriptionId(IDS_DESC_BOOTS_LEATHER));
	Item::iron_boots		= static_cast<ArmorItem *>((new ArmorItem(53, ArmorItem::ArmorMaterial::IRON, 2, ArmorItem::SLOT_FEET))->setBaseItemTypeAndMaterial(eBaseItemType_boots, eMaterial_iron)->setIconName(L"bootsIron")->setDescriptionId(IDS_ITEM_BOOTS_IRON)->setUseDescriptionId(IDS_DESC_BOOTS_IRON));
	Item::diamond_boots		= static_cast<ArmorItem *>((new ArmorItem(57, ArmorItem::ArmorMaterial::DIAMOND, 3, ArmorItem::SLOT_FEET))->setBaseItemTypeAndMaterial(eBaseItemType_boots, eMaterial_diamond)->setIconName(L"bootsDiamond")->setDescriptionId(IDS_ITEM_BOOTS_DIAMOND)->setUseDescriptionId(IDS_DESC_BOOTS_DIAMOND));
	Item::golden_boots		= static_cast<ArmorItem *>((new ArmorItem(61, ArmorItem::ArmorMaterial::GOLD, 4, ArmorItem::SLOT_FEET))->setBaseItemTypeAndMaterial(eBaseItemType_boots, eMaterial_gold)->setIconName(L"bootsGold")->setDescriptionId(IDS_ITEM_BOOTS_GOLD)->setUseDescriptionId(IDS_DESC_BOOTS_GOLD));
	Item::copper_boots		= static_cast<ArmorItem *>((new ArmorItem(copper_boots_Id - 256, ArmorItem::ArmorMaterial::COPPER, 5, ArmorItem::SLOT_FEET))->setBaseItemTypeAndMaterial(eBaseItemType_boots, eMaterial_copper)->setIconName(L"copper_boots")->setDescriptionId(IDS_ITEM_BOOTS_COPPER)->setUseDescriptionId(IDS_DESC_BOOTS_COPPER));
	Item::rose_gold_boots	= static_cast<ArmorItem *>((new ArmorItem(rose_gold_boots_Id - 256, ArmorItem::ArmorMaterial::ROSE_GOLD, 6, ArmorItem::SLOT_FEET))->setBaseItemTypeAndMaterial(eBaseItemType_boots, eMaterial_rose_gold)->setIconName(L"rose_gold_boots")->setDescriptionId(IDS_ITEM_BOOTS_ROSE_GOLD)->setUseDescriptionId(IDS_DESC_BOOTS_ROSE_GOLD));

	Item::iron_ingot = ( new Item(9) )->setIconName(L"ingotIron")					->setBaseItemTypeAndMaterial(eBaseItemType_treasure,	eMaterial_iron)->setDescriptionId(IDS_ITEM_INGOT_IRON)->setUseDescriptionId(IDS_DESC_INGOT);
	Item::gold_ingot = ( new Item(10) )->setIconName(L"ingotGold")					->setBaseItemTypeAndMaterial(eBaseItemType_treasure,	eMaterial_gold)->setDescriptionId(IDS_ITEM_INGOT_GOLD)->setUseDescriptionId(IDS_DESC_INGOT);
	Item::copper_ingot = ( new Item(Item::copper_ingot_Id - 256) )->setIconName(L"copper_ingot")					->setBaseItemTypeAndMaterial(eBaseItemType_ingot,	eMaterial_copper)->setDescriptionId(IDS_ITEM_COPPER_INGOT)->setUseDescriptionId(IDS_DESC_INGOT);
	Item::copper_nugget = ( new Item(Item::copper_nugget_Id - 256) )->setIconName(L"copper_nugget")					->setBaseItemTypeAndMaterial(eBaseItemType_ingot,	eMaterial_copper)->setDescriptionId(IDS_ITEM_COPPER_NUGGET)->setUseDescriptionId(IDS_DESC_COPPER_NUGGET);
	Item::raw_copper = ( new Item(Item::raw_copper_Id - 256) )->setIconName(L"raw_copper")					->setBaseItemTypeAndMaterial(eBaseItemType_ingot,	eMaterial_copper)->setDescriptionId(IDS_ITEM_RAW_COPPER)->setUseDescriptionId(IDS_DESC_RAW_COPPER);
	Item::raw_rose_gold = ( new Item(Item::raw_rose_gold_Id - 256) )->setIconName(L"raw_rose_gold")					->setBaseItemTypeAndMaterial(eBaseItemType_ingot,	eMaterial_rose_gold)->setDescriptionId(IDS_ITEM_RAW_ROSE_GOLD)->setUseDescriptionId(IDS_DESC_RAW_ROSE_GOLD);
	Item::rose_gold_ingot = ( new Item(Item::rose_gold_ingot_Id - 256) )->setIconName(L"rose_gold_ingot")					->setBaseItemTypeAndMaterial(eBaseItemType_ingot,	eMaterial_rose_gold)->setDescriptionId(IDS_ITEM_ROSE_GOLD_INGOT)->setUseDescriptionId(IDS_DESC_INGOT);
	Item::rose_gold_nugget = ( new Item(Item::rose_gold_nugget_Id - 256) )->setIconName(L"rose_gold_nugget")					->setBaseItemTypeAndMaterial(eBaseItemType_ingot,	eMaterial_rose_gold)->setDescriptionId(IDS_ITEM_ROSE_GOLD_NUGGET)->setUseDescriptionId(IDS_DESC_ROSE_GOLD_NUGGET);


	// 4J-PB - todo - add materials and base types to the ones below
	Item::bucket		= ( new BucketItem(69, 0) )					->setBaseItemTypeAndMaterial(eBaseItemType_utensil,	eMaterial_water)->setIconName(L"bucket")->setDescriptionId(IDS_ITEM_BUCKET)->setUseDescriptionId(IDS_DESC_BUCKET)->setMaxStackSize(16);
	Item::bowl = ( new Item(25) )										->setBaseItemTypeAndMaterial(eBaseItemType_utensil,	eMaterial_wood)->setIconName(L"bowl")->setDescriptionId(IDS_ITEM_BOWL)->setUseDescriptionId(IDS_DESC_BOWL)->setMaxStackSize(64);

	Item::water_bucket		= ( new BucketItem(70, Tile::flowing_water_Id) )	->setIconName(L"bucketWater")->setDescriptionId(IDS_ITEM_BUCKET_WATER)->setCraftingRemainingItem(Item::bucket)->setUseDescriptionId(IDS_DESC_BUCKET_WATER);
	Item::lava_bucket		= ( new BucketItem(71, Tile::flowing_lava_Id) )		->setIconName(L"bucketLava")->setDescriptionId(IDS_ITEM_BUCKET_LAVA)->setCraftingRemainingItem(Item::bucket)->setUseDescriptionId(IDS_DESC_BUCKET_LAVA);
	Item::milk_bucket				= ( new MilkBucketItem(79) )->setIconName(L"milk")->setDescriptionId(IDS_ITEM_BUCKET_MILK)->setCraftingRemainingItem(Item::bucket)->setUseDescriptionId(IDS_DESC_BUCKET_MILK);

	Item::bow = static_cast<BowItem *>((new BowItem(5))->setIconName(L"bow")->setBaseItemTypeAndMaterial(eBaseItemType_bow, eMaterial_bow)->setDescriptionId(IDS_ITEM_BOW)->setUseDescriptionId(IDS_DESC_BOW));
	Item::arrow = ( new Item(6) )													->setIconName(L"arrow")->setBaseItemTypeAndMaterial(eBaseItemType_bow,	eMaterial_arrow)	->setDescriptionId(IDS_ITEM_ARROW)->setUseDescriptionId(IDS_DESC_ARROW);

	Item::compass = ( new CompassItem(89) )											->setIconName(L"compass")->setBaseItemTypeAndMaterial(eBaseItemType_pockettool,	eMaterial_compass)		->setDescriptionId(IDS_ITEM_COMPASS)->setUseDescriptionId(IDS_DESC_COMPASS);
	Item::clock = ( new ClockItem(91) )												->setIconName(L"clock")->setBaseItemTypeAndMaterial(eBaseItemType_pockettool,	eMaterial_clock)		->setDescriptionId(IDS_ITEM_CLOCK)->setUseDescriptionId(IDS_DESC_CLOCK);
	Item::map = static_cast<MapItem *>((new MapItem(102))->setIconName(L"map")->setBaseItemTypeAndMaterial(eBaseItemType_pockettool, eMaterial_map)->setDescriptionId(IDS_ITEM_MAP)->setUseDescriptionId(IDS_DESC_MAP));

	Item::flint_and_steel = ( new FlintAndSteelItem(3) )								->setIconName(L"flint_and_steel")->setBaseItemTypeAndMaterial(eBaseItemType_devicetool,	eMaterial_flintandsteel)->setDescriptionId(IDS_ITEM_FLINT_AND_STEEL)->setUseDescriptionId(IDS_DESC_FLINTANDSTEEL);
	Item::apple = ( new FoodItem(4, 4, FoodConstants::FOOD_SATURATION_LOW, false) )	->setIconName(L"apple")->setDescriptionId(IDS_ITEM_APPLE)->setUseDescriptionId(IDS_DESC_APPLE);
	Item::coal = ( new CoalItem(7) )												->setBaseItemTypeAndMaterial(eBaseItemType_treasure,	eMaterial_coal)->setIconName(L"coal")->setDescriptionId(IDS_ITEM_COAL)->setUseDescriptionId(IDS_DESC_COAL);
	Item::diamond = ( new Item(8) )													->setBaseItemTypeAndMaterial(eBaseItemType_treasure,	eMaterial_diamond)->setIconName(L"diamond")->setDescriptionId(IDS_ITEM_DIAMOND)->setUseDescriptionId(IDS_DESC_DIAMONDS);
	Item::stick = ( new Item(24) )													->setBaseItemTypeAndMaterial(Item::eBaseItemType_stick, Item::eMaterial_wood)->setIconName(L"stick")->handEquipped()->setDescriptionId(IDS_ITEM_STICK)->setUseDescriptionId(IDS_DESC_STICK);
	Item::mushroom_stew = ( new BowlFoodItem(26, 6) )								->setIconName(L"mushroom_stew")->setDescriptionId(IDS_ITEM_MUSHROOM_STEW)->setUseDescriptionId(IDS_DESC_MUSHROOMSTEW);
	Item::rabbit_stew = ( new BowlFoodItem(157, 10) )								->setIconName(L"rabbit_stew")->setDescriptionId(IDS_ITEM_MUSHROOM_STEW)->setUseDescriptionId(IDS_DESC_MUSHROOMSTEW);

	Item::string = ( new TilePlanterItem(31, Tile::tripWire) )						->setIconName(L"string")->setDescriptionId(IDS_ITEM_STRING)->setUseDescriptionId(IDS_DESC_STRING);
	Item::feather = ( new Item(32) )												->setIconName(L"feather")->setDescriptionId(IDS_ITEM_FEATHER)->setUseDescriptionId(IDS_DESC_FEATHER);
	Item::gunpowder = ( new Item(33) )												->setIconName(L"sulphur")->setDescriptionId(IDS_ITEM_SULPHUR)->setUseDescriptionId(IDS_DESC_SULPHUR)->setPotionBrewingFormula(PotionBrewing::MOD_GUNPOWDER);


	Item::wheat_seeds = ( new SeedItem(39, Tile::wheat_Id, Tile::farmland_Id) )			->setIconName(L"seeds")->setDescriptionId(IDS_ITEM_WHEAT_SEEDS)->setUseDescriptionId(IDS_DESC_WHEAT_SEEDS);
	Item::wheat = ( new Item(40) )														->setBaseItemTypeAndMaterial(eBaseItemType_treasure,	eMaterial_wheat)->setIconName(L"wheat")->setDescriptionId(IDS_ITEM_WHEAT)->setUseDescriptionId(IDS_DESC_WHEAT);
	Item::bread = ( new FoodItem(41, 5, FoodConstants::FOOD_SATURATION_NORMAL, false) )	->setIconName(L"bread")->setDescriptionId(IDS_ITEM_BREAD)->setUseDescriptionId(IDS_DESC_BREAD);


	Item::flint = ( new Item(62) )																->setIconName(L"flint")->setDescriptionId(IDS_ITEM_FLINT)->setUseDescriptionId(IDS_DESC_FLINT);
	Item::raw_porkchop = ( new FoodItem(63, 3, FoodConstants::FOOD_SATURATION_LOW, true) )		->setIconName(L"porkchopRaw")->setDescriptionId(IDS_ITEM_PORKCHOP_RAW)->setUseDescriptionId(IDS_DESC_PORKCHOP_RAW);
	Item::cooked_porkchop = ( new FoodItem(64, 8, FoodConstants::FOOD_SATURATION_GOOD, true) )	->setIconName(L"porkchopCooked")->setDescriptionId(IDS_ITEM_PORKCHOP_COOKED)->setUseDescriptionId(IDS_DESC_PORKCHOP_COOKED);
	Item::painting = ( new HangingEntityItem(65,eTYPE_PAINTING) )								->setBaseItemTypeAndMaterial(eBaseItemType_HangingItem,	eMaterial_cloth)->setIconName(L"painting")->setDescriptionId(IDS_ITEM_PAINTING)->setUseDescriptionId(IDS_DESC_PICTURE);

	Item::golden_apple = ( new GoldenAppleItem(66, 4, FoodConstants::FOOD_SATURATION_SUPERNATURAL, false) )->setCanAlwaysEat()->setEatEffect(MobEffect::regeneration->id, 5, 1, 1.0f)
																										->setBaseItemTypeAndMaterial(eBaseItemType_giltFruit,eMaterial_apple)->setIconName(L"appleGold")->setDescriptionId(IDS_ITEM_APPLE_GOLD);//->setUseDescriptionId(IDS_DESC_GOLDENAPPLE);

	Item::sign = ( new SignItem(67) )															->setBaseItemTypeAndMaterial(eBaseItemType_HangingItem, eMaterial_wood)->setIconName(L"sign")->setDescriptionId(IDS_ITEM_SIGN)->setUseDescriptionId(IDS_DESC_SIGN);
	Item::bamboo_door = (new DoorItem(462-256, Material::wood, L"doorBamboo"))->setBaseItemTypeAndMaterial(eBaseItemType_door, eMaterial_wood)->setIconName(L"bamboo_door")->setDescriptionId(IDS_ITEM_DOOR_BAMBOO)->setUseDescriptionId(IDS_DESC_DOOR_WOOD);
	// neoLegacy - Copper Age (Phase 3). DoorItem ctor takes (id - 256);
	// the Item ctor adds 256 back, so these land on ids 497 / 498.
	Item::copper_door = (new DoorItem(copper_door_Id - 256, Material::metal, L"doorCopper"))->setBaseItemTypeAndMaterial(eBaseItemType_door, eMaterial_copper)->setIconName(L"doorCopper")->setDescriptionId(IDS_ITEM_DOOR_COPPER)->setUseDescriptionId(IDS_DESC_DOOR_COPPER);
	Item::rose_gold_door = (new DoorItem(rose_gold_door_Id - 256, Material::metal, L"doorRoseGold"))->setBaseItemTypeAndMaterial(eBaseItemType_door, eMaterial_rose_gold)->setIconName(L"doorRoseGold")->setDescriptionId(IDS_ITEM_DOOR_ROSE_GOLD)->setUseDescriptionId(IDS_DESC_DOOR_ROSE_GOLD);
	// neoLegacy - Copper Age (Phase 4). Honeycomb: a plain loose item like
	// gunpowder (no base item type — it is never a Complex Table result,
	// only a Wandering Trader good and a waxing ingredient), icon at
	// items.png row 19 col 11.
	Item::honeycomb = ( new Item(Item::honeycomb_Id - 256) )->setIconName(L"honeycomb")->setDescriptionId(IDS_ITEM_HONEYCOMB)->setUseDescriptionId(IDS_DESC_HONEYCOMB);
	// neoLegacy - Copper Age (Phase 6). Rose-gold foods (jar: rosegold mod).
	Item::rose_gilded_beetroot = (new RoseGoldFoodItem(rose_gilded_beetroot_Id - 256, 6, 0.6f))
		->addEatEffectEntry(MobEffect::regeneration->id, 6, 10)->addEatEffectEntry(MobEffect::damageBoost->id, 400, 0)
		->setCanAlwaysEat()->setBaseItemTypeAndMaterial(eBaseItemType_utensil, eMaterial_rose_gold)->setIconName(L"rose_gilded_beetroot")->setDescriptionId(IDS_ITEM_ROSE_GILDED_BEETROOT)->setUseDescriptionId(IDS_DESC_ROSE_GILDED_BEETROOT);
	Item::rose_gilded_mushroom = (new RoseGoldFoodItem(rose_gilded_mushroom_Id - 256, 2, 0.2f))
		->addEatEffectEntry(MobEffect::regeneration->id, 2, 10)
		->setUseDurationTicks(16)->setCanAlwaysEat()->setBaseItemTypeAndMaterial(eBaseItemType_utensil, eMaterial_rose_gold)->setIconName(L"rose_gilded_mushroom")->setDescriptionId(IDS_ITEM_ROSE_GILDED_MUSHROOM)->setUseDescriptionId(IDS_DESC_ROSE_GILDED_MUSHROOM);
	Item::rose_gilded_cookie = (new RoseGoldFoodItem(rose_gilded_cookie_Id - 256, 2, 0.4f))
		->addEatEffectEntry(MobEffect::regeneration->id, 2, 10)->addEatEffectEntry(MobEffect::movementSpeed->id, 300, 0)
		->setUseDurationTicks(16)->setCanAlwaysEat()->setBaseItemTypeAndMaterial(eBaseItemType_utensil, eMaterial_rose_gold)->setIconName(L"rose_gilded_cookie")->setDescriptionId(IDS_ITEM_ROSE_GILDED_COOKIE)->setUseDescriptionId(IDS_DESC_ROSE_GILDED_COOKIE);
	Item::rose_full_jelly = (new RoseGoldFoodItem(rose_full_jelly_Id - 256, 8, 1.2f))
		->addEatEffectEntry(MobEffect::regeneration->id, 6, 10)->addEatEffectEntry(MobEffect::movementSpeed->id, 800, 1)
		->setConvertsTo(Item::rose_half_jelly_Id)->setCanAlwaysEat()->setMaxStackSize(1)->setBaseItemTypeAndMaterial(eBaseItemType_utensil, eMaterial_rose_gold)->setIconName(L"rose_full_jelly")->setDescriptionId(IDS_ITEM_ROSE_FULL_JELLY)->setUseDescriptionId(IDS_DESC_ROSE_FULL_JELLY);
	Item::rose_half_jelly = (new RoseGoldFoodItem(rose_half_jelly_Id - 256, 8, 1.2f))
		->addEatEffectEntry(MobEffect::regeneration->id, 6, 10)->addEatEffectEntry(MobEffect::movementSpeed->id, 800, 1)
		->setCanAlwaysEat()->setMaxStackSize(1)->setBaseItemTypeAndMaterial(eBaseItemType_utensil, eMaterial_rose_gold)->setIconName(L"rose_half_jelly")->setDescriptionId(IDS_ITEM_ROSE_HALF_JELLY)->setUseDescriptionId(IDS_DESC_ROSE_HALF_JELLY);
	Item::rose_mushroom_omelette = (new RoseGoldFoodItem(rose_mushroom_omelette_Id - 256, 5, 0.4f))
		->addEatEffectEntry(MobEffect::regeneration->id, 6, 10)->addEatEffectEntry(MobEffect::digSpeed->id, 800, 1)
		->setConvertsTo(Item::rose_mushroom_partial_Id)->setCanAlwaysEat()->setMaxStackSize(1)->setBaseItemTypeAndMaterial(eBaseItemType_utensil, eMaterial_rose_gold)->setIconName(L"rose_mushroom_omelette")->setDescriptionId(IDS_ITEM_ROSE_MUSHROOM_OMELETTE)->setUseDescriptionId(IDS_DESC_ROSE_MUSHROOM_OMELETTE);
	Item::rose_mushroom_partial = (new RoseGoldFoodItem(rose_mushroom_partial_Id - 256, 5, 0.4f))
		->addEatEffectEntry(MobEffect::regeneration->id, 6, 10)->addEatEffectEntry(MobEffect::digSpeed->id, 800, 1)
		->setConvertsTo(Item::rose_mushroom_crumbs_Id)->setCanAlwaysEat()->setMaxStackSize(1)->setBaseItemTypeAndMaterial(eBaseItemType_utensil, eMaterial_rose_gold)->setIconName(L"rose_mushroom_partial")->setDescriptionId(IDS_ITEM_ROSE_MUSHROOM_PARTIAL)->setUseDescriptionId(IDS_DESC_ROSE_MUSHROOM_PARTIAL);
	Item::rose_mushroom_crumbs = (new RoseGoldFoodItem(rose_mushroom_crumbs_Id - 256, 5, 0.4f))
		->addEatEffectEntry(MobEffect::regeneration->id, 6, 10)->addEatEffectEntry(MobEffect::digSpeed->id, 800, 1)
		->setConvertsTo(Item::bowl_Id)->setCanAlwaysEat()->setMaxStackSize(1)->setBaseItemTypeAndMaterial(eBaseItemType_utensil, eMaterial_rose_gold)->setIconName(L"rose_mushroom_crumbs")->setDescriptionId(IDS_ITEM_ROSE_MUSHROOM_CRUMBS)->setUseDescriptionId(IDS_DESC_ROSE_MUSHROOM_CRUMBS);
	Item::rosey_beetroot_stew = (new RoseGoldFoodItem(rosey_beetroot_stew_Id - 256, 14, 1.0f))
		->addEatEffectEntry(MobEffect::heal->id, 1, 2)->addEatEffectEntry(MobEffect::damageBoost->id, 1200, 1)->addEatEffectEntry(MobEffect::digSpeed->id, 1200, 1)->addEatEffectEntry(MobEffect::regeneration->id, 400, 1)
		->setConvertsTo(Item::bowl_Id)->setCanAlwaysEat()->setMaxStackSize(1)->setBaseItemTypeAndMaterial(eBaseItemType_utensil, eMaterial_rose_gold)->setIconName(L"rosey_beetroot_stew")->setDescriptionId(IDS_ITEM_ROSEY_BEETROOT_STEW)->setUseDescriptionId(IDS_DESC_ROSEY_BEETROOT_STEW);
	Item::bamboo = ( new BambooItem(461-256) )->setIconName(L"bamboo")->setDescriptionId(IDS_TILE_BAMBOO)->setUseDescriptionId(IDS_DESC_REEDS);



	Item::minecart = ( new MinecartItem(72, Minecart::TYPE_RIDEABLE) )		->setIconName(L"minecart")->setDescriptionId(IDS_ITEM_MINECART)->setUseDescriptionId(IDS_DESC_MINECART);
	Item::saddle = ( new SaddleItem(73) )								->setIconName(L"saddle")->setDescriptionId(IDS_ITEM_SADDLE)->setUseDescriptionId(IDS_DESC_SADDLE);
	Item::redstone = ( new RedStoneItem(75) )							->setBaseItemTypeAndMaterial(eBaseItemType_treasure,	eMaterial_redstone)->setIconName(L"redstone")->setDescriptionId(IDS_ITEM_REDSTONE)->setUseDescriptionId(IDS_DESC_REDSTONE_DUST)->setPotionBrewingFormula(PotionBrewing::MOD_REDSTONE);
	Item::snowball = ( new SnowballItem(76) )							->setIconName(L"snowball")->setDescriptionId(IDS_ITEM_SNOWBALL)->setUseDescriptionId(IDS_DESC_SNOWBALL);

	Item::boat = ( new BoatItem(77) )									->setIconName(L"boat")->setDescriptionId(IDS_ITEM_BOAT)->setUseDescriptionId(IDS_DESC_BOAT);

	Item::leather = ( new Item(78) )									->setIconName(L"leather")->setDescriptionId(IDS_ITEM_LEATHER)->setUseDescriptionId(IDS_DESC_LEATHER)->setBaseItemTypeAndMaterial(Item::eBaseItemType_decoration,Item::eMaterial_cloth); 
	Item::brick = ( new Item(80) )										->setIconName(L"brick")->setDescriptionId(IDS_ITEM_BRICK)->setUseDescriptionId(IDS_DESC_BRICK);
	Item::clay = ( new Item(81) )										->setIconName(L"clay")->setDescriptionId(IDS_ITEM_CLAY)->setUseDescriptionId(IDS_DESC_CLAY);
	Item::reeds = ( new TilePlanterItem(82, Tile::reeds) )				->setIconName(L"reeds")->setDescriptionId(IDS_ITEM_REEDS)->setUseDescriptionId(IDS_DESC_REEDS);
	Item::paper = ( new Item(83) )										->setBaseItemTypeAndMaterial(Item::eBaseItemType_paper, Item::eMaterial_paper)->setIconName(L"paper")->setDescriptionId(IDS_ITEM_PAPER)->setUseDescriptionId(IDS_DESC_PAPER);
	Item::book = ( new BookItem(84) )									->setBaseItemTypeAndMaterial(Item::eBaseItemType_paper, Item::eMaterial_book)->setIconName(L"book")->setDescriptionId(IDS_ITEM_BOOK)->setUseDescriptionId(IDS_DESC_BOOK);
	Item::slime_ball = ( new Item(85) )									->setBaseItemTypeAndMaterial(Item::eBaseItemType_treasure, Item::eMaterial_slime)->setIconName(L"slimeball")->setDescriptionId(IDS_ITEM_SLIMEBALL)->setUseDescriptionId(IDS_DESC_SLIMEBALL);
	Item::chest_minecart = ( new MinecartItem(86, Minecart::TYPE_CHEST) )	->setIconName(L"chest_minecart")->setDescriptionId(IDS_ITEM_MINECART_CHEST)->setUseDescriptionId(IDS_DESC_MINECARTWITHCHEST);
	Item::furnace_minecart = ( new MinecartItem(87, Minecart::TYPE_FURNACE) )->setIconName(L"furnace_minecart")->setDescriptionId(IDS_ITEM_MINECART_FURNACE)->setUseDescriptionId(IDS_DESC_MINECARTWITHFURNACE);
	Item::egg = ( new EggItem(88) )										->setIconName(L"egg")->setDescriptionId(IDS_ITEM_EGG)->setUseDescriptionId(IDS_DESC_EGG);
	Item::fishing_rod = static_cast<FishingRodItem *>((new FishingRodItem(90))->setBaseItemTypeAndMaterial(eBaseItemType_rod, eMaterial_wood)->setIconName(L"fishing_rod")->setDescriptionId(IDS_ITEM_FISHING_ROD)->setUseDescriptionId(IDS_DESC_FISHINGROD));
	Item::glowstone_dust = ( new Item(92) )									->setIconName(L"glowstone_dust")->setDescriptionId(IDS_ITEM_YELLOW_DUST)->setUseDescriptionId(IDS_DESC_YELLOW_DUST)->setPotionBrewingFormula(PotionBrewing::MOD_GLOWSTONE);
	Item::raw_fish = ( new FishFoodItem(93, false) )			->setIconName(L"fishRaw")->setDescriptionId(IDS_ITEM_FISH_RAW)->setUseDescriptionId(IDS_DESC_FISH_RAW)->setStackedByData(true)->setPotionBrewingFormula(PotionBrewing::MOD_PUFFERFISH);
	Item::cooked_fish = (new FishFoodItem(94, true))	->setIconName(L"fishCooked")->setDescriptionId(IDS_ITEM_FISH_COOKED)->setUseDescriptionId(IDS_DESC_FISH_COOKED)->setStackedByData(true);

	Item::dye = ( new DyePowderItem(95) )			->setBaseItemTypeAndMaterial(eBaseItemType_dyepowder,	eMaterial_dye)->setIconName(L"dyePowder")->setDescriptionId(IDS_ITEM_DYE_POWDER)->setUseDescriptionId(-1);

	Item::bone = ( new Item(96) )										->setIconName(L"bone")->setDescriptionId(IDS_ITEM_BONE)->handEquipped()->setUseDescriptionId(IDS_DESC_BONE);
	Item::sugar = ( new Item(97) )										->setIconName(L"sugar")->setDescriptionId(IDS_ITEM_SUGAR)->setUseDescriptionId(IDS_DESC_SUGAR)->setPotionBrewingFormula(PotionBrewing::MOD_SUGAR);
	// 4J-PB  - changing the cake to be stackable - Jens ok'ed this 23/10/12
	//Item::cake = ( new TilePlanterItem(98, Tile::cake) )->setMaxStackSize(1)->setIcon(13, 1)->setDescriptionId(IDS_ITEM_CAKE)->setUseDescriptionId(IDS_DESC_CAKE);
	Item::cake = ( new TilePlanterItem(98, Tile::cake) )				->setIconName(L"cake")->setDescriptionId(IDS_ITEM_CAKE)->setUseDescriptionId(IDS_DESC_CAKE);

	Item::bed = ( new BedItem(99) )										->setMaxStackSize(1)->setIconName(L"bed")->setDescriptionId(IDS_ITEM_BED)->setUseDescriptionId(IDS_DESC_BED);

	Item::repeater = ( new TilePlanterItem(100, static_cast<Tile *>(Tile::unpowered_repeater)) )			->setIconName(L"diode")->setDescriptionId(IDS_ITEM_DIODE)->setUseDescriptionId(IDS_DESC_REDSTONEREPEATER);
	Item::cookie = ( new FoodItem(101, 2, FoodConstants::FOOD_SATURATION_POOR, false) )	->setIconName(L"cookie")->setDescriptionId(IDS_ITEM_COOKIE)->setUseDescriptionId(IDS_DESC_COOKIE);


	Item::shears = static_cast<ShearsItem *>((new ShearsItem(103))->setIconName(L"shears")->setBaseItemTypeAndMaterial(eBaseItemType_devicetool, eMaterial_shears)->setDescriptionId(IDS_ITEM_SHEARS)->setUseDescriptionId(IDS_DESC_SHEARS));

	Item::melon = (new FoodItem(104, 2, FoodConstants::FOOD_SATURATION_LOW, false))		->setIconName(L"melon")->setDescriptionId(IDS_ITEM_MELON_SLICE)->setUseDescriptionId(IDS_DESC_MELON_SLICE);

	Item::seeds_pumpkin = (new SeedItem(105, Tile::pumpkin_stem_Id, Tile::farmland_Id))	->setIconName(L"seeds_pumpkin")->setBaseItemTypeAndMaterial(eBaseItemType_seed,	eMaterial_pumpkin)->setDescriptionId(IDS_ITEM_PUMPKIN_SEEDS)->setUseDescriptionId(IDS_DESC_PUMPKIN_SEEDS);
	Item::seeds_melon = (new SeedItem(106, Tile::melon_stem_Id, Tile::farmland_Id))		->setIconName(L"seeds_melon")->setBaseItemTypeAndMaterial(eBaseItemType_seed,	eMaterial_melon)->setDescriptionId(IDS_ITEM_MELON_SEEDS)->setUseDescriptionId(IDS_DESC_MELON_SEEDS);

	Item::raw_beef = (new FoodItem(107, 3, FoodConstants::FOOD_SATURATION_LOW, true))	->setIconName(L"beefRaw")->setDescriptionId(IDS_ITEM_BEEF_RAW)->setUseDescriptionId(IDS_DESC_BEEF_RAW);
	Item::cooked_beef = (new FoodItem(108, 8, FoodConstants::FOOD_SATURATION_GOOD, true))->setIconName(L"beefCooked")->setDescriptionId(IDS_ITEM_BEEF_COOKED)->setUseDescriptionId(IDS_DESC_BEEF_COOKED);
	Item::raw_chicken = (new FoodItem(109, 2, FoodConstants::FOOD_SATURATION_LOW, true))->setEatEffect(MobEffect::hunger->id, 30, 0, .3f)->setIconName(L"chickenRaw")->setDescriptionId(IDS_ITEM_CHICKEN_RAW)->setUseDescriptionId(IDS_DESC_CHICKEN_RAW);
	Item::cooked_chicken = (new FoodItem(110, 6, FoodConstants::FOOD_SATURATION_NORMAL, true))->setIconName(L"chickenCooked")->setDescriptionId(IDS_ITEM_CHICKEN_COOKED)->setUseDescriptionId(IDS_DESC_CHICKEN_COOKED);
	Item::rotten_flesh = (new FoodItem(111, 4, FoodConstants::FOOD_SATURATION_POOR, true))->setEatEffect(MobEffect::hunger->id, 30, 0, .8f)->setIconName(L"rottenFlesh")->setDescriptionId(IDS_ITEM_ROTTEN_FLESH)->setUseDescriptionId(IDS_DESC_ROTTEN_FLESH);

	Item::ender_pearl =			(new EnderpearlItem(112))											->setIconName(L"ender_pearl")->setDescriptionId(IDS_ITEM_ENDER_PEARL)->setUseDescriptionId(IDS_DESC_ENDER_PEARL);

	Item::blaze_rod =			(new Item(113)	)													->setIconName(L"blaze_rod")->setDescriptionId(IDS_ITEM_BLAZE_ROD)->setUseDescriptionId(IDS_DESC_BLAZE_ROD)->handEquipped();
	Item::ghast_tear =			(new Item(114) )													->setIconName(L"ghast_tear")->setDescriptionId(IDS_ITEM_GHAST_TEAR)->setUseDescriptionId(IDS_DESC_GHAST_TEAR)->setPotionBrewingFormula(PotionBrewing::MOD_GHASTTEARS);
	Item::gold_nugget =			(new Item(115) )													->setBaseItemTypeAndMaterial(eBaseItemType_treasure,	eMaterial_gold)->setIconName(L"gold_nugget")->setDescriptionId(IDS_ITEM_GOLD_NUGGET)->setUseDescriptionId(IDS_DESC_GOLD_NUGGET);

	Item::netherwart_seeds =	(new SeedItem(116, Tile::nether_wart_Id, Tile::soul_sand_Id) )		->setIconName(L"netherStalkSeeds")->setDescriptionId(IDS_ITEM_NETHER_STALK_SEEDS)->setUseDescriptionId(IDS_DESC_NETHER_STALK_SEEDS)->setPotionBrewingFormula(PotionBrewing::MOD_NETHERWART);

	Item::potion =				static_cast<PotionItem *>((new PotionItem(117))->setIconName(L"potion")->setDescriptionId(IDS_ITEM_POTION)->setUseDescriptionId(IDS_DESC_POTION));
	Item::glassBottle =			(new BottleItem(118) )												->setBaseItemTypeAndMaterial(eBaseItemType_utensil,	eMaterial_glass)->setIconName(L"glassBottle")->setDescriptionId(IDS_ITEM_GLASS_BOTTLE)->setUseDescriptionId(IDS_DESC_GLASS_BOTTLE);

	Item::spider_eye =			(new FoodItem(119, 2, FoodConstants::FOOD_SATURATION_GOOD, false) )	->setEatEffect(MobEffect::poison->id, 5, 0, 1.0f)->setIconName(L"spider_eye")->setDescriptionId(IDS_ITEM_SPIDER_EYE)->setUseDescriptionId(IDS_DESC_SPIDER_EYE)->setPotionBrewingFormula(PotionBrewing::MOD_SPIDEREYE);
	Item::fermented_spider_eye =	(new Item(120) )													->setIconName(L"fermented_spider_eye")->setDescriptionId(IDS_ITEM_FERMENTED_SPIDER_EYE)->setUseDescriptionId(IDS_DESC_FERMENTED_SPIDER_EYE)->setPotionBrewingFormula(PotionBrewing::MOD_FERMENTEDEYE);

	Item::blaze_powder =			(new Item(121) )													->setIconName(L"blaze_powder")->setDescriptionId(IDS_ITEM_BLAZE_POWDER)->setUseDescriptionId(IDS_DESC_BLAZE_POWDER)->setPotionBrewingFormula(PotionBrewing::MOD_BLAZEPOWDER);
	Item::magma_cream =			(new Item(122) )													->setIconName(L"magma_cream")->setDescriptionId(IDS_ITEM_MAGMA_CREAM)->setUseDescriptionId(IDS_DESC_MAGMA_CREAM)->setPotionBrewingFormula(PotionBrewing::MOD_MAGMACREAM);

	Item::brewing_stand =		(new TilePlanterItem(123, Tile::brewingStand) )						->setBaseItemTypeAndMaterial(eBaseItemType_device,	eMaterial_blaze)->setIconName(L"brewing_stand")->setDescriptionId(IDS_ITEM_BREWING_STAND)->setUseDescriptionId(IDS_DESC_BREWING_STAND);
	Item::cauldron =			(new TilePlanterItem(124, Tile::cauldron) )							->setBaseItemTypeAndMaterial(eBaseItemType_utensil,	eMaterial_iron)->setIconName(L"cauldron")->setDescriptionId(IDS_ITEM_CAULDRON)->setUseDescriptionId(IDS_DESC_CAULDRON);
	Item::eye_of_ender =			(new EnderEyeItem(125) )											->setBaseItemTypeAndMaterial(eBaseItemType_pockettool,	eMaterial_ender)->setIconName(L"eye_of_ender")->setDescriptionId(IDS_ITEM_EYE_OF_ENDER)->setUseDescriptionId(IDS_DESC_EYE_OF_ENDER);
	Item::speckled_melon =		(new Item(126) )													->setBaseItemTypeAndMaterial(eBaseItemType_giltFruit,	eMaterial_melon)->setIconName(L"speckled_melon")->setDescriptionId(IDS_ITEM_SPECKLED_MELON)->setUseDescriptionId(IDS_DESC_SPECKLED_MELON)->setPotionBrewingFormula(PotionBrewing::MOD_SPECKLEDMELON);

	Item::spawn_egg =			(new SpawnEggItem(127))												->setIconName(L"monsterPlacer")->setDescriptionId(IDS_ITEM_MONSTER_SPAWNER)->setUseDescriptionId(IDS_DESC_MONSTER_SPAWNER);

	// 4J Stu - Brought this forward
	Item::experience_bottle =			(new ExperienceItem(128))											->setIconName(L"experience_bottle")->setDescriptionId(IDS_ITEM_EXP_BOTTLE)->setUseDescriptionId(IDS_DESC_EXP_BOTTLE);

	Item::record_01 =			( new RecordingItem(2000, L"13") )									->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_01)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_02 =			( new RecordingItem(2001, L"cat") )									->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_02)->setUseDescriptionId(IDS_DESC_RECORD);

	// 4J - new records brought forward from 1.2.3
	Item::record_03 =			( new RecordingItem(2002, L"blocks") )								->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_03)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_04 =			( new RecordingItem(2003, L"chirp") )								->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_04)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_05 =			( new RecordingItem(2004, L"far") )									->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_05)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_06 =			( new RecordingItem(2005, L"mall") )								->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_06)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_07 =			( new RecordingItem(2006, L"mellohi") )								->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_07)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_09 =			( new RecordingItem(2007, L"stal") )								->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_08)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_10 =			( new RecordingItem(2008, L"strad") )								->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_09)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_11 =			( new RecordingItem(2009, L"ward") )								->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_10)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_12 =			( new RecordingItem(2010, L"11") )									->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_11)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_08 =			( new RecordingItem(2011, L"where are we now") )					->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_12)->setUseDescriptionId(IDS_DESC_RECORD);

	// TU9
	// putting the fire charge in as a torch, so that it stacks without being near the middle of the selection boxes
	Item::fireball =			(new FireChargeItem(129))											->setBaseItemTypeAndMaterial(eBaseItemType_torch,	eMaterial_setfire)->setIconName(L"fireball")->setDescriptionId(IDS_ITEM_FIREBALL)->setUseDescriptionId(IDS_DESC_FIREBALL);
	Item::frame =				(new HangingEntityItem(133,eTYPE_ITEM_FRAME))						->setBaseItemTypeAndMaterial(eBaseItemType_HangingItem,	eMaterial_glass)->setIconName(L"frame")->setDescriptionId(IDS_ITEM_ITEMFRAME)->setUseDescriptionId(IDS_DESC_ITEMFRAME);


	// TU12
	Item::skull =				(new SkullItem(141))												->setIconName(L"skull")->setDescriptionId(IDS_ITEM_SKULL)->setUseDescriptionId(IDS_DESC_SKULL);

	// TU14
	//Item::writable_book = (new WritingBookItem(130))->setIcon(11, 11)->setDescriptionId("writable_book");
	//Item::written_book = (new WrittenBookItem(131))->setIcon(12, 11)->setDescriptionId("written_book");
	//Item::book = ( new BookItem(84) )									->setBaseItemTypeAndMaterial(Item::eBaseItemType_paper, Item::eMaterial_book)->setIconName(L"book")->setDescriptionId(IDS_ITEM_BOOK)->setUseDescriptionId(IDS_DESC_BOOK);
	//->setBaseItemTypeAndMaterial(Item::eBaseItemType_paper, Item::eMaterial_book)
	Item::writable_book = (new WritingBookItem(130))->setBaseItemTypeAndMaterial(Item::eBaseItemType_paper, Item::eMaterial_book)->setIconName(L"writable_book")->setDescriptionId(IDS_ITEM_WRITINGBOOK)->setUseDescriptionId(IDS_DESC_WRITINGBOOK)->setMaxStackSize(1);
	Item::written_book = (new WrittenBookItem(131))->setIconName(L"written_book")->setDescriptionId(IDS_ITEM_WRITTENBOOK)->setUseDescriptionId(IDS_DESC_WRITTENBOOK)->setMaxStackSize(1);

	Item::emerald =				(new Item(132))														->setBaseItemTypeAndMaterial(eBaseItemType_treasure, eMaterial_emerald)->setIconName(L"emerald")->setDescriptionId(IDS_ITEM_EMERALD)->setUseDescriptionId(IDS_DESC_EMERALD);

	Item::flower_pot = (new TilePlanterItem(134, Tile::flower_pot))									->setIconName(L"flower_pot")->setDescriptionId(IDS_FLOWERPOT)->setUseDescriptionId(IDS_DESC_FLOWERPOT)->setBaseItemTypeAndMaterial(eBaseItemType_decoration,eMaterial_brick);

	Item::carrots = (new SeedFoodItem(135, 4, FoodConstants::FOOD_SATURATION_NORMAL, Tile::carrots_Id, Tile::farmland_Id))	->setIconName(L"carrots")->setDescriptionId(IDS_CARROTS)->setUseDescriptionId(IDS_DESC_CARROTS);
	Item::potato = (new SeedFoodItem(136, 1, FoodConstants::FOOD_SATURATION_LOW, Tile::potatoes_Id, Tile::farmland_Id))		->setIconName(L"potato")->setDescriptionId(IDS_POTATO)->setUseDescriptionId(IDS_DESC_POTATO);
	Item::baked_potato = (new FoodItem(137, 6, FoodConstants::FOOD_SATURATION_NORMAL, false))								->setIconName(L"baked_potato")->setDescriptionId(IDS_ITEM_POTATO_BAKED)->setUseDescriptionId(IDS_DESC_POTATO_BAKED);
	Item::poisonous_potato = (new FoodItem(138, 2, FoodConstants::FOOD_SATURATION_LOW, false))								->setEatEffect(MobEffect::poison->id, 5, 0, .6f)->setIconName(L"poisonous_potato")->setDescriptionId(IDS_ITEM_POTATO_POISONOUS)->setUseDescriptionId(IDS_DESC_POTATO_POISONOUS);

	Item::empty_map = (EmptyMapItem*)(new EmptyMapItem(139))->setIconName(L"map_empty")->setBaseItemTypeAndMaterial(eBaseItemType_pockettool, eMaterial_map)->setDescriptionId(IDS_ITEM_MAP_EMPTY)->setUseDescriptionId(IDS_DESC_MAP_EMPTY);

	Item::golden_carrot = (new FoodItem(140, 6, FoodConstants::FOOD_SATURATION_SUPERNATURAL, false))			->setBaseItemTypeAndMaterial(eBaseItemType_giltFruit,	eMaterial_carrot)->setIconName(L"golden_carrot")->setPotionBrewingFormula(PotionBrewing::MOD_GOLDENCARROT)->setDescriptionId(IDS_ITEM_CARROT_GOLDEN)->setUseDescriptionId(IDS_DESC_CARROT_GOLDEN);

	Item::carrot_on_a_stick = (new CarrotOnAStickItem(142))													->setBaseItemTypeAndMaterial(eBaseItemType_rod, eMaterial_carrot)->setIconName(L"carrot_on_a_stick")->setDescriptionId(IDS_ITEM_CARROT_ON_A_STICK)->setUseDescriptionId(IDS_DESC_CARROT_ON_A_STICK);
	Item::nether_star = (new SimpleFoiledItem(143))													->setIconName(L"nether_star")->setDescriptionId(IDS_NETHER_STAR)->setUseDescriptionId(IDS_DESC_NETHER_STAR);
	Item::pumpkin_pie = (new FoodItem(144, 8, FoodConstants::FOOD_SATURATION_LOW, false))			->setIconName(L"pumpkin_pie")->setDescriptionId(IDS_ITEM_PUMPKIN_PIE)->setUseDescriptionId(IDS_DESC_PUMPKIN_PIE);
	Item::fireworks = (new FireworksItem(145))														->setBaseItemTypeAndMaterial(Item::eBaseItemType_fireworks,	Item::eMaterial_undefined)->setIconName(L"fireworks")->setDescriptionId(IDS_FIREWORKS)->setUseDescriptionId(IDS_DESC_FIREWORKS);
	Item::firework_charge = (new FireworksChargeItem(146))											->setBaseItemTypeAndMaterial(Item::eBaseItemType_fireworks,	Item::eMaterial_undefined)->setIconName(L"fireworks_charge")->setDescriptionId(IDS_FIREWORKS_CHARGE)->setUseDescriptionId(IDS_DESC_FIREWORKS_CHARGE);
	EnchantedBookItem::enchanted_book = static_cast<EnchantedBookItem *>((new EnchantedBookItem(147))->setMaxStackSize(1)->setIconName(L"enchanted_book")->setDescriptionId(IDS_ITEM_ENCHANTED_BOOK)->setUseDescriptionId(IDS_DESC_ENCHANTED_BOOK));
	Item::comparator = (new TilePlanterItem(148, Tile::comparator_off))								->setIconName(L"comparator")->setDescriptionId(IDS_ITEM_COMPARATOR)->setUseDescriptionId(IDS_DESC_COMPARATOR);
	Item::netherbrick =	(new Item(149))																->setIconName(L"netherbrick")->setDescriptionId(IDS_ITEM_NETHERBRICK)->setUseDescriptionId(IDS_DESC_ITEM_NETHERBRICK);
	Item::nether_quartz = (new Item(150))															->setIconName(L"netherquartz")->setDescriptionId(IDS_ITEM_NETHER_QUARTZ)->setUseDescriptionId(IDS_DESC_NETHER_QUARTZ);
	Item::tnt_minecart = (new MinecartItem(151, Minecart::TYPE_TNT))								->setIconName(L"tnt_minecart")->setDescriptionId(IDS_ITEM_MINECART_TNT)->setUseDescriptionId(IDS_DESC_MINECART_TNT);
	Item::hopper_minecart = (new MinecartItem(152, Minecart::TYPE_HOPPER))							->setIconName(L"hopper_minecart")->setDescriptionId(IDS_ITEM_MINECART_HOPPER)->setUseDescriptionId(IDS_DESC_MINECART_HOPPER);
	// neoLegacy - Diamond Minecart: reuses the minecart icon, tinted diamond-cyan (MinecartItem::getColor)
	Item::diamond_minecart = (new MinecartItem(diamond_minecart_Id - 256, Minecart::TYPE_DIAMOND))->setIconName(L"minecart")->setDescriptionId(IDS_ITEM_MINECART_DIAMOND)->setUseDescriptionId(IDS_DESC_MINECART_DIAMOND);

	Item::iron_horse_armor = (new Item(161))															->setIconName(L"iron_horse_armor")->setMaxStackSize(1)->setDescriptionId(IDS_ITEM_IRON_HORSE_ARMOR)->setUseDescriptionId(IDS_DESC_IRON_HORSE_ARMOR);
	Item::golden_horse_armor = (new Item(162))															->setIconName(L"golden_horse_armor")->setMaxStackSize(1)->setDescriptionId(IDS_ITEM_GOLD_HORSE_ARMOR)->setUseDescriptionId(IDS_DESC_GOLD_HORSE_ARMOR);
	Item::diamond_horse_armor = (new Item(163))														->setIconName(L"diamond_horse_armor")->setMaxStackSize(1)->setDescriptionId(IDS_ITEM_DIAMOND_HORSE_ARMOR)->setUseDescriptionId(IDS_DESC_DIAMOND_HORSE_ARMOR);
	Item::copper_horse_armor = (new Item(copper_horse_armor_Id - 256))															->setIconName(L"copper_horse_armor")->setMaxStackSize(1)->setDescriptionId(IDS_ITEM_COPPER_HORSE_ARMOR)->setUseDescriptionId(IDS_DESC_COPPER_HORSE_ARMOR);
	Item::lead = (new LeashItem(164))																->setBaseItemTypeAndMaterial(eBaseItemType_pockettool,	eMaterial_undefined)->setIconName(L"lead")->setDescriptionId(IDS_ITEM_LEAD)->setUseDescriptionId(IDS_DESC_LEAD);
	Item::name_tag = (new NameTagItem(165))															->setIconName(L"name_tag")->setDescriptionId(IDS_ITEM_NAME_TAG)->setUseDescriptionId(IDS_DESC_NAME_TAG);

	Item::raw_mutton = (new FoodItem(167, 2, FoodConstants::FOOD_SATURATION_LOW, true))->setIconName(L"muttonRaw")->setDescriptionId(IDS_ITEM_MUTTON_RAW)->setUseDescriptionId(IDS_DESC_MUTTON_RAW);
	Item::cooked_mutton = (new FoodItem(168, 6, FoodConstants::FOOD_SATURATION_NORMAL, true))->setIconName(L"muttonCooked")->setDescriptionId(IDS_ITEM_MUTTON_COOKED)->setUseDescriptionId(IDS_DESC_MUTTON_COOKED);
	Item::raw_rabbit = (new FoodItem(155, 1, FoodConstants::FOOD_SATURATION_NORMAL, true))->setIconName(L"rabbitRaw")->setDescriptionId(IDS_ITEM_RABBIT_RAW)->setUseDescriptionId(IDS_DESC_RABBIT_RAW);
	Item::cooked_rabbit = (new FoodItem(156, 5, FoodConstants::FOOD_SATURATION_NORMAL, true))->setIconName(L"rabbitCooked")->setDescriptionId(IDS_ITEM_RABBIT_COOKED)->setUseDescriptionId(IDS_DESC_RABBIT_COOKED);

	Item::spruce_door = (new DoorItem(171, Material::wood, L"doorSpruce"))->setBaseItemTypeAndMaterial(eBaseItemType_door, eMaterial_wood)->setIconName(L"doorSpruce")->setDescriptionId(IDS_ITEM_DOOR_SPRUCE)->setUseDescriptionId(IDS_DESC_DOOR_WOOD);
	Item::birch_door = (new DoorItem(172, Material::wood, L"doorBirch"))->setBaseItemTypeAndMaterial(eBaseItemType_door, eMaterial_wood)->setIconName(L"doorBirch")->setDescriptionId(IDS_ITEM_DOOR_BIRCH)->setUseDescriptionId(IDS_DESC_DOOR_WOOD);
	Item::jungle_door = (new DoorItem(173, Material::wood, L"doorJungle"))->setBaseItemTypeAndMaterial(eBaseItemType_door, eMaterial_wood)->setIconName(L"doorJungle")->setDescriptionId(IDS_ITEM_DOOR_JUNGLE)->setUseDescriptionId(IDS_DESC_DOOR_WOOD);
	Item::acacia_door = (new DoorItem(174, Material::wood, L"doorAcacia"))->setBaseItemTypeAndMaterial(eBaseItemType_door, eMaterial_wood)->setIconName(L"doorAcacia")->setDescriptionId(IDS_ITEM_DOOR_ACACIA)->setUseDescriptionId(IDS_DESC_DOOR_WOOD);
	Item::dark_oak_door = (new DoorItem(175, Material::wood, L"doorDark"))->setBaseItemTypeAndMaterial(eBaseItemType_door, eMaterial_wood)->setIconName(L"doorDark")->setDescriptionId(IDS_ITEM_DOOR_DARK)->setUseDescriptionId(IDS_DESC_DOOR_WOOD);

	Item::rabbit_hide = ( new Item(159) )									->setIconName(L"rabbitHide")->setDescriptionId(IDS_ITEM_RABBIT_HIDE)->setUseDescriptionId(IDS_DESC_RABBIT_HIDE);
	Item::rabbit_foot = ( new Item(158) )									->setIconName(L"rabbitsFoot")->setDescriptionId(IDS_ITEM_RABBIT_FOOT)->setUseDescriptionId(IDS_DESC_RABBIT_FOOT)->setPotionBrewingFormula(PotionBrewing::MOD_RABBITS_FOOT);;

	Item::armor_stand = (new ArmorStandItem(160))							->setBaseItemTypeAndMaterial(eBaseItemType_HangingItem,eMaterial_cloth)->setIconName(L"armorStand")->setDescriptionId(IDS_ITEM_ARMOR_STAND)->setUseDescriptionId(IDS_DESC_ARMOR_STAND);
	Item::prismarine_crystal = (new Item(154))->setIconName(L"prismarineCrystal")->setDescriptionId(IDS_ITEM_PRISMARINE_CRYSTAL)->setUseDescriptionId(IDS_ITEM_PRISMARINE_CRYSTAL_DESC);
	Item::prismarine_shard = (new Item(153))->setIconName(L"prismarineShard")->setDescriptionId(IDS_ITEM_PRISMARINE_SHARD)->setUseDescriptionId(IDS_ITEM_PRISMARINE_SHARD_DESC);
	Item::elytra = (new ElytraItem())->setBaseItemTypeAndMaterial(eBaseItemType_chestplate, eMaterial_cloth)->setIconName(L"elytra")->setDescriptionId(IDS_ITEM_ELYTRA)->setUseDescriptionId(IDS_ITEM_ELYTRA);

	Item::beetroot = (new FoodItem(178, 1, 0.6f, false))->setIconName(L"beetroot")->setDescriptionId(IDS_BEETROOT)->setUseDescriptionId(IDS_DESC_BEETROOT);
	Item::beetroot_seeds = (new SeedItem(179, Tile::beetroots_Id, Tile::farmland_Id))->setIconName(L"beetroot_seeds")->setDescriptionId(IDS_BEETROOT_SEEDS)->setUseDescriptionId(IDS_DESC_BEETROOT_SEEDS);
	Item::beetroot_soup = (new BowlFoodItem(180, 6))->setIconName(L"beetroot_soup")->setDescriptionId(IDS_ITEM_BEETROOT_SOUP)->setUseDescriptionId(IDS_DESC_BEETROOT_SOUP);
}



// 4J Stu - We need to do this after the staticCtor AND after staticCtors for other class
// eg Recipes
void Item::staticInit()
{
	Stats::buildItemStats();
}


_Tier::Tier(int level, int uses, float speed, float damage, int enchantmentValue) :
level( level ),
	uses( uses ),
	speed( speed ),
	damage( damage ),
	enchantmentValue( enchantmentValue )
{
}


int _Tier::getUses() const
{
	return uses;
}

float _Tier::getSpeed() const
{
	return speed;
}

float _Tier::getAttackDamageBonus() const
{
	return damage;
}

int _Tier::getLevel() const
{
	return level;
}

int _Tier::getEnchantmentValue() const
{
	return enchantmentValue;
}

int _Tier::getTierItemId() const
{
	if (this == Tier::WOOD)
	{
		return Tile::planks_Id;
	}
	else if (this == Tier::STONE)
	{
		return Tile::cobblestone_Id;
	}
	else if (this == Tier::GOLD)
	{
		return Item::gold_ingot_Id;
	}
	else if (this == Tier::IRON)
	{
		return Item::iron_ingot_Id;
	}
	else if (this == Tier::DIAMOND)
	{
		return Item::diamond_Id;
	}
	else if (this == Tier::JDMON)
	{
		// neoLegacy - JDemon and CMonster swords repair with diamonds, like the Zoro swords.
		return Item::diamond_Id;
	}
	else if (this == Tier::CMON)
	{
		return Item::diamond_Id;
	}
	else if (this == Tier::COPPER)
	{
		return Item::copper_ingot_Id;
	}
	else if (this == Tier::ROSE_GOLD)
	{
		return Item::rose_gold_ingot_Id;
	}
	return 0;
}

Item::Item(int id) : id( 256 + id )
{
	maxStackSize = Item::MAX_STACK_SIZE;
	maxDamage = 0;
	icon = nullptr;
	m_handEquipped = false;
	m_isStackedByData = false;

	craftingRemainingItem = nullptr;
	potionBrewingFormula = L"";

	m_iMaterial=eMaterial_undefined;
	m_iBaseItemType=eBaseItemType_undefined;
	m_textureName = L"";

	// TODO Init this string
	//string descriptionId;

	//this->id = 256 + id;
	if (items[256 + id] != nullptr)
	{
		app.DebugPrintf("CONFLICT @ %d" , id);
	}

	items[256 + id] = this;
}

// 4J-PB - adding so we can class different items together for the new crafting menu
// so pickaxe_stone would get tagged with pickaxe and stone
Item *Item::setBaseItemTypeAndMaterial(int iType,int iMaterial)
{
	this->m_iBaseItemType = iType;
	this->m_iMaterial = iMaterial;
	return this;
}

int Item::getBaseItemType()
{
	return this->m_iBaseItemType;
}

int Item::getMaterial()
{
	return this->m_iMaterial;
}

Item *Item::setIconName(const wstring &name)
{
	m_textureName = name;

	return this;
}

wstring Item::getIconName()
{
	return m_textureName;
}

Item *Item::setMaxStackSize(int max)
{
	maxStackSize = max;
	return this;
}

int Item::getIconType()
{
	return Icon::TYPE_ITEM;
}

Icon *Item::getIcon(int auxValue)
{
	return icon;
}

Icon *Item::getIcon(shared_ptr<ItemInstance> itemInstance)
{
	return getIcon(itemInstance->getAuxValue());
}

bool Item::useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	return false;
}

float Item::getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile)
{
	return 1;
}

bool Item::TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	return false;
}

shared_ptr<ItemInstance> Item::use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	return itemInstance;
}

shared_ptr<ItemInstance> Item::useTimeDepleted(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	return itemInstance;
}

int Item::getMaxStackSize() const
{
	return maxStackSize;
}

int Item::getLevelDataForAuxValue(int auxValue)
{
	return 0;
}

bool Item::isStackedByData()
{
	return m_isStackedByData;
}


Item *Item::setStackedByData(bool isStackedByData)
{
	this->m_isStackedByData = isStackedByData;
	return this;
}


int Item::getMaxDamage()
{
	return maxDamage;
}


Item *Item::setMaxDamage(int maxDamage)
{
	this->maxDamage = maxDamage;
	return this;
}


bool Item::canBeDepleted()
{
	return maxDamage > 0 && !m_isStackedByData;
}

/**
* Returns true when the item was used to deal more than default damage
*
* @param itemInstance
* @param mob
* @param attacker
* @return
*/
bool Item::hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker)
{
	return false;
}

/**
* Returns true when the item was used to mine more efficiently
*
* @param itemInstance
* @param tile
* @param x
* @param y
* @param z
* @param owner
* @return
*/
bool Item::mineBlock(shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, shared_ptr<LivingEntity> owner)
{
	return false;
}

int Item::getAttackDamage(shared_ptr<Entity> entity)
{
	return 1;
}

bool Item::canDestroySpecial(Tile *tile)
{
	return false;
}

bool Item::interactEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, shared_ptr<LivingEntity> mob)
{
	return false;
}

Item *Item::handEquipped()
{
	m_handEquipped = true;
	return this;
}

bool Item::isHandEquipped()
{
	return m_handEquipped;
}

bool Item::isMirroredArt()
{
	return false;
}

Item *Item::setDescriptionId(unsigned int id)
{
	this->descriptionId = id;
	return this;
}

LPCWSTR Item::getDescription()
{
	return app.GetString(getDescriptionId());
	//return I18n::get(getDescriptionId());
}

LPCWSTR Item::getDescription(shared_ptr<ItemInstance> instance)
{
	return app.GetString(getDescriptionId(instance));
	//return I18n::get(getDescriptionId(instance));
}

unsigned int Item::getDescriptionId(int iData /*= -1*/)
{
	return descriptionId;
}

unsigned int Item::getDescriptionId(shared_ptr<ItemInstance> instance)
{
	return descriptionId;
}

Item *Item::setUseDescriptionId(unsigned int id)
{
	this->useDescriptionId = id;
	return this;
}

unsigned int Item::getUseDescriptionId()
{
	return useDescriptionId;
}

unsigned int Item::getUseDescriptionId(shared_ptr<ItemInstance> instance)
{
	return useDescriptionId;
}

Item *Item::setCraftingRemainingItem(Item *craftingRemainingItem)
{
	this->craftingRemainingItem = craftingRemainingItem;
	return this;
}

bool Item::shouldMoveCraftingResultToInventory(shared_ptr<ItemInstance> instance)
{
	// Default is good for the vast majority of items
	return true;
}

bool Item::shouldOverrideMultiplayerNBT()
{
	return true;
}

Item *Item::getCraftingRemainingItem()
{
	return craftingRemainingItem;
}

bool Item::hasCraftingRemainingItem()
{
	return craftingRemainingItem != nullptr;
}

wstring Item::getName()
{
	return L"";//I18n::get(getDescriptionId() + L".name");
}

int Item::getColor(shared_ptr<ItemInstance> item, int spriteLayer)
{
	return 0xffffff;
}

void Item::inventoryTick(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Entity> owner, int slot, bool selected) {
}

void Item::onCraftedBy(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
}

bool Item::isComplex()
{
	return false;
}

UseAnim Item::getUseAnimation(shared_ptr<ItemInstance> itemInstance)
{
	return UseAnim_none;
}

int Item::getUseDuration(shared_ptr<ItemInstance> itemInstance)
{
	return 0;
}

void Item::releaseUsing(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, int durationLeft)
{
}

Item *Item::setPotionBrewingFormula(const wstring &potionBrewingFormula)
{
	this->potionBrewingFormula = potionBrewingFormula;
	return this;
}

wstring Item::getPotionBrewingFormula()
{
	return potionBrewingFormula;
}

bool Item::hasPotionBrewingFormula()
{
	return !potionBrewingFormula.empty();
}

void Item::appendHoverText(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, vector<HtmlString> *lines, bool advanced)
{
}

wstring Item::getHoverName(shared_ptr<ItemInstance> itemInstance)
{
	//String elementName = ("" + Language.getInstance().getElementName(getDescription(itemInstance))).trim();
	//return elementName;
	return app.GetString(getDescriptionId(itemInstance));
}

bool Item::isFoil(shared_ptr<ItemInstance> itemInstance)
{
	if (itemInstance->isEnchanted()) return true;
	return false;
}

const Rarity *Item::getRarity(shared_ptr<ItemInstance> itemInstance)
{
	if (itemInstance->isEnchanted()) return Rarity::rare;
	return Rarity::common;
}

bool Item::isEnchantable(shared_ptr<ItemInstance> itemInstance)
{
	return getMaxStackSize() == 1 && canBeDepleted();
}

HitResult *Item::getPlayerPOVHitResult(Level *level, shared_ptr<Player> player, bool alsoPickLiquid)
{
	float a = 1;

	float xRot = player->xRotO + (player->xRot - player->xRotO) * a;
	float yRot = player->yRotO + (player->yRot - player->yRotO) * a;


	double x = player->xo + (player->x - player->xo) * a;
	double y = player->yo + (player->y - player->yo) * a + 1.62 - player->heightOffset;
	double z = player->zo + (player->z - player->zo) * a;

	Vec3 *from = Vec3::newTemp(x, y, z);

	float yCos = (float) cos(-yRot * Mth::RAD_TO_GRAD - PI);
	float ySin = (float) sin(-yRot * Mth::RAD_TO_GRAD - PI);
	float xCos = (float) -cos(-xRot * Mth::RAD_TO_GRAD);
	float xSin = (float) sin(-xRot * Mth::RAD_TO_GRAD);

	float xa = ySin * xCos;
	float ya = xSin;
	float za = yCos * xCos;

	double range = 5;
	Vec3 *to = from->add(xa * range, ya * range, za * range);
	return level->clip(from, to, alsoPickLiquid, !alsoPickLiquid);
}

int Item::getEnchantmentValue()
{
	return 0;
}

bool Item::hasMultipleSpriteLayers()
{
	return false;
}

Icon *Item::getLayerIcon(int auxValue, int spriteLayer)
{
	return getIcon(auxValue);
}

bool Item::mayBePlacedInAdventureMode()
{
	return true;
}

bool Item::isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem)
{
	return false;
}

void Item::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(m_textureName);
}

attrAttrModMap *Item::getDefaultAttributeModifiers()
{
	return new attrAttrModMap();
}


/*
	4J: These are necesary on the PS3.
		(and 4 and Vita).
*/
#if (defined __PS3__ || defined __ORBIS__ || defined __PSVITA__)
const int Item::iron_shovel_Id		;
const int Item::iron_pickaxe_Id		;
const int Item::iron_axe_Id		;
const int Item::flint_and_steel_Id	;
const int Item::apple_Id			;
const int Item::bow_Id				;
const int Item::arrow_Id			;
const int Item::coal_Id				;
const int Item::diamond_Id			;
const int Item::iron_ingot_Id		;
const int Item::gold_ingot_Id		;
const int Item::iron_sword_Id		;
const int Item::wooden_sword_Id		;
const int Item::wooden_shovel_Id		;
const int Item::wooden_pickaxe_Id		;
const int Item::wooden_axe_Id		;
const int Item::stone_sword_Id		;
const int Item::stone_shovel_Id		;
const int Item::stone_pickaxe_Id	;
const int Item::stone_axe_Id	;
const int Item::diamond_sword_Id	;
const int Item::diamond_shovel_Id	;
const int Item::diamond_pickaxe_Id	;
const int Item::diamond_axe_Id	;
const int Item::stick_Id			;
const int Item::bowl_Id				;
const int Item::mushroom_stew_Id		;
const int Item::rabbit_stew_Id		;
const int Item::golden_sword_Id		;
const int Item::golden_shovel_Id		;
const int Item::golden_pickaxe_Id		;
const int Item::golden_axe_Id		;
const int Item::string_Id			;
const int Item::feather_Id			;
const int Item::gunpowder_Id		;
const int Item::wooden_hoe_Id			;
const int Item::stone_hoe_Id		;
const int Item::iron_hoe_Id			;
const int Item::diamond_hoe_Id		;
const int Item::golden_hoe_Id			;
const int Item::wheat_seeds_Id		;
const int Item::wheat_Id			;
const int Item::bread_Id			;
const int Item::leather_helmet_Id		;
const int Item::leather_chestplate_Id	;
const int Item::leather_leggings_Id	;
const int Item::leather_boots_Id		;
const int Item::chainmail_helmet_Id		;
const int Item::chainmail_chestplate_Id	;
const int Item::chainmail_leggings_Id	;
const int Item::chainmail_boots_Id		;
const int Item::iron_helmet_Id		;
const int Item::iron_chestplate_Id	;
const int Item::iron_leggings_Id	;
const int Item::iron_boots_Id		;
const int Item::diamond_helmet_Id	;
const int Item::diamond_chestplate_Id;
const int Item::diamond_leggings_Id	;
const int Item::diamond_boots_Id	;
const int Item::golden_helmet_Id		;
const int Item::golden_chestplate_Id	;
const int Item::golden_leggings_Id	;
const int Item::golden_boots_Id		;
const int Item::flint_Id			;
const int Item::porkchop_Id		;
const int Item::cooked_porkchop_Id	;
const int Item::painting_Id			;
const int Item::golden_apple_Id		;
const int Item::standing_sign_Id				;
const int Item::wooden_door_Id			;
const int Item::bucket_Id		;
const int Item::water_bucket_Id		;
const int Item::lava_bucket_Id		;
const int Item::minecart_Id			;
const int Item::saddle_Id			;
const int Item::iron_door_Id			;
const int Item::redstone_Id			;
const int Item::snowball_Id			;
const int Item::boat_Id				;
const int Item::leather_Id			;
const int Item::milk_bucket_Id				;
const int Item::brick_Id				;
const int Item::clay_Id				;
const int Item::reeds_Id				;
const int Item::paper_Id				;
const int Item::book_Id				;
const int Item::slime_ball_Id			;
const int Item::chest_minecart_Id	;
const int Item::furnace_minecart_Id	;
const int Item::egg_Id				;
const int Item::compass_Id			;
const int Item::fishing_rod_Id		;
const int Item::clock_Id				;
const int Item::glowstone_dust_Id		;
const int Item::fish_Id			;
const int Item::cooked_fish_Id		;
const int Item::dye_Id		;
const int Item::bone_Id				;
const int Item::sugar_Id				;
const int Item::cake_Id				;
const int Item::bed_Id				;
const int Item::repeater_Id				;
const int Item::cookie_Id			;
const int Item::mace_Id;		// neoLegacy
const int Item::jelly_ball_Id;	// neoLegacy
const int Item::copper_ingot_Id;	// neoLegacy - Copper Age (Phase 1)
const int Item::copper_nugget_Id;	// neoLegacy - Copper Age (Phase 1)
const int Item::raw_copper_Id;	// neoLegacy - Copper Age (Phase 1)
const int Item::map_Id				;
const int Item::shears_Id			;
const int Item::melon_block_Id				;
const int Item::pumpkin_seeds_Id		;
const int Item::melon_seeds_Id		;
const int Item::beef_Id			;
const int Item::cooked_beef_Id		;
const int Item::chicken_Id		;
const int Item::cooked_chicken_Id	;
const int Item::rotten_flesh_Id		;
const int Item::ender_pearl_Id		;
const int Item::blaze_rod_Id			;
const int Item::ghast_tear_Id			;
const int Item::gold_nugget_Id		;
const int Item::netherwart_seeds_Id;
const int Item::potion_Id			;
const int Item::glass_bottle_Id		;
const int Item::spider_eye_Id			;
const int Item::fermented_spider_eye_Id;
const int Item::blaze_powder_Id		;
const int Item::magma_cream_Id		;
const int Item::brewing_stand_Id		;
const int Item::cauldron_Id			;
const int Item::eye_of_ender_Id		;
const int Item::speckled_melon_block_Id		;
const int Item::spawn_egg_Id;
const int Item::experience_bottle_Id			;
const int Item::skull_Id				;
const int Item::record_13_Id			;
const int Item::record_cat_Id			;
const int Item::record_blocks_Id			;
const int Item::record_chirp_Id			;
const int Item::record_far_Id			;
const int Item::record_mall_Id			;
const int Item::record_mellohi_Id			;
const int Item::record_stal_Id			;
const int Item::record_strad_Id			;
const int Item::record_ward_Id		    ;
const int Item::record_11_Id		    ;
const int Item::record_wait_Id			;
const int Item::fire_charge_Id			;
const int Item::item_frame_Id			;
const int Item::nether_brick_Id		;
const int Item::emerald_Id			;
const int Item::flower_pot_Id			;
const int Item::carrot_Id			;
const int Item::potato_Id			;
const int Item::baked_potato_Id		;
const int Item::poisonous_potato_Id	;
const int Item::golden_carrot_Id		;
const int Item::carrot_on_a_stick_Id	;
const int Item::pumpkin_pie_Id		;
const int Item::enchanted_book_Id		;
const int Item::quartz_Id		;
const int Item::beetroot_Id		;
const int Item::beetroot_seeds_Id	;
const int Item::beetroot_soup_Id	;
#endif

