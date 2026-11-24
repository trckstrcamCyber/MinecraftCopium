#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.item.alchemy.h"
#include "com.mojang.nbt.h"
#include "MobEffectInstance.h"
#include "Material.h"
#include "AreaEffectCloud.h"

void AreaEffectCloud::_init()
{
	this->defineSynchedData();

	potionValue = 0;
	fixedColor = false;
	durationOnUse = 0;
	radiusOnUse = 0.0f;
	radiusPerTick = 0.0f;
	duration = 600;
	waitTime = 20;
	reapplicationDelay = 20;
	noPhysics = true;
	fireImmune = true;
	setRadius(3.0f);
}

AreaEffectCloud::AreaEffectCloud(Level *level) : Entity(level)
{
	_init();
}

AreaEffectCloud::AreaEffectCloud(Level *level, double x, double y, double z) : Entity(level)
{
	_init();
	setPos(x, y, z);
}

AreaEffectCloud::~AreaEffectCloud()
{
	for (auto &effect : extraEffects)
	{
		delete effect;
	}
}

void AreaEffectCloud::defineSynchedData()
{
	entityData->define(DATA_RADIUS, 0.5f);
	entityData->define(DATA_COLOR, 0);
	entityData->define(DATA_WAITING, static_cast<byte>(0));
	entityData->define(DATA_PARTICLE, static_cast<int>(eParticleType_mobSpell));
	entityData->define(DATA_PARTICLE_RATE, 0);
}

void AreaEffectCloud::setRadius(float radius)
{
	double ox = x;
	double oy = y;
	double oz = z;
	setSize(radius * 2.0f, 0.5f);
	setPos(ox, oy, oz);
	if (level != nullptr && !level->isClientSide)
	{
		entityData->set(DATA_RADIUS, radius);
	}
}

float AreaEffectCloud::getRadius()
{
	return entityData->getFloat(DATA_RADIUS);
}

bool AreaEffectCloud::isWaiting()
{
	return entityData->getByte(DATA_WAITING) != 0;
}

void AreaEffectCloud::setWaiting(bool waiting)
{
	entityData->set(DATA_WAITING, static_cast<byte>(waiting ? 1 : 0));
}

int AreaEffectCloud::getColor()
{
	return entityData->getInteger(DATA_COLOR);
}

void AreaEffectCloud::setFixedColor(int color)
{
	fixedColor = true;
	entityData->set(DATA_COLOR, color);
}

ePARTICLE_TYPE AreaEffectCloud::getParticle()
{
	return static_cast<ePARTICLE_TYPE>(entityData->getInteger(DATA_PARTICLE));
}

void AreaEffectCloud::setParticle(ePARTICLE_TYPE particle)
{
	entityData->set(DATA_PARTICLE, static_cast<int>(particle));
}

void AreaEffectCloud::setParticleSpawnRateOnTick(int rate)
{
	entityData->set(DATA_PARTICLE_RATE, rate);
}

void AreaEffectCloud::setPotionValue(int potionValue)
{
	this->potionValue = potionValue;
	if (!fixedColor) updateColor();
}

void AreaEffectCloud::addEffect(MobEffectInstance *effect)
{
	extraEffects.push_back(effect);
	if (!fixedColor) updateColor();
}

void AreaEffectCloud::updateColor()
{
	vector<MobEffectInstance *> all;
	vector<MobEffectInstance *> *potionEffects = PotionBrewing::getEffects(potionValue, false);
	if (potionEffects != nullptr)
	{
		all.insert(all.end(), potionEffects->begin(), potionEffects->end());
	}
	all.insert(all.end(), extraEffects.begin(), extraEffects.end());

	entityData->set(DATA_COLOR, PotionBrewing::getColorValue(&all));

	if (potionEffects != nullptr)
	{
		for (auto &effect : *potionEffects) delete effect;
		delete potionEffects;
	}
}

void AreaEffectCloud::setOwner(shared_ptr<LivingEntity> owner)
{
	this->owner = owner;
}

shared_ptr<LivingEntity> AreaEffectCloud::getOwner()
{
	return owner.lock();
}

void AreaEffectCloud::tick()
{
	Entity::tick();

	bool waiting = isWaiting();
	float radius = getRadius();

	if (level->isClientSide)
	{
		if (fabs(bbWidth - radius * 2.0f) > 0.001f)
		{
			setSize(radius * 2.0f, 0.5f);
			setPos(x, y, z);
		}
		generateParticles();
		return;
	}

	if (tickCount >= waitTime + duration)
	{
		remove();
		return;
	}

	bool shouldWait = tickCount < waitTime;
	if (waiting != shouldWait) setWaiting(shouldWait);
	if (shouldWait) return;

	if (radiusPerTick != 0.0f)
	{
		radius += radiusPerTick;
		if (radius < 0.5f)
		{
			remove();
			return;
		}
		setRadius(radius);
	}

	applyMobEffects();
}

void AreaEffectCloud::applyMobEffects()
{
	if (tickCount % 5 != 0) return;

	for (auto it = reapplicationDelayMap.begin(); it != reapplicationDelayMap.end();)
	{
		if (tickCount >= it->second) it = reapplicationDelayMap.erase(it);
		else ++it;
	}

	vector<MobEffectInstance *> effects;
	vector<MobEffectInstance *> *potionEffects = PotionBrewing::getEffects(potionValue, false);
	if (potionEffects != nullptr)
	{
		for (auto &effect : *potionEffects)
		{
			effects.push_back(new MobEffectInstance(effect->getId(), effect->getDuration() / 4, effect->getAmplifier()));
			delete effect;
		}
		delete potionEffects;
	}
	for (auto &effect : extraEffects)
	{
		effects.push_back(new MobEffectInstance(effect));
	}

	if (effects.empty())
	{
		reapplicationDelayMap.clear();
		return;
	}

	float radius = getRadius();
	vector<shared_ptr<Entity> > *nearby = level->getEntitiesOfClass(typeid(LivingEntity), bb);
	if (nearby != nullptr)
	{
		for (auto &it : *nearby)
		{
			shared_ptr<LivingEntity> target = dynamic_pointer_cast<LivingEntity>(it);
			if (target == nullptr || reapplicationDelayMap.find(target->entityId) != reapplicationDelayMap.end()) continue;

			double dx = target->x - x;
			double dz = target->z - z;
			if (dx * dx + dz * dz > radius * radius) continue;

			reapplicationDelayMap[target->entityId] = tickCount + reapplicationDelay;

			for (auto &effect : effects)
			{
				MobEffect *mobEffect = MobEffect::effects[effect->getId()];
				if (mobEffect == nullptr) continue;
				if (mobEffect->isInstantenous())
				{
					mobEffect->applyInstantenousEffect(getOwner(), target, effect->getAmplifier(), 0.5);
				}
				else
				{
					target->addEffect(new MobEffectInstance(effect));
				}
			}

			if (radiusOnUse != 0.0f)
			{
				radius += radiusOnUse;
				if (radius < 0.5f)
				{
					remove();
					break;
				}
				setRadius(radius);
			}

			if (durationOnUse != 0)
			{
				duration += durationOnUse;
				if (duration < 1)
				{
					remove();
					break;
				}
			}
		}
		delete nearby;
	}

	for (auto &effect : effects) delete effect;
}

void AreaEffectCloud::generateParticles()
{
	float radius = getRadius();
	ePARTICLE_TYPE particle = getParticle();
	bool spell = particle == eParticleType_mobSpell;
	int color = getColor();

	int count;
	if (isWaiting())
	{
		if (!random->nextBoolean()) return;
		count = 2;
		radius = 0.2f;
	}
	else
	{
		int rate = entityData->getInteger(DATA_PARTICLE_RATE);
		count = rate >= 1 ? rate : static_cast<int>(PI * radius * radius);
	}

	for (int i = 0; i < count; ++i)
	{
		float angle = random->nextFloat() * PI * 2.0f;
		float dist = sqrt(random->nextFloat()) * radius;
		double px = x + cos(angle) * dist;
		double pz = z + sin(angle) * dist;

		if (spell)
		{
			int c = (isWaiting() && random->nextBoolean()) ? 0xffffff : color;
			level->addParticle(eParticleType_mobSpell, px, y, pz,
				((c >> 16) & 0xff) / 255.0f, ((c >> 8) & 0xff) / 255.0f, (c & 0xff) / 255.0f);
		}
		else if (isWaiting())
		{
			level->addParticle(particle, px, y, pz, 0, 0, 0);
		}
		else
		{
			level->addParticle(particle, px, y, pz, (0.5 - random->nextDouble()) * 0.15, 0.01, (0.5 - random->nextDouble()) * 0.15);
		}
	}
}

int AreaEffectCloud::getPistonPushReaction()
{
	return Material::PUSH_BLOCK;
}

void AreaEffectCloud::addAdditonalSaveData(CompoundTag *tag)
{
	tag->putInt(L"Age", tickCount);
	tag->putInt(L"Duration", duration);
	tag->putInt(L"WaitTime", waitTime);
	tag->putInt(L"ReapplicationDelay", reapplicationDelay);
	tag->putInt(L"DurationOnUse", durationOnUse);
	tag->putFloat(L"RadiusOnUse", radiusOnUse);
	tag->putFloat(L"RadiusPerTick", radiusPerTick);
	tag->putFloat(L"Radius", getRadius());
	tag->putInt(L"Particle", static_cast<int>(getParticle()));
	tag->putInt(L"PotionValue", potionValue);
	if (fixedColor) tag->putInt(L"Color", getColor());

	if (!extraEffects.empty())
	{
		ListTag<CompoundTag> *list = new ListTag<CompoundTag>();
		for (auto &effect : extraEffects)
		{
			list->add(effect->save(new CompoundTag()));
		}
		tag->put(L"Effects", list);
	}
}

void AreaEffectCloud::readAdditionalSaveData(CompoundTag *tag)
{
	tickCount = tag->getInt(L"Age");
	duration = tag->getInt(L"Duration");
	waitTime = tag->getInt(L"WaitTime");
	reapplicationDelay = tag->getInt(L"ReapplicationDelay");
	durationOnUse = tag->getInt(L"DurationOnUse");
	radiusOnUse = tag->getFloat(L"RadiusOnUse");
	radiusPerTick = tag->getFloat(L"RadiusPerTick");
	setRadius(tag->getFloat(L"Radius"));
	if (tag->contains(L"Particle")) setParticle(static_cast<ePARTICLE_TYPE>(tag->getInt(L"Particle")));
	if (tag->contains(L"Color")) setFixedColor(tag->getInt(L"Color"));
	setPotionValue(tag->getInt(L"PotionValue"));

	if (tag->contains(L"Effects"))
	{
		ListTag<CompoundTag> *list = (ListTag<CompoundTag> *) tag->getList(L"Effects");
		for (int i = 0; i < list->size(); i++)
		{
			MobEffectInstance *effect = MobEffectInstance::load(list->get(i));
			if (effect != nullptr) addEffect(effect);
		}
	}
}
