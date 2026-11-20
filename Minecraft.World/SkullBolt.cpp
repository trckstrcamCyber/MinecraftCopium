#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.monster.h"
#include "MobEffectInstance.h"
#include "MobType.h"
#include "SkullBolt.h"

SkullBolt::SkullBolt(Level *level, shared_ptr<LivingEntity> owner, double xa, double ya, double za) : WitherSkull(level, owner, xa, ya, za)
{
}

void SkullBolt::onHit(HitResult *res)
{
	if (!level->isClientSide)
	{
		if (res->entity != nullptr && res->entity->instanceof(eTYPE_LIVINGENTITY))
		{
			shared_ptr<LivingEntity> target = dynamic_pointer_cast<LivingEntity>(res->entity);
			if (target->getMobType() == UNDEAD)
			{
				// it is its master's skull: the undead drink it in
				target->heal(5);
				target->addEffect(new MobEffectInstance(MobEffect::movementSpeed->id, 100, 2));
			}
			else
			{
				DamageSource *damageSource = DamageSource::indirectMagic(shared_from_this(), owner);
				target->hurt(damageSource, 7);
				delete damageSource;
				target->addEffect(new MobEffectInstance(MobEffect::movementSlowdown->id, 100, 1));
			}
		}
		remove();
	}
}
