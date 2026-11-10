#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.damagesource.h"
#include "Boat.h"

const double Boat::MAX_SPEED = 0.35;
const double Boat::MAX_COLLISION_SPEED = MAX_SPEED * 0.75;
const double Boat::MIN_ACCELERATION = 0.07;
const double Boat::MAX_ACCELERATION = 0.35;

// 4J - added for common ctor code
void Boat::_init()
{
	doLerp = true;
	acceleration = MIN_ACCELERATION;

	rowingTime = rowingTimeO = 0.0f;
	paddleBlend = paddleBlendO = 0.0f;
	paddleLastX = paddleLastZ = 0.0;
	paddleInit = false;

	lSteps = 0;
	lx = ly = lz = lyr = lxr = 0.0;
	lxd = lyd = lzd = 0.0;

	blocksBuilding = true;
	setSize(1.5f, 0.6f);
	heightOffset = (bbHeight / 2.0f) + 0.2f;

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
}



Boat::Boat(Level *level) : Entity( level )
{

	_init();
}


bool Boat::makeStepSound()
{
	return false;
}

void Boat::defineSynchedData()
{
	entityData->define(DATA_ID_HURT, 0);
	entityData->define(DATA_ID_HURTDIR, 1);
	entityData->define(DATA_ID_DAMAGE, 0.0f);
	entityData->define(DATA_ID_WOOD, 0);
	entityData->define(DATA_ID_DRIVER, -1);
}

// ---- seats (neoLegacy) -----------------------------------------------------------------------------

// Every player whose "riding" is this boat (same answer on the server and on the clients).
void Boat::collectRiders(vector<shared_ptr<Entity> > &out)
{
	for (size_t i = 0; i < level->players.size(); i++)
	{
		shared_ptr<Player> p = level->players[i];
		if (p != nullptr && !p->removed && p->riding.get() == this) out.push_back(p);
	}
}

int Boat::countRiders()
{
	vector<shared_ptr<Entity> > riders;
	collectRiders(riders);
	return static_cast<int>(riders.size());
}

shared_ptr<Entity> Boat::getDriver()
{
	int id = entityData->getInteger(DATA_ID_DRIVER);
	if (id < 0) return nullptr;
	vector<shared_ptr<Entity> > riders;
	collectRiders(riders);
	for (size_t i = 0; i < riders.size(); i++)
	{
		if (riders[i]->entityId == id) return riders[i];
	}
	return nullptr;
}

// Server only: keep the driver seat filled. If the driver got off, whoever is left takes the oars.
void Boat::updateDriverSeat()
{
	vector<shared_ptr<Entity> > riders;
	collectRiders(riders);
	int driverId = entityData->getInteger(DATA_ID_DRIVER);
	bool driverStillAboard = false;
	for (size_t i = 0; i < riders.size(); i++)
	{
		if (riders[i]->entityId == driverId) driverStillAboard = true;
	}
	if (!driverStillAboard)
	{
		entityData->set(DATA_ID_DRIVER, riders.empty() ? -1 : riders[0]->entityId);
	}
}

int Boat::getWoodType()
{
	int t = entityData->getInteger(DATA_ID_WOOD);
	return (t < 0 || t > 6) ? 0 : t;
}

void Boat::setWoodType(int woodType)
{
	if (woodType < 0 || woodType > 6) woodType = 0;
	entityData->set(DATA_ID_WOOD, woodType);
}


AABB *Boat::getCollideAgainstBox(shared_ptr<Entity> entity)
{
	return entity->bb;
}

AABB *Boat::getCollideBox()
{
	return bb;
}

bool Boat::isPushable()
{
	return true;
}

Boat::Boat(Level *level, double x, double y, double z) : Entity( level )
{
	_init();
	setPos(x, y + heightOffset, z);

	xd = 0;
	yd = 0;
	zd = 0;

	xo = x;
	yo = y;
	zo = z;
}

double Boat::getRideHeight()
{
	return heightOffset - 0.4f;
}

bool Boat::hurt(DamageSource *source, float hurtDamage)
{
	if (isInvulnerable()) return false;
	if (level->isClientSide || removed) return true;

	// 4J-JEV: Fix for #88212,
	// Untrusted players shouldn't be able to damage minecarts or boats.
	if (dynamic_cast<EntityDamageSource *>(source) != nullptr)
	{
		shared_ptr<Entity> attacker = source->getDirectEntity();

		if ( attacker->instanceof(eTYPE_PLAYER) && !dynamic_pointer_cast<Player>(attacker)->isAllowedToHurtEntity( shared_from_this() ))
		{
			return false;
		}
	}

	setHurtDir(-getHurtDir());
	setHurtTime(10);

	// 4J Stu - If someone is riding in this, then it can tick multiple times which causes the damage to
	// decrease too quickly. So just make the damage a bit higher to start with for similar behaviour
	// to an unridden one. Only do this change if the riding player is attacking it.
	if( source->getEntity() != nullptr && source->getEntity()->riding.get() == this ) hurtDamage += 1;

	setDamage(getDamage() + hurtDamage * 10);
	markHurt();

	// 4J Stu - Brought froward from 12w36 to fix #46611 - TU5: Gameplay: Minecarts and boat requires more hits than one to be destroyed in creative mode
	// 4J-PB - Fix for XB1 #175735 - [CRASH] [Multi-Plat]: Code: Gameplay: Placing a boat on harmful surfaces causes the game to crash
	bool creativePlayer = (source->getEntity() != nullptr) && source->getEntity()->instanceof(eTYPE_PLAYER) && dynamic_pointer_cast<Player>(source->getEntity())->abilities.instabuild;

	if (creativePlayer || getDamage() > 20 * 2)
	{
		{
			// neoLegacy - both passengers get out, not just the one the engine remembers
			vector<shared_ptr<Entity> > riders;
			collectRiders(riders);
			for (size_t i = 0; i < riders.size(); i++) riders[i]->ride(nullptr);
		}
		if (!creativePlayer) spawnAtLocation(Item::boat_Id, 1, getWoodType());	// neoLegacy: drops the same wood it was made from
		remove();
	}
	return true;
}

void Boat::animateHurt()
{
	setHurtDir(-getHurtDir());
	setHurtTime(10);
	setDamage(getDamage() * 11);
}

bool Boat::isPickable()
{
	return !removed;
}


void Boat::lerpTo(double x, double y, double z, float yRot, float xRot, int steps)
{
	if (doLerp)
	{
		lSteps = steps +5;
	}
	else
	{
		double xdiff = x - this->x;
		double ydiff = y - this->y;
		double zdiff = z - this->z;
		double diff = xdiff * xdiff + ydiff * ydiff + zdiff * zdiff;

		if (diff > 1)
		{
			lSteps = 3;
		}
		else
		{
			return;
		}
	}

	lx = x;
	ly = y;
	lz = z;
	lyr = yRot;
	lxr = xRot;

	xd = lxd;
	yd = lyd;
	zd = lzd;
}

void Boat::lerpMotion(double xd, double yd, double zd)
{
	lxd = this->xd = xd;
	lyd = this->yd = yd;
	lzd = this->zd = zd;
}

void Boat::tick()
{
	Entity::tick();




	if (getHurtTime() > 0) setHurtTime(getHurtTime() - 1);
	if (getDamage() > 0) setDamage(getDamage() - 1);
	xo = x;
	yo = y;
	zo = z;

	// neoLegacy - drive the paddle animation from how far the boat actually moved last tick
	{
		double mdx = x - paddleLastX;
		double mdz = z - paddleLastZ;
		if (!paddleInit) { mdx = mdz = 0.0; paddleInit = true; }
		paddleLastX = x;
		paddleLastZ = z;
		float moved = static_cast<float>(sqrt(mdx * mdx + mdz * mdz));
		bool paddling = countRiders() > 0 && moved > 0.02f;

		rowingTimeO = rowingTime;
		paddleBlendO = paddleBlend;
		if (paddling) rowingTime += 0.25f + moved * 1.6f;
		paddleBlend += ((paddling ? 1.0f : 0.0f) - paddleBlend) * 0.35f;
	}

	// Check how much of the boat is in water
	int steps = 5;
	double waterPercentage = 0;
	for (int i = 0; i < steps; i++)
	{
		double y0 = bb->y0 + (bb->y1 - bb->y0) * (i + 0) / steps + 1.5f / 16.0f;
		double y1 = bb->y0 + (bb->y1 - bb->y0) * (i + 1) / steps + 1.5f / 16.0f;
		AABB *bb2 = AABB::newTemp(bb->x0, y0, bb->z0, bb->x1, y1, bb->z1);
		if (level->containsLiquid(bb2, Material::water))
		{
			waterPercentage += 1.0 / steps;
		}
	}

	// Create particles
	double lastSpeed = sqrt(xd * xd + zd * zd);
	if (lastSpeed > MAX_COLLISION_SPEED && waterPercentage > 0)
	{
		createSplash(lastSpeed);
	}

	// Interpolation
	if (level->isClientSide && doLerp)
	{
		if (lSteps > 0)
		{
			double xt = x + (lx - x) / lSteps;
			double yt = y + (ly - y) / lSteps;
			double zt = z + (lz - z) / lSteps;

			double yrd = Mth::wrapDegrees(lyr - yRot);

			yRot += (float) ( (yrd) / lSteps );
			xRot += (float) ( (lxr - xRot) / lSteps );

			lSteps--;
			setPos(xt, yt, zt);
			setRot(yRot, xRot);
		}
		else
		{
			// Original
			//double xt = x + xd;
			//double yt = y + yd;
			//double zt = z + zd;
			//this->setPos(xt, yt, zt);

			// 4J Stu - Fix for various boat bugs, ensure that we check collision on client-side movement
			move(xd,yd,zd);

			if (onGround)
			{
				xd *= 0.5f;
				yd *= 0.5f;
				zd *= 0.5f;
			}
			xd *= 0.99f;
			yd *= 0.95f;
			zd *= 0.99f;
		}
		return;
	}

	// neoLegacy - the driver is whoever the synched driver id points at (server decides, clients just follow)
	if (!level->isClientSide) updateDriverSeat();

	// Bob in water & gravity
	if (waterPercentage < 1.0)
	{
		double bob = waterPercentage * 2.0 - 1.0;
		yd += 0.04f * bob;
	}
	else
	{
		if (yd < 0.0)
		{
			yd /= 2.0;
		}
		yd += 0.007f;
	}

	// Rider controls
	shared_ptr<Entity> driver = getDriver();
	if ( driver != nullptr && driver->instanceof(eTYPE_LIVINGENTITY) )
	{
		shared_ptr<LivingEntity> livingRider = dynamic_pointer_cast<LivingEntity>(driver);
		double forward = livingRider->yya;

		if (forward != 0)
		{
			double riderXd = -sin(livingRider->yRot * PI / 180);
			double riderZd = cos(livingRider->yRot * PI / 180);
			double currentSpeed = sqrt(xd * xd + zd * zd);
			float moveFactor = (float)forward;
			if (forward < 0) moveFactor *= 0.5f; // Move slower backwards
			xd += riderXd * acceleration * 0.05f * moveFactor;
			zd += riderZd * acceleration * 0.05f * moveFactor;
		}
	}

	double curSpeed = sqrt(xd * xd + zd * zd);
	double maxSpeed = MAX_SPEED;

	if (curSpeed > maxSpeed)
	{
		double ratio = maxSpeed / curSpeed;

		xd *= ratio;
		zd *= ratio;
		curSpeed = maxSpeed;
	}

	if (curSpeed > lastSpeed && acceleration < MAX_ACCELERATION)
	{
		acceleration += (MAX_ACCELERATION - acceleration) / 35;
		if (acceleration > MAX_ACCELERATION) acceleration = MAX_ACCELERATION;
	}
	else
	{
		acceleration -= (acceleration - MIN_ACCELERATION) / 35;
		if (acceleration < MIN_ACCELERATION) acceleration = MIN_ACCELERATION;
	}

	// Slow on ground
	if (onGround)
	{
		xd *= 0.5f;
		yd *= 0.5f;
		zd *= 0.5f;
	}
	move(xd, yd, zd);

	// Break boat on high speed collision
	float breakThreshold = (countRiders() > 0) ? 0.35f : 0.20f;
	if ((horizontalCollision && lastSpeed > 0.20))
	{
		if (!level->isClientSide && !removed)
		{
			
			remove();
			for (int i = 0; i < 3; i++)
			{
				spawnAtLocation(Tile::planks_Id, 1, 0);
			}
			for (int i = 0; i < 2; i++)
			{
				spawnAtLocation(Item::stick->id, 1, 0);
			}
			
			

		}
	}
	else
	{
		xd *= 0.99f;
		yd *= 0.95f;
		zd *= 0.99f;
	}

	xRot = 0;
	double yRotT = yRot;
	double xDiff = xo - x;
	double zDiff = zo - z;
	if (xDiff * xDiff + zDiff * zDiff > 0.001)
	{
		yRotT = (float) (atan2(zDiff, xDiff) * 180 / PI);
	}

	double rotDiff = Mth::wrapDegrees(yRotT - yRot);

	if (rotDiff > 20) rotDiff = 20;
	if (rotDiff < -20) rotDiff = -20;

	yRot += (float) rotDiff;
	setRot(yRot, xRot);

	// Server code after this
	if(level->isClientSide) return;

	vector<shared_ptr<Entity> > *entities = level->getEntities(shared_from_this(), bb->grow(0.2f, 0, 0.2f));
	if (entities && !entities->empty())
	{
		for ( auto& e : *entities )
		{
			if (e->isPushable() && e->GetType() ==  eTYPE_BOAT)
			{
				e->push(shared_from_this());
			}
		}
	}

	for (int i = 0; i < 4; i++)
	{
		int xx = Mth::floor(x + ((i % 2) - 0.5) * 0.8);
		int zz = Mth::floor(z + ((i / 2) - 0.5) * 0.8);

		for (int j = 0; j < 2; j++)
		{
			int yy = Mth::floor(y) + j;
			int tile = level->getTile(xx, yy, zz);

			if (tile == Tile::snow_layer_Id)
			{
				level->removeTile(xx, yy, zz);
			}
			else if (tile == Tile::waterlily_Id)
			{
				level->destroyTile(xx, yy, zz, true);
			}
		}

	}
}

void Boat::createSplash(double particleStrengh) {
	double xa = cos(yRot * PI / 180);
	double za = sin(yRot * PI / 180);

	for (int i = 0; i < 1 + particleStrengh * 60; i++)
	{
		double side = (random->nextFloat() * 2 - 1);

		double side2 = (random->nextInt(2) * 2 - 1) * 0.7;
		if (random->nextBoolean())
		{
			double xx = x - xa * side * 0.8 + za * side2;
			double zz = z - za * side * 0.8 - xa * side2;
			level->addParticle(eParticleType_splash, xx, y - 2 / 16.0f, zz, +xd, yd, +zd);
		}
		else
		{
			double xx = x + xa + za * side * 0.7;
			double zz = z + za - xa * side * 0.7;
			level->addParticle(eParticleType_splash, xx, y - 2 / 16.0f, zz, +xd, yd, +zd);
		}
	}
}

void Boat::positionRider()
{
	// neoLegacy - two seats. (cos yRot, sin yRot) points to the STERN of the boat (yRot is worked out from the direction
	// the boat came from), so a positive offset sits further back. A lone rider keeps the old spot (0.4 back); with two
	// aboard they take the Java Edition seats: the driver 0.2 forward of centre and the passenger 0.6 back.
	vector<shared_ptr<Entity> > riders;
	collectRiders(riders);
	if (riders.empty()) return;

	shared_ptr<Entity> driver = getDriver();
	double ca = cos(yRot * PI / 180);
	double sa = sin(yRot * PI / 180);
	for (size_t i = 0; i < riders.size(); i++)
	{
		double off = 0.4;
		if (riders.size() >= 2) off = (riders[i] == driver) ? -0.2 : 0.6;
		// We minus 0.4 to ensure that the player is not hovering above the boat.
		riders[i]->setPos(x + ca * off, y + getRideHeight() + riders[i]->getRidingHeight() - 0.4, z + sa * off);
	}
}


void Boat::addAdditonalSaveData(CompoundTag *base)
{
	base->putInt(L"WoodType", getWoodType());
}

void Boat::readAdditionalSaveData(CompoundTag *base)
{
	setWoodType(base->getInt(L"WoodType"));	// old saves have no tag -> 0 = oak
}


float Boat::getShadowHeightOffs()
{
	return 0;
}

wstring Boat::getName()
{
	return L"Boat";
}

bool Boat::interact(shared_ptr<Player> player)
{
	if (!level->isClientSide)
	{
		// neoLegacy - "am I aboard" is asked of the player, not of the boat's single remembered rider
		bool isRiding = (player->riding.get() == this);

		if (isRiding)
		{
			player->xd = 0;
			player->yd = 0;
			player->zd = 0;
			// 4J HEG - Fixed issue with player not being able to dismount boat (issue #4446)
			player->ride(nullptr);
		}
		else
		{
			// neoLegacy - two seats; a third player has to wait
			if (countRiders() >= MAX_RIDERS) return true;

			bool first = (countRiders() == 0);
			player->ride(shared_from_this());
			if (first || getDriver() == nullptr) entityData->set(DATA_ID_DRIVER, player->entityId);
		}
	}
	return true;
}

void Boat::setDamage(float damage)
{
	entityData->set(DATA_ID_DAMAGE, damage);
}

float Boat::getDamage()
{
	return entityData->getFloat(DATA_ID_DAMAGE);
}

void Boat::setHurtTime(int hurtTime)
{
	entityData->set(DATA_ID_HURT, hurtTime);
}

int Boat::getHurtTime()
{
	return entityData->getInteger(DATA_ID_HURT);
}

void Boat::setHurtDir(int hurtDir)
{
	entityData->set(DATA_ID_HURTDIR, hurtDir);
}

int Boat::getHurtDir()
{
	return entityData->getInteger(DATA_ID_HURTDIR);
}

bool Boat::getDoLerp()
{
	return doLerp;
}

void Boat::setDoLerp(bool doLerp)
{
	this->doLerp = doLerp;
}

