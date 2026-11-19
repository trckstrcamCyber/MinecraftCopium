#pragma once

#include "EnchantmentCategory.h"

class DamageSource;
class Mob;

class Enchantment //implements Descriptive<Enchantment> {
{
public :
	//static Enchantment *enchantments[256];
	static EnchantmentArray enchantments;
	static vector<Enchantment *> validEnchantments;

	static const int FREQ_COMMON = 10;
	static const int FREQ_UNCOMMON = 5;
	static const int FREQ_RARE = 2;
	static const int FREQ_VERY_RARE = 1;

	// armor
	static Enchantment *allDamageProtection;
	static Enchantment *fireProtection;
	static Enchantment *fallProtection;
	static Enchantment *explosionProtection;
	static Enchantment *projectileProtection;
	static Enchantment *drownProtection;
	static Enchantment *waterWorker;
    static Enchantment *waterWalker;
	static Enchantment *climb;
	static Enchantment *softStep;
	static Enchantment *frostWalker;
	static Enchantment *thorns;

	// neoLegacy custom armor enchantments
	static Enchantment *sonic;
	static Enchantment *adrenaline;
	static Enchantment *berserk;

	// weapon
	static Enchantment *damageBonus;
	static Enchantment *damageBonusUndead;
	static Enchantment *damageBonusArthropods;
	static Enchantment *knockback;
	static Enchantment *fireAspect;
	static Enchantment *lootBonus;

	// neoLegacy custom weapon enchantments
	static Enchantment *lifesteal;

	// digger
	static Enchantment *diggingBonus;
	static Enchantment *untouching;
	static Enchantment *digDurability;
	static Enchantment *resourceBonus;

	// neoLegacy custom digger enchantments
	static Enchantment *veinminer;
	static Enchantment *autoSmelt;
	static Enchantment *treeCapitator;

	// bows
	static Enchantment *arrowBonus;
	static Enchantment *arrowKnockback;
	static Enchantment *arrowFire;
	static Enchantment *arrowInfinite;

	// neoLegacy custom bow enchantments
	static Enchantment *explosive;

	// neoLegacy - crossbow enchantments (CrossbowItem.h)
	static Enchantment *quickCharge;
	static Enchantment *multishot;
	static Enchantment *piercing;
	static Enchantment *torched;

	// fishing rod
	static Enchantment *lure;
	static Enchantment *luckOfTheSea;

	// misc / treasure
	static Enchantment *mending;

	// neoLegacy curses (see CurseEnchantments.h)
	static Enchantment *hookOfMuddyWaters;
	static Enchantment *bindingCurse;
	static Enchantment *vanishingCurse;

	// neoLegacy - MAC-10 firearm enchantments
	static Enchantment *autoFire;
	static Enchantment *extendedMag;
	static Enchantment *rapidReload;
	static Enchantment *precisionBarrel;
	static Enchantment *hollowPoint;
	static Enchantment *reinforcedParts;

	// neoLegacy custom enchantments
	static Enchantment *airJump;
	static Enchantment *critStacking;	// neoLegacy - Zoro swords only
	static Enchantment *rageStacking;	// neoLegacy - Zoro swords only
	static Enchantment *pull;
	static Enchantment *miningPlus;
	static Enchantment *brightVision;
	static Enchantment *lavaWalker;

	// neoLegacy - Mace enchantments (MaceEnchantments.h)
	static Enchantment *density;
	static Enchantment *breach;
	static Enchantment *windBurst;

	const int id;

	static void staticCtor();

private:
	const int frequency;

public:
	const EnchantmentCategory *category;

protected:
	int descriptionId;

private:
	void _init(int id);

protected:
	Enchantment(int id, int frequency, const EnchantmentCategory *category);
	Enchantment(int id);

public:
	virtual int getFrequency();
	virtual int getMinLevel();
	virtual int getMaxLevel();
	virtual int getMinCost(int level);
	virtual int getMaxCost(int level);
	virtual int getDamageProtection(int level, DamageSource *source);
	virtual float getDamageBonus(int level, shared_ptr<LivingEntity> target);
	virtual bool isCompatibleWith(Enchantment *other) const;
	virtual bool isTreasureEnchantment() const { return false; }
	// Upstream renamed this to isTreasureOnly (525b894). The legacy name is kept because
	// EnchantmentEncyclopedia still queries it, and isTreasureOnly delegates to it so
	// subclasses overriding either name are honoured by the enchanting-table gating.
	virtual bool isTreasureOnly() { return isTreasureEnchantment(); }

	// neoLegacy: true for curses (Hook of Muddy Waters, Curse of Binding, Curse of Vanishing). Curses are shown in red, survive
	// the grindstone (GrindstoneMenu) and are never offered by the enchanting table or put on random books.
	virtual bool isCurse() const { return false; }
	virtual Enchantment *setDescriptionId(int id);
	virtual int getDescriptionId();
	virtual HtmlString getFullname(int level);
	virtual bool canEnchant(shared_ptr<ItemInstance> item);
	// 4J Added
	wstring getLevelString(int level);
	// neoLegacy - " II" style suffix for names; empty when the enchantment only has one level ("Mending", not "Mending I")
	wstring getLevelSuffix(int level);

private:
	
};