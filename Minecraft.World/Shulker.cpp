#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.damagesource.h"
#include "com.mojang.nbt.h"
#include "Facing.h"
#include "SoundTypes.h"
#include "ShulkerBullet.h"
#include "Difficulty.h"
#include "SharedMonsterAttributes.h"
#include "Shulker.h"


Shulker::ShulkerAttackGoal::ShulkerAttackGoal(Shulker *shulker) : Goal(), shulker(shulker), attackTime(0)
{
	setRequiredControlFlags(3);
}

bool Shulker::ShulkerAttackGoal::canUse()
{
	shared_ptr<LivingEntity> target = shulker->getTarget();
	return target != nullptr && target->isAlive() && shulker->level->difficulty != Difficulty::PEACEFUL;
}

void Shulker::ShulkerAttackGoal::start()
{
	attackTime = 20;
	shulker->setRawPeekAmount(100);
}

void Shulker::ShulkerAttackGoal::stop()
{
	shulker->setRawPeekAmount(0);
}

void Shulker::ShulkerAttackGoal::tick()
{
	if (shulker->level->difficulty == Difficulty::PEACEFUL) return;

	--attackTime;
	shared_ptr<LivingEntity> target = shulker->getTarget();
	if (target == nullptr) return;
	shulker->getLookControl()->setLookAt(target, 180.0f, 180.0f);

	if (shulker->distanceToSqr(target) < 400.0)
	{
		if (attackTime <= 0)
		{
			attackTime = 20 + shulker->getRandom()->nextInt(10) * 20 / 2;
			int axis = shulker->getAttachFace() / 2;
			shulker->level->addEntity(std::make_shared<ShulkerBullet>(shulker->level, dynamic_pointer_cast<LivingEntity>(shulker->shared_from_this()), target, axis));
			shulker->playSound(eSoundType_ENTITY_SHULKER_SHOOT, 2.0f, (shulker->getRandom()->nextFloat() - shulker->getRandom()->nextFloat()) * 0.2f + 1.0f);
		}
	}
	else
	{
		shulker->setTarget(nullptr);
	}
}

Shulker::ShulkerPeekGoal::ShulkerPeekGoal(Shulker *shulker) : Goal(), shulker(shulker), peekTime(0)
{
}

bool Shulker::ShulkerPeekGoal::canUse()
{
	return shulker->getTarget() == nullptr && shulker->getRandom()->nextInt(40) == 0;
}

bool Shulker::ShulkerPeekGoal::canContinueToUse()
{
	return shulker->getTarget() == nullptr && peekTime > 0;
}

void Shulker::ShulkerPeekGoal::start()
{
	peekTime = 20 * (1 + shulker->getRandom()->nextInt(3));
	shulker->setRawPeekAmount(30);
}

void Shulker::ShulkerPeekGoal::stop()
{
	if (shulker->getTarget() == nullptr) shulker->setRawPeekAmount(0);
}

void Shulker::ShulkerPeekGoal::tick()
{
	--peekTime;
}

Shulker::ShulkerNearestAttackGoal::ShulkerNearestAttackGoal(Shulker *shulker) : TargetGoal(shulker, true), shulker(shulker)
{
	setRequiredControlFlags(TargetFlag);
}

bool Shulker::ShulkerNearestAttackGoal::canUse()
{
	if (shulker->level->difficulty == Difficulty::PEACEFUL) return false;
	if (shulker->getRandom()->nextInt(10) != 0) return false;

	double within = getFollowDistance();
	double wx = within, wy = 4.0, wz = within;
	int face = shulker->getAttachFace();
	if (face == Facing::WEST || face == Facing::EAST) { wx = 4.0; wy = within; }
	else if (face == Facing::NORTH || face == Facing::SOUTH) { wz = 4.0; wy = within; }

	vector<shared_ptr<Entity> > *players = shulker->level->getEntitiesOfClass(typeid(Player), shulker->bb->grow(wx, wy, wz));
	shared_ptr<LivingEntity> best = nullptr;
	double bestDist = 0;
	if (players != nullptr)
	{
		for (auto &it : *players)
		{
			shared_ptr<LivingEntity> candidate = dynamic_pointer_cast<LivingEntity>(it);
			if (candidate == nullptr || !canAttack(candidate, false)) continue;
			double d = shulker->distanceToSqr(candidate);
			if (best == nullptr || d < bestDist)
			{
				best = candidate;
				bestDist = d;
			}
		}
		delete players;
	}
	target = best;
	return best != nullptr;
}

void Shulker::ShulkerNearestAttackGoal::start()
{
	shulker->setTarget(target.lock());
	TargetGoal::start();
}


Shulker::Shulker(Level *level) : Monster(level)
{
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	oldPeekAmount = 0.0f;
	peekAmount = 0.0f;
	oldAttachX = oldAttachY = oldAttachZ = 0;
	hasOldAttach = false;
	clientSideTeleportInterpolation = 0;

	setSize(1.0f, 1.0f);
	fireImmune = true;
	xpReward = XP_REWARD_MEDIUM;
	yRot = yRotO = 180.0f;
	yHeadRot = yHeadRotO = 180.0f;
	yBodyRot = yBodyRotO = 180.0f;
	xRot = xRotO = 0.0f;

	goalSelector.addGoal(1, new LookAtPlayerGoal(this, typeid(Player), 8.0f));
	goalSelector.addGoal(4, new ShulkerAttackGoal(this));
	goalSelector.addGoal(7, new ShulkerPeekGoal(this));
	goalSelector.addGoal(8, new RandomLookAroundGoal(this));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(2, new ShulkerNearestAttackGoal(this));
}

void Shulker::defineSynchedData()
{
	Monster::defineSynchedData();
	entityData->define(DATA_ATTACH_FACE, static_cast<byte>(Facing::DOWN));
	entityData->define(DATA_ATTACH_X, NO_ATTACH_POSITION);
	entityData->define(DATA_ATTACH_Y, NO_ATTACH_POSITION);
	entityData->define(DATA_ATTACH_Z, NO_ATTACH_POSITION);
	entityData->define(DATA_PEEK, static_cast<byte>(0));
	entityData->define(DATA_COLOR, static_cast<byte>(10));
}

void Shulker::registerAttributes()
{
	Monster::registerAttributes();
	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(30);
}

bool Shulker::useNewAi()
{
	return true;
}

int Shulker::getAmbientSound()
{
	return eSoundType_ENTITY_SHULKER_AMBIENT;
}

int Shulker::getHurtSound()
{
	return isClosed() ? eSoundType_ENTITY_SHULKER_HURT_CLOSED : eSoundType_ENTITY_SHULKER_HURT;
}

int Shulker::getDeathSound()
{
	return eSoundType_ENTITY_SHULKER_DEATH;
}

bool Shulker::makeStepSound()
{
	return false;
}

bool Shulker::removeWhenFarAway()
{
	return false;
}

MobGroupData *Shulker::finalizeMobSpawn(MobGroupData *groupData, int extraData)
{
	yRot = yRotO = 180.0f;
	yHeadRot = yHeadRotO = 180.0f;
	yBodyRot = yBodyRotO = 180.0f;
	return Monster::finalizeMobSpawn(groupData, extraData);
}

bool Shulker::isSolidAt(int x, int y, int z)
{
	return level->isSolidBlockingTileInLoadedChunk(x, y, z, false);
}

int Shulker::getAttachFace()
{
	int face = entityData->getByte(DATA_ATTACH_FACE);
	return (face < 0 || face > 5) ? Facing::DOWN : face;
}

void Shulker::setAttachFace(int face)
{
	entityData->set(DATA_ATTACH_FACE, static_cast<byte>(face));
}

bool Shulker::hasAttachPosition()
{
	return entityData->getInteger(DATA_ATTACH_Y) != NO_ATTACH_POSITION;
}

int Shulker::getAttachX() { return entityData->getInteger(DATA_ATTACH_X); }
int Shulker::getAttachY() { return entityData->getInteger(DATA_ATTACH_Y); }
int Shulker::getAttachZ() { return entityData->getInteger(DATA_ATTACH_Z); }

void Shulker::setAttachPosition(int ax, int ay, int az)
{
	entityData->set(DATA_ATTACH_X, ax);
	entityData->set(DATA_ATTACH_Y, ay);
	entityData->set(DATA_ATTACH_Z, az);
}

int Shulker::getRawPeekAmount()
{
	return entityData->getByte(DATA_PEEK);
}

void Shulker::setRawPeekAmount(int amount)
{
	if (!level->isClientSide)
	{
		int old = getRawPeekAmount();
		if (amount == 0 && old != 0) playSound(eSoundType_ENTITY_SHULKER_CLOSE, 1.0f, 1.0f);
		else if (amount != 0 && old == 0) playSound(eSoundType_ENTITY_SHULKER_OPEN, 1.0f, 1.0f);
	}
	entityData->set(DATA_PEEK, static_cast<byte>(amount));
}

bool Shulker::isClosed()
{
	return getRawPeekAmount() == 0;
}

float Shulker::getClientPeekAmount(float a)
{
	return oldPeekAmount + (peekAmount - oldPeekAmount) * a;
}

int Shulker::getColor()
{
	return entityData->getByte(DATA_COLOR);
}

bool Shulker::teleportSomewhere()
{
	if (!isAlive()) return true;

	int ox = Mth::floor(x);
	int oy = Mth::floor(y);
	int oz = Mth::floor(z);
	for (int attempt = 0; attempt < 5; ++attempt)
	{
		int cx = ox + 8 - random->nextInt(17);
		int cy = oy + 8 - random->nextInt(17);
		int cz = oz + 8 - random->nextInt(17);
		if (cy <= 0 || !level->isEmptyTile(cx, cy, cz)) continue;
		if (!level->getCubes(shared_from_this(), AABB::newTemp(cx, cy, cz, cx + 1.0, cy + 1.0, cz + 1.0))->empty()) continue;

		for (int face = 0; face < 6; ++face)
		{
			if (isSolidAt(cx + Facing::STEP_X[face], cy + Facing::STEP_Y[face], cz + Facing::STEP_Z[face]))
			{
				setAttachFace(face);
				setAttachPosition(cx, cy, cz);
				setRawPeekAmount(0);
				setTarget(nullptr);
				playSound(eSoundType_MOB_ENDERMEN_PORTAL, 1.0f, 1.0f);
				return true;
			}
		}
	}
	return false;
}

void Shulker::snapToAttachPosition()
{
	x = xo = xOld = getAttachX() + 0.5;
	y = yo = yOld = getAttachY();
	z = zo = zOld = getAttachZ() + 0.5;
}

void Shulker::tick()
{
	PathfinderMob::tick();

	if (riding == nullptr)
	{
		yRot = yRotO = 180.0f;
		yBodyRot = yBodyRotO = 180.0f;
	}

	if (!hasAttachPosition() && !level->isClientSide)
	{
		setAttachPosition(Mth::floor(x), Mth::floor(y), Mth::floor(z));
	}

	if (riding != nullptr)
	{
		yBodyRot = yRot = yBodyRotO = riding->yRot;
		clientSideTeleportInterpolation = 0;
	}
	else if (!level->isClientSide && hasAttachPosition())
	{
		int ax = getAttachX();
		int ay = getAttachY();
		int az = getAttachZ();

		int inside = level->getTile(ax, ay, az);
		if (inside != 0)
		{
			if (inside == Tile::piston_extension_Id || inside == Tile::piston_head_Id)
			{
				int facing = level->getData(ax, ay, az) & 7;
				if (facing < 6 && level->isEmptyTile(ax + Facing::STEP_X[facing], ay + Facing::STEP_Y[facing], az + Facing::STEP_Z[facing]))
				{
					setAttachPosition(ax + Facing::STEP_X[facing], ay + Facing::STEP_Y[facing], az + Facing::STEP_Z[facing]);
				}
				else
				{
					teleportSomewhere();
				}
			}
			else
			{
				teleportSomewhere();
			}
			ax = getAttachX(); ay = getAttachY(); az = getAttachZ();
		}

		int face = getAttachFace();
		if (!isSolidAt(ax + Facing::STEP_X[face], ay + Facing::STEP_Y[face], az + Facing::STEP_Z[face]))
		{
			bool attached = false;
			for (int f = 0; f < 6; ++f)
			{
				if (isSolidAt(ax + Facing::STEP_X[f], ay + Facing::STEP_Y[f], az + Facing::STEP_Z[f]))
				{
					setAttachFace(f);
					attached = true;
					break;
				}
			}
			if (!attached) teleportSomewhere();
		}

		face = getAttachFace();
		int opposite = Facing::OPPOSITE_FACING[face];
		if (isSolidAt(getAttachX() + Facing::STEP_X[opposite], getAttachY() + Facing::STEP_Y[opposite], getAttachZ() + Facing::STEP_Z[opposite]))
		{
			teleportSomewhere();
		}
	}

	float targetPeek = getRawPeekAmount() * 0.01f;
	oldPeekAmount = peekAmount;
	if (peekAmount > targetPeek)
	{
		peekAmount = max(peekAmount - 0.05f, targetPeek);
	}
	else if (peekAmount < targetPeek)
	{
		peekAmount = min(peekAmount + 0.05f, targetPeek);
	}

	if (!hasAttachPosition() || riding != nullptr) return;

	if (level->isClientSide)
	{
		int ax = getAttachX(), ay = getAttachY(), az = getAttachZ();
		if (!hasOldAttach)
		{
			oldAttachX = ax; oldAttachY = ay; oldAttachZ = az;
			hasOldAttach = true;
		}
		else if (ax != oldAttachX || ay != oldAttachY || az != oldAttachZ)
		{
			if (clientSideTeleportInterpolation == 0) clientSideTeleportInterpolation = 6;
			if (--clientSideTeleportInterpolation <= 0)
			{
				oldAttachX = ax; oldAttachY = ay; oldAttachZ = az;
				clientSideTeleportInterpolation = 0;
			}
		}
	}

	snapToAttachPosition();

	double peekOffset = 0.5 - Mth::sin((peekAmount + 0.5f) * PI) * 0.5;
	double oldPeekOffset = 0.5 - Mth::sin((oldPeekAmount + 0.5f) * PI) * 0.5;
	double pushAmount = peekOffset - oldPeekOffset;
	double px = 0.0, py = 0.0, pz = 0.0;
	switch (getAttachFace())
	{
	case Facing::DOWN:
		bb->set(x - 0.5, y, z - 0.5, x + 0.5, y + 1.0 + peekOffset, z + 0.5);
		py = pushAmount;
		break;
	case Facing::UP:
		bb->set(x - 0.5, y - peekOffset, z - 0.5, x + 0.5, y + 1.0, z + 0.5);
		py = -pushAmount;
		break;
	case Facing::NORTH:
		bb->set(x - 0.5, y, z - 0.5, x + 0.5, y + 1.0, z + 0.5 + peekOffset);
		pz = pushAmount;
		break;
	case Facing::SOUTH:
		bb->set(x - 0.5, y, z - 0.5 - peekOffset, x + 0.5, y + 1.0, z + 0.5);
		pz = -pushAmount;
		break;
	case Facing::WEST:
		bb->set(x - 0.5, y, z - 0.5, x + 0.5 + peekOffset, y + 1.0, z + 0.5);
		px = pushAmount;
		break;
	case Facing::EAST:
		bb->set(x - 0.5 - peekOffset, y, z - 0.5, x + 0.5, y + 1.0, z + 0.5);
		px = -pushAmount;
		break;
	}

	if (pushAmount > 0.0)
	{
		vector<shared_ptr<Entity> > nearby = *level->getEntities(shared_from_this(), bb);
		for (auto &e : nearby)
		{
			if (!e->instanceof(eTYPE_SHULKER) && !e->noPhysics)
			{
				e->move(px, py, pz);
			}
		}
	}
}

void Shulker::aiStep()
{
	Monster::aiStep();
	xd = yd = zd = 0.0;
	yBodyRotO = yBodyRot = yRot = 180.0f;
}

bool Shulker::hurt(DamageSource *source, float dmg)
{
	if (isClosed())
	{
		shared_ptr<Entity> direct = source->getDirectEntity();
		if (direct != nullptr && direct->instanceof(eTYPE_PROJECTILE)) return false;
		if (!source->isBypassArmor()) dmg *= 0.2f;
	}

	if (!Monster::hurt(source, dmg)) return false;

	if (getHealth() < getMaxHealth() * 0.5f && random->nextInt(4) == 0)
	{
		teleportSomewhere();
	}
	return true;
}

AABB *Shulker::getCollideBox()
{
	return isAlive() ? bb : nullptr;
}

bool Shulker::isPushable()
{
	return false;
}

void Shulker::push(shared_ptr<Entity> e)
{
}

float Shulker::getHeadHeight()
{
	return 0.5f;
}

int Shulker::getMaxHeadXRot()
{
	return 180;
}

void Shulker::addAdditonalSaveData(CompoundTag *tag)
{
	Monster::addAdditonalSaveData(tag);
	tag->putByte(L"AttachFace", static_cast<byte>(getAttachFace()));
	tag->putByte(L"Peek", static_cast<byte>(getRawPeekAmount()));
	tag->putByte(L"Color", static_cast<byte>(getColor()));
	if (hasAttachPosition())
	{
		tag->putInt(L"APX", getAttachX());
		tag->putInt(L"APY", getAttachY());
		tag->putInt(L"APZ", getAttachZ());
	}
}

void Shulker::readAdditionalSaveData(CompoundTag *tag)
{
	Monster::readAdditionalSaveData(tag);
	if (tag->contains(L"AttachFace")) setAttachFace(tag->getByte(L"AttachFace"));
	if (tag->contains(L"Peek")) setRawPeekAmount(tag->getByte(L"Peek"));
	if (tag->contains(L"Color")) entityData->set(DATA_COLOR, tag->getByte(L"Color"));
	if (tag->contains(L"APX"))
	{
		setAttachPosition(tag->getInt(L"APX"), tag->getInt(L"APY"), tag->getInt(L"APZ"));
	}
}
