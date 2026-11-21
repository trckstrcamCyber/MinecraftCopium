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
//   * Source: a copper chest (Tile::copper_chest_Id) that holds at least
//     one item. Search radius 32 horizontal / 8 vertical (Java
//     TRANSPORT_ITEM_HORIZONTAL/VERTICAL_SEARCH_RADIUS). (Java gates on
//     shouldQueueForTarget — a non-empty opener list — but openCount is
//     a LIVE viewer count here, so that gate made the golem never find
//     a source; any copper chest holding an item is a valid source.)
//   * Destination: a normal chest (Tile::chest_Id) or trapped chest
//     (Tile::chest_trap_Id) that can be opened (ChestTile::getContainer
//     non-null — no solid block on top, no cat sitting on it) AND that
//     accepts the carried item: Java only deposits into a chest that is
//     empty or already holds the same item type. The destination is
//     picked AFTER the pickup, once the item type is known — picking
//     the nearest chest up front is what made the golem dump items
//     into the first chest it saw instead of sorting them.
//   * The golem carries up to 16 items of ONE type at a time in its main
//     hand (SLOT_WEAPON, via getCarriedItem / setEquippedSlot; Java cap).
//     Picking up moves up to 16 items from the source chest into its
//     hand; dropping moves them into the destination chest. Source empty
//     -> GETTING_NO_ITEM; no accepting destination -> DROPPING_NO_ITEM.
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

	// Scan the 32/8 box around the golem for work. When the golem is
	// already carrying an item (interrupted run), finds the nearest
	// chest that accepts that item (findMatchingDest) and stores it in
	// destX/Y/Z. When not carrying, finds the nearest copper chest
	// holding an item and stores it in sourceX/Y/Z — the destination
	// is NOT picked here, it is picked after the pickup, once the item
	// type is known (tickInteract, PHASE_AT_SOURCE).
	bool findTransportPair();
	// The nearest openable wooden/trapped chest that accepts the given
	// stack (empty, or already holding the same item type, with room
	// for it). Stores it in destX/Y/Z. Returns false if no chest in
	// the 32/8 box accepts the stack.
	bool findMatchingDest(shared_ptr<ItemInstance> stack);
	// Java deposit rule: a chest accepts a stack only if it is empty
	// or already holds the same item type (with room to merge, or an
	// empty slot to take it). A chest holding only other item types
	// never accepts it, even if it has empty slots.
	bool containerAcceptsItem(shared_ptr<Container> container, shared_ptr<ItemInstance> stack);
	// The openable Container at a chest position (nullptr if the chest is
	// blocked, has no TE, or is not a chest). Handles double chests via
	// ChestTile::getContainer (CompoundContainer).
	shared_ptr<Container> getChestContainer(int x, int y, int z);
	bool containerHasItem(shared_ptr<Container> container);
	// Move up to 16 items of one type from the first non-empty slot
	// into the golem's hand (Java cap — never a whole stack). Returns
	// the taken stack, or nullptr if the container is empty.
	shared_ptr<ItemInstance> takeFirstStack(shared_ptr<Container> container);
	// Move a stack into the container (merge into a same-item stack with
	// room, else an empty slot). Refuses a chest that holds only other
	// item types (Java: empty or same-type only) — the destination scan
	// enforces this too, this guards against contents changing mid-run.
	// Returns false if it does not fit whole.
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
