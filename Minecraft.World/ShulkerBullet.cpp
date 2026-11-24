#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.damagesource.h"
#include "com.mojang.nbt.h"
#include "MobEffectInstance.h"
#include "SharedConstants.h"
#include "Facing.h"
#include "Difficulty.h"
#include "ShulkerBullet.h"

void ShulkerBullet::_init()
{
	this->defineSynchedData();

	moveDirection = -1;
	steps = 0;
	powerX = powerY = powerZ = 0.0;
	ownerUUID.clear();
	targetUUID.clear();
	ownerX = ownerY = ownerZ = 0;
	targetX = targetY = targetZ = 0;
	setSize(0.3125f, 0.3125f);
	noPhysics = true;
}

ShulkerBullet::ShulkerBullet(Level *level) : Entity(level)
{
	_init();
}

ShulkerBullet::ShulkerBullet(Level *level, double x, double y, double z, double xd, double yd, double zd) : Entity(level)
{
	_init();
	moveTo(x, y, z, yRot, xRot);
	this->xd = xd;
	this->yd = yd;
	this->zd = zd;
}

ShulkerBullet::ShulkerBullet(Level *level, shared_ptr<LivingEntity> owner, shared_ptr<Entity> target, int axis) : Entity(level)
{
	_init();
	this->owner = owner;
	this->target = target;
	moveTo(Mth::floor(owner->x) + 0.5, Mth::floor(owner->y) + 0.5, Mth::floor(owner->z) + 0.5, yRot, xRot);
	moveDirection = Facing::UP;
	selectNextMoveDirection(axis);
}

void ShulkerBullet::defineSynchedData()
{
}

void ShulkerBullet::resolveReferences()
{
	if (!owner.lock())
	{
		if (!ownerUUID.empty())
		{
			owner = level->getPlayerByUUID(ownerUUID);
		}
	}

	if (!target.lock() && !targetUUID.empty())
	{
		vector<shared_ptr<Entity> > *entities = level->getEntities(nullptr,
			AABB::newTemp(targetX - 8, targetY - 8, targetZ - 8,
					 targetX + 9, targetY + 9, targetZ + 9));
		if (entities != nullptr)
		{
			for (shared_ptr<Entity> entity : *entities)
			{
				if (entity->getUUID() == targetUUID)
				{
					target = entity;
					break;
				}
			}
		}
	}
}

void ShulkerBullet::selectNextMoveDirection(int axis)
{
	shared_ptr<Entity> t = target.lock();

	int destX, destY, destZ;
	double heightOffset = 0.5;
	if (t == nullptr)
	{
		destX = Mth::floor(x);
		destY = Mth::floor(y) - 1;
		destZ = Mth::floor(z);
	}
	else
	{
		heightOffset = t->bbHeight * 0.5;
		destX = Mth::floor(t->x);
		destY = Mth::floor(t->y + heightOffset);
		destZ = Mth::floor(t->z);
	}

	double targetX = destX + 0.5;
	double targetY = destY + heightOffset;
	double targetZ = destZ + 0.5;
	int direction = -1;

	double cdx = destX + 0.5 - x, cdy = destY + 0.5 - y, cdz = destZ + 0.5 - z;
	if (cdx * cdx + cdy * cdy + cdz * cdz >= 4.0)
	{
		int cx = Mth::floor(x), cy = Mth::floor(y), cz = Mth::floor(z);
		vector<int> choices;
		if (axis != 2)
		{
			if (cx < destX && level->isEmptyTile(cx + 1, cy, cz)) choices.push_back(Facing::EAST);
			else if (cx > destX && level->isEmptyTile(cx - 1, cy, cz)) choices.push_back(Facing::WEST);
		}
		if (axis != 0)
		{
			if (cy < destY && level->isEmptyTile(cx, cy + 1, cz)) choices.push_back(Facing::UP);
			else if (cy > destY && level->isEmptyTile(cx, cy - 1, cz)) choices.push_back(Facing::DOWN);
		}
		if (axis != 1)
		{
			if (cz < destZ && level->isEmptyTile(cx, cy, cz + 1)) choices.push_back(Facing::SOUTH);
			else if (cz > destZ && level->isEmptyTile(cx, cy, cz - 1)) choices.push_back(Facing::NORTH);
		}

		direction = random->nextInt(6);
		if (choices.empty())
		{
			for (int attempts = 5; attempts > 0 && !level->isEmptyTile(cx + Facing::STEP_X[direction], cy + Facing::STEP_Y[direction], cz + Facing::STEP_Z[direction]); --attempts)
			{
				direction = random->nextInt(6);
			}
		}
		else
		{
			direction = choices[random->nextInt(static_cast<int>(choices.size()))];
		}

		targetX = x + Facing::STEP_X[direction];
		targetY = y + Facing::STEP_Y[direction];
		targetZ = z + Facing::STEP_Z[direction];
	}

	moveDirection = direction;
	double dx = targetX - x;
	double dy = targetY - y;
	double dz = targetZ - z;
	double length = sqrt(dx * dx + dy * dy + dz * dz);
	if (length == 0.0)
	{
		powerX = powerY = powerZ = 0.0;
	}
	else
	{
		powerX = dx / length * 0.15;
		powerY = dy / length * 0.15;
		powerZ = dz / length * 0.15;
	}
	hasImpulse = true;
	steps = 10 + random->nextInt(5) * 10;
}

void ShulkerBullet::onHit(HitResult *res)
{
	if (res->entity == nullptr) playSound(eSoundType_ENTITY_SHULKER_BULLET_HIT, 1.0f, 1.0f);
	if (res->entity != nullptr)
	{
		shared_ptr<LivingEntity> shooter = owner.lock();
		DamageSource *source = DamageSource::thrown(shared_from_this(), shooter);
		if (res->entity->hurt(source, 4.0f) && res->entity->instanceof(eTYPE_LIVINGENTITY))
		{
			dynamic_pointer_cast<LivingEntity>(res->entity)->addEffect(new MobEffectInstance(MobEffect::levitation->id, LEVITATION_DURATION, 0));
		}
		delete source;
	}
	remove();
}

void ShulkerBullet::tick()
{
	if (!level->isClientSide && level->difficulty == Difficulty::PEACEFUL)
	{
		remove();
		return;
	}

	Entity::tick();

	if (!level->isClientSide)
	{
		resolveReferences();
		shared_ptr<Entity> t = target.lock();
		bool tracking = t != nullptr && t->isAlive();
		if (tracking && t->instanceof(eTYPE_PLAYER) && dynamic_pointer_cast<Player>(t)->isSpectator()) tracking = false;

		if (tracking)
		{
			powerX = Mth::clamp(static_cast<float>(powerX * 1.025), -1.0f, 1.0f);
			powerY = Mth::clamp(static_cast<float>(powerY * 1.025), -1.0f, 1.0f);
			powerZ = Mth::clamp(static_cast<float>(powerZ * 1.025), -1.0f, 1.0f);
			xd += (powerX - xd) * 0.2;
			yd += (powerY - yd) * 0.2;
			zd += (powerZ - zd) * 0.2;
		}
		else
		{
			yd -= 0.04;
		}

		Vec3 *from = Vec3::newTemp(x, y, z);
		Vec3 *to = Vec3::newTemp(x + xd, y + yd, z + zd);
		HitResult *res = level->clip(from, to);
		from = Vec3::newTemp(x, y, z);
		to = Vec3::newTemp(x + xd, y + yd, z + zd);
		if (res != nullptr) to = Vec3::newTemp(res->pos->x, res->pos->y, res->pos->z);

		shared_ptr<LivingEntity> shooter = owner.lock();
		shared_ptr<Entity> hitEntity = nullptr;
		double nearest = 0;
		vector<shared_ptr<Entity> > *objects = level->getEntities(shared_from_this(), bb->expand(xd, yd, zd)->grow(1, 1, 1));
		for (size_t i = 0; objects != nullptr && i < objects->size(); i++)
		{
			shared_ptr<Entity> e = objects->at(i);
			if (!e->isPickable() || e == shooter || e->noPhysics) continue;
			AABB *hitBox = e->bb->grow(0.3, 0.3, 0.3);
			HitResult *p = hitBox->clip(from, to);
			if (p != nullptr)
			{
				double dd = from->distanceTo(p->pos);
				delete p;
				if (dd < nearest || nearest == 0)
				{
					hitEntity = e;
					nearest = dd;
				}
			}
		}
		if (hitEntity != nullptr)
		{
			if (res != nullptr) delete res;
			res = new HitResult(hitEntity);
		}

		if (res != nullptr)
		{
			onHit(res);
			delete res;
		}
	}

	setPos(x + xd, y + yd, z + zd);

	float sd = static_cast<float>(sqrt(xd * xd + zd * zd));
	yRot = static_cast<float>(atan2(xd, zd) * 180 / PI);
	xRot = static_cast<float>(atan2(yd, static_cast<double>(sd)) * 180 / PI);

	if (level->isClientSide)
	{
		level->addParticle(eParticleType_endRod, x - xd, y - yd + 0.15, z - zd, 0, 0, 0);
	}
	else
	{
		shared_ptr<Entity> t = target.lock();
		if (t != nullptr && !t->removed)
		{
			if (steps > 0 && --steps == 0)
			{
				selectNextMoveDirection(moveDirection < 0 ? -1 : moveDirection / 2);
			}

			if (moveDirection >= 0)
			{
				int cx = Mth::floor(x), cy = Mth::floor(y), cz = Mth::floor(z);
				int axis = moveDirection / 2;
				if (level->isSolidBlockingTileInLoadedChunk(cx + Facing::STEP_X[moveDirection], cy + Facing::STEP_Y[moveDirection], cz + Facing::STEP_Z[moveDirection], false))
				{
					selectNextMoveDirection(axis);
				}
				else if ((axis == 2 && cx == Mth::floor(t->x)) || (axis == 1 && cz == Mth::floor(t->z)) || (axis == 0 && cy == Mth::floor(t->y)))
				{
					selectNextMoveDirection(axis);
				}
			}
		}
	}
}

bool ShulkerBullet::hurt(DamageSource *source, float damage)
{
	if (!level->isClientSide)
	{
		playSound(eSoundType_ENTITY_SHULKER_BULLET_HIT, 1.0f, 1.0f);
		remove();
	}
	return true;
}

bool ShulkerBullet::isPickable()
{
	return true;
}

bool ShulkerBullet::isOnFire()
{
	return false;
}

float ShulkerBullet::getBrightness(float a)
{
	return 1.0f;
}

int ShulkerBullet::getLightColor(float a)
{
	return SharedConstants::FULLBRIGHT_LIGHTVALUE;
}

bool ShulkerBullet::shouldRenderAtSqrDistance(double distance)
{
	return distance < 16384.0;
}

void ShulkerBullet::addAdditonalSaveData(CompoundTag *tag)
{
	shared_ptr<LivingEntity> currentOwner = owner.lock();
	shared_ptr<Entity> currentTarget = target.lock();
	if (currentOwner != nullptr)
	{
		ownerUUID = currentOwner->getUUID();
		ownerX = Mth::floor(currentOwner->x);
		ownerY = Mth::floor(currentOwner->y);
		ownerZ = Mth::floor(currentOwner->z);
	}
	if (currentTarget != nullptr)
	{
		targetUUID = currentTarget->getUUID();
		targetX = Mth::floor(currentTarget->x);
		targetY = Mth::floor(currentTarget->y);
		targetZ = Mth::floor(currentTarget->z);
	}

	if (!ownerUUID.empty())
	{
		CompoundTag *ownerTag = new CompoundTag();
		ownerTag->putString(L"UUID", ownerUUID);
		ownerTag->putInt(L"X", ownerX);
		ownerTag->putInt(L"Y", ownerY);
		ownerTag->putInt(L"Z", ownerZ);
		tag->put(L"Owner", ownerTag);
	}
	if (!targetUUID.empty())
	{
		CompoundTag *targetTag = new CompoundTag();
		targetTag->putString(L"UUID", targetUUID);
		targetTag->putInt(L"X", targetX);
		targetTag->putInt(L"Y", targetY);
		targetTag->putInt(L"Z", targetZ);
		tag->put(L"Target", targetTag);
	}

	tag->putInt(L"Dir", moveDirection);
	tag->putInt(L"Steps", steps);
	tag->putDouble(L"TXD", powerX);
	tag->putDouble(L"TYD", powerY);
	tag->putDouble(L"TZD", powerZ);
}

void ShulkerBullet::readAdditionalSaveData(CompoundTag *tag)
{
	if (tag->contains(L"Owner"))
	{
		CompoundTag *ownerTag = tag->getCompound(L"Owner");
		ownerUUID = ownerTag->getString(L"UUID");
		ownerX = ownerTag->getInt(L"X");
		ownerY = ownerTag->getInt(L"Y");
		ownerZ = ownerTag->getInt(L"Z");
	}
	if (tag->contains(L"Target"))
	{
		CompoundTag *targetTag = tag->getCompound(L"Target");
		targetUUID = targetTag->getString(L"UUID");
		targetX = targetTag->getInt(L"X");
		targetY = targetTag->getInt(L"Y");
		targetZ = targetTag->getInt(L"Z");
	}

	moveDirection = tag->contains(L"Dir") ? tag->getInt(L"Dir") : -1;
	steps = tag->getInt(L"Steps");
	powerX = tag->getDouble(L"TXD");
	powerY = tag->getDouble(L"TYD");
	powerZ = tag->getDouble(L"TZD");
}
