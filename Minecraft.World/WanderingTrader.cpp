#include "stdafx.h"
#include "Random.h"
#include "Inventory.h"
#include "PathNavigation.h"
#include "LookControl.h"
#include "AttributeInstance.h"
#include "ItemInstance.h"
#include "Player.h"
#include "net.minecraft.world.entity.h"
#include "SharedMonsterAttributes.h"
#include "MobEffectInstance.h"
#include "MobEffect.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.inventory.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.trading.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "com.mojang.nbt.h"
#include "SharedConstants.h"
#include "WanderingTrader.h"
#include "EnchantmentHelper.h"

// Wandering Trader trades, following the Java trade table (items that don't exist in this version are left out).
// A trade is "trader wants <want> -> player gets <give>". Each trader rolls 2 PURCHASE trades (it buys goods from you
// for emeralds), 2 SPECIAL trades and 5 ORDINARY trades, each with a limited number of uses.
struct TraderOffer
{
	int wantId; int wantAux; int wantCount;		// what the trader asks for
	int giveId; int giveAux; int giveCount;		// what the player gets
	int maxUses;
};

#define EM Item::emerald_Id

// Purchase: the trader buys these for emeralds.
static const TraderOffer PURCHASE_OFFERS[] =
{
	{ Tile::hay_block_Id,           0, 1,  EM, 0, 1, 2 },
	{ Item::potion_Id,              0, 1,  EM, 0, 1, 2 },		// water bottle
	{ Item::baked_potato_Id,        0, 4,  EM, 0, 1, 2 },
	{ Item::milk_bucket_Id,         0, 1,  EM, 0, 2, 2 },
	{ Item::water_bucket_Id,        0, 1,  EM, 0, 2, 2 },
	{ Item::fermented_spider_eye_Id,0, 1,  EM, 0, 3, 2 },
};
static const int PURCHASE_COUNT = sizeof(PURCHASE_OFFERS) / sizeof(PURCHASE_OFFERS[0]);

// Special: rarer goods (the enchanted iron pickaxe is added separately, see updateTrades).
static const TraderOffer SPECIAL_OFFERS[] =
{
	{ EM, 0, 1,  Tile::packed_ice_Id, 0, 1, 6 },
	{ EM, 0, 1,  Item::gunpowder_Id,  0, 4, 2 },
	{ EM, 0, 1,  Tile::log_Id,  0, 8, 4 },		// oak log
	{ EM, 0, 1,  Tile::log_Id,  1, 8, 4 },		// spruce log
	{ EM, 0, 1,  Tile::log_Id,  2, 8, 4 },		// birch log
	{ EM, 0, 1,  Tile::log_Id,  3, 8, 4 },		// jungle log
	{ EM, 0, 1,  Tile::log2_Id, 0, 8, 4 },		// acacia log
	{ EM, 0, 1,  Tile::log2_Id, 1, 8, 4 },		// dark oak log
	{ EM, 0, 3,  Tile::dirt_Id, 2, 3, 6 },		// podzol
	{ EM, 0, 5,  Item::potion_Id, 8206, 1, 1 },	// potion of invisibility
	// neoLegacy - Copper Age (Phase 4). Honeycomb: there are no bees in
	// this tree, so the Wandering Trader is the only source (Cameron's
	// decision, 2026-11-19). 1 emerald -> 3 honeycomb, max 5 uses; it is
	// the waxing ingredient, so it needs to be obtainable in bulk.
	{ EM, 0, 1,  Item::honeycomb_Id, 0, 3, 5 },
};
static const int SPECIAL_COUNT = sizeof(SPECIAL_OFFERS) / sizeof(SPECIAL_OFFERS[0]);
static const int SPECIAL_ENTRIES = SPECIAL_COUNT + 1;		// + the enchanted iron pickaxe

// Ordinary: everyday goods.
static const TraderOffer ORDINARY_OFFERS[] =
{
	{ EM,0,1, Tile::tallgrass_Id, 2, 1, 12 },	// fern
	{ EM,0,1, Item::reeds_Id, 0, 1, 8 },		// sugar cane
	{ EM,0,1, Tile::pumpkin_Id, 0, 1, 4 },
	{ EM,0,1, Tile::yellow_flower_Id, 0, 1, 12 },	// dandelion
	{ EM,0,1, Tile::red_flower_Id, 0, 1, 12 },	// poppy
	{ EM,0,1, Tile::red_flower_Id, 2, 1, 12 },	// allium
	{ EM,0,1, Tile::red_flower_Id, 3, 1, 12 },	// azure bluet
	{ EM,0,1, Tile::red_flower_Id, 4, 1, 12 },	// red tulip
	{ EM,0,1, Tile::red_flower_Id, 5, 1, 12 },	// orange tulip
	{ EM,0,1, Tile::red_flower_Id, 6, 1, 12 },	// white tulip
	{ EM,0,1, Tile::red_flower_Id, 7, 1, 12 },	// pink tulip
	{ EM,0,1, Tile::red_flower_Id, 8, 1, 12 },	// oxeye daisy
	{ EM,0,1, Tile::red_flower_Id, 1, 1, 8 },	// blue orchid
	{ EM,0,1, Item::wheat_seeds_Id, 0, 1, 12 },
	{ EM,0,1, Item::beetroot_seeds_Id, 0, 1, 12 },
	{ EM,0,1, Item::pumpkin_seeds_Id, 0, 1, 12 },
	{ EM,0,1, Item::melon_seeds_Id, 0, 1, 12 },
	{ EM,0,1, Item::name_tag_Id, 0, 1, 5 },
	{ EM,0,1, Item::dye_Id, 15, 3, 12 },		// white dye
	{ EM,0,1, Item::dye_Id, 7,  3, 12 },		// light gray
	{ EM,0,1, Item::dye_Id, 8,  3, 12 },		// gray
	{ EM,0,1, Item::dye_Id, 0,  3, 12 },		// black
	{ EM,0,1, Item::dye_Id, 3,  3, 12 },		// brown
	{ EM,0,1, Item::dye_Id, 1,  3, 12 },		// red
	{ EM,0,1, Item::dye_Id, 14, 3, 12 },		// orange
	{ EM,0,1, Item::dye_Id, 11, 3, 12 },		// yellow
	{ EM,0,1, Item::dye_Id, 10, 3, 12 },		// lime
	{ EM,0,1, Item::dye_Id, 2,  3, 12 },		// green
	{ EM,0,1, Item::dye_Id, 6,  3, 12 },		// cyan
	{ EM,0,1, Item::dye_Id, 12, 3, 12 },		// light blue
	{ EM,0,1, Item::dye_Id, 4,  3, 12 },		// blue (lapis lazuli in this version)
	{ EM,0,1, Item::dye_Id, 5,  3, 12 },		// purple
	{ EM,0,1, Item::dye_Id, 13, 3, 12 },		// magenta
	{ EM,0,1, Item::dye_Id, 9,  3, 12 },		// pink
	{ EM,0,1, Tile::vine_Id, 0, 3, 4 },
	{ EM,0,1, Tile::mushroom_brown_Id, 0, 3, 4 },
	{ EM,0,1, Tile::mushroom_red_Id, 0, 3, 4 },
	{ EM,0,1, Tile::waterlily_Id, 0, 5, 2 },	// lily pad
	{ EM,0,1, Tile::sand_Id, 0, 8, 8 },
	{ EM,0,1, Tile::sand_Id, 1, 4, 6 },		// red sand
	{ EM,0,2, Tile::glowstone_Id, 0, 1, 5 },
	{ EM,0,3, Tile::cactus_Id, 0, 1, 8 },
	{ EM,0,4, Item::slime_ball_Id, 0, 1, 5 },
	{ EM,0,5, Tile::sapling_Id, 0, 1, 8 },		// oak sapling
	{ EM,0,5, Tile::sapling_Id, 1, 1, 8 },		// spruce sapling
	{ EM,0,5, Tile::sapling_Id, 2, 1, 8 },		// birch sapling
	{ EM,0,5, Tile::sapling_Id, 3, 1, 8 },		// jungle sapling
	{ EM,0,5, Tile::sapling_Id, 4, 1, 8 },		// acacia sapling
	{ EM,0,5, Tile::sapling_Id, 5, 1, 8 },		// dark oak sapling
};
static const int ORDINARY_COUNT = sizeof(ORDINARY_OFFERS) / sizeof(ORDINARY_OFFERS[0]);

static const int TRADER_LIFETIME_MIN = 48000;		// ticks (40 min) ... up to 72000 (60 min) before it wanders off
static const int TRADER_LIFETIME_RANGE = 24000;

WanderingTrader::WanderingTrader(Level *level) : AgableMob(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	offers = nullptr;
	despawnDelay = TRADER_LIFETIME_MIN + random->nextInt(TRADER_LIFETIME_RANGE + 1);

	this->setSize(0.6f, 1.8f);

	getNavigation()->setCanOpenDoors(true);
	getNavigation()->setAvoidWater(true);
	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new AvoidPlayerGoal(this, typeid(Zombie), 8, 0.5, 0.5));
	goalSelector.addGoal(1, new PanicGoal(this, 0.5));
	goalSelector.addGoal(5, new MoveTowardsRestrictionGoal(this, 0.35));
	goalSelector.addGoal(6, new RandomStrollGoal(this, 0.35));
	goalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 6));
	goalSelector.addGoal(8, new RandomLookAroundGoal(this));
}

WanderingTrader::~WanderingTrader()
{
	delete offers;
}

bool WanderingTrader::useNewAi()
{
	return true;
}

void WanderingTrader::registerAttributes()
{
	AgableMob::registerAttributes();
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(20);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5f);
}

// ---- trades ------------------------------------------------------------------------------------

static void addOffer(MerchantRecipeList *list, const TraderOffer &o)
{
	list->push_back(new MerchantRecipe(
		std::make_shared<ItemInstance>(o.wantId, o.wantCount, o.wantAux), nullptr,
		std::make_shared<ItemInstance>(o.giveId, o.giveCount, o.giveAux), 0, o.maxUses));
}

// Picks `picks` distinct entries out of `count` (partial Fisher-Yates) and calls addIndex for each.
static void pickDistinct(Random *random, int count, int picks, int *scratch, int *chosen)
{
	for (int i = 0; i < count; i++) scratch[i] = i;
	for (int i = 0; i < picks && i < count; i++)
	{
		int j = i + random->nextInt(count - i);
		int t = scratch[i]; scratch[i] = scratch[j]; scratch[j] = t;
		chosen[i] = scratch[i];
	}
}

void WanderingTrader::updateTrades()
{
	if (offers != nullptr) return;
	offers = new MerchantRecipeList();

	int scratch[64];
	int chosen[8];

	// 2 purchase trades
	pickDistinct(random, PURCHASE_COUNT, 2, scratch, chosen);
	for (int i = 0; i < 2; i++) addOffer(offers, PURCHASE_OFFERS[chosen[i]]);

	// 2 special trades (the enchanted iron pickaxe is one more entry in the pool)
	pickDistinct(random, SPECIAL_ENTRIES, 2, scratch, chosen);
	for (int i = 0; i < 2; i++)
	{
		if (chosen[i] < SPECIAL_COUNT)
		{
			addOffer(offers, SPECIAL_OFFERS[chosen[i]]);
		}
		else
		{
			// 6-20 emeralds for an enchanted iron pickaxe
			shared_ptr<ItemInstance> pick = std::make_shared<ItemInstance>(Item::iron_pickaxe, 1);
			EnchantmentHelper::enchantItem(random, pick, 5 + random->nextInt(15));
			offers->push_back(new MerchantRecipe(
				std::make_shared<ItemInstance>(Item::emerald_Id, 6 + random->nextInt(15), 0), nullptr, pick, 0, 1));
		}
	}

	// 5 ordinary trades
	pickDistinct(random, ORDINARY_COUNT, 5, scratch, chosen);
	for (int i = 0; i < 5; i++) addOffer(offers, ORDINARY_OFFERS[chosen[i]]);
}

MerchantRecipeList *WanderingTrader::getOffers(shared_ptr<Player> forPlayer)
{
	updateTrades();
	if (offers != nullptr) offers->applyHeroDiscount(forPlayer);	// neoLegacy - Hero of the Village
	return offers;
}

void WanderingTrader::overrideOffers(MerchantRecipeList *recipeList)
{
}

void WanderingTrader::setTradingPlayer(shared_ptr<Player> player)
{
	tradingPlayer = weak_ptr<Player>(player);
}

shared_ptr<Player> WanderingTrader::getTradingPlayer()
{
	return tradingPlayer.lock();
}

void WanderingTrader::notifyTrade(MerchantRecipe *activeRecipe)
{
	activeRecipe->increaseUses();
	ambientSoundTime = -getAmbientSoundInterval();
	playSound(eSoundType_MOB_VILLAGER_YES, getSoundVolume(), getVoicePitch());
}

void WanderingTrader::notifyTradeUpdated(shared_ptr<ItemInstance> item)
{
	if (!level->isClientSide && (ambientSoundTime > (-getAmbientSoundInterval() + SharedConstants::TICKS_PER_SECOND)))
	{
		ambientSoundTime = -getAmbientSoundInterval();
		playSound(item != nullptr ? eSoundType_MOB_VILLAGER_YES : eSoundType_MOB_VILLAGER_NO, getSoundVolume(), getVoicePitch());
	}
}

wstring WanderingTrader::getDisplayName()
{
	if (hasCustomName()) return getCustomName();
	return app.GetString(IDS_WANDERING_TRADER);
}

bool WanderingTrader::mobInteract(shared_ptr<Player> player)
{
	shared_ptr<ItemInstance> item = player->inventory->getSelected();
	bool holdingSpawnEgg = item != nullptr && item->id == Item::spawn_egg_Id;

	if (!player->isSneaking() && !holdingSpawnEgg && isAlive() && !isTrading())
	{
		if (!level->isClientSide)
		{
			updateTrades();
			setTradingPlayer(player);
			player->openTrading(dynamic_pointer_cast<Merchant>(shared_from_this()), getDisplayName());
		}
		return true;
	}
	return AgableMob::mobInteract(player);
}

// ---- behaviour ---------------------------------------------------------------------------------

void WanderingTrader::aiStep()
{
	AgableMob::aiStep();

	if (level->isClientSide) return;

	// Stand still and face the customer while trading; let go when they close the window or walk away.
	shared_ptr<Player> p = tradingPlayer.lock();
	if (p != nullptr)
	{
		if (!p->isAlive() || p->removed || p->containerMenu == p->inventoryMenu || distanceToSqr(p) > 16 * 16)
		{
			setTradingPlayer(nullptr);
		}
		else
		{
			getNavigation()->stop();
			getLookControl()->setLookAt(p, 30, 30);
		}
	}
	else
	{
		// Wanders off after its lifetime (never while someone is trading with it).
		if (--despawnDelay <= 0)
		{
			remove();
			return;
		}
	}

	// Potion of invisibility at night, washed off again in the morning. It waits a minute after appearing
	// and never vanishes on someone standing right next to it.
	if (!level->isDay())
	{
		if (!hasEffect(MobEffect::invisibility) && !isTrading() && tickCount > 1200
			&& level->getNearestPlayer(shared_from_this(), 8) == nullptr)
		{
			addEffect(new MobEffectInstance(MobEffect::invisibility->id, 2400, 0));
		}
	}
	else if (hasEffect(MobEffect::invisibility) && !isTrading())
	{
		removeEffect(MobEffect::invisibility->id);
	}
}

void WanderingTrader::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	// A trader that has drunk its potion may still be carrying a (rare) spare.
	if (hasEffect(MobEffect::invisibility) && random->nextFloat() < 0.085f)
	{
		spawnAtLocation(std::make_shared<ItemInstance>(Item::potion_Id, 1, 8206), 0);
	}
}

void WanderingTrader::addAdditonalSaveData(CompoundTag *tag)
{
	AgableMob::addAdditonalSaveData(tag);
	tag->putInt(L"DespawnDelay", despawnDelay);
	if (offers != nullptr)
	{
		tag->putCompound(L"Offers", offers->createTag());
	}
}

void WanderingTrader::readAdditionalSaveData(CompoundTag *tag)
{
	AgableMob::readAdditionalSaveData(tag);
	if (tag->contains(L"DespawnDelay")) despawnDelay = tag->getInt(L"DespawnDelay");
	if (tag->contains(L"Offers"))
	{
		CompoundTag *offersTag = tag->getCompound(L"Offers");
		delete offers;
		offers = new MerchantRecipeList(offersTag);
	}
}

int WanderingTrader::getAmbientSound()
{
	return isTrading() ? eSoundType_MOB_VILLAGER_HAGGLE : eSoundType_MOB_VILLAGER_IDLE;
}

int WanderingTrader::getHurtSound()
{
	return eSoundType_MOB_VILLAGER_HIT;
}

int WanderingTrader::getDeathSound()
{
	return eSoundType_MOB_VILLAGER_DEATH;
}
