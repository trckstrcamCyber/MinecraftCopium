#include "stdafx.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "CopperGolem.h"
#include "CopperGolemAi.h"
#include "CopperGolemStatueBlock.h"
#include "CopperGolemStatueBlockEntity.h"
#include "ChestTileEntity.h"
#include "HatchetItem.h"
#include "SoundTypes.h"
#include "SynchedEntityData.h"
#include "ParticleTypes.h"

CopperGolem::CopperGolem(Level *level) : Golem(level)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	nextWeatheringTick = -1;
	antennaItem = nullptr;

	// Transport cooldown (Phase 5b, Java CopperGolem ctor sets the
	// TRANSPORT_ITEMS_COOLDOWN_TICKS memory to a random 60-100): the
	// golem does not start transporting immediately on spawn.
	transportCooldownTicks = CopperGolemAi::TRANSPORT_COOLDOWN_MIN_TICKS +
		random->nextInt(CopperGolemAi::TRANSPORT_COOLDOWN_MAX_TICKS -
			CopperGolemAi::TRANSPORT_COOLDOWN_MIN_TICKS + 1);

	// ~1 block tall (the antenna tip reaches ~1.75 blocks visually; the
	// 10x14 statue column corroborates the ~1-block body). Playtest check:
	// hitbox vs the model's 24-unit height.
	setSize(0.6f, 1.0f);

	getNavigation()->setCanOpenDoors(true);
	getNavigation()->setAvoidWater(true);

	CopperGolemAi::registerGoals(this);
}

void CopperGolem::defineSynchedData()
{
	Golem::defineSynchedData();
	// One byte, Sheep-style (DATA_WOOL_ID shares sheared + colour in a
	// byte): bits 0-1 weather state, bit 2 waxed. The client-side renderer
	// reads this byte to pick the body/eyes textures, so oxidation must be
	// synched.
	entityData->define(DATA_WEATHER_ID, static_cast<byte>(0));
	// Transport state byte (Phase 5b): the goal sets it server-side and
	// the client-side model reads it for the reach-in / carry poses.
	// Defaults to 0 = COPPER_GOLEM_STATE_IDLE.
	entityData->define(DATA_STATE_ID, static_cast<byte>(0));
}

bool CopperGolem::useNewAi()
{
	return true;
}

void CopperGolem::registerAttributes()
{
	Golem::registerAttributes();

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(12);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.2f);
}

int CopperGolem::getWeatherState()
{
	return entityData->getByte(DATA_WEATHER_ID) & 3;
}

void CopperGolem::setWeatherState(int state)
{
	byte current = entityData->getByte(DATA_WEATHER_ID);
	entityData->set(DATA_WEATHER_ID, static_cast<byte>((current & ~3) | (state & 3)));
}

bool CopperGolem::isWaxed()
{
	return (entityData->getByte(DATA_WEATHER_ID) & 4) != 0;
}

void CopperGolem::setWaxed(bool waxed)
{
	byte current = entityData->getByte(DATA_WEATHER_ID);
	if (waxed) current = static_cast<byte>(current | 4);
	else current = static_cast<byte>(current & ~4);
	entityData->set(DATA_WEATHER_ID, current);
}

CopperGolemState CopperGolem::getState()
{
	return static_cast<CopperGolemState>(entityData->getByte(DATA_STATE_ID));
}

void CopperGolem::setState(CopperGolemState state)
{
	entityData->set(DATA_STATE_ID, static_cast<byte>(state));
}

int CopperGolem::getTransportCooldown()
{
	return transportCooldownTicks;
}

void CopperGolem::setTransportCooldown(int ticks)
{
	transportCooldownTicks = ticks;
}

void CopperGolem::resetTransportCooldown()
{
	transportCooldownTicks = CopperGolemAi::TRANSPORT_COOLDOWN_MIN_TICKS +
		random->nextInt(CopperGolemAi::TRANSPORT_COOLDOWN_MAX_TICKS -
			CopperGolemAi::TRANSPORT_COOLDOWN_MIN_TICKS + 1);
}

shared_ptr<ItemInstance> CopperGolem::getAntennaItem()
{
	return antennaItem;
}

void CopperGolem::setAntennaItem(shared_ptr<ItemInstance> item)
{
	antennaItem = item;
}

bool CopperGolem::readyForShearing()
{
	// Java: tag SHEARABLE_FROM_COPPER_GOLEM — the antenna holds a block
	// item. Empty antenna = nothing to shear.
	return antennaItem != nullptr;
}

void CopperGolem::shear()
{
	if (antennaItem == nullptr) return;
	spawnAtLocation(antennaItem, 1.0f);
	antennaItem = nullptr;
}

float CopperGolem::getSpinPhase()
{
	int inCycle = tickCount % SPIN_PERIOD_TICKS;
	if (inCycle >= SPIN_DURATION_TICKS) return -1.0f;
	return static_cast<float>(inCycle) / static_cast<float>(SPIN_DURATION_TICKS);
}

int CopperGolem::getSpinSound()
{
	switch (getWeatherState())
	{
	case WEATHER_STATE_WEATHERED:
		return eSoundType_MOB_COPPER_GOLEM_WEATHERED_SPIN;
	case WEATHER_STATE_OXIDIZED:
		return eSoundType_MOB_COPPER_GOLEM_OXIDIZED_SPIN;
	default:
		// UNAFFECTED and EXPOSED share the regular set (Java
		// CopperGolemOxidationLevels).
		return eSoundType_MOB_COPPER_GOLEM_SPIN;
	}
}

int CopperGolem::getHurtSoundForState()
{
	switch (getWeatherState())
	{
	case WEATHER_STATE_WEATHERED:
		return eSoundType_MOB_COPPER_GOLEM_WEATHERED_HURT;
	case WEATHER_STATE_OXIDIZED:
		return eSoundType_MOB_COPPER_GOLEM_OXIDIZED_HURT;
	default:
		return eSoundType_MOB_COPPER_GOLEM_HURT;
	}
}

int CopperGolem::getDeathSoundForState()
{
	switch (getWeatherState())
	{
	case WEATHER_STATE_WEATHERED:
		return eSoundType_MOB_COPPER_GOLEM_WEATHERED_DEATH;
	case WEATHER_STATE_OXIDIZED:
		return eSoundType_MOB_COPPER_GOLEM_OXIDIZED_DEATH;
	default:
		return eSoundType_MOB_COPPER_GOLEM_DEATH;
	}
}

int CopperGolem::getStepSoundForState()
{
	switch (getWeatherState())
	{
	case WEATHER_STATE_WEATHERED:
		return eSoundType_MOB_COPPER_GOLEM_WEATHERED_STEP;
	case WEATHER_STATE_OXIDIZED:
		return eSoundType_MOB_COPPER_GOLEM_OXIDIZED_STEP;
	default:
		return eSoundType_MOB_COPPER_GOLEM_STEP;
	}
}

int CopperGolem::getAmbientSound()
{
	// Silent ambient, like the iron golem: the head-spin sound is played
	// explicitly from tick() at +10 ticks into the spin, not on the
	// ambient interval.
	return -1;
}

int CopperGolem::getHurtSound()
{
	return getHurtSoundForState();
}

int CopperGolem::getDeathSound()
{
	return getDeathSoundForState();
}

void CopperGolem::playStepSound(int xt, int yt, int zt, int t)
{
	// VillagerGolem precedent: the golem plays its own step sound instead
	// of the stepped-on tile's. Volume 0.15 matches Entity::playStepSound.
	playSound(getStepSoundForState(), 0.15f, 1.0f);
}

void CopperGolem::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	// jar1201 loot table copper_golem.json: 1 pool, rolls 1, copper_ingot
	// count uniform 1-3, looting_enchant 0-1 per level.
	int count = 1 + random->nextInt(3);
	if (playerBonusLevel > 0) count += random->nextInt(playerBonusLevel + 1);
	for (int i = 0; i < count; i++)
	{
		spawnAtLocation(Item::copper_ingot_Id, 1);
	}
}

void CopperGolem::tick()
{
	Golem::tick();

	CopperGolemAi::updateActivity(this);

	if (level->isClientSide) return;

	// Head-spin sound at +10 ticks into each spin cycle (Java
	// CopperGolemAi SPIN_SOUND_TIME_INTERVAL_OFFSET). Only while idle —
	// the spin is an idle animation, so a walking golem neither spins
	// (the model gates on walk speed) nor plays the sound.
	if ((tickCount % SPIN_PERIOD_TICKS) == SPIN_SOUND_OFFSET_TICKS &&
		xd * xd + zd * zd <= MoveControl::MIN_SPEED_SQR)
	{
		playSound(getSpinSound(), 1.0f, 1.0f);
	}

	updateWeathering();
}

void CopperGolem::updateWeathering()
{
	// Waxed golems never oxidize (the block system skips waxed tiles in
	// Level::advanceCopperOxidation the same way). Java sets
	// nextWeatheringTick = -2 on waxing; this tree uses the waxed flag
	// instead — no -2 sentinel.
	if (isWaxed()) return;

	if (getWeatherState() >= WEATHER_STATE_OXIDIZED)
	{
		// OXIDIZED: 0.58% chance per tick, while standing in air, to turn
		// into a statue (Java CopperGolem.canTurnToStatue).
		int bx = Mth::floor(x);
		int by = Mth::floor(y);
		int bz = Mth::floor(z);
		if (level->getTile(bx, by, bz) == 0 && random->nextFloat() <= 0.0058f)
		{
			turnToStatue();
		}
		return;
	}

	if (nextWeatheringTick < 0)
	{
		nextWeatheringTick = level->getGameTime() + WEATHERING_TICK_FROM +
			random->nextInt(static_cast<int>(WEATHERING_TICK_TO - WEATHERING_TICK_FROM));
		return;
	}

	if (level->getGameTime() >= nextWeatheringTick)
	{
		setWeatherState(getWeatherState() + 1);
		nextWeatheringTick = level->getGameTime() + WEATHERING_TICK_FROM +
			random->nextInt(static_cast<int>(WEATHERING_TICK_TO - WEATHERING_TICK_FROM));
	}
}

void CopperGolem::turnToStatue()
{
	if (level->isClientSide) return;

	int bx = Mth::floor(x);
	int by = Mth::floor(y);
	int bz = Mth::floor(z);

	// Random pose; facing from the golem's yaw (ChestTile convention:
	// 0 NORTH, 1 EAST, 2 SOUTH, 3 WEST).
	// Oxidation workaround: the nibble holds oxidation ONLY (bits 0-1).
	// Pose/facing live in the statue's TE — write them there after
	// createStatue and broadcast so clients see them live.
	int pose = random->nextInt(4);
	int facing = (Mth::floor(yRot * 4.0f / 360.0f + 0.5f)) & 3;
	int data = getWeatherState() & 3;

	level->setTileAndData(bx, by, bz, Tile::copper_golem_statue_Id, data, Tile::UPDATE_ALL);

	// The statue keeps the golem's custom name (Java createStatue).
	{
		shared_ptr<TileEntity> te = level->getTileEntity(bx, by, bz);
		shared_ptr<CopperGolemStatueBlockEntity> statueTe =
			dynamic_pointer_cast<CopperGolemStatueBlockEntity>(te);
		if (statueTe != nullptr)
		{
			statueTe->createStatue(dynamic_pointer_cast<CopperGolem>(shared_from_this()));
			statueTe->setPose(pose);
			statueTe->setFacing(facing);
			level->broadcastTileEntityUpdate(statueTe);
		}
	}

	// Drop whatever the golem was carrying / wearing on its antenna.
	shared_ptr<ItemInstance> carried = getCarriedItem();
	if (carried != nullptr)
	{
		spawnAtLocation(carried, 1.0f);
		setEquippedSlot(SLOT_WEAPON, nullptr);
	}
	if (antennaItem != nullptr)
	{
		spawnAtLocation(antennaItem, 1.0f);
		antennaItem = nullptr;
	}

	playSound(eSoundType_COPPER_STATUE_BECOME_STATUE, 1.0f, 1.0f);
	remove();
}

bool CopperGolem::spawnFromBuild(Level *level, int x, int y, int z, int chestFacing)
{
	// Java build pattern: a carved pumpkin / jack o'lantern on top of a
	// copper block. (x, y, z) is the pumpkin. The copper block becomes a
	// copper chest (facing chestFacing, oxidation inherited, never waxed)
	// and a golem spawns with the same oxidation. Mirrors the
	// PumpkinTile::onPlace iron-golem branch.
	if (level->isClientSide) return false;
	if (!level->canCreateMore(eTYPE_COPPER_GOLEM, Level::eSpawnType_Egg)) return false;

	// Read the oxidation BEFORE clearing: aux bits 0-1 are the stage,
	// bit 2 is waxed. Java: the golem and the chest inherit the stage
	// but never the wax.
	int stage = level->getData(x, y - 1, z) & 3;

	level->setTileAndData(x, y, z, 0, 0, Tile::UPDATE_CLIENTS);
	level->setTileAndData(x, y - 1, z, Tile::copper_chest_Id, chestFacing, Tile::UPDATE_CLIENTS);

	// The chest's oxidation lives in its tile entity (oxidation workaround):
	// set it now and broadcast so clients see the right stage live.
	{
		shared_ptr<TileEntity> te = level->getTileEntity(x, y - 1, z);
		shared_ptr<ChestTileEntity> chestTe = dynamic_pointer_cast<ChestTileEntity>(te);
		if (chestTe != nullptr)
		{
			chestTe->setOxidation(stage);
			level->broadcastTileEntityUpdate(chestTe);
		}
	}

	shared_ptr<CopperGolem> golem = std::make_shared<CopperGolem>(level);
	golem->setWeatherState(stage);
	golem->setWaxed(false);
	golem->moveTo(x + 0.5, y - 1.95, z + 0.5, 0, 0);
	level->addEntity(golem);

	for (int i = 0; i < 120; i++)
	{
		level->addParticle(eParticleType_snowballpoof, x + level->random->nextDouble(),
			y - 2 + level->random->nextDouble() * 3.9, z + level->random->nextDouble(), 0, 0, 0);
	}

	level->tileUpdated(x, y, z, 0);
	level->tileUpdated(x, y - 1, z, 0);
	return true;
}

bool CopperGolem::mobInteract(shared_ptr<Player> player)
{
	shared_ptr<ItemInstance> item = player->inventory->getSelected();

	if (!player->isAllowedToInteract(shared_from_this()))
		return false;

	if (item != nullptr && item->id == Item::shears->id && readyForShearing())
	{
		// Shears remove the antenna block item (Java shear()). The shear
		// sound is the vanilla sheep shear — no new entry (SoundTypes.h
		// already has eSoundType_MOB_SHEEP_SHEAR).
		if (!level->isClientSide)
		{
			shear();
		}
		item->hurtAndBreak(1, player);
		playSound(eSoundType_MOB_SHEEP_SHEAR, 1, 1);
		return true;
	}

	if (item != nullptr && item->id == Item::honeycomb_Id && !isWaxed())
	{
		// Honeycomb waxes the golem: weathering pauses (see
		// updateWeathering). Java also sets nextWeatheringTick = -2; this
		// tree uses the waxed flag instead. No dedicated wax sound exists
		// in the golem's sound set, so none is played.
		// Audit fix: the honeycomb is CONSUMED (it was infinite free
		// waxing before — the golem never used the item up).
		if (!level->isClientSide)
		{
			setWaxed(true);
			player->inventory->removeItem(player->inventory->selected, 1);
		}
		return true;
	}

	if (item != nullptr && dynamic_cast<HatchetItem *>(item->getItem()) != nullptr)
	{
		if (isWaxed())
		{
			// Axe on a waxed golem: scrape the wax off, weathering resumes.
			if (!level->isClientSide)
			{
				setWaxed(false);
				nextWeatheringTick = -1;
			}
			item->hurtAndBreak(1, player);
			return true;
		}
		if (getWeatherState() > WEATHER_STATE_UNAFFECTED)
		{
			// Axe on an oxidized golem: de-oxidize one stage.
			if (!level->isClientSide)
			{
				setWeatherState(getWeatherState() - 1);
				nextWeatheringTick = -1;
			}
			item->hurtAndBreak(1, player);
			return true;
		}
	}

	if (item == nullptr)
	{
		// Empty hand: take the item the golem is holding (Java mobInteract
		// takes the held item into the player's hand).
		shared_ptr<ItemInstance> carried = getCarriedItem();
		if (carried != nullptr)
		{
			if (!level->isClientSide)
			{
				// Hand it to the player by dropping it at the golem: the
				// tree has no direct "give to player hand" path for mobs,
				// so the item drops as an entity the player can pick up.
				spawnAtLocation(carried, 1.0f);
				setEquippedSlot(SLOT_WEAPON, nullptr);
			}
			return true;
		}
	}

	return Golem::mobInteract(player);
}

void CopperGolem::addAdditonalSaveData(CompoundTag *tag)
{
	Golem::addAdditonalSaveData(tag);
	tag->putInt(L"WeatherState", getWeatherState());
	tag->putBoolean(L"Waxed", isWaxed());
	tag->putLong(L"NextWeatheringTick", nextWeatheringTick);
	tag->putInt(L"GolemState", static_cast<int>(getState()));
	if (antennaItem != nullptr)
	{
		CompoundTag *antennaTag = new CompoundTag();
		antennaItem->save(antennaTag);
		tag->putCompound(L"AntennaItem", antennaTag);
	}
}

void CopperGolem::readAdditionalSaveData(CompoundTag *tag)
{
	Golem::readAdditionalSaveData(tag);
	if (tag->contains(L"WeatherState")) setWeatherState(tag->getInt(L"WeatherState"));
	if (tag->contains(L"Waxed")) setWaxed(tag->getBoolean(L"Waxed"));
	if (tag->contains(L"NextWeatheringTick")) nextWeatheringTick = tag->getLong(L"NextWeatheringTick");
	if (tag->contains(L"GolemState")) setState(static_cast<CopperGolemState>(tag->getInt(L"GolemState")));
	if (tag->contains(L"AntennaItem"))
	{
		antennaItem = ItemInstance::fromTag(tag->getCompound(L"AntennaItem"));
	}
}
