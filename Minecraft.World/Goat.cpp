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
	hornsLeft = 2;		// one per horn; a ram into a solid block drops one (see GoatRamGoal)
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
}

bool Goat::isRamming()
{
	return entityData->getByte(DATA_RAMMING) != 0;
}

void Goat::setRamming(bool ramming)
{
	entityData->set(DATA_RAMMING, static_cast<byte>(ramming ? 1 : 0));
}

// neoLegacy - drop one horn as an item when a ram slams the goat into a solid block
// (Java: Goat.dropHorn(), up to two per goat — one per horn). Babies and hornless goats
// drop nothing. Called from GoatRamGoal when the charge hits a wall.
bool Goat::dropHorn()
{
	if (isBaby() || hornsLeft <= 0) return false;
	spawnAtLocation(Item::goat_horn_Id, 1);
	hornsLeft--;
	return true;
}

void Goat::addAdditonalSaveData(CompoundTag *tag)
{
	Animal::addAdditonalSaveData(tag);
	tag->putInt(L"HornsLeft", hornsLeft);
}

void Goat::readAdditionalSaveData(CompoundTag *tag)
{
	Animal::readAdditionalSaveData(tag);
	if (tag->contains(L"HornsLeft")) hornsLeft = tag->getInt(L"HornsLeft");
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
	playSound(eSoundType_MOB_SHEEP_STEP, 0.15f, 1);
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
