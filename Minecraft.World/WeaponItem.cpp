#include "stdafx.h"
#include "AABB.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.h"
#include "com.mojang.nbt.h"	// neoLegacy - JDemon/CMonster v2: the 1-minute cooldown lives on the item tag (HornOfSightItem.cpp)
#include "WeaponItem.h"
#include "MobEffect.h"
#include "MobEffectInstance.h"

WeaponItem::WeaponItem(int id, const Tier *tier, ZoroSwordType zoroType) : Item(id), tier(tier), zoroType(zoroType)
{
	maxStackSize = 1;
	setMaxDamage(tier->getUses());

	damage = 4 + tier->getAttackDamageBonus();
}

float WeaponItem::getTierDamage()
{
	return tier->getAttackDamageBonus();
}

float WeaponItem::getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile)
{
	if (tile->id == Tile::bamboo_Id || tile->id == Tile::bamboo_shoot_Id)
	{
		return 15;	// neoLegacy - a sword cuts bamboo instantly
	}
	if (tile->id == Tile::web_Id)
	{
		// swords can quickly cut web
		return 15;
	}
	// this change modifies which tiles the swords can destroy in creative
	// mode (>1 == yes)
	Material *material = tile->material;
	if (material == Material::plant || material == Material::replaceable_plant || material == Material::coral || material == Material::leaves || material == Material::vegetable)
	{
		return 1.5f;
	}
	return 1.0f;
}

bool WeaponItem::hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker) 
{
	itemInstance->hurtAndBreak(1, attacker);
	return true;
}

bool WeaponItem::mineBlock(shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, shared_ptr<LivingEntity> owner)
{
	// Don't damage weapons if the tile can be destroyed in one hit.
	if (Tile::tiles[tile]->getDestroySpeed(level, x, y, z) != 0.0) itemInstance->hurtAndBreak(2, owner);
	return true;
}

bool WeaponItem::isHandEquipped()
{
	return true;
}

UseAnim WeaponItem::getUseAnimation(shared_ptr<ItemInstance> itemInstance)
{
	return UseAnim_block;
}    

int WeaponItem::getUseDuration(shared_ptr<ItemInstance> itemInstance)
{
	return 20 * 60 * 60; // Block for a maximum of one hour!
}

shared_ptr<ItemInstance> WeaponItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	player->startUsingItem(instance, getUseDuration(instance));
	return instance;
}

bool WeaponItem::canDestroySpecial(Tile *tile)
{
	return tile->id == Tile::web_Id;
}

int WeaponItem::getEnchantmentValue()
{
	return tier->getEnchantmentValue();
}

const Item::Tier *WeaponItem::getTier()
{
	return tier;
}

bool WeaponItem::isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem)
{
	if (repairItem == nullptr)
		return false;

	if (tier->getTierItemId() == repairItem->id)
	{
		return true;
	}

	// Zoro swords are forged as a diamond-tier+ material and can be repaired
	// with diamonds without introducing a new repair material.
	if (isZoroSword() && repairItem->id == Item::diamond_Id)
	{
		return true;
	}

	return Item::isValidRepairItem(source, repairItem);
}

attrAttrModMap *WeaponItem::getDefaultAttributeModifiers()
{
	attrAttrModMap *result = Item::getDefaultAttributeModifiers();

	result->insert(attrAttrModMap::value_type( SharedMonsterAttributes::ATTACK_DAMAGE->getId(), new AttributeModifier(eModifierId_ITEM_BASEDAMAGE, damage, AttributeModifier::OPERATION_ADDITION) ) );

	return result;
}

bool WeaponItem::isZoroSword() const
{
	return zoroType != ZORO_NONE;
}

WeaponItem::ZoroSwordType WeaponItem::getZoroSwordType() const
{
	return zoroType;
}

float WeaponItem::getZoroDamageMultiplier(int comboStep) const
{
	// All four swords intentionally deal exactly the same base damage as a
	// vanilla diamond sword. The combo changes the animation and attack
	// direction, not raw damage, so these weapons stay balanced.
	return 1.0f;
}

float WeaponItem::getZoroAttackRange() const
{
	switch (zoroType)
	{
	case ZORO_WADO_ICHIMONJI: return 3.25f;
	case ZORO_SANDAI_KITETSU: return 3.35f;
	case ZORO_ENMA: return 3.10f;
	case ZORO_SHUSUI: return 3.45f;
	default: return 3.0f;
	}
}

float WeaponItem::getZoroAttackAngle() const
{
	switch (zoroType)
	{
	case ZORO_WADO_ICHIMONJI: return 105.0f;
	case ZORO_SANDAI_KITETSU: return 115.0f;
	case ZORO_ENMA: return 90.0f;
	case ZORO_SHUSUI: return 125.0f;
	default: return 90.0f;
	}
}

int WeaponItem::getZoroAttackCooldown() const
{
	// 10-12 ticks gives the player enough time to actually see each cut
	// without making the swords feel sluggish.
	switch (zoroType)
	{
	case ZORO_WADO_ICHIMONJI: return 10;
	case ZORO_SANDAI_KITETSU: return 10;
	case ZORO_ENMA: return 12;
	case ZORO_SHUSUI: return 11;
	default: return 0;
	}
}

JDemonSword::JDemonSword(int id, const Tier *tier) : WeaponItem(id, tier)
{
}

bool JDemonSword::hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker)
{
	mob->setOnFire(5);
	return WeaponItem::hurtEnemy(itemInstance, mob, attacker);
}

shared_ptr<ItemInstance> JDemonSword::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	// neoLegacy - JDemon/CMonster v2: the invisibility special lasts 30 seconds
	// (EFFECT_TICKS) and then needs a 1-minute cooldown (COOLDOWN_TICKS) before it
	// can be granted again. The cooldown is armed on the server-side item tag
	// when the special is granted, following the Horn of Sight
	// (HornOfSightItem.cpp): the server applies the effect and it syncs to the
	// client. Blocking itself still works on cooldown, it just grants no
	// invisibility, and the timer is not re-armed by blocking again.
	if (!level->isClientSide)
	{
		int64_t now = level->getGameTime();
		if (!instance->hasTag()) instance->setTag(new CompoundTag());
		CompoundTag *tag = instance->getTag();
		if (now >= tag->getLong(L"JDemonReady"))
		{
			tag->putLong(L"JDemonReady", now + COOLDOWN_TICKS);
			// Grant the invisibility EFFECT for the 30-second special, not just the flag:
			// LivingEntity's effect tick re-syncs FLAG_INVISIBLE from the active
			// effects (LivingEntity.cpp, effectsDirty block: empty list clears the
			// flag, otherwise it is set from hasEffect(invisibility)), so a bare
			// setInvisible(true) would be wiped the next time any effect is added,
			// removed or expires while the player is still blocking. The effect
			// keeps the flag in sync through the effect system. setInvisible(true)
			// is kept as well for immediacy.
			player->addEffect(new MobEffectInstance(MobEffect::invisibility->id, EFFECT_TICKS, 0));
			player->setInvisible(true);
		}
	}
	return WeaponItem::use(instance, level, player);
}

shared_ptr<ItemInstance> JDemonSword::useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	player->removeEffect(MobEffect::invisibility->id);
	player->setInvisible(false);
	return instance;
}

void JDemonSword::releaseUsing(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player, int durationLeft)
{
	// Blocking ends here when the player lets go early. useTimeDepleted only
	// fires if the full use duration elapses, which is an hour for a sword,
	// so invisibility must be cleared here too or it would stick.
	player->removeEffect(MobEffect::invisibility->id);
	player->setInvisible(false);
}

CMonsterSword::CMonsterSword(int id, const Tier *tier) : WeaponItem(id, tier)
{
}

bool CMonsterSword::hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker)
{
	mob->addEffect(new MobEffectInstance(MobEffect::poison->id, 100, 1));
	return WeaponItem::hurtEnemy(itemInstance, mob, attacker);
}

shared_ptr<ItemInstance> CMonsterSword::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	// neoLegacy - JDemon/CMonster v2: the night-vision + highlight special lasts
	// 30 seconds (EFFECT_TICKS) and then needs a 1-minute cooldown
	// (COOLDOWN_TICKS) before it can be granted again. Same arming as the
	// JDemon sword above (server-side item tag, Horn of Sight pattern):
	// blocking still works on cooldown, it just grants no effects, and the
	// timer is not re-armed by blocking again.
	if (!level->isClientSide)
	{
		int64_t now = level->getGameTime();
		if (!instance->hasTag()) instance->setTag(new CompoundTag());
		CompoundTag *tag = instance->getTag();
		if (now >= tag->getLong(L"CMonsterReady"))
		{
			tag->putLong(L"CMonsterReady", now + COOLDOWN_TICKS);
			// Night vision for the 30-second special, not just 100 ticks (5 seconds):
			// blocking can last up to the full use duration (an hour), and the old
			// effect expired while the player was still blocking.
			player->addEffect(new MobEffectInstance(MobEffect::nightVision->id, EFFECT_TICKS, 0));
			// Enemy highlight: the Horn of Sight scan (HornOfSightItem.cpp), but for
			// EVERY monster, not just illager kin, and with no line-of-sight check —
			// the point is revealing enemies through walls. Server-side only; the
			// glow is applied by the server and synced to clients. One-shot on block
			// start: there is no per-tick hook while an item is being used (Item.h has
			// only use / useTimeDepleted / releaseUsing), so monsters that arrive
			// later are not highlighted until the player blocks again.
			// Amplifier 1 marks this as the CMonster highlight: renderGlow colours
			// amplifier-1 glows blue for players and red for every mob, while the
			// Horn's amplifier-0 glows keep their rainbow/purple/green/red colours.
			AABB *box = player->bb->grow(SCAN_RANGE, 48.0, SCAN_RANGE);
			vector<shared_ptr<Entity> > *found = level->getEntitiesOfClass(typeid(Monster), box);
			if (found != nullptr)
			{
				for (size_t i = 0; i < found->size(); i++)
				{
					shared_ptr<Monster> monster = dynamic_pointer_cast<Monster>(found->at(i));
					if (monster == nullptr || !monster->isAlive()) continue;
					if (monster->distanceToSqr(player) > SCAN_RANGE * SCAN_RANGE) continue;
					monster->addEffect(new MobEffectInstance(MobEffect::glowing->id, EFFECT_TICKS, 1));
				}
				delete found;
			}
			// Players in range glow too, so the CMonster user can tell friend from
			// foe through walls (blue for players, red for mobs — see renderGlow).
			// The user themself is excluded.
			vector<shared_ptr<Entity> > *players = level->getEntitiesOfClass(typeid(Player), box);
			if (players != nullptr)
			{
				for (size_t i = 0; i < players->size(); i++)
				{
					shared_ptr<Player> other = dynamic_pointer_cast<Player>(players->at(i));
					if (other == nullptr || other == player || !other->isAlive()) continue;
					if (other->distanceToSqr(player) > SCAN_RANGE * SCAN_RANGE) continue;
					other->addEffect(new MobEffectInstance(MobEffect::glowing->id, EFFECT_TICKS, 1));
				}
				delete players;
			}
		}
	}
	return WeaponItem::use(instance, level, player);
}

shared_ptr<ItemInstance> CMonsterSword::useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	player->removeEffect(MobEffect::nightVision->id);
	return instance;
}

void CMonsterSword::releaseUsing(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player, int durationLeft)
{
	// Blocking ends here when the player lets go early; useTimeDepleted only
	// fires if the full use duration elapses, which is an hour for a sword.
	player->removeEffect(MobEffect::nightVision->id);
}
