#pragma once

#include "Goal.h"

class Container;
class CopperGolem;
class ItemInstance;

// neoLegacy - Copper Age (Phase 5b). Goal-based port of Java
// CopperGolemAi's TransportItemsBetweenContainers behaviour (the copper
// golem sorting items from a copper chest into a normal / trapped chest).
//
// Java uses a Brain/Activity system (ActivityData, MemoryModuleType,
// TransportItemsBetweenContainers); this tree has no Brain, it has Goals,
// so the behaviour is translated into a single Goal:
//
//   * Source: a copper chest (Tile::copper_chest_Id) that a player (or
//     another golem) currently has open (ChestTileEntity::openCount > 0,
//     the Java shouldQueueForTarget gate) and that holds at least one
//     item. Search radius 32 horizontal / 8 vertical (Java
//     TRANSPORT_ITEM_HORIZONTAL/VERTICAL_SEARCH_RADIUS).
//   * Destination: a normal chest (Tile::chest_Id) or trapped chest
//     (Tile::chest_trap_Id) that can be opened (ChestTile::getContainer
//     non-null — no solid block on top, no cat sitting on it).
//   * The golem carries ONE item stack at a time in its main hand
//     (SLOT_WEAPON, via getCarriedItem / setEquippedSlot). Picking up
//     moves one stack from the source chest into its hand; dropping
//     moves it into the destination chest. Source empty ->
//     GETTING_NO_ITEM; destination full -> DROPPING_NO_ITEM.
//   * Interaction timing matches Java's onReachedTargetInteraction:
//     tick 1 opens the chest (startOpen) and takes / deposits the stack
//     (state set from the result), tick 9 plays the get / drop sound,
//     tick 60 closes the chest (stopOpen) and the golem moves on.
//   * Cooldown: after a run (success, failure, or interruption) the
//     golem waits a random 60-100 ticks before the next run (Java
//     TRANSPORT_ITEMS_COOLDOWN_TICKS). It also starts life with a random
//     60-100 tick cooldown so it does not transport immediately on spawn.
//   * Priority: registered at 3 in CopperGolemAi::registerGoals, above
//     RandomStroll (4) so it pre-empts wandering when work exists, but
//     below Panic (1) and Float (0). It claims Move + Look control, so it
//     cannot run at the same time as RandomStroll.
//   * The chest inventory is the existing Container interface
//     (ChestTileEntity IS a Container; ChestTile::getContainer returns it,
//     or a CompoundContainer for a double chest). No chest redesign was
//     needed. See golem-transport-nov20.md for the full mapping.
class CopperGolemTransportGoal : public Goal
{
public:
	CopperGolemTransportGoal(CopperGolem *golem);

	virtual bool canUse() override;
	virtual bool canContinueToUse() override;
	virtual void start() override;
	virtual void stop() override;
	virtual void tick() override;

private:
	enum Phase
	{
		PHASE_NONE,
		PHASE_TO_SOURCE,
		PHASE_AT_SOURCE,
		PHASE_TO_DEST,
		PHASE_AT_DEST,
		PHASE_DONE,
		PHASE_FAILED
	};

	// Throttle the chest scan (a 65x17x65 box is ~72k getTile calls);
	// the golem only looks for work every 5 seconds.
	static const int SCAN_INTERVAL_TICKS = 100;
	// Give up on a leg if the golem has not arrived in 30 seconds.
	static const int TRAVEL_TIMEOUT_TICKS = 600;

	// Scan the 32/8 box around the golem for a source + destination pair.
	// Stores them in sourceX/Y/Z and destX/Y/Z. Returns true if a usable
	// pair exists (or a destination only, when the golem is already
	// carrying an item from an interrupted run).
	bool findTransportPair();
	// The openable Container at a chest position (nullptr if the chest is
	// blocked, has no TE, or is not a chest). Handles double chests via
	// ChestTile::getContainer (CompoundContainer).
	shared_ptr<Container> getChestContainer(int x, int y, int z);
	bool containerHasItem(shared_ptr<Container> container);
	// Move one whole stack from the first non-empty slot into the golem's
	// hand (returns the stack, or nullptr if the container is empty).
	shared_ptr<ItemInstance> takeFirstStack(shared_ptr<Container> container);
	// Move a stack into the container (merge into a same-item stack with
	// room, else an empty slot). Returns false if it does not fit whole.
	bool depositStack(shared_ptr<Container> container, shared_ptr<ItemInstance> stack);
	bool isAtChest(int cx, int cy, int cz);
	// One leg of the journey: walk to (cx, cy, cz); on arrival switch to
	// arrivePhase, on failure switch to PHASE_FAILED.
	void tickTravel(int cx, int cy, int cz, Phase arrivePhase);
	// The 1 / 9 / 60 tick interaction at a chest. isSource selects the
	// source (take) or the destination (deposit) behaviour.
	void tickInteract(bool isSource);

	CopperGolem *golem;
	Phase phase;
	// Ticks spent at the current chest (the 1 / 9 / 60 timing).
	int ticksAtTarget;
	// Ticks spent travelling on the current leg (the timeout).
	int travelTicks;
	int sourceX, sourceY, sourceZ;
	int destX, destY, destZ;
	// The chest currently held open (so stop() can close it if the goal
	// is interrupted mid-interaction).
	shared_ptr<Container> openContainer;
	// Golem tickCount at the last chest scan (the SCAN_INTERVAL throttle).
	int lastScanTick;
	// Sound id to play at tick 9 of the interaction (-1 = none).
	int pendingSound;
};
