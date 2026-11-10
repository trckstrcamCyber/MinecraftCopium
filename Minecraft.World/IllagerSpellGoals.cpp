#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "PathNavigation.h"
#include "IllagerSpellGoals.h"

SpellcasterCastingSpellGoal::SpellcasterCastingSpellGoal(SpellcasterIllager *mob)
{
	this->mob = mob;
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
}

bool SpellcasterCastingSpellGoal::canUse()
{
	return mob->getSpellCastingTime() > 0;
}

void SpellcasterCastingSpellGoal::start()
{
	Goal::start();
	mob->getNavigation()->stop();
}

void SpellcasterCastingSpellGoal::stop()
{
	Goal::stop();
	mob->setIsCastingSpell(SpellcasterIllager::SPELL_NONE);
}

void SpellcasterCastingSpellGoal::tick()
{
	shared_ptr<LivingEntity> target = mob->getTarget();
	if (target != nullptr) mob->getLookControl()->setLookAt(target, 30.0f, 30.0f);
}

SpellcasterUseSpellGoal::SpellcasterUseSpellGoal(SpellcasterIllager *mob)
{
	this->mob = mob;
	attackWarmupDelay = 0;
	nextAttackTickCount = 0;
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
}

bool SpellcasterUseSpellGoal::canUse()
{
	shared_ptr<LivingEntity> target = mob->getTarget();
	if (target == nullptr || !target->isAlive()) return false;
	if (mob->isCastingSpell()) return false;
	return mob->tickCount >= nextAttackTickCount;
}

bool SpellcasterUseSpellGoal::canContinueToUse()
{
	shared_ptr<LivingEntity> target = mob->getTarget();
	return target != nullptr && target->isAlive() && attackWarmupDelay > 0;
}

// The stand-still goal (higher priority, same control flags) must not be allowed to cut a spell off during its warm-up: that is what made the
// casters wave their arms for ever without anything happening.
bool SpellcasterUseSpellGoal::canInterrupt()
{
	return false;
}

void SpellcasterUseSpellGoal::start()
{
	attackWarmupDelay = getCastWarmupTime();
	mob->setSpellCastingTime(getCastingTime());
	nextAttackTickCount = mob->tickCount + getCastingInterval();

	int prepareSound = getSpellPrepareSound();
	if (prepareSound >= 0) mob->playSound(prepareSound, 1.0f, 1.0f);

	mob->setIsCastingSpell(getSpell());
}

void SpellcasterUseSpellGoal::tick()
{
	attackWarmupDelay--;
	if (attackWarmupDelay == 0)
	{
		performSpellCasting();
		mob->playSound(mob->getCastingSoundId(), 1.0f, 1.0f);
	}
}
