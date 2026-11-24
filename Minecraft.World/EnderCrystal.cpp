#include "stdafx.h"
#include "EndDragonFight.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.boss.enderdragon.h"
#include "EnderCrystal.h"
#include "DamageSource.h"



void EnderCrystal::_init(Level *level)
{		
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	blocksBuilding = true;
	setSize(2.0f, 2.0f);
	heightOffset = bbHeight / 2.0f;
	life = MAX_LIFE;

	time = random->nextInt(100000);
}

EnderCrystal::EnderCrystal(Level *level) : Entity( level )
{
	_init(level);

}

EnderCrystal::EnderCrystal(Level *level, double x, double y, double z) : Entity( level )
{
	_init(level);
	setPos(x, y, z);

}

bool EnderCrystal::makeStepSound()
{
	return false;
}

void EnderCrystal::defineSynchedData()
{
	entityData->define(DATA_REMAINING_LIFE, life);
	entityData->define(DATA_SHOW_BOTTOM, static_cast<byte>(1));
	entityData->define(DATA_BEAM_TARGET_X, 0);
	entityData->define(DATA_BEAM_TARGET_Y, -1);
	entityData->define(DATA_BEAM_TARGET_Z, 0);
}

void EnderCrystal::tick()
{
	xo = x;
	yo = y;
	zo = z;
	time++;

	entityData->set(DATA_REMAINING_LIFE, life);

	// Don't set the tile directly on the client, as this can end up in the updatesToReset queue in the MultiPlayerLevel, and the perpetually end up removing/adding these fire tiles causing timing
	// glitches from the lighting changes requried.
	if (!level->isClientSide)
	{
		int xt = Mth::floor(x);
		int yt = Mth::floor(y);
		int zt = Mth::floor(z);
		if (level->getTile(xt, yt, zt) != Tile::fire_Id)
		{
			level->setTileAndUpdate(xt, yt, zt, Tile::fire_Id);
		}
	}
}


void EnderCrystal::addAdditonalSaveData(CompoundTag *tag)
{
	tag->putBoolean(L"ShowBottom", showsBottom());
	if (hasBeamTarget())
	{
		CompoundTag *beam = new CompoundTag();
		beam->putInt(L"X", getBeamTargetX());
		beam->putInt(L"Y", getBeamTargetY());
		beam->putInt(L"Z", getBeamTargetZ());
		tag->putCompound(L"BeamTarget", beam);
	}
}

void EnderCrystal::readAdditionalSaveData(CompoundTag *tag)
{
	if (tag->contains(L"ShowBottom")) setShowBottom(tag->getBoolean(L"ShowBottom"));
	if (tag->contains(L"BeamTarget"))
	{
		CompoundTag *beam = tag->getCompound(L"BeamTarget");
		setBeamTarget(beam->getInt(L"X"), beam->getInt(L"Y"), beam->getInt(L"Z"));
	}
}


float EnderCrystal::getShadowHeightOffs()
{
	return 0;
}

bool EnderCrystal::isPickable()
{
	return true;
}

bool EnderCrystal::hurt(DamageSource *source, float damage)
{
	if (isInvulnerable()) return false;

	// 4J-PB - if the owner of the source is the enderdragon, then ignore it (where the dragon's fireball hits an endercrystal)
	if ( source->getEntity() != nullptr && source->getEntity()->instanceof(eTYPE_ENDERDRAGON) )
	{
		return false;
	}

	if (!removed && !level->isClientSide)
	{
		life = 0;
		if (life <= 0)
		{
			remove();
			if (!level->isClientSide)
			{
				level->explode(nullptr, x, y, z, 6, true);

				vector<shared_ptr<Entity> > entities = level->getAllEntities();
				shared_ptr<EnderDragon> dragon = nullptr;
				for ( auto& e : entities )
				{
					dragon = dynamic_pointer_cast<EnderDragon>(e);
					if ( dragon )
					{
						dragon->handleCrystalDestroyed(source);
						break;
					}
				}

				EndDragonFight *fight = EndDragonFight::get(level);
				if (fight != nullptr) fight->onCrystalDestroyed(dynamic_pointer_cast<EnderCrystal>(shared_from_this()));
			}
		}
	}
	return true;
}
void EnderCrystal::setShowBottom(bool show)
{
	entityData->set(DATA_SHOW_BOTTOM, static_cast<byte>(show ? 1 : 0));
}

bool EnderCrystal::showsBottom()
{
	return entityData->getByte(DATA_SHOW_BOTTOM) != 0;
}

void EnderCrystal::setBeamTarget(int x, int y, int z)
{
	entityData->set(DATA_BEAM_TARGET_X, x);
	entityData->set(DATA_BEAM_TARGET_Y, y);
	entityData->set(DATA_BEAM_TARGET_Z, z);
}

void EnderCrystal::clearBeamTarget()
{
	entityData->set(DATA_BEAM_TARGET_Y, -1);
}

bool EnderCrystal::hasBeamTarget()
{
	return getBeamTargetY() >= 0;
}

int EnderCrystal::getBeamTargetX()
{
	return entityData->getInteger(DATA_BEAM_TARGET_X);
}

int EnderCrystal::getBeamTargetY()
{
	return entityData->getInteger(DATA_BEAM_TARGET_Y);
}

int EnderCrystal::getBeamTargetZ()
{
	return entityData->getInteger(DATA_BEAM_TARGET_Z);
}
