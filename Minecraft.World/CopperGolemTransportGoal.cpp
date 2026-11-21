#include "stdafx.h"
#include "CopperGolemTransportGoal.h"
#include "ChestTile.h"
#include "ChestTileEntity.h"
#include "Container.h"
#include "CopperGolem.h"
#include "CopperGolemAi.h"
#include "Mth.h"
#include "SoundTypes.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.h"

// Interaction range, squared (Java CopperGolem
// getContainerInteractionRange() = 3.0 blocks).
static const double INTERACT_RANGE_SQR = 9.0;

CopperGolemTransportGoal::CopperGolemTransportGoal(CopperGolem *golem)
{
	this->golem = golem;
	phase = PHASE_NONE;
	ticksAtTarget = 0;
	travelTicks = 0;
	sourceX = sourceY = sourceZ = 0;
	destX = destY = destZ = 0;
	openContainer = nullptr;
	// Start ready to scan (the spawn cooldown, not the scan throttle, is
	// what delays the first run).
	lastScanTick = -SCAN_INTERVAL_TICKS;
	pendingSound = -1;
	// Claim Move + Look, like RandomStrollGoal: while transporting, the
	// golem must not wander or look at players instead.
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
}

bool CopperGolemTransportGoal::canUse()
{
	if (golem->level->isClientSide) return false;
	if (golem->getTransportCooldown() > 0) return false;
	if (golem->tickCount - lastScanTick < SCAN_INTERVAL_TICKS) return false;
	lastScanTick = golem->tickCount;
	return findTransportPair();
}

bool CopperGolemTransportGoal::canContinueToUse()
{
	return phase != PHASE_DONE && phase != PHASE_FAILED;
}

void CopperGolemTransportGoal::start()
{
	ticksAtTarget = 0;
	travelTicks = 0;
	openContainer = nullptr;
	pendingSound = -1;
	if (golem->getCarriedItem() != nullptr)
	{
		// Already carrying an item (an interrupted run) — go straight to
		// the destination and deposit it; there is no source leg.
		phase = PHASE_TO_DEST;
		golem->getNavigation()->moveTo(destX + 0.5, destY, destZ + 0.5, 1.0);
	}
	else
	{
		phase = PHASE_TO_SOURCE;
		golem->getNavigation()->moveTo(sourceX + 0.5, sourceY, sourceZ + 0.5, 1.0);
	}
}

void CopperGolemTransportGoal::stop()
{
	// Close any chest left open by an interrupted interaction, so it does
	// not stay open (and its openCount does not leak).
	if (openContainer != nullptr)
	{
		openContainer->stopOpen();
		openContainer = nullptr;
	}
	golem->getNavigation()->stop();
	golem->setState(COPPER_GOLEM_STATE_IDLE);
	// A run ends here whether it succeeded, failed, or was interrupted:
	// wait a random 60-100 ticks before looking for work again (Java
	// TRANSPORT_ITEMS_COOLDOWN_TICKS).
	golem->resetTransportCooldown();
	phase = PHASE_NONE;
}

void CopperGolemTransportGoal::tick()
{
	switch (phase)
	{
	case PHASE_TO_SOURCE:
		tickTravel(sourceX, sourceY, sourceZ, PHASE_AT_SOURCE);
		break;
	case PHASE_TO_DEST:
		tickTravel(destX, destY, destZ, PHASE_AT_DEST);
		break;
	case PHASE_AT_SOURCE:
		tickInteract(true);
		break;
	case PHASE_AT_DEST:
		tickInteract(false);
		break;
	default:
		break;
	}
}

bool CopperGolemTransportGoal::findTransportPair()
{
	Level *level = golem->level;
	int gx = Mth::floor(golem->x);
	int gy = Mth::floor(golem->y);
	int gz = Mth::floor(golem->z);
	bool carrying = golem->getCarriedItem() != nullptr;

	// One pass over the 32 horizontal / 8 vertical box around the golem
	// (Java TRANSPORT_ITEM_HORIZONTAL/VERTICAL_SEARCH_RADIUS). Track the
	// nearest usable source and the nearest usable destination.
	bool foundSource = false;
	bool foundDest = false;
	int bestSourceDist = 0;
	int bestDestDist = 0;
	int sx = 0, sy = 0, sz = 0;
	int dx = 0, dy = 0, dz = 0;

	for (int y = gy - CopperGolemAi::TRANSPORT_ITEM_VERTICAL_SEARCH_RADIUS;
		y <= gy + CopperGolemAi::TRANSPORT_ITEM_VERTICAL_SEARCH_RADIUS; y++)
	{
		for (int x = gx - CopperGolemAi::TRANSPORT_ITEM_HORIZONTAL_SEARCH_RADIUS;
			x <= gx + CopperGolemAi::TRANSPORT_ITEM_HORIZONTAL_SEARCH_RADIUS; x++)
		{
			for (int z = gz - CopperGolemAi::TRANSPORT_ITEM_HORIZONTAL_SEARCH_RADIUS;
				z <= gz + CopperGolemAi::TRANSPORT_ITEM_HORIZONTAL_SEARCH_RADIUS; z++)
			{
				int tileId = level->getTile(x, y, z);
				if (!carrying && tileId == Tile::copper_chest_Id)
				{
					// Source: a copper chest someone currently has open
					// (Java shouldQueueForTarget — the opener list is
					// non-empty) that holds at least one item.
					shared_ptr<ChestTileEntity> te =
						dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(x, y, z));
					if (te == nullptr || te->openCount <= 0) continue;
					shared_ptr<Container> container = getChestContainer(x, y, z);
					if (container == nullptr || !containerHasItem(container)) continue;
					int d = (x - gx) * (x - gx) + (y - gy) * (y - gy) + (z - gz) * (z - gz);
					if (!foundSource || d < bestSourceDist)
					{
						foundSource = true;
						bestSourceDist = d;
						sx = x; sy = y; sz = z;
					}
				}
				else if (tileId == Tile::chest_Id || tileId == Tile::chest_trap_Id)
				{
					// Destination: a normal or trapped chest that can be
					// opened (getContainer returns nullptr when a solid
					// block sits on top or a cat is sitting on it).
					shared_ptr<Container> container = getChestContainer(x, y, z);
					if (container == nullptr) continue;
					int d = (x - gx) * (x - gx) + (y - gy) * (y - gy) + (z - gz) * (z - gz);
					if (!foundDest || d < bestDestDist)
					{
						foundDest = true;
						bestDestDist = d;
						dx = x; dy = y; dz = z;
					}
				}
			}
		}
	}

	if (carrying)
	{
		// Already holding a stack: only a destination is needed.
		if (!foundDest) return false;
	}
	else
	{
		if (!foundSource || !foundDest) return false;
		sourceX = sx; sourceY = sy; sourceZ = sz;
	}
	destX = dx; destY = dy; destZ = dz;
	return true;
}

shared_ptr<Container> CopperGolemTransportGoal::getChestContainer(int x, int y, int z)
{
	int tileId = golem->level->getTile(x, y, z);
	Tile *tile = Tile::tiles[tileId];
	if (tile == nullptr) return nullptr;
	ChestTile *chestTile = dynamic_cast<ChestTile *>(tile);
	if (chestTile == nullptr) return nullptr;
	// Returns the ChestTileEntity itself for a single chest, or a
	// CompoundContainer spanning both halves for a double chest.
	// Returns nullptr when the chest cannot be opened (solid block on
	// top, cat sitting on it, or no TileEntity).
	return chestTile->getContainer(golem->level, x, y, z);
}

bool CopperGolemTransportGoal::containerHasItem(shared_ptr<Container> container)
{
	if (container == nullptr) return false;
	for (unsigned int i = 0; i < container->getContainerSize(); i++)
	{
		if (container->getItem(i) != nullptr) return true;
	}
	return false;
}

shared_ptr<ItemInstance> CopperGolemTransportGoal::takeFirstStack(shared_ptr<Container> container)
{
	if (container == nullptr) return nullptr;
	for (unsigned int i = 0; i < container->getContainerSize(); i++)
	{
		if (container->getItem(i) != nullptr)
		{
			shared_ptr<ItemInstance> stack = container->removeItemNoUpdate(i);
			if (stack != nullptr)
			{
				container->setChanged();
				return stack;
			}
		}
	}
	return nullptr;
}

bool CopperGolemTransportGoal::depositStack(shared_ptr<Container> container, shared_ptr<ItemInstance> stack)
{
	if (container == nullptr || stack == nullptr) return false;
	// First try to merge into an existing stack of the same item that has
	// room for the whole stack.
	for (unsigned int i = 0; i < container->getContainerSize(); i++)
	{
		shared_ptr<ItemInstance> existing = container->getItem(i);
		if (existing != nullptr && existing->sameItem(stack) && container->canPlaceItem(i, stack))
		{
			int limit = container->getMaxStackSize();
			int itemMax = stack->getMaxStackSize();
			if (itemMax < limit) limit = itemMax;
			if (existing->count + stack->count <= limit)
			{
				existing->count += stack->count;
				container->setChanged();
				return true;
			}
		}
	}
	// Then try an empty slot.
	for (unsigned int i = 0; i < container->getContainerSize(); i++)
	{
		if (container->getItem(i) == nullptr && container->canPlaceItem(i, stack))
		{
			container->setItem(i, stack);
			return true;
		}
	}
	return false;
}

bool CopperGolemTransportGoal::isAtChest(int cx, int cy, int cz)
{
	return golem->distanceToSqr(cx + 0.5, cy + 0.5, cz + 0.5) <= INTERACT_RANGE_SQR;
}

void CopperGolemTransportGoal::tickTravel(int cx, int cy, int cz, Phase arrivePhase)
{
	travelTicks++;
	if (isAtChest(cx, cy, cz))
	{
		golem->getNavigation()->stop();
		phase = arrivePhase;
		ticksAtTarget = 0;
		return;
	}
	// The navigation finished (or the leg timed out) without the golem
	// getting within reach — the chest is unreachable, so give up. The
	// 20-tick grace lets the path get established after moveTo() before
	// isDone() is believed.
	if ((golem->getNavigation()->isDone() && travelTicks > 20) ||
		travelTicks > TRAVEL_TIMEOUT_TICKS)
	{
		phase = PHASE_FAILED;
	}
}

void CopperGolemTransportGoal::tickInteract(bool isSource)
{
	ticksAtTarget++;
	if (ticksAtTarget == 1)
	{
		// Tick 1 (Java onReachedTargetInteraction): open the chest and
		// take / deposit the stack. The state (and the sound to play at
		// tick 9) follows from whether the transfer succeeded.
		int cx = isSource ? sourceX : destX;
		int cy = isSource ? sourceY : destY;
		int cz = isSource ? sourceZ : destZ;
		openContainer = getChestContainer(cx, cy, cz);
		if (openContainer == nullptr)
		{
			// The chest is gone or can no longer be opened mid-run.
			phase = PHASE_FAILED;
			return;
		}
		openContainer->startOpen();
		if (isSource)
		{
			shared_ptr<ItemInstance> stack = takeFirstStack(openContainer);
			if (stack != nullptr)
			{
				golem->setEquippedSlot(SLOT_WEAPON, stack);
				golem->setState(COPPER_GOLEM_STATE_GETTING_ITEM);
				pendingSound = eSoundType_MOB_COPPER_GOLEM_NO_ITEM_GET;
			}
			else
			{
				golem->setState(COPPER_GOLEM_STATE_GETTING_NO_ITEM);
				pendingSound = eSoundType_MOB_COPPER_GOLEM_NO_ITEM_NO_GET;
			}
		}
		else
		{
			shared_ptr<ItemInstance> held = golem->getCarriedItem();
			bool deposited = false;
			if (held != nullptr)
			{
				deposited = depositStack(openContainer, held);
				if (deposited)
				{
					golem->setEquippedSlot(SLOT_WEAPON, nullptr);
				}
			}
			if (deposited)
			{
				golem->setState(COPPER_GOLEM_STATE_DROPPING_ITEM);
				pendingSound = eSoundType_MOB_COPPER_GOLEM_ITEM_DROP;
			}
			else
			{
				golem->setState(COPPER_GOLEM_STATE_DROPPING_NO_ITEM);
				pendingSound = eSoundType_MOB_COPPER_GOLEM_ITEM_NO_DROP;
			}
		}
	}
	else if (ticksAtTarget == 9)
	{
		// Tick 9: the get / drop sound.
		if (pendingSound >= 0)
		{
			golem->playSound(pendingSound, 1.0f, 1.0f);
		}
	}
	else if (ticksAtTarget >= 60)
	{
		// Tick 60: close the chest and move on. The state returns to IDLE
		// while the golem travels (Java onTravelling).
		if (openContainer != nullptr)
		{
			openContainer->stopOpen();
			openContainer = nullptr;
		}
		golem->setState(COPPER_GOLEM_STATE_IDLE);
		if (isSource)
		{
			if (golem->getCarriedItem() != nullptr)
			{
				// Got a stack — carry it to the destination.
				phase = PHASE_TO_DEST;
				travelTicks = 0;
				golem->getNavigation()->moveTo(destX + 0.5, destY, destZ + 0.5, 1.0);
			}
			else
			{
				// The source was empty — the run is over.
				phase = PHASE_DONE;
			}
		}
		else
		{
			// Deposit done (or the destination was full) — the run is over.
			phase = PHASE_DONE;
		}
	}
}
