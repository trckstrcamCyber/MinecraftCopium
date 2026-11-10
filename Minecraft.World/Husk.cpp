#include "stdafx.h"
#include "MobEffectInstance.h"
#include "MobEffect.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.level.h"
#include "Husk.h"
#include "ItemInstance.h"
#include "Material.h"

Husk::Husk(Level *level) : Zombie(level)
{
	// Zombie's constructor already registered attributes, goals and targets: a husk has the same
	// stats and AI as a zombie.
	underWaterTicks = 0;
}

void Husk::convertToZombie()
{
	// A husk left in water for 30 seconds turns into a plain zombie (which can then become a drowned).
	shared_ptr<Zombie> zombie = std::make_shared<Zombie>(level);
	zombie->copyPosition(shared_from_this());
	zombie->setBaby(isBaby());
	for (int slot = 0; slot < 5; slot++)
	{
		shared_ptr<ItemInstance> item = getCarried(slot);
		if (item != nullptr) zombie->setEquippedSlot(slot, item->copy());
	}
	zombie->setHealth(getHealth() / getMaxHealth() * zombie->getMaxHealth());
	level->addEntity(zombie);
	playSound(eSoundType_MOB_HUSK_CONVERT, 1.0f, 1.0f);
	remove();
}

void Husk::tick()
{
	if (!level->isClientSide && isUnderLiquid(Material::water))
	{
		if (++underWaterTicks >= 600) { convertToZombie(); return; }
	}
	else
	{
		underWaterTicks = 0;
	}
	Zombie::tick();
}

int Husk::getAmbientSound() { return eSoundType_MOB_HUSK_IDLE; }
int Husk::getHurtSound() { return eSoundType_MOB_HUSK_HURT; }
int Husk::getDeathSound() { return eSoundType_MOB_HUSK_DEATH; }

void Husk::playStepSound(int xt, int yt, int zt, int t)
{
	playSound(eSoundType_MOB_HUSK_STEP, 0.15f, 1.0f);
}

bool Husk::isSunSensitive()
{
	return false;
}

bool Husk::doHurtTarget(shared_ptr<Entity> target)
{
	bool result = Zombie::doHurtTarget(target);

	if (result && getCarriedItem() == nullptr)
	{
		shared_ptr<LivingEntity> living = dynamic_pointer_cast<LivingEntity>(target);
		if (living != nullptr)
		{
			// 7 seconds of Hunger per difficulty level (Easy 1, Normal 2, Hard 3)
			int seconds = 7 * level->difficulty;
			living->addEffect(new MobEffectInstance(MobEffect::hunger->id, seconds * SharedConstants::TICKS_PER_SECOND, 0));
		}
	}
	return result;
}
