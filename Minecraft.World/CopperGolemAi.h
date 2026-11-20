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
	// TRANSPORT_ITEM_HORIZONTAL/VERTICAL_SEARCH_RADIUS). Defined here for
	// Phase 5b; unused until the chest goals land.
	static const int TRANSPORT_ITEM_HORIZONTAL_SEARCH_RADIUS = 32;
	static const int TRANSPORT_ITEM_VERTICAL_SEARCH_RADIUS = 8;

	// Register the golem's goals (called from the CopperGolem ctor).
	static void registerGoals(CopperGolem *golem);

	// Per-tick activity update (called from CopperGolem::tick()). Today it
	// only keeps the state at IDLE: the transport state machine (Phase 5b)
	// has no precedent in this tree — no mob/AI code ever touches a
	// TileEntity inventory — so the GETTING/DROPPING transitions are not
	// implemented. When 5b lands, this is where they go.
	static void updateActivity(CopperGolem *golem);
};
