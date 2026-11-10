#pragma once
using namespace std;

#include "AgableMob.h"
#include "Merchant.h"
#include "Npc.h"

class Player;
class Level;
class MerchantRecipeList;

// neoLegacy - Wandering Trader (modern Minecraft). A travelling merchant who shows up near players, sells
// a handful of odd goods for emeralds, and wanders off (despawns) after about 40 minutes.
//   * 9 random trades following the Java table: 2 it buys from you for emeralds, 2 special and 5 ordinary
//     goods it sells; each with limited uses;
//   * drinks a potion of invisibility at night and cleans it off again by day;
//   * flees from zombies (and husks/drowned), stands still while you trade;
//   * spawned by the world roughly once per in-game day (see ServerLevel::trySpawnWanderingTrader).
// It arrives with two leashed blue sheep (standing in for the trader llamas, which don't exist yet).
// Npc is what makes it a "Creature": the entity tracker only sends Creature mobs to clients (Villager does the same).
class WanderingTrader : public AgableMob, public Npc, public Merchant
{
public:
	eINSTANCEOF GetType() { return eTYPE_WANDERING_TRADER; }
	static Entity *create(Level *level) { return new WanderingTrader(level); }

private:
	weak_ptr<Player> tradingPlayer;
	MerchantRecipeList *offers;
	int despawnDelay;

	void updateTrades();

public:
	WanderingTrader(Level *level);
	virtual ~WanderingTrader();

	virtual bool useNewAi();
	virtual bool mobInteract(shared_ptr<Player> player);
	virtual shared_ptr<AgableMob> getBreedOffspring(shared_ptr<AgableMob> target) { return nullptr; }
	virtual void aiStep();
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual bool removeWhenFarAway() { return false; }

	bool isTrading() { return tradingPlayer.lock() != nullptr; }

	// Merchant
	virtual void setTradingPlayer(shared_ptr<Player> player);
	virtual shared_ptr<Player> getTradingPlayer();
	virtual MerchantRecipeList *getOffers(shared_ptr<Player> forPlayer);
	virtual void overrideOffers(MerchantRecipeList *recipeList);
	virtual void notifyTrade(MerchantRecipe *activeRecipe);
	virtual void notifyTradeUpdated(shared_ptr<ItemInstance> item);
	virtual wstring getDisplayName();

protected:
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);
	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
};
