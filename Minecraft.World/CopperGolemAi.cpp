#include "stdafx.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.player.h"
#include "CopperGolem.h"
#include "CopperGolemAi.h"
#include "CopperGolemTransportGoal.h"

void CopperGolemAi::registerGoals(CopperGolem *golem)
{
	// Java CopperGolemAi, adapted to this tree's Goal system. The golem is
	// passive: it never attacks, so the targetSelector stays empty.
	//   0 Float — don't drown.
	//   1 Panic 1.5x when hurt or on fire (PanicGoal's own canUse covers
	//     the hurt/fire window).
	//   2 OpenDoor — the golem opens doors (navigation canOpenDoors is set
	//     in the ctor alongside this goal).
	//   3 Transport (Phase 5b) — carry one item stack from an open copper
	//     chest to a normal / trapped chest. Above RandomStroll so it
	//     pre-empts wandering when work exists (the Java idle activity's
	//     first entry), but below Panic/Float. Claims Move + Look, so it
	//     cannot run at the same time as RandomStroll.
	//   4 Wander at speed 1.0.
	//   5 Look at nearby players (6 blocks).
	//   6 Look around idly.
	// Head-spin is NOT a goal: it is a time-based idle animation driven
	// off tickCount (CopperGolem::getSpinPhase, sound at +10 ticks in
	// CopperGolem::tick), so it needs no goal slot.
	golem->goalSelector.addGoal(0, new FloatGoal(golem));
	golem->goalSelector.addGoal(1, new PanicGoal(golem, 1.5));
	golem->goalSelector.addGoal(2, new OpenDoorGoal(golem, true));
	golem->goalSelector.addGoal(3, new CopperGolemTransportGoal(golem));
	golem->goalSelector.addGoal(4, new RandomStrollGoal(golem, 1.0));
	golem->goalSelector.addGoal(5, new LookAtPlayerGoal(golem, typeid(Player), 6.0f, 1.0f));
	golem->goalSelector.addGoal(6, new RandomLookAroundGoal(golem));
}

void CopperGolemAi::updateActivity(CopperGolem *golem)
{
	// Phase 5b: count down the transport cooldown (server-side only).
	// The GETTING/DROPPING state itself is owned by
	// CopperGolemTransportGoal while it runs (it sets the state at the
	// Java 1 / 9 / 60 tick marks and resets it to IDLE when it stops),
	// so this deliberately does NOT force the state to IDLE any more —
	// doing so every tick would clobber the goal's state.
	if (!golem->level->isClientSide && golem->getTransportCooldown() > 0)
	{
		golem->setTransportCooldown(golem->getTransportCooldown() - 1);
	}
}
