#pragma once

class CopperGolem;

// neoLegacy - Copper Age (Phase 5). Goal-based AI for the copper golem,
// ported from Java CopperGolemAi (copper-code.txt :627) and adapted to this
// tree's Goal system (NOT Java Brain). The Java file's transport behaviour
// (chest search / take / deposit) lives in CopperGolemState's GETTING /
// DROPPING states; see the 5b note in updateActivity().
class CopperGolemAi
{
public:
	// Transport search radii (Java CopperGolemAi
	// TRANSPORT_ITEM_HORIZONTAL/VERTICAL_SEARCH_RADIUS).
	static const int TRANSPORT_ITEM_HORIZONTAL_SEARCH_RADIUS = 32;
	static const int TRANSPORT_ITEM_VERTICAL_SEARCH_RADIUS = 8;

	// Transport cooldown (Java CopperGolemAi
	// TRANSPORT_ITEMS_COOLDOWN_TICKS): after a run the golem waits a
	// random 60-100 ticks before the next one. Also the initial cooldown
	// at spawn, so it does not transport immediately.
	static const int TRANSPORT_COOLDOWN_MIN_TICKS = 60;
	static const int TRANSPORT_COOLDOWN_MAX_TICKS = 100;

	// Register the golem's goals (called from the CopperGolem ctor).
	static void registerGoals(CopperGolem *golem);

	// Per-tick activity update (called from CopperGolem::tick()). Counts
	// down the transport cooldown (Phase 5b). It deliberately does NOT
	// force the state to IDLE: the transport goal owns the
	// GETTING/DROPPING state while it runs and resets it to IDLE when it
	// stops. (Phase 5 had this forcing IDLE as the 5b seam; that is gone
	// now that the goal exists.)
	static void updateActivity(CopperGolem *golem);
};
