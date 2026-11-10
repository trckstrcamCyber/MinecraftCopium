#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.storage.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.damagesource.h"
#include "com.mojang.nbt.h"
#include "EntityEvent.h"
#include "ParticleTypes.h"
#include "JellySlime.h"
#include "../Minecraft.Client/Textures.h"
#include "SoundTypes.h"

// ---------------------------------------------------------------------------------------------------------------------------
// neoLegacy - Jelly Slime (see JellySlime.h)
// ---------------------------------------------------------------------------------------------------------------------------

JellySlime::JellySlime(Level *level) : Slime(level), sizeInitialised(false)
{
	// Slime's constructor has already defined its own synched data (the size); this adds the owner.
	entityData->define(DATA_OWNER_UUID, L"");

	setSize(JELLY_SIZE);	// virtual: JellySlime::setSize (the Slime constructor could only call its own)
	jumpDelay = random->nextInt(20) + 10;
}

void JellySlime::setSize(int size)
{
	// The size is fixed: a Jelly Slime is always the same. Slime::setSize resets health to size*size, so keep the current health
	// when the size is re-applied while loading a saved one.
	const float oldHealth = getHealth();
	Slime::setSize(JELLY_SIZE);
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(JELLY_MAX_HEALTH);
	if (sizeInitialised && oldHealth > 0.0f)
	{
		setHealth(oldHealth < JELLY_MAX_HEALTH ? oldHealth : JELLY_MAX_HEALTH);
	}
	else
	{
		setHealth(getMaxHealth());
	}
	xpReward = 8;
	sizeInitialised = true;
}

// ---- taming ----------------------------------------------------------------------------------------------------------------
bool JellySlime::isTame()
{
	return !getOwnerUUID().empty();
}

wstring JellySlime::getOwnerUUID()
{
	return entityData->getString(DATA_OWNER_UUID);
}

void JellySlime::setOwnerUUID(const wstring &uuid)
{
	entityData->set(DATA_OWNER_UUID, uuid);
}

void JellySlime::tame(const wstring &ownerUUID)
{
	setOwnerUUID(ownerUUID);
	setPersistenceRequired();	// a pet never despawns
	setHealth(getMaxHealth());
}

bool JellySlime::mobInteract(shared_ptr<Player> player)
{
	shared_ptr<ItemInstance> item = player->inventory->getSelected();

	// sugar tames a wild one
	if (!isTame() && item != nullptr && item->id == Item::sugar_Id)
	{
		if (player->abilities.instabuild == false)
		{
			item->count--;
			if (item->count <= 0)
			{
				player->inventory->setItem(player->inventory->selected, nullptr);
			}
		}

		if (!level->isClientSide)
		{
			if (random->nextInt(3) == 0)
			{
				tame(player->getUUID());
				level->broadcastEntityEvent(shared_from_this(), EntityEvent::TAMING_SUCCEEDED);
			}
			else
			{
				level->broadcastEntityEvent(shared_from_this(), EntityEvent::TAMING_FAILED);
			}
		}
		return true;
	}

	return Slime::mobInteract(player);
}

void JellySlime::spawnTamingParticles(bool success)
{
	ePARTICLE_TYPE particle = success ? eParticleType_heart : eParticleType_smoke;
	for (int i = 0; i < 7; i++)
	{
		double xa = random->nextGaussian() * 0.02;
		double ya = random->nextGaussian() * 0.02;
		double za = random->nextGaussian() * 0.02;
		level->addParticle(particle, x + random->nextFloat() * bbWidth * 2 - bbWidth, y + .5f + random->nextFloat() * bbHeight, z + random->nextFloat() * bbWidth * 2 - bbWidth, xa, ya, za);
	}
}

void JellySlime::handleEntityEvent(byte id)
{
	if (id == EntityEvent::TAMING_SUCCEEDED)
	{
		spawnTamingParticles(true);
	}
	else if (id == EntityEvent::TAMING_FAILED)
	{
		spawnTamingParticles(false);
	}
	else
	{
		Slime::handleEntityEvent(id);
	}
}

void JellySlime::addAdditonalSaveData(CompoundTag *tag)
{
	Slime::addAdditonalSaveData(tag);
	tag->putString(L"JellyOwner", getOwnerUUID());
}

void JellySlime::readAdditionalSaveData(CompoundTag *tag)
{
	Slime::readAdditionalSaveData(tag);
	wstring owner = tag->getString(L"JellyOwner");
	if (!owner.empty())
	{
		setOwnerUUID(owner);
		setPersistenceRequired();
	}
}

// ---- behaviour -------------------------------------------------------------------------------------------------------------
void JellySlime::tick()
{
	// Same as Slime::tick, except that a TAME Jelly Slime is not removed on Peaceful (Slime::tick removes every slime there).
	if (!level->isClientSide && level->difficulty == Difficulty::PEACEFUL && !isTame())
	{
		removed = true;
	}

	squish = squish + (targetSquish - squish) * .5f;

	oSquish = squish;
	bool wasOnGround = onGround;
	Mob::tick();
	if (onGround && !wasOnGround)
	{
		int size = getSize();
		for (int i = 0; i < size * 8; i++)
		{
			float dir = random->nextFloat() * PI * 2;
			float d = random->nextFloat() * 0.5f + 0.5f;
			float xd = Mth::sin(dir) * size * 0.5f * d;
			float zd = Mth::cos(dir) * size * 0.5f * d;
			level->addParticle(getParticleName(), x + xd, bb->y0, z + zd, 0, 0, 0);
		}

		if (doPlayLandSound())
		{
			playSound(getSquishSound(), getSoundVolume(), ((random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f) / 0.8f);
		}
		targetSquish = -0.5f;
	}
	else if (!onGround && wasOnGround)
	{
		targetSquish = 1;
	}
	decreaseSquish();

	if (level->isClientSide)
	{
		int size = getSize();
		setSize(0.6f * size, 0.6f * size);
	}
}

void JellySlime::serverAiStep()
{
	if (!isTame())
	{
		Slime::serverAiStep();		// wild: hop after the nearest player like any slime
		return;
	}

	// Tame: follows its owner at a distance, otherwise wanders with an occasional lazy hop. Never attacks.
	shared_ptr<Player> owner = level->getPlayerByUUID(getOwnerUUID());
	bool follow = false;
	if (owner != nullptr)
	{
		double distSqr = distanceToSqr(owner);
		follow = distSqr > FOLLOW_MIN_DIST * FOLLOW_MIN_DIST && distSqr < FOLLOW_MAX_DIST * FOLLOW_MAX_DIST;
		if (follow) lookAt(owner, 10, 20);
	}

	if (onGround && jumpDelay-- <= 0)
	{
		jumpDelay = follow ? random->nextInt(10) + 8 : random->nextInt(60) + 40;
		if (follow || random->nextInt(3) == 0)
		{
			jumping = true;
			if (doPlayJumpSound())
			{
				playSound(getSquishSound(), getSoundVolume(), ((random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f) * 0.8f);
			}
			xxa = follow ? 0.0f : 1 - random->nextFloat() * 2;
			yya = static_cast<float>(1) * getSize();
		}
		else
		{
			jumping = false;
			xxa = yya = 0;
		}
	}
	else
	{
		jumping = false;
		if (onGround)
		{
			xxa = yya = 0;
		}
	}
}

ePARTICLE_TYPE JellySlime::getParticleName()
{
	return eParticleType_jelly;
}

bool JellySlime::isDealsDamage()
{
	return !isTame();
}

int JellySlime::getAttackDamage()
{
	return JELLY_ATTACK_DAMAGE;
}

void JellySlime::remove()
{
	// A normal slime splits into smaller ones here; a Jelly Slime does not.
	Mob::remove();
}

bool JellySlime::canSpawn()
{
	if (level->difficulty == Difficulty::PEACEFUL) return false;

	// Very rare: only about 1 attempt in 45 gets through, so daytime spawns do not fill the monster cap.
	// (They like the warm biomes - see the weights in Desert/Savanna/Mesa vs the low one in Plains. Normal slimes are not affected.)
	if (random->nextInt(45) != 0) return false;

	// surface only (Pinky lives above ground)
	if (!level->canSeeSky(Mth::floor(x), Mth::floor(y), Mth::floor(z))) return false;

	return Mob::canSpawn();
}

void JellySlime::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	int count = 1 + random->nextInt(3);
	if (playerBonusLevel > 0)
	{
		count += random->nextInt(playerBonusLevel + 1);
	}
	for (int i = 0; i < count; i++)
	{
		spawnAtLocation(Item::jelly_ball_Id, 1);
	}
}
