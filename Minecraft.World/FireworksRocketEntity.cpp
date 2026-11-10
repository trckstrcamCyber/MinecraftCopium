#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.phys.h"
#include "FireworksRocketEntity.h"

// neoLegacy - set to false to stop firework stars from hurting entities when the rocket bursts
static const bool FIREWORK_STAR_DAMAGE = true;

FireworksRocketEntity::FireworksRocketEntity(Level *level) : Entity(level)
{
	defineSynchedData();

	life = 0;
	lifetime = 0;
	setSize(0.25f, 0.25f);
}

void FireworksRocketEntity::defineSynchedData()
{
	entityData->defineNULL(DATA_ID_FIREWORKS_ITEM, nullptr);
	entityData->define(DATA_ID_ATTACHED, -1);
}

bool FireworksRocketEntity::shouldRenderAtSqrDistance(double distance)
{
	return distance < 64 * 64;
}

FireworksRocketEntity::FireworksRocketEntity(Level *level, double x, double y, double z, shared_ptr<ItemInstance> sourceItem) : Entity(level)
{
	defineSynchedData();

	life = 0;

	setSize(0.25f, 0.25f);

	setPos(x, y, z);
	heightOffset = 0;

	int flightCount = 1;
	if (sourceItem != nullptr && sourceItem->hasTag())
	{
		entityData->set(DATA_ID_FIREWORKS_ITEM, sourceItem);

		CompoundTag *tag = sourceItem->getTag();
		CompoundTag *compound = tag->getCompound(FireworksItem::TAG_FIREWORKS);
		if (compound != nullptr)
		{
			flightCount += compound->getByte(FireworksItem::TAG_FLIGHT);
		}
	}
	xd = random->nextGaussian() * .001;
	zd = random->nextGaussian() * .001;
	yd = 0.05;

	lifetime = (SharedConstants::TICKS_PER_SECOND / 2) * flightCount + random->nextInt(6) + random->nextInt(7);
}

// neoLegacy - shared by the boosting constructor: keep the item and work out how long the rocket burns
void FireworksRocketEntity::initFlight(shared_ptr<ItemInstance> sourceItem)
{
	int flightCount = 1;
	if (sourceItem != nullptr && sourceItem->hasTag())
	{
		entityData->set(DATA_ID_FIREWORKS_ITEM, sourceItem);

		CompoundTag *tag = sourceItem->getTag();
		CompoundTag *compound = tag->getCompound(FireworksItem::TAG_FIREWORKS);
		if (compound != nullptr)
		{
			flightCount += compound->getByte(FireworksItem::TAG_FLIGHT);
		}
	}
	lifetime = (SharedConstants::TICKS_PER_SECOND / 2) * flightCount + random->nextInt(6) + random->nextInt(7);
}

// neoLegacy - the boosting rocket: starts on the flier and follows them (see tick)
FireworksRocketEntity::FireworksRocketEntity(Level *level, shared_ptr<ItemInstance> sourceItem, shared_ptr<Entity> attachedTo) : Entity(level)
{
	defineSynchedData();

	life = 0;
	lifetime = 0;

	setSize(0.25f, 0.25f);
	heightOffset = 0;

	xd = yd = zd = 0;
	if (attachedTo != nullptr)
	{
		setPos(attachedTo->x, attachedTo->y, attachedTo->z);
		entityData->set(DATA_ID_ATTACHED, attachedTo->entityId);
	}

	initFlight(sourceItem);
}

shared_ptr<Entity> FireworksRocketEntity::getAttachedEntity()
{
	int id = entityData->getInteger(DATA_ID_ATTACHED);
	if (id < 0) return nullptr;

	shared_ptr<Entity> e = level->getEntity(id);
	if (e != nullptr) return e;

	// the local player may not be in the id map on every client, so also look through the player list
	for (size_t i = 0; i < level->players.size(); i++)
	{
		shared_ptr<Player> p = level->players[i];
		if (p != nullptr && !p->removed && p->entityId == id) return p;
	}
	return nullptr;
}

int FireworksRocketEntity::getExplosionCount()
{
	shared_ptr<ItemInstance> item = entityData->getItemInstance(DATA_ID_FIREWORKS_ITEM);
	if (item == nullptr || !item->hasTag()) return 0;

	CompoundTag *fireTag = item->getTag()->getCompound(FireworksItem::TAG_FIREWORKS);
	if (fireTag == nullptr) return 0;

	ListTag<CompoundTag> *explosions = (ListTag<CompoundTag> *) fireTag->getList(FireworksItem::TAG_EXPLOSIONS);
	return explosions != nullptr ? explosions->size() : 0;
}

// Java: 5 + 2 per star to the boosted entity, and the same damage (fading with distance) to every living thing within 5 blocks.
void FireworksRocketEntity::dealExplosionDamage()
{
	if (!FIREWORK_STAR_DAMAGE) return;

	int stars = getExplosionCount();
	if (stars <= 0) return;

	float damage = 5.0f + static_cast<float>(stars * 2);

	shared_ptr<Entity> attached = getAttachedEntity();
	if (attached != nullptr && attached->instanceof(eTYPE_LIVINGENTITY))
	{
		attached->hurt(DamageSource::explosion(nullptr), damage);
	}

	const double R = 5.0;
	vector<shared_ptr<Entity> > *nearby = level->getEntitiesOfClass(typeid(LivingEntity), bb->grow(R, R, R));
	if (nearby != nullptr && !nearby->empty())
	{
		for (size_t i = 0; i < nearby->size(); i++)
		{
			shared_ptr<Entity> e = (*nearby)[i];
			if (e == nullptr || e == attached || e.get() == this) continue;

			double d2 = distanceToSqr(e);
			if (d2 > R * R) continue;

			float falloff = damage * static_cast<float>(sqrt((R - sqrt(d2)) / R));
			if (falloff > 0.0f) e->hurt(DamageSource::explosion(nullptr), falloff);
		}
	}
}

void FireworksRocketEntity::lerpMotion(double xd, double yd, double zd)
{
	xd = xd;
	yd = yd;
	zd = zd;
	if (xRotO == 0 && yRotO == 0)
	{
		double sd = Mth::sqrt(xd * xd + zd * zd);
		yRotO = yRot = static_cast<float>(atan2(xd, zd) * 180 / PI);
		xRotO = xRot = static_cast<float>(atan2(yd, sd) * 180 / PI);
	}
}

void FireworksRocketEntity::tick()
{
	xOld = x;
	yOld = y;
	zOld = z;
	Entity::tick();

	shared_ptr<Entity> attached = getAttachedEntity();
	if (attached != nullptr)
	{
		// neoLegacy - boosting rocket. Players move themselves on their own client, so the push is applied there.
		// Same maths as Java: pull the glider's velocity towards 1.5x their look direction, plus a little extra.
		if (level->isClientSide && attached->instanceof(eTYPE_LOCALPLAYER))
		{
			shared_ptr<Player> flier = dynamic_pointer_cast<Player>(attached);
			if (flier != nullptr && flier->isElytraFlying())
			{
				Vec3 *look = flier->getLookAngle();
				flier->xd += look->x * 0.1 + (look->x * 1.5 - flier->xd) * 0.5;
				flier->yd += look->y * 0.1 + (look->y * 1.5 - flier->yd) * 0.5;
				flier->zd += look->z * 0.1 + (look->z * 1.5 - flier->zd) * 0.5;
			}
		}

		// the rocket itself just rides along with the flier
		setPos(attached->x, attached->y, attached->z);
		xd = attached->xd;
		yd = attached->yd;
		zd = attached->zd;
	}
	else if (!level->isClientSide && entityData->getInteger(DATA_ID_ATTACHED) >= 0)
	{
		// the flier is gone (logged out / died / changed dimension): drop the rocket without a bang
		remove();
		return;
	}
	else
	{
		xd *= 1.15;
		zd *= 1.15;
		yd += .04;
		move(xd, yd, zd);
	}

	double sd = Mth::sqrt(xd * xd + zd * zd);
	yRot = static_cast<float>(atan2(xd, zd) * 180 / PI);
	xRot = static_cast<float>(atan2(yd, sd) * 180 / PI);

	while (xRot - xRotO < -180)
		xRotO -= 360;
	while (xRot - xRotO >= 180)
		xRotO += 360;

	while (yRot - yRotO < -180)
		yRotO -= 360;
	while (yRot - yRotO >= 180)
		yRotO += 360;

	xRot = xRotO + (xRot - xRotO) * 0.2f;
	yRot = yRotO + (yRot - yRotO) * 0.2f;

	if (!level->isClientSide )
	{
		if (life == 0)
		{
			level->playEntitySound(shared_from_this(), eSoundType_FIREWORKS_LAUNCH, 3, 1);
		}
	}

	life++;
	if (level->isClientSide && (life % 2) < 2)
	{
		level->addParticle(eParticleType_fireworksspark, x, y - (attached != nullptr ? .6 : .3), z, random->nextGaussian() * .05, (attached != nullptr ? 0.0 : -yd * .5), random->nextGaussian() * .05);
	}
	if (!level->isClientSide && life > lifetime)
	{
		dealExplosionDamage();
		level->broadcastEntityEvent(shared_from_this(), EntityEvent::FIREWORKS_EXPLODE);
		remove();
	}
}

void FireworksRocketEntity::handleEntityEvent(byte eventId)
{
	if (eventId == EntityEvent::FIREWORKS_EXPLODE && level->isClientSide)
	{
		shared_ptr<ItemInstance> sourceItem = entityData->getItemInstance(DATA_ID_FIREWORKS_ITEM);
		CompoundTag *tag = nullptr;
		if (sourceItem != nullptr && sourceItem->hasTag())
		{
			tag = sourceItem->getTag()->getCompound(FireworksItem::TAG_FIREWORKS);
		}
		level->createFireworks(x, y, z, xd, yd, zd, tag);
	}
	Entity::handleEntityEvent(eventId);
}

void FireworksRocketEntity::addAdditonalSaveData(CompoundTag *tag)
{
	tag->putInt(L"Life", life);
	tag->putInt(L"LifeTime", lifetime);
	shared_ptr<ItemInstance> itemInstance = entityData->getItemInstance(DATA_ID_FIREWORKS_ITEM);
	if (itemInstance != nullptr)
	{
		CompoundTag *itemTag = new CompoundTag();
		itemInstance->save(itemTag);
		tag->putCompound(L"FireworksItem", itemTag);
	}

}

void FireworksRocketEntity::readAdditionalSaveData(CompoundTag *tag)
{
	life = tag->getInt(L"Life");
	lifetime = tag->getInt(L"LifeTime");

	CompoundTag *itemTag = tag->getCompound(L"FireworksItem");
	if (itemTag != nullptr)
	{
		shared_ptr<ItemInstance> fromTag = ItemInstance::fromTag(itemTag);
		if (fromTag != nullptr)
		{
			entityData->set(DATA_ID_FIREWORKS_ITEM, fromTag);
		}
	}
}

float FireworksRocketEntity::getShadowHeightOffs()
{
	return 0;
}

float FireworksRocketEntity::getBrightness(float a)
{
	return Entity::getBrightness(a);
}

int FireworksRocketEntity::getLightColor(float a)
{
	return Entity::getLightColor(a);
}

bool FireworksRocketEntity::isAttackable()
{
	return false;
}