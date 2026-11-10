#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.h"
#include "ExplosiveEnchantment.h"
#include "HtmlString.h"

ExplosiveEnchantment::ExplosiveEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::bow)
{
	// Name comes from the getFullname() override below instead of a
	// descriptionId - see AdrenalineEnchantment.cpp for why.
}

HtmlString ExplosiveEnchantment::getFullname(int level)
{
	wchar_t formatted[64];
	swprintf(formatted, 64, L"Explosive%ls", getLevelSuffix(level).c_str());
	return HtmlString(formatted, eHTMLColor_f);
}

int ExplosiveEnchantment::getMinCost(int level)
{
	return 20 + (level - 1) * 20;
}

int ExplosiveEnchantment::getMaxCost(int level)
{
	return getMinCost(level) + 30;
}

int ExplosiveEnchantment::getMaxLevel()
{
	return 3;
}

void ExplosiveEnchantment::triggerExplosion(Level *level, shared_ptr<Entity> owner, double x, double y, double z, int enchantLevel)
{
	if (enchantLevel <= 0) return;
	if (level == nullptr || level->isClientSide) return;

	// lvl1 ~1.75 radius, lvl2 2.5, lvl3 3.25 - noticeable but not a TNT-sized crater.
	float radius = 1.0f + enchantLevel * 0.75f;

	// destroyBlocks = false: this is meant to be a combat tool, not a mining/griefing one.
	level->explode(owner, x, y, z, radius, false);
}
