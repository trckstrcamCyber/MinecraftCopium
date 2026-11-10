#include "stdafx.h"
#include "Random.h"
#include "AttributeInstance.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "SharedMonsterAttributes.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "SoundTypes.h"
#include "Mth.h"
#include "Illager.h"
#include "Vex.h"
#include "IceCube.h"

const float IceCube::FALL_DAMAGE_PER_BLOCK = 3.0f;

IceCube::IceCube(Level *level) : Mob(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	setSize(2.0f, 1.5f);
	floatTicks = FLOAT_TICKS;
	fallTime = 0;
	landed = false;
	dropping = false;
	dropStartY = 0;
}

IceCube::IceCube(Level *level, shared_ptr<LivingEntity> caster, shared_ptr<LivingEntity> target, int hoverTicks) : Mob(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	setSize(2.0f, 1.5f);
	floatTicks = hoverTicks;
	fallTime = 0;
	landed = false;
	dropping = false;
	dropStartY = 0;
	this->caster = caster;
	this->target = target;
	xd = yd = zd = 0;
	if (target != nullptr) setPos(target->x, target->y + 1.0 + 3.0 + (1.0f - bbHeight) / 2.0f, target->z);
}

void IceCube::registerAttributes()
{
	Mob::registerAttributes();
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(1);
}

bool IceCube::hurt(DamageSource *source, float dmg)
{
	return false;	// invulnerable
}

// no walking, no swimming, no ordinary gravity: tick() moves the cube itself
void IceCube::travel(float xa, float ya) {}

void IceCube::tick()
{
	Entity::tick();

	if (floatTicks == 20 && !level->isClientSide) playSound(eSoundType_MOB_ICEOLOGER_CUBE_AMBIENT, 1.0f, 1.0f);

	if (floatTicks > 0)
	{
		// hovering: it stays EXACTLY where it was conjured (above where the victim stood when the spell fired), so a victim who
		// keeps moving can walk out from under it - that is the whole point of the telegraph
		floatTicks--;
		return;
	}

	// falling
	if (!dropping)
	{
		dropping = true;
		dropStartY = y;
	}
	fallTime++;
	yd -= 0.04 * 8.0;
	move(xd, yd, zd);

	if (!level->isClientSide)
	{
		if (!onGround)
		{
			if (fallTime > 600 || (fallTime > 100 && (y < 1 || y > 256))) remove();
		}
		else if (!landed)
		{
			// landed: damage everything clearly underneath it (decided here, not by the engine's fall-damage hook), then shatter
			landed = true;
			int blocks = Mth::ceil(static_cast<float>(dropStartY - y) - 1.0f);
			if (blocks > 0)
			{
				const double HALF = 1.0 - 0.35;	// the cube is 2 wide; the hit zone is its footprint shrunk by 0.35 on every side
				vector<shared_ptr<Entity> > *hit = level->getEntitiesOfClass(typeid(LivingEntity), bb->grow(1.5, 1.0, 1.5));
				if (hit != nullptr)
				{
					for (size_t i = 0; i < hit->size(); i++)
					{
						shared_ptr<LivingEntity> victim = dynamic_pointer_cast<LivingEntity>((*hit)[i]);
						if (victim == nullptr) continue;
						double reach = HALF + victim->bbWidth * 0.5;
						if (fabs(victim->x - x) > reach || fabs(victim->z - z) > reach) continue;
						if (victim->bb->y1 <= bb->y0 || victim->bb->y0 >= bb->y1) continue;
						damageEntity(victim, blocks);
					}
				}
			}
			for (int i = 0; i < 16; i++)
			{
				level->addParticle(eParticleType_hugeexplosion, x + (random->nextDouble() - 0.5) * 2.0, y + 0.3 + random->nextDouble(), z + (random->nextDouble() - 0.5) * 2.0, 0, 0, 0);
			}
			playSound(eSoundType_MOB_ICEOLOGER_CUBE_HIT, 1.0f, 1.0f);
			remove();
		}
	}
	xd *= 0.98;
	yd *= 0.98;
	zd *= 0.98;
}

// the engine's own fall-damage hook is not used (it would only hurt the cube itself, which is invulnerable)
void IceCube::causeFallDamage(float distance)
{
}

void IceCube::damageEntity(shared_ptr<LivingEntity> victim, int blocksFallen)
{
	shared_ptr<LivingEntity> source = caster.lock();
	if (victim.get() == this || victim == source || !victim->isAlive()) return;
	if (dynamic_cast<IceCube *>(victim.get()) != nullptr) return;
	// the Iceologer's own side is spared
	if (dynamic_cast<Illager *>(victim.get()) != nullptr || dynamic_cast<Vex *>(victim.get()) != nullptr) return;

	float amount = static_cast<float>(Mth::floor(blocksFallen * FALL_DAMAGE_PER_BLOCK));
	if (source == nullptr) victim->hurt(DamageSource::magic, amount);
	else victim->hurt(DamageSource::indirectMagic(shared_from_this(), source), amount);

	// Nausea and (very heavy) Slowness for 5 seconds
	victim->addEffect(new MobEffectInstance(MobEffect::confusion->id, 100, 4));
	victim->addEffect(new MobEffectInstance(MobEffect::movementSlowdown->id, 100, 4));
}
