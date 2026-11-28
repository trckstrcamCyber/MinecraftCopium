#pragma once
using namespace std;

#include "Item.h"

class WeaponItem : public Item
{
public:
	enum ZoroSwordType
	{
		ZORO_NONE = 0,
		ZORO_WADO_ICHIMONJI,
		ZORO_SANDAI_KITETSU,
		ZORO_ENMA,
		ZORO_SHUSUI
	};

private:
	float damage;
	const Tier *tier;
	ZoroSwordType zoroType;

public:
	WeaponItem(int id, const Tier *tier, ZoroSwordType zoroType = ZORO_NONE);
	virtual float getTierDamage();
	virtual float getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile);
	virtual bool hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker);
	virtual bool mineBlock(shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, shared_ptr<LivingEntity> owner);
	virtual bool isHandEquipped();
	virtual UseAnim getUseAnimation(shared_ptr<ItemInstance> itemInstance);
	virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance);
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual bool canDestroySpecial(Tile *tile);
	virtual int getEnchantmentValue();

	const Tier *getTier();
	bool isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem);
	attrAttrModMap *getDefaultAttributeModifiers();

	bool isZoroSword() const;
	ZoroSwordType getZoroSwordType() const;
	float getZoroDamageMultiplier(int comboStep) const;
	float getZoroAttackRange() const;
	float getZoroAttackAngle() const;
	int getZoroAttackCooldown() const;
};

// neoLegacy - JDemon sword: sets enemies on fire on hit; holding block turns
// the wielder invisible until they stop blocking.
class JDemonSword : public WeaponItem
{
public:
	JDemonSword(int id, const Tier *tier);
	virtual bool hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker);
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual void releaseUsing(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player, int durationLeft);
};

// neoLegacy - CMonster sword: poisons enemies on hit; holding block grants
// night vision while blocking and highlights nearby monsters (glowing)
// so they show through walls.
class CMonsterSword : public WeaponItem
{
public:
	CMonsterSword(int id, const Tier *tier);
	virtual bool hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker);
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);
	virtual void releaseUsing(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player, int durationLeft);
};