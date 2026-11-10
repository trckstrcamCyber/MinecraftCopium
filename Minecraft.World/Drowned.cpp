#include "stdafx.h"
#include "AABB.h"
#include "Random.h"
#include "PathNavigation.h"
#include "LookControl.h"
#include "MoveControl.h"
#include "Material.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "Mth.h"
#include "Drowned.h"

Drowned::Drowned(Level *level) : Zombie(level)
{
	// Zombie's constructor already gave it the zombie goals/targets. Drowned just need to be willing
	// to path through water and add swimming in aiStep().
	waterSearchCooldown = 0;
}

// Looks for open water within ~14 blocks (random sampling, a handful of tries).
bool Drowned::findNearbyWater(double *wx, double *wy, double *wz)
{
	for (int i = 0; i < 24; i++)
	{
		int xt = Mth::floor(x) + random->nextInt(29) - 14;
		int yt = Mth::floor(y) + random->nextInt(7) - 3;
		int zt = Mth::floor(z) + random->nextInt(29) - 14;
		if (level->getMaterial(xt, yt, zt) == Material::water)
		{
			*wx = xt + 0.5; *wy = yt; *wz = zt + 0.5;
			return true;
		}
	}
	return false;
}

void Drowned::aiStep()
{
	if (!level->isClientSide)
	{
		shared_ptr<LivingEntity> target = getTarget();

		if (isInWater() && target != nullptr)
		{
			// Swim at the target in 3D: face it, then accelerate along the straight line to it.
			double dx = target->x - x;
			double dy = (target->y + target->bbHeight * 0.5) - (y + bbHeight * 0.5);
			double dz = target->z - z;
			double len = sqrt(dx * dx + dy * dy + dz * dz);
			if (len > 0.8)
			{
				getLookControl()->setLookAt(target, 30, 30);
				getMoveControl()->setWantedPosition(target->x, target->y, target->z, 1.0);
				xd += dx / len * 0.025;
				yd += dy / len * 0.025;
				zd += dz / len * 0.025;
			}
		}
		else if (!isInWater() && target == nullptr && level->isDay() && --waterSearchCooldown <= 0)
		{
			// Daytime on land: go back to the water (a zombie-style sunburn is what's chasing it).
			waterSearchCooldown = 40;
			double wx, wy, wz;
			if (findNearbyWater(&wx, &wy, &wz))
			{
				getNavigation()->moveTo(wx, wy, wz, 1.0);
			}
		}
	}
	Zombie::aiStep();
}

bool Drowned::canSpawn()
{
	// Zombie rules (dark enough, room to stand) plus: standing near water.
	if (!Zombie::canSpawn()) return false;

	int xt = Mth::floor(x);
	int yt = Mth::floor(bb->y0);
	int zt = Mth::floor(z);
	for (int dx = -4; dx <= 4; dx++)
		for (int dz = -4; dz <= 4; dz++)
			for (int dy = -2; dy <= 1; dy++)
				if (level->getMaterial(xt + dx, yt + dy, zt + dz) == Material::water) return true;
	return false;
}

int Drowned::getAmbientSound() { return isInWater() ? eSoundType_MOB_DROWNED_WATER_IDLE : eSoundType_MOB_DROWNED_IDLE; }
int Drowned::getHurtSound() { return isInWater() ? eSoundType_MOB_DROWNED_WATER_HURT : eSoundType_MOB_DROWNED_HURT; }
int Drowned::getDeathSound() { return isInWater() ? eSoundType_MOB_DROWNED_WATER_DEATH : eSoundType_MOB_DROWNED_DEATH; }

void Drowned::playStepSound(int xt, int yt, int zt, int t)
{
	playSound(eSoundType_MOB_DROWNED_STEP, 0.15f, 1.0f);
}

bool Drowned::isSunSensitive()
{
	return false;
}

int Drowned::decreaseAirSupply(int currentSupply)
{
	return currentSupply;		// breathes underwater
}

void Drowned::dropRareDeathLoot(int rareLootLevel)
{
	// Rare drop: a gold ingot (instead of the zombie's iron/carrot/potato).
	spawnAtLocation(Item::gold_ingot_Id, 1);
}
