#include "stdafx.h"
#include "AABB.h"
#include "Random.h"
#include "Inventory.h"
#include "Abilities.h"
#include "PathNavigation.h"
#include "AttributeInstance.h"
#include "ItemInstance.h"
#include "Player.h"
#include "net.minecraft.world.entity.h"
#include "SynchedEntityData.h"
#include "SharedMonsterAttributes.h"
#include "DamageSource.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "com.mojang.nbt.h"
#include "Mth.h"
#include "Camel.h"

// Tuning
static const float CAMEL_WALK_SPEED = 0.10f;		// AI walking speed attribute
static const float CAMEL_RIDDEN_SPEED = 0.16f;		// when a player steers it
// Jump-dash: ~12 blocks forward, ~1 block up, 2.75 s (55 ticks) cooldown.
static const float CAMEL_DASH_SPEED = 1.7f;			// initial horizontal speed (decays with air/ground friction)
static const float CAMEL_DASH_LIFT = 0.38f;			// initial upward speed (~1 block apex)
static const int   CAMEL_DASH_COOLDOWN = 55;

Camel::Camel(Level *level) : Animal(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	sitTicksLeft = 0;
	wasJumping = false;
	dashCooldown = 0;

	this->setSize(1.7f, 2.375f);
	footSize = 1.5f;		// steps up 1.5 blocks

	getNavigation()->setAvoidWater(true);
	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new PanicGoal(this, 1.6f));
	goalSelector.addGoal(2, new BreedGoal(this, 1.0f));
	goalSelector.addGoal(3, new TemptGoal(this, 1.2f, Tile::cactus_Id, false));
	goalSelector.addGoal(4, new FollowParentGoal(this, 1.2f));
	goalSelector.addGoal(5, new RandomStrollGoal(this, 1.0f));
	goalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 6));
	goalSelector.addGoal(7, new RandomLookAroundGoal(this));
}

void Camel::defineSynchedData()
{
	Animal::defineSynchedData();
	entityData->define(DATA_FLAGS, static_cast<byte>(0));
	entityData->define(DATA_DRIVER, -1);
}

void Camel::setFlag(int flag, bool value)
{
	int cur = entityData->getByte(DATA_FLAGS);
	entityData->set(DATA_FLAGS, static_cast<byte>(value ? (cur | flag) : (cur & ~flag)));
}

bool Camel::getFlag(int flag)
{
	return (entityData->getByte(DATA_FLAGS) & flag) != 0;
}

bool Camel::isSaddled() { return getFlag(FLAG_SADDLED); }
void Camel::setSaddled(bool saddled) { setFlag(FLAG_SADDLED, saddled); }
bool Camel::isSitting() { return getFlag(FLAG_SITTING); }
void Camel::setSitting(bool sitting)
{
	if (sitting != isSitting() && !level->isClientSide)
	{
		playSound(sitting ? eSoundType_MOB_CAMEL_SIT : eSoundType_MOB_CAMEL_STAND, 0.8f, 1.0f);
	}
	setFlag(FLAG_SITTING, sitting);
}

// ---- seats ------------------------------------------------------------------------------------------

// Every player whose "riding" is this camel (same on the server and the client).
void Camel::collectRiders(vector<shared_ptr<Entity> > &out)
{
	for (size_t i = 0; i < level->players.size(); i++)
	{
		shared_ptr<Player> p = level->players[i];
		if (p != nullptr && !p->removed && p->riding.get() == this) out.push_back(p);
	}
}

int Camel::countRiders()
{
	vector<shared_ptr<Entity> > riders;
	collectRiders(riders);
	return static_cast<int>(riders.size());
}

shared_ptr<Entity> Camel::getDriver()
{
	int id = entityData->getInteger(DATA_DRIVER);
	if (id < 0) return nullptr;
	vector<shared_ptr<Entity> > riders;
	collectRiders(riders);
	for (size_t i = 0; i < riders.size(); i++)
	{
		if (riders[i]->entityId == id) return riders[i];
	}
	return nullptr;
}

// Driver sits forward, the passenger behind them.
void Camel::positionRider()
{
	vector<shared_ptr<Entity> > riders;
	collectRiders(riders);
	if (riders.empty()) return;

	shared_ptr<Entity> driver = getDriver();
	float rad = yRot * PI / 180.0f;
	for (size_t i = 0; i < riders.size(); i++)
	{
		double off = (riders[i] == driver || riders.size() == 1) ? 0.4 : -0.7;
		riders[i]->setPos(x - Mth::sin(rad) * off, y + getRideHeight() + riders[i]->getRidingHeight(), z + Mth::cos(rad) * off);
	}
}

bool Camel::useNewAi()
{
	return true;
}

void Camel::registerAttributes()
{
	Animal::registerAttributes();
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(32);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(CAMEL_WALK_SPEED);
}

bool Camel::canSpawn()
{
	int xt = Mth::floor(x);
	int yt = Mth::floor(bb->y0);
	int zt = Mth::floor(z);
	int below = level->getTile(xt, yt - 1, zt);
	bool goodGround = below == Tile::sand_Id || below == Tile::grass_Id || below == Tile::sandstone_Id;
	return goodGround && level->getDaytimeRawBrightness(xt, yt, zt) > 8 && AgableMob::canSpawn();
}

bool Camel::isFood(shared_ptr<ItemInstance> item)
{
	return item != nullptr && item->id == Tile::cactus_Id;
}

double Camel::getRideHeight()
{
	return 2.0;
}

bool Camel::mobInteract(shared_ptr<Player> player)
{
	shared_ptr<ItemInstance> item = player->inventory->getSelected();

	// Saddle it.
	if (item != nullptr && item->id == Item::saddle_Id && !isSaddled() && !isBaby())
	{
		if (!level->isClientSide)
		{
			setSaddled(true);
			if (!player->abilities.instabuild) item->count--;
		}
		return true;
	}

	// Feeding (breeding) takes priority over mounting.
	if (item != nullptr && isFood(item))
	{
		bool fed = Animal::mobInteract(player);
		if (fed && !level->isClientSide) playSound(eSoundType_MOB_CAMEL_EAT, 1.0f, 1.0f);
		return fed;
	}

	// Ride it. No taming needed, just a saddle. Two seats: the first player drives, the second sits behind.
	if (isSaddled() && !isBaby())
	{
		if (!level->isClientSide)
		{
			if (countRiders() >= 2) return true;		// full

			bool first = countRiders() == 0;
			setSitting(false);
			sitTicksLeft = 0;
			player->yRot = yRot;
			player->xRot = xRot;
			player->ride(shared_from_this());
			if (first || getDriver() == nullptr) entityData->set(DATA_DRIVER, player->entityId);
		}
		return true;
	}

	return Animal::mobInteract(player);
}

shared_ptr<AgableMob> Camel::getBreedOffspring(shared_ptr<AgableMob> target)
{
	if (level->canCreateMore(GetType(), Level::eSpawnType_Breed))
	{
		return std::make_shared<Camel>(level);
	}
	return nullptr;
}

void Camel::aiStep()
{
	Animal::aiStep();

	if (level->isClientSide) return;

	if (dashCooldown > 0 && --dashCooldown == 0 && getDriver() != nullptr)
	{
		playSound(eSoundType_MOB_CAMEL_DASH_READY, 0.8f, 1.0f);		// recovered: ready to dash again
	}

	// Keep the driver seat filled: if the driver got off, the remaining rider takes over.
	{
		vector<shared_ptr<Entity> > riders;
		collectRiders(riders);
		int driverId = entityData->getInteger(DATA_DRIVER);
		bool driverStillAboard = false;
		for (size_t i = 0; i < riders.size(); i++) if (riders[i]->entityId == driverId) driverStillAboard = true;
		if (!driverStillAboard) entityData->set(DATA_DRIVER, riders.empty() ? -1 : riders[0]->entityId);
	}

	// Random sit-down when left alone; stand up after a while, or immediately if ridden/hurt/bred.
	bool ridden = countRiders() > 0;
	if (isSitting())
	{
		if (ridden || hurtTime > 0 || isInLove() || --sitTicksLeft <= 0 || !onGround) setSitting(false);
	}
	else if (!ridden && onGround && !isInWater() && !isBaby() && !isInLove() && getTarget() == nullptr
		&& random->nextInt(2400) == 0)
	{
		setSitting(true);
		sitTicksLeft = 300 + random->nextInt(600);	// 15-45 s
		getNavigation()->stop();
	}
}

void Camel::travel(float xa, float ya)
{
	if (isSitting())
	{
		// Sitting camels stay put (gravity/knockback still apply through the normal travel).
		Animal::travel(0, 0);
		return;
	}

	shared_ptr<Entity> r = getDriver();
	if (r == nullptr || !isSaddled() || !r->instanceof(eTYPE_LIVINGENTITY))
	{
		footSize = 1.5f;
		flyingSpeed = .02f;
		Animal::travel(xa, ya);
		return;
	}

	// Ridden: steered like a horse. The camel faces where the rider looks.
	yRotO = yRot = r->yRot;
	xRot = r->xRot * 0.5f;
	setRot(yRot, xRot);
	yHeadRot = yBodyRot = yRot;

	shared_ptr<LivingEntity> livingRider = dynamic_pointer_cast<LivingEntity>(r);
	xa = livingRider->xxa * .5f;
	ya = livingRider->yya;
	if (ya <= 0) ya *= .25f;	// much slower backwards

	// Dash: a fresh press of the jump key launches the camel forward and up (only from the ground).
	bool jumpNow = livingRider->isJumping();
	if (!level->isClientSide && jumpNow && !wasJumping && onGround && dashCooldown <= 0)
	{
		float rad = yRot * PI / 180.0f;
		xd = -Mth::sin(rad) * CAMEL_DASH_SPEED;
		zd = Mth::cos(rad) * CAMEL_DASH_SPEED;
		yd = CAMEL_DASH_LIFT;
		dashCooldown = CAMEL_DASH_COOLDOWN;
		hurtMarked = true;		// push the new velocity to the clients
		playSound(eSoundType_MOB_CAMEL_DASH, 1.0f, 1.0f);
	}
	wasJumping = jumpNow;

	footSize = 1.5f;
	float speed = CAMEL_RIDDEN_SPEED;
	flyingSpeed = speed * .1f;
	if (!level->isClientSide)
	{
		setSpeed(speed);
		Animal::travel(xa, ya);
	}

	walkAnimSpeedO = walkAnimSpeed;
	double dx = x - xo;
	double dz = z - zo;
	float wst = Mth::sqrt(dx * dx + dz * dz) * 4.0f;
	if (wst > 1.0f) wst = 1.0f;
	walkAnimSpeed += (wst - walkAnimSpeed) * 0.4f;
	walkAnimPos += walkAnimSpeed;
}

void Camel::die(DamageSource *source)
{
	if (!level->isClientSide && isSaddled())
	{
		spawnAtLocation(Item::saddle_Id, 1);
	}
	Animal::die(source);
}

void Camel::addAdditonalSaveData(CompoundTag *tag)
{
	Animal::addAdditonalSaveData(tag);
	tag->putBoolean(L"Saddled", isSaddled());
}

void Camel::readAdditionalSaveData(CompoundTag *tag)
{
	Animal::readAdditionalSaveData(tag);
	setSaddled(tag->getBoolean(L"Saddled"));
}

int Camel::getAmbientSound() { return eSoundType_MOB_CAMEL_AMBIENT; }
int Camel::getHurtSound() { return eSoundType_MOB_CAMEL_HURT; }
int Camel::getDeathSound() { return eSoundType_MOB_CAMEL_DEATH; }

void Camel::playStepSound(int xt, int yt, int zt, int t)
{
	// Sandy ground has its own, softer footfall.
	bool sandy = (t == Tile::sand_Id || t == Tile::sandstone_Id || t == Tile::red_sandstone_Id);
	playSound(sandy ? eSoundType_MOB_CAMEL_STEP_SAND : eSoundType_MOB_CAMEL_STEP, 0.4f, 1.0f);
}

float Camel::getVoicePitch()
{
	return Animal::getVoicePitch();
}
