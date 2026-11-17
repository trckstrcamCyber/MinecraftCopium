#include "stdafx.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.item.h"
#include "Enchantment.h"

#include "WaterWalkerEnchantment.h"
#include "ClimbEnchantment.h"
#include "SoftStepEnchantment.h"
#include "FrostWalkerEnchantment.h"
#include "MendingEnchantment.h"
#include "LuckOfTheSeaEnchantment.h"
#include "LureEnchantment.h"

// neoLegacy custom enchantments
#include "SonicEnchantment.h"
#include "AdrenalineEnchantment.h"
#include "BerserkEnchantment.h"
#include "LifestealEnchantment.h"
#include "VeinminerEnchantment.h"
#include "AutoSmeltEnchantment.h"
#include "TreeCapitatorEnchantment.h"
#include "ExplosiveEnchantment.h"
#include "AirJumpEnchantment.h"
#include "PullEnchantment.h"
#include "MiningPlusEnchantment.h"
#include "BrightVisionEnchantment.h"
#include "LavaWalkerEnchantment.h"
#include "CurseEnchantments.h"
#include "FirearmEnchantment.h"
#include "MaceEnchantments.h"
#include "MaceItem.h"
#include "CrossbowItem.h"

//Enchantment *Enchantment::enchantments[256];
EnchantmentArray Enchantment::enchantments = EnchantmentArray( 256 );
vector<Enchantment *> Enchantment::validEnchantments;

Enchantment *Enchantment::allDamageProtection = nullptr;
Enchantment *Enchantment::fireProtection = nullptr;
Enchantment *Enchantment::fallProtection = nullptr;
Enchantment *Enchantment::explosionProtection = nullptr;
Enchantment *Enchantment::projectileProtection = nullptr;
Enchantment *Enchantment::drownProtection = nullptr;
Enchantment *Enchantment::waterWorker = nullptr;
Enchantment *Enchantment::waterWalker = nullptr;
Enchantment *Enchantment::climb = nullptr;
Enchantment *Enchantment::softStep = nullptr;
Enchantment *Enchantment::frostWalker = nullptr;
Enchantment *Enchantment::thorns = nullptr;

// neoLegacy custom enchantments
Enchantment *Enchantment::sonic = nullptr;
Enchantment *Enchantment::adrenaline = nullptr;
Enchantment *Enchantment::berserk = nullptr;
Enchantment *Enchantment::lifesteal = nullptr;
Enchantment *Enchantment::veinminer = nullptr;
Enchantment *Enchantment::autoSmelt = nullptr;
Enchantment *Enchantment::treeCapitator = nullptr;
Enchantment *Enchantment::explosive = nullptr;
Enchantment *Enchantment::quickCharge = nullptr;	// neoLegacy - crossbow
Enchantment *Enchantment::multishot = nullptr;
Enchantment *Enchantment::piercing = nullptr;
Enchantment *Enchantment::torched = nullptr;
Enchantment *Enchantment::airJump = nullptr;
Enchantment *Enchantment::critStacking = nullptr;
Enchantment *Enchantment::rageStacking = nullptr;
Enchantment *Enchantment::pull = nullptr;
Enchantment *Enchantment::miningPlus = nullptr;
Enchantment *Enchantment::brightVision = nullptr;
Enchantment *Enchantment::lavaWalker = nullptr;
Enchantment *Enchantment::density = nullptr;	// neoLegacy - Mace
Enchantment *Enchantment::breach = nullptr;
Enchantment *Enchantment::windBurst = nullptr;

// weapon
Enchantment *Enchantment::damageBonus = nullptr;
Enchantment *Enchantment::damageBonusUndead = nullptr;
Enchantment *Enchantment::damageBonusArthropods = nullptr;
Enchantment *Enchantment::knockback = nullptr;
Enchantment *Enchantment::fireAspect = nullptr;
Enchantment *Enchantment::lootBonus = nullptr;

// digger
Enchantment *Enchantment::diggingBonus = nullptr;
Enchantment *Enchantment::untouching = nullptr;
Enchantment *Enchantment::digDurability = nullptr;
Enchantment *Enchantment::resourceBonus = nullptr;

// bows
Enchantment *Enchantment::arrowBonus = nullptr;
Enchantment *Enchantment::arrowKnockback = nullptr;
Enchantment *Enchantment::arrowFire = nullptr;
Enchantment *Enchantment::arrowInfinite = nullptr;

// fishing rod
Enchantment *Enchantment::lure = nullptr;
Enchantment *Enchantment::luckOfTheSea = nullptr;

// misc / treasure
Enchantment *Enchantment::mending = nullptr;

// neoLegacy curses
Enchantment *Enchantment::hookOfMuddyWaters = nullptr;
Enchantment *Enchantment::bindingCurse = nullptr;
Enchantment *Enchantment::vanishingCurse = nullptr;
Enchantment *Enchantment::autoFire = nullptr;
Enchantment *Enchantment::extendedMag = nullptr;
Enchantment *Enchantment::rapidReload = nullptr;
Enchantment *Enchantment::precisionBarrel = nullptr;
Enchantment *Enchantment::hollowPoint = nullptr;
Enchantment *Enchantment::reinforcedParts = nullptr;

void Enchantment::staticCtor()
{
	allDamageProtection = new ProtectionEnchantment(0, FREQ_COMMON, ProtectionEnchantment::ALL);
	fireProtection = new ProtectionEnchantment(1, FREQ_UNCOMMON, ProtectionEnchantment::FIRE);
	fallProtection = new ProtectionEnchantment(2, FREQ_UNCOMMON, ProtectionEnchantment::FALL);
	explosionProtection = new ProtectionEnchantment(3, FREQ_RARE, ProtectionEnchantment::EXPLOSION);
	projectileProtection = new ProtectionEnchantment(4, FREQ_UNCOMMON, ProtectionEnchantment::PROJECTILE);
	drownProtection = new OxygenEnchantment(5, FREQ_RARE);
	waterWorker = new WaterWorkerEnchantment(6, FREQ_RARE);
    waterWalker = new WaterWalkerEnchantment(8, FREQ_RARE);
	climb = new ClimbEnchantment(10, FREQ_RARE);
	softStep = new SoftStepEnchantment(11, FREQ_UNCOMMON);
	frostWalker = new FrostWalkerEnchantment(9, FREQ_RARE);
	thorns = new ThornsEnchantment(7, FREQ_VERY_RARE);

	// neoLegacy custom armor enchantments
	sonic = new SonicEnchantment(12, FREQ_UNCOMMON);
	adrenaline = new AdrenalineEnchantment(13, FREQ_RARE);
	berserk = new BerserkEnchantment(14, FREQ_RARE);

	// weapon
	damageBonus = new DamageEnchantment(16, FREQ_COMMON, DamageEnchantment::ALL);
	damageBonusUndead = new DamageEnchantment(17, FREQ_UNCOMMON, DamageEnchantment::UNDEAD);
	damageBonusArthropods = new DamageEnchantment(18, FREQ_UNCOMMON, DamageEnchantment::ARTHROPODS);
	knockback = new KnockbackEnchantment(19, FREQ_UNCOMMON);
	fireAspect = new FireAspectEnchantment(20, FREQ_RARE);
	lootBonus = new LootBonusEnchantment(21, FREQ_RARE, EnchantmentCategory::weapon);

	// neoLegacy custom weapon enchantments
	lifesteal = new LifestealEnchantment(22, FREQ_RARE);

	// digger
	diggingBonus = new DiggingEnchantment(32, FREQ_COMMON);
	untouching = new UntouchingEnchantment(33, FREQ_VERY_RARE);
	digDurability = new DigDurabilityEnchantment(34, FREQ_UNCOMMON);
	resourceBonus = new LootBonusEnchantment(35, FREQ_RARE, EnchantmentCategory::digger);

	// neoLegacy custom digger enchantments
	veinminer = new VeinminerEnchantment(36, FREQ_VERY_RARE);
	autoSmelt = new AutoSmeltEnchantment(37, FREQ_RARE);
	treeCapitator = new TreeCapitatorEnchantment(38, FREQ_VERY_RARE);

	// bows
	arrowBonus = new ArrowDamageEnchantment(48, FREQ_COMMON);
	arrowKnockback = new ArrowKnockbackEnchantment(49, FREQ_RARE);
	arrowFire = new ArrowFireEnchantment(50, FREQ_RARE);
	arrowInfinite = new ArrowInfiniteEnchantment(51, FREQ_VERY_RARE);

	// neoLegacy custom bow enchantments
	explosive = new ExplosiveEnchantment(52, FREQ_VERY_RARE);

	// neoLegacy - crossbow enchantments (ids 53-56 are the free slots right after the bow block)
	quickCharge = new QuickChargeEnchantment(53, FREQ_UNCOMMON);
	multishot = new MultishotEnchantment(54, FREQ_RARE);
	piercing = new PiercingEnchantment(55, FREQ_COMMON);
	torched = new TorchedEnchantment(56, FREQ_RARE);

	// fishing rod
	lure = new LureEnchantment(64, FREQ_RARE);
	luckOfTheSea = new LuckOfTheSeaEnchantment(65, FREQ_RARE);

	// misc / treasure
	mending = new MendingEnchantment(70, FREQ_RARE);

	// neoLegacy custom enchantments
	airJump = new AirJumpEnchantment(71, FREQ_RARE);
	pull = new PullEnchantment(72, FREQ_VERY_RARE);
	miningPlus = new MiningPlusEnchantment(73, FREQ_RARE);
	brightVision = new BrightVisionEnchantment(74, FREQ_RARE);
	lavaWalker = new LavaWalkerEnchantment(75, FREQ_RARE);

	// neoLegacy - Mace enchantments (ids 25-27 are the free slots in the melee block)
	density = new DensityEnchantment(25, FREQ_UNCOMMON);
	breach = new BreachEnchantment(26, FREQ_RARE);
	windBurst = new WindBurstEnchantment(27, FREQ_RARE);

	// neoLegacy curses
	hookOfMuddyWaters = new HookOfMuddyWatersEnchantment(66, FREQ_VERY_RARE);
	bindingCurse = new BindingCurseEnchantment(80, FREQ_VERY_RARE);
	vanishingCurse = new VanishingCurseEnchantment(81, FREQ_VERY_RARE);

	// neoLegacy - MAC-10 firearm enchantments (IDs 82-87 were unused in this branch).
	autoFire = new FirearmEnchantment(82, FREQ_VERY_RARE, FirearmEnchantment::AutoFire, IDS_ENCHANTMENT_AUTOFIRE);
	extendedMag = new FirearmEnchantment(83, FREQ_RARE, FirearmEnchantment::ExtendedMag, IDS_ENCHANTMENT_EXTENDED_MAG);
	rapidReload = new FirearmEnchantment(84, FREQ_RARE, FirearmEnchantment::RapidReload, IDS_ENCHANTMENT_RAPID_RELOAD);
	precisionBarrel = new FirearmEnchantment(85, FREQ_UNCOMMON, FirearmEnchantment::PrecisionBarrel, IDS_ENCHANTMENT_PRECISION_BARREL);
	hollowPoint = new FirearmEnchantment(86, FREQ_RARE, FirearmEnchantment::HollowPoint, IDS_ENCHANTMENT_HOLLOW_POINT);
	reinforcedParts = new FirearmEnchantment(87, FREQ_UNCOMMON, FirearmEnchantment::ReinforcedParts, IDS_ENCHANTMENT_REINFORCED_PARTS);

	// neoLegacy Zoro sword enchantments
	critStacking = new CritStackingEnchantment(23, FREQ_RARE);
	rageStacking = new RageStackingEnchantment(24, FREQ_RARE);

	for(unsigned int i = 0; i < 256; ++i)
	{
		Enchantment *enchantment = enchantments[i];
		if (enchantment != nullptr && !enchantment->isTreasureEnchantment() && !enchantment->isCurse())
		{
			validEnchantments.push_back(enchantment);
		}
	}
}

void Enchantment::_init(int id)
{
	if (enchantments[id] != nullptr)
	{
		app.DebugPrintf("Duplicate enchantment id!");
#ifndef _CONTENT_PACKAGE
		DEBUG_BREAK();
#endif
		//throw new IllegalArgumentException("Duplicate enchantment id!");
	}
	enchantments[id] = this;
}

Enchantment::Enchantment(int id, int frequency, const EnchantmentCategory *category) : id(id), frequency(frequency), category(category)
{
	_init(id);
}

Enchantment::Enchantment(int id) : id(id), frequency(FREQ_COMMON), category(EnchantmentCategory::all)
{
	_init(id);
}

int Enchantment::getFrequency()
{
	return frequency;
}

int Enchantment::getMinLevel()
{
	return 1;
}

int Enchantment::getMaxLevel()
{
	return 1;
}

int Enchantment::getMinCost(int level)
{
	return 1 + level * 10;
}

int Enchantment::getMaxCost(int level)
{
	return getMinCost(level) + 5;
}

int Enchantment::getDamageProtection(int level, DamageSource *source)
{
	return 0;
}

float Enchantment::getDamageBonus(int level, shared_ptr<LivingEntity> target)
{
	return 0.0f;
}

bool Enchantment::isCompatibleWith(Enchantment *other) const
{
	return this != other;
}

Enchantment *Enchantment::setDescriptionId(int id)
{
	descriptionId = id;
	return this;
}

int Enchantment::getDescriptionId()
{
	return descriptionId;
}

HtmlString Enchantment::getFullname(int level)
{
	wchar_t formatted[256];
	swprintf(formatted, 256, L"%ls%ls", app.GetString(getDescriptionId()), getLevelSuffix(level).c_str());

	return HtmlString(formatted, isCurse() ? eHTMLColor_c : eHTMLColor_f);	// neoLegacy - curses are red
}

bool Enchantment::canEnchant(shared_ptr<ItemInstance> item)
{
	// neoLegacy - the Mace can only take the enchantments in MaceItem::isEnchantmentAllowed (Java's list).
	if (MaceItem::isMace(item) && !MaceItem::isEnchantmentAllowed(this))
	{
		return false;
	}
	// neoLegacy - the Crossbow only takes the enchantments in CrossbowItem::isEnchantmentAllowed
	if (CrossbowItem::isCrossbow(item) && !CrossbowItem::isEnchantmentAllowed(this))
	{
		return false;
	}
	return category->canEnchant(item->getItem());
}

// 4J Added
wstring Enchantment::getLevelString(int level)
{
	int stringId = IDS_ENCHANTMENT_LEVEL_1;
	switch(level)
	{
	case 2:
		stringId = IDS_ENCHANTMENT_LEVEL_2;
		break;
	case 3:
		stringId = IDS_ENCHANTMENT_LEVEL_3;
		break;
	case 4:
		stringId = IDS_ENCHANTMENT_LEVEL_4;
		break;
	case 5:
		stringId = IDS_ENCHANTMENT_LEVEL_5;
		break;
	case 6:
		stringId = IDS_ENCHANTMENT_LEVEL_6;
		break;
	case 7:
		stringId = IDS_ENCHANTMENT_LEVEL_7;
		break;
	case 8:
		stringId = IDS_ENCHANTMENT_LEVEL_8;
		break;
	case 9:
		stringId = IDS_ENCHANTMENT_LEVEL_9;
		break;
	case 10:
		stringId = IDS_ENCHANTMENT_LEVEL_10;
		break;
	};
	return app.GetString(stringId); //I18n.get("enchantment.level." + level);
}

// neoLegacy - enchantments with a single level show just their name; multi-level ones show " <level>"
wstring Enchantment::getLevelSuffix(int level)
{
	if (getMaxLevel() <= 1) return L"";
	return wstring(L" ") + getLevelString(level);
}
