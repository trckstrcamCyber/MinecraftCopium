#pragma once
#include "Monster.h"
#include "RaidMember.h"

// neoLegacy - base class of the illagers (Vindicator, Evoker, Iceologer; Pillager and friends follow).
// Illagers share one thing the client needs: whether the mob is hunting something (it can't see the target, only the
// server can), so the server keeps a synched "aggressive" flag and the model reads it to pick the arm pose.
//
// A captain (the leader of a patrol or a raid wave) wears the Ominous Banner on its head, drops it, and gives the player who kills it Bad Omen.
class Illager : public Monster, public RaidMember
{
public:
	enum ArmPose { CROSSED, ATTACKING, SPELLCASTING, CELEBRATING, NEUTRAL, BOW_AND_ARROW, CROSSBOW_HOLD, CROSSBOW_CHARGE };

protected:
	static const int DATA_FLAGS_ID = 16;	// bit 0 (1): aggressive; bits 1 and 2 (2, 4): the Pillager's crossbow is being drawn / is loaded; bit 3 (8): captain
	static const int FLAG_AGGRESSIVE = 1;
	static const int FLAG_CAPTAIN = 8;

	// server side only (nothing here is synched or saved): raid and patrol membership
	bool inRaid;
	int raidX, raidY, raidZ;
	bool patrolling;
	bool patrolTargetSet;
	int patrolX, patrolZ;
	weak_ptr<Illager> patrolLeaderRef;

public:
	Illager(Level *level);

	virtual void tick();
	virtual void die(DamageSource *source);
	virtual bool isAggressive();

	bool isCaptain();
	void makeCaptain();		// the Ominous Banner on its head (dropped for certain), and the flag the client draws it by

	// RaidMember
	virtual bool isInRaid() { return inRaid; }
	virtual void getRaidCenter(int *x, int *y, int *z) { *x = raidX; *y = raidY; *z = raidZ; }
	virtual void joinRaid(int x, int y, int z) { inRaid = true; raidX = x; raidY = y; raidZ = z; }

	// patrols (a captain with followers, see RaidManager)
	void startPatrol(shared_ptr<Illager> leader, int targetX, int targetZ);	// leader == nullptr for the captain itself
	void endPatrol() { patrolling = false; }
	bool isPatrolling() { return patrolling; }
	bool hasPatrolTarget() { return patrolTargetSet; }
	int patrolTargetX() { return patrolX; }
	int patrolTargetZ() { return patrolZ; }
	void setPatrolTarget(int x, int z) { patrolX = x; patrolZ = z; patrolTargetSet = true; }
	shared_ptr<Illager> getPatrolLeader() { return patrolLeaderRef.lock(); }

	virtual ArmPose getArmPose();

	// illagers and their summons never turn on each other
	virtual bool isAlliedTo(shared_ptr<LivingEntity> other);

protected:
	// Without this the mob falls back to the old pre-goal AI: it just walks up and hits, never casts, and getTarget() stays empty
	// (so the aggressive flag / arm pose never changes). Zombie does the same.
	virtual bool useNewAi();
};
