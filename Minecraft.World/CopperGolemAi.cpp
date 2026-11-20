#include "stdafx.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.player.h"
#include "CopperGolem.h"
#include "CopperGolemAi.h"

void CopperGolemAi::registerGoals(CopperGolem *golem)
{
	// Java CopperGolemAi, adapted to this tree's Goal system. The golem is
	// passive: it never attacks, so the targetSelector stays empty.
	//   0 Float — don't drown.
	//   1 Panic 1.5x when hurt or on fire (PanicGoal's own canUse covers
	//     the hurt/fire window).
	//   2 OpenDoor — the golem opens doors (navigation canOpenDoors is set
	//     in the ctor alongside this goal).
	//   3 Wander at speed 1.0.
	//   4 Look at nearby players (6 blocks).
	//   5 Look around idly.
	// Head-spin is NOT a goal: it is a time-based idle animation driven
	// off tickCount (CopperGolem::getSpinPhase, sound at +10 ticks in
	// CopperGolem::tick), so it needs no goal slot.
	golem->goalSelector.addGoal(0, new FloatGoal(golem));
	golem->goalSelector.addGoal(1, new PanicGoal(golem, 1.5));
	golem->goalSelector.addGoal(2, new OpenDoorGoal(golem, true));
	golem->goalSelector.addGoal(3, new RandomStrollGoal(golem, 1.0));
	golem->goalSelector.addGoal(4, new LookAtPlayerGoal(golem, typeid(Player), 6.0f, 1.0f));
	golem->goalSelector.addGoal(5, new RandomLookAroundGoal(golem));
}

void CopperGolemAi::updateActivity(CopperGolem *golem)
{
	// Phase 5b seam. Today the golem is always IDLE: there is no chest to
	// walk to, because no mob/AI code in this tree has ever accessed a
	// TileEntity inventory (the only TE access is tile-side, e.g.
	// ChestTile, and player-UI-side). Building the chest-search (32
	// horizontal / 8 vertical), 60-tick take-stack, and deposit goals
	// would be unverifiable new machinery with no Deck build to test it,
	// so 5b is deferred — see copper-phase5-REPORT.md. When it lands, the
	// GETTING_ITEM / GETTING_NO_ITEM / DROPPING_ITEM / DROPPING_NO_ITEM
	// transitions (CopperGolemState) go here, and the golem's held item
	// (SLOT_WEAPON) is the item being transported.
	golem->setState(COPPER_GOLEM_STATE_IDLE);
}
