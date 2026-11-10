#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "SharedMonsterAttributes.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "Mth.h"
#include "MaceItem.h"
#include "MaceEnchantments.h"
#include "LevelEvent.h"
#include "SoundTypes.h"

// ---------------------------------------------------------------------------------------------------------------------------
// neoLegacy - Mace. See MaceItem.h for the rules. Numbers are copied from Java's MaceItem.
// ---------------------------------------------------------------------------------------------------------------------------

MaceItem::MaceItem(int id) : Item(id)
{
	maxStackSize = 1;
	setMaxDamage(DURABILITY);
}

bool MaceItem::hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker)
{
	itemInstance->hurtAndBreak(1, attacker);
	return true;
}

bool MaceItem::mineBlock(shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, shared_ptr<LivingEntity> owner)
{
	// Like a sword: no wear when the block breaks instantly.
	if (Tile::tiles[tile]->getDestroySpeed(level, x, y, z) != 0.0) itemInstance->hurtAndBreak(2, owner);
	return true;
}

bool MaceItem::isHandEquipped()
{
	return true;
}

int MaceItem::getEnchantmentValue()
{
	return 15;
}

bool MaceItem::isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem)
{
	// Java repairs the mace with a breeze rod. There are none here, so the blaze rod stands in for it.
	if (repairItem != nullptr && repairItem->id == Item::blaze_rod_Id) return true;
	return Item::isValidRepairItem(source, repairItem);
}

attrAttrModMap *MaceItem::getDefaultAttributeModifiers()
{
	attrAttrModMap *result = Item::getDefaultAttributeModifiers();
	result->insert(attrAttrModMap::value_type(SharedMonsterAttributes::ATTACK_DAMAGE->getId(),
		new AttributeModifier(eModifierId_ITEM_BASEDAMAGE, ATTACK_DAMAGE, AttributeModifier::OPERATION_ADDITION)));
	return result;
}

// ---- smash attack helpers ------------------------------------------------------------------------------------------------

bool MaceItem::isMace(shared_ptr<ItemInstance> item)
{
	return item != nullptr && item->getItem() != nullptr && dynamic_cast<MaceItem *>(item->getItem()) != nullptr;
}

bool MaceItem::canSmashAttack(shared_ptr<LivingEntity> attacker)
{
	if (attacker == nullptr) return false;
	if (attacker->fallDistance <= SMASH_FALL_THRESHOLD) return false;

	// Gliding on an elytra is not a fall (Java: !attacker.isFallFlying())
	if (attacker->instanceof(eTYPE_PLAYER))
	{
		shared_ptr<Player> player = dynamic_pointer_cast<Player>(attacker);
		if (player != nullptr && player->isElytraFlying()) return false;
	}
	return true;
}

bool MaceItem::isEnchantmentAllowed(Enchantment *e)
{
	if (e == nullptr) return false;
	return e == Enchantment::density || e == Enchantment::breach || e == Enchantment::windBurst
		|| e == Enchantment::damageBonusUndead || e == Enchantment::damageBonusArthropods || e == Enchantment::fireAspect
		|| e == Enchantment::digDurability || e == Enchantment::mending || e == Enchantment::vanishingCurse;
}

float MaceItem::getSmashBonusDamage(float fallDistance, shared_ptr<ItemInstance> mace)
{
	// +4 per block for the first 3 blocks, +2 per block for the next 5, +1 per block after that.
	double damage;
	if (fallDistance <= 3.0f)
	{
		damage = 4.0 * fallDistance;
	}
	else if (fallDistance <= 8.0f)
	{
		damage = 12.0 + 2.0 * (fallDistance - 3.0);
	}
	else
	{
		damage = 22.0 + (fallDistance - 8.0);
	}

	// Density: +0.5 damage per block fallen, per level (the whole fall counts, not just the first blocks)
	int density = EnchantmentHelper::getEnchantmentLevel(Enchantment::density->id, mace);
	if (density > 0) damage += 0.5 * density * fallDistance;

	return static_cast<float>(damage);
}

void MaceItem::doSmashEffects(shared_ptr<Player> attacker, shared_ptr<LivingEntity> target, float fallDistance)
{
	Level *level = attacker->level;
	if (level == nullptr || target == nullptr) return;

	const bool heavy = fallDistance > HEAVY_SMASH_THRESHOLD;

	// Sound, like Java: a heavy ground thud after a long fall, a normal ground smash otherwise, and a different one when the target is
	// still in the air. (Several recordings of each; the sound engine picks one.)
	if (target->onGround)
	{
		attacker->playSound(heavy ? eSoundType_ITEM_MACE_SMASH_GROUND_HEAVY : eSoundType_ITEM_MACE_SMASH_GROUND, 1.0f, 1.0f);

		// Effects (client side, see Minecraft.Client/MaceEffectParticle): rings, flash, sparks, dust, and with Breach the armor scraps.
		// x,y,z = the block under the target; the flags say how big a smash it was and which armor slots hold diamond.
		int flags = heavy ? SMASH_FLAG_HEAVY : 0;
		shared_ptr<ItemInstance> weapon = attacker->getSelectedItem();
		if (EnchantmentHelper::getEnchantmentLevel(Enchantment::breach->id, weapon) > 0)
		{
			flags |= SMASH_FLAG_BREACH;
			for (int slot = 0; slot < 4; ++slot)
			{
				shared_ptr<ItemInstance> armor = target->getArmor(slot);
				if (armor != nullptr && (armor->id == Item::diamond_boots_Id || armor->id == Item::diamond_leggings_Id
					|| armor->id == Item::diamond_chestplate_Id || armor->id == Item::diamond_helmet_Id))
				{
					flags |= 1 << (SMASH_FLAG_ARMOR_SHIFT + slot);
				}
			}
		}
		level->levelEvent(LevelEvent::PARTICLES_MACE_SMASH, Mth::floor(target->x), Mth::floor(target->bb->y0 - 0.2), Mth::floor(target->z), flags);
	}
	else
	{
		attacker->playSound(eSoundType_ITEM_MACE_SMASH_AIR, 1.0f, 1.0f);
	}

	// Shockwave: everything living within 3.5 blocks of the target is thrown away from it
	AABB *area = target->bb->grow(KNOCKBACK_RADIUS, KNOCKBACK_RADIUS, KNOCKBACK_RADIUS);
	vector<shared_ptr<Entity> > *found = level->getEntities(target, area);
	if (found == nullptr || found->empty()) return;

	vector<shared_ptr<Entity> > nearby(found->begin(), found->end());	// pushing can change the level's lists, so iterate a copy
	for (auto &e : nearby)
	{
		if (e == nullptr || e == attacker || e == target) continue;
		if (!e->instanceof(eTYPE_LIVINGENTITY)) continue;
		if (e->instanceof(eTYPE_ARMORSTAND)) continue;

		// the attacker's own tamed pets are not thrown around
		if (e->instanceof(eTYPE_TAMABLE_ANIMAL))
		{
			shared_ptr<TamableAnimal> pet = dynamic_pointer_cast<TamableAnimal>(e);
			if (pet != nullptr && pet->isTame() && pet->getOwnerUUID() == attacker->getUUID()) continue;
		}

		// a creative player who is flying is left alone
		if (e->instanceof(eTYPE_PLAYER))
		{
			shared_ptr<Player> other = dynamic_pointer_cast<Player>(e);
			if (other != nullptr && other->abilities.instabuild && other->abilities.flying) continue;
		}

		double dx = e->x - target->x;
		double dy = e->y - target->y;
		double dz = e->z - target->z;
		double dist = sqrt(dx * dx + dy * dy + dz * dz);
		if (dist > KNOCKBACK_RADIUS || dist < 0.0001) continue;

		shared_ptr<LivingEntity> living = dynamic_pointer_cast<LivingEntity>(e);
		double resist = 0.0;
		if (living != nullptr)
		{
			AttributeInstance *kb = living->getAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE);
			if (kb != nullptr) resist = kb->getValue();
		}

		double power = (KNOCKBACK_RADIUS - dist) * KNOCKBACK_POWER * (heavy ? 2.0 : 1.0) * (1.0 - resist);
		if (power <= 0.0) continue;

		// push() also flags the entity so the server tells the client about the new motion (this is what moves other players)
		e->push((dx / dist) * power, 0.7, (dz / dist) * power);
	}
}

void MaceItem::applySmashToAttacker(shared_ptr<Player> attacker, shared_ptr<ItemInstance> mace)
{
	if (attacker == nullptr) return;

	// The smash stops the fall dead (Java sets the vertical speed to 0.01) and wipes the fall distance, so landing never hurts.
	attacker->yd = 0.01;
	attacker->fallDistance = 0.0f;

	// Fall damage is ignored for everything above the point of impact (see Player::causeFallDamage)
	attacker->ignoreFallFromImpulse = true;
	attacker->impulseImpactY = static_cast<float>(attacker->bb->y0);

	int windBurst = EnchantmentHelper::getEnchantmentLevel(Enchantment::windBurst->id, mace);
	if (windBurst > 0)
	{
		// Only the local copy of the player matters for movement (the client owns it), so this deliberately does NOT set hasImpulse:
		// that would make the server echo the launch speed back a few ticks later and boost the player twice.
		attacker->yd = getWindBurstLaunchSpeed(windBurst);
		attacker->onGround = false;
		attacker->fallDistance = 0.0f;
	}
}

float MaceItem::getWindBurstLaunchSpeed(int level)
{
	// blocks per tick; with this engine's gravity (0.08) and drag (0.98) these peak about 7, 11 and 17 blocks above the launch point
	switch (level)
	{
	case 1:		return 1.10f;
	case 2:		return 1.45f;
	default:	return 1.80f;
	}
}
