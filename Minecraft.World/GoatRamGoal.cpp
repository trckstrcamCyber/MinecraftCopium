#include "stdafx.h"
#include "Random.h"
#include "Abilities.h"
#include "PathNavigation.h"
#include "MoveControl.h"
#include "Mth.h"
#include "Material.h"
#include "LookControl.h"
#include "Sensing.h"
#include "Player.h"
#include "DamageSource.h"
#include "AABB.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.sensing.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "Goat.h"
#include "GoatRamGoal.h"

static const int WINDUP_TICKS = 30;			// 1.5 s of head-down warning
static const int CHARGE_TICKS = 40;			// give up the charge after 2 s
static const float RAM_RANGE = 10.0f;
static const float RAM_DAMAGE = 2.0f;
static const float RAM_KNOCKBACK = 1.3f;
static const double STUCK_DIST_SQR = 0.0025;	// < 0.05 blocks moved in a tick = not moving (charge is ~0.5/tick)
static const int STUCK_TICKS = 3;			// consecutive stuck ticks before calling it a wall hit

GoatRamGoal::GoatRamGoal(Goat *goat)
{
	this->goat = goat;
	phase = 0;
	timer = 0;
	nextRamTick = 600 + goat->level->random->nextInt(600);
	lastChargeX = goat->x;
	lastChargeZ = goat->z;
	stuckTicks = 0;
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
}

shared_ptr<LivingEntity> GoatRamGoal::findTarget()
{
	// Anything living nearby that isn't another goat (players in creative are left alone).
	AABB *box = goat->bb->grow(RAM_RANGE, 3, RAM_RANGE);
	vector<shared_ptr<Entity> > *nearby = goat->level->getEntitiesOfClass(typeid(LivingEntity), box);
	shared_ptr<LivingEntity> best = nullptr;
	double bestDist = RAM_RANGE * RAM_RANGE;
	if (nearby != nullptr)
	{
		for (size_t i = 0; i < nearby->size(); i++)
		{
			shared_ptr<LivingEntity> e = dynamic_pointer_cast<LivingEntity>(nearby->at(i));
			if (e == nullptr || e.get() == goat || !e->isAlive()) continue;
			if (e->instanceof(eTYPE_GOAT)) continue;
			if (e->instanceof(eTYPE_PLAYER))
			{
				shared_ptr<Player> p = dynamic_pointer_cast<Player>(e);
				if (p != nullptr && p->abilities.invulnerable) continue;
			}
			if (!goat->getSensing()->canSee(e)) continue;
			double d = goat->distanceToSqr(e);
			if (d < bestDist) { bestDist = d; best = e; }
		}
		delete nearby;
	}
	return best;
}

bool GoatRamGoal::canUse()
{
	if (goat->isBaby() || goat->tickCount < nextRamTick) return false;
	if (!goat->onGround || goat->isInWater()) return false;
	if (goat->level->random->nextInt(40) != 0) return false;		// "randomly": roughly once per few seconds once off cooldown

	shared_ptr<LivingEntity> t = findTarget();
	if (t == nullptr) return false;
	target = weak_ptr<LivingEntity>(t);
	return true;
}

bool GoatRamGoal::canContinueToUse()
{
	shared_ptr<LivingEntity> t = target.lock();
	return phase != 0 && t != nullptr && t->isAlive() && goat->isAlive();
}

void GoatRamGoal::start()
{
	phase = 1;
	timer = WINDUP_TICKS;
	goat->setRamming(true);
	goat->getNavigation()->stop();
}

void GoatRamGoal::stop()
{
	phase = 0;
	goat->setRamming(false);
	goat->getNavigation()->stop();
	target = weak_ptr<LivingEntity>();
	nextRamTick = goat->tickCount + 600 + goat->level->random->nextInt(1200);	// 30-90 s before the next one
}

void GoatRamGoal::tick()
{
	shared_ptr<LivingEntity> t = target.lock();
	if (t == nullptr) { phase = 0; return; }

	goat->getLookControl()->setLookAt(t, 30, 30);

	if (phase == 1)
	{
		goat->getNavigation()->stop();
		if (--timer <= 0)
		{
			phase = 2;
			timer = CHARGE_TICKS;
			lastChargeX = goat->x;		// fresh reference so the first charge tick can't false-trigger stuck
			lastChargeZ = goat->z;
			stuckTicks = 0;
		}
		return;
	}

	// Charging — straight line at the target (MoveControl drives directly at the wanted position,
	// no pathfinding, so a wall stops the goat instead of routing it around). The navigation stays
	// stopped from start(); the MoveControl flag is already required by this goal.
	goat->getMoveControl()->setWantedPosition(t->x, t->y, t->z, 2.4);
	if (--timer <= 0) { phase = 0; return; }

	double reach = goat->bbWidth * 0.5 + t->bbWidth * 0.5 + 0.9;
	if (goat->distanceToSqr(t) <= reach * reach)
	{
		t->hurt(DamageSource::mobAttack(dynamic_pointer_cast<LivingEntity>(goat->shared_from_this())), RAM_DAMAGE);

		double dx = t->x - goat->x;
		double dz = t->z - goat->z;
		double len = sqrt(dx * dx + dz * dz);
		if (len < 0.001) { dx = 1; dz = 0; len = 1; }
		t->xd += dx / len * RAM_KNOCKBACK;
		t->zd += dz / len * RAM_KNOCKBACK;
		t->yd = 0.4;
		t->hurtMarked = true;		// players move client-side: tell the client about the shove

		goat->playSound(eSoundType_MOB_GOAT_RAM_IMPACT, 1.0f, 1.0f);
		phase = 0;		// ram done -> canContinueToUse() false -> stop()
		return;
	}

	// Stuck against a wall? The goat is driving forward but barely moving. (A free charge moves
	// ~0.5 blocks/tick; a wall stops it dead.) Three stuck ticks in a row = a wall hit.
	double movedX = goat->x - lastChargeX;
	double movedZ = goat->z - lastChargeZ;
	if (movedX * movedX + movedZ * movedZ < STUCK_DIST_SQR) stuckTicks++;
	else stuckTicks = 0;
	lastChargeX = goat->x;
	lastChargeZ = goat->z;
	if (stuckTicks >= STUCK_TICKS)
	{
		if (isBlockedBySolidBlock())
		{
			// Java: the ram into a natural hard block knocks a horn loose (up to two per goat).
			// dropHorn() is a no-op once both horns are gone; the impact sound plays either way.
			goat->dropHorn();
			goat->playSound(eSoundType_MOB_GOAT_RAM_IMPACT, 1.0f, 1.0f);
		}
		phase = 0;		// ram over -> canContinueToUse() false -> stop()
	}
}

// True if a solid block is directly ahead of the goat at feet or head height, in the direction
// of the ram target. Confirms the "stuck" reading really is a wall (not e.g. the goat turning
// in place or the target standing on top of it). Checks both levels so a 1-block-high wall
// counts as well as a taller one.
bool GoatRamGoal::isBlockedBySolidBlock()
{
	shared_ptr<LivingEntity> t = target.lock();
	if (t == nullptr) return false;

	double dx = t->x - goat->x;
	double dz = t->z - goat->z;
	double len = sqrt(dx * dx + dz * dz);
	if (len < 0.001) return false;
	dx /= len;
	dz /= len;

	// Just past the goat's front (half-width 0.45) so the block the goat is pressed against
	// is the one sampled, at both feet level and head level (goat is 1.3 tall).
	int fx = Mth::floor(goat->x + dx * 0.7);
	int fz = Mth::floor(goat->z + dz * 0.7);
	int fy = Mth::floor(goat->y);
	Material *feet = goat->level->getMaterial(fx, fy, fz);
	Material *head = goat->level->getMaterial(fx, fy + 1, fz);
	return (feet != nullptr && feet->blocksMotion()) || (head != nullptr && head->blocksMotion());
}
