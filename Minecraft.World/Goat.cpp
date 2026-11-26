#include "stdafx.h"
#include "AABB.h"
#include "Inventory.h"
#include "Abilities.h"
#include "PathNavigation.h"
#include "AttributeInstance.h"
#include "CompoundTag.h"
#include "ItemInstance.h"
#include "Player.h"
#include "net.minecraft.world.entity.h"
#include "SynchedEntityData.h"
#include "SharedMonsterAttributes.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "Mth.h"
#include "Goat.h"
#include "GoatRamGoal.h"
#include "GoatLeapGoal.h"

// Upward speed of an ordinary jump (a normal animal is 0.42, ~1.25 blocks). 0.62 is ~2.4 blocks, so
// goats scramble up 2-block cliffs. (Their random big leaps are in GoatLeapGoal.)
static const float GOAT_JUMP_SPEED = 0.62f;

// Extra safe-fall distance on top of the normal 3 blocks: damage starts after 3 + 7 = 10 blocks.
static const float GOAT_EXTRA_SAFE_FALL = 7.0f;

Goat::Goat(Level *level) : Animal(level)
{
	// Horns start attached (DATA_HORNS defaults to 3 = both, in defineSynchedData).
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	this->setSize(0.9f, 1.3f);

	getNavigation()->setAvoidWater(true);
	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new PanicGoal(this, 1.6f));
	goalSelector.addGoal(2, new BreedGoal(this, 1.0f));
	goalSelector.addGoal(3, new TemptGoal(this, 1.25f, Item::wheat_Id, false));
	goalSelector.addGoal(4, new FollowParentGoal(this, 1.25f));
	goalSelector.addGoal(4, new GoatRamGoal(this));		// random headbutt
	goalSelector.addGoal(5, new GoatLeapGoal(this));		// random big leap
	goalSelector.addGoal(5, new RandomStrollGoal(this, 1.0f));
	goalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 6));
	goalSelector.addGoal(7, new RandomLookAroundGoal(this));
}

void Goat::defineSynchedData()
{
	Animal::defineSynchedData();
	entityData->define(DATA_RAMMING, static_cast<byte>(0));
	entityData->define(DATA_HORNS, static_cast<byte>(3));		// both horns attached
}

bool Goat::isRamming()
{
	return entityData->getByte(DATA_RAMMING) != 0;
}

void Goat::setRamming(bool ramming)
{
	entityData->set(DATA_RAMMING, static_cast<byte>(ramming ? 1 : 0));
}

// Synched horn state, readable on the client (the model hides a lost horn).
bool Goat::hasLeftHorn()
{
	return (entityData->getByte(DATA_HORNS) & 1) != 0;
}

bool Goat::hasRightHorn()
{
	return (entityData->getByte(DATA_HORNS) & 2) != 0;
}

// neoLegacy - drop one horn as an item when a ram slams the goat into a solid block
// (Java: Goat.dropHorn(), up to two per goat — one per horn, picked at random among the
// ones still attached). Babies and hornless goats drop nothing. The lost horn's bit is
// cleared from DATA_HORNS, so the model hides it and it never comes back (no regrowth).
// Called from GoatRamGoal when the charge hits a wall.
bool Goat::dropHorn()
{
	if (isBaby()) return false;
	byte mask = entityData->getByte(DATA_HORNS);
	if (mask == 0) return false;
	// Pick one of the remaining horns at random (Java-faithful); with one left it is that one.
	byte drop;
	if (mask == 3) drop = (level->random->nextInt(2) == 0) ? static_cast<byte>(1) : static_cast<byte>(2);
	else drop = mask;
	entityData->set(DATA_HORNS, static_cast<byte>(mask & ~drop));
	spawnAtLocation(Item::goat_horn_Id, 1);
	return true;
}

void Goat::addAdditonalSaveData(CompoundTag *tag)
{
	Animal::addAdditonalSaveData(tag);
	tag->putBoolean(L"HasLeftHorn", hasLeftHorn());
	tag->putBoolean(L"HasRightHorn", hasRightHorn());
}

void Goat::readAdditionalSaveData(CompoundTag *tag)
{
	Animal::readAdditionalSaveData(tag);
	if (tag->contains(L"HasLeftHorn") && tag->contains(L"HasRightHorn"))
	{
		byte mask = 0;
		if (tag->getBoolean(L"HasLeftHorn")) mask |= 1;
		if (tag->getBoolean(L"HasRightHorn")) mask |= 2;
		entityData->set(DATA_HORNS, mask);
	}
	else if (tag->contains(L"HornsLeft"))
	{
		// Pre-visual saves stored only a count: 2 = both horns, 1 = one horn, 0 = none.
		// Which side was lost is unrecorded, so a one-horn goat keeps its left horn.
		int left = tag->getInt(L"HornsLeft");
		byte mask = (left >= 2) ? static_cast<byte>(3) : (left == 1) ? static_cast<byte>(1) : static_cast<byte>(0);
		entityData->set(DATA_HORNS, mask);
	}
	// else: fresh goat, the ctor default (both horns) stands.
}

bool Goat::useNewAi()
{
	return true;
}

void Goat::registerAttributes()
{
	Animal::registerAttributes();

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(10);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.22f);
}

bool Goat::canSpawn()
{
	// Animal::canSpawn() only allows grass, but the hills these live in are often bare stone or
	// gravel on top, so accept those too.
	int xt = Mth::floor(x);
	int yt = Mth::floor(bb->y0);
	int zt = Mth::floor(z);

	int below = level->getTile(xt, yt - 1, zt);
	bool goodGround = below == Tile::grass_Id || below == Tile::stone_Id || below == Tile::gravel_Id || below == Tile::dirt_Id;

	return goodGround && level->getDaytimeRawBrightness(xt, yt, zt) > 8 && AgableMob::canSpawn();
}

int Goat::getAmbientSound()
{
	return eSoundType_MOB_GOAT_AMBIENT;
}

int Goat::getHurtSound()
{
	return eSoundType_MOB_GOAT_HURT;
}

int Goat::getDeathSound()
{
	return eSoundType_MOB_GOAT_DEATH;
}

void Goat::playStepSound(int xt, int yt, int zt, int t)
{
	playSound(eSoundType_MOB_GOAT_STEP, 0.15f, 1);
}

float Goat::getVoicePitch()
{
	// The recorded goat screams are used as-is; babies get the usual higher pitch from Animal.
	return Animal::getVoicePitch();
}

void Goat::jumpFromGround()
{
	Animal::jumpFromGround();
	if (yd < GOAT_JUMP_SPEED)
	{
		yd = GOAT_JUMP_SPEED;
	}
	playSound(eSoundType_MOB_GOAT_JUMP, 0.6f, 1.0f);
}

void Goat::causeFallDamage(float distance)
{
	// LivingEntity applies damage for (distance - 3); pretending the fall was shorter moves that
	// threshold out to ~10 blocks.
	Animal::causeFallDamage(distance - GOAT_EXTRA_SAFE_FALL);
}

bool Goat::mobInteract(shared_ptr<Player> player)
{
	shared_ptr<ItemInstance> item = player->inventory->getSelected();
	if (item != nullptr && item->id == Item::bucket->id && !player->abilities.instabuild)
	{
		if (item->count-- == 0)
		{
			player->inventory->setItem(player->inventory->selected, std::make_shared<ItemInstance>(Item::milk_bucket));
		}
		else if (!player->inventory->add(std::make_shared<ItemInstance>(Item::milk_bucket)))
		{
			player->drop(std::make_shared<ItemInstance>(Item::milk_bucket));
		}

		return true;
	}

	// Feeding wheat (breeding): the goat's eat sound. Mirrors Animal::mobInteract's
	// feeding condition so it only plays when the wheat is actually about to be eaten.
	// Server-side only; the client hears it via the sound broadcast.
	if (!level->isClientSide && item != nullptr && isFood(item) && getAge() == 0 && getInLoveValue() <= 0)
	{
		playSound(eSoundType_MOB_GOAT_EAT, 1.0f, 1.0f);
	}
	return Animal::mobInteract(player);
}

shared_ptr<AgableMob> Goat::getBreedOffspring(shared_ptr<AgableMob> target)
{
	// 4J - added limit to number of animals that can be bred
	if (level->canCreateMore(GetType(), Level::eSpawnType_Breed))
	{
		return std::make_shared<Goat>(level);
	}
	else
	{
		return nullptr;
	}
}
