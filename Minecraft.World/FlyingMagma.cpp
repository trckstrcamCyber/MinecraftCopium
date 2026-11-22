#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.h"
#include "FlyingMagma.h"

FlyingMagma::FlyingMagma(Level *level, shared_ptr<LivingEntity> owner, double xa, double ya, double za) : LargeFireball(level, owner, xa, ya, za)
{
	explosionPower = 1;
}

void FlyingMagma::onHit(HitResult *res)
{
	if (!level->isClientSide)
	{
		if (res->entity != nullptr)
		{
			DamageSource *damageSource = DamageSource::indirectMagic(shared_from_this(), owner);
			res->entity->hurt(damageSource, 12.0f);
			delete damageSource;
		}
		level->explode(nullptr, x, y, z, 1.0f, true, level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING));
		remove();
	}
}
