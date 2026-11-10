#pragma once
using namespace std;

#include "Item.h"

class Player;
class LivingEntity;
class Enchantment;

// ---------------------------------------------------------------------------------------------------------------------------
// neoLegacy - Mace (Java 1.21 "Tricky Trials").
//
//   * 5 attack damage on top of the player's 1 = 6 base damage.
//   * SMASH ATTACK: hitting something while falling more than 1.5 blocks (and not gliding on an elytra) adds bonus damage per
//     block fallen - +4 for the first 3 blocks, +2 for the next 5, +1 for every block after that - negates the fall damage and
//     sends a knockback shockwave out around the target (radius 3.5 blocks, twice as strong after a fall of more than 5).
//   * Density (I-V): +0.5 smash damage per block fallen, per level.        (MaceEnchantments.h)
//   * Breach (I-IV): ignores 15% of the target's armor per level.          (hooked in LivingEntity::getDamageAfterArmorAbsorb)
//   * Wind Burst (I-III): launches the attacker back up after a smash, and the fall back down does not hurt.
//
// The numbers are straight from Java's MaceItem. Player::attack calls the static helpers below, so every piece of the
// smash lives in this one file and the engine hooks stay tiny.
// ---------------------------------------------------------------------------------------------------------------------------
class MaceItem : public Item
{
public:
	static constexpr int   DURABILITY = 500;
	static constexpr float ATTACK_DAMAGE = 5.0f;			// + the player's base 1.0 = 6
	static constexpr float SMASH_FALL_THRESHOLD = 1.5f;		// blocks of fall needed for a smash attack
	static constexpr float HEAVY_SMASH_THRESHOLD = 5.0f;	// fall distance that makes the smash "heavy"
	static constexpr double KNOCKBACK_RADIUS = 3.5;
	static constexpr float KNOCKBACK_POWER = 0.7f;

	// Bits of the data sent with LevelEvent::PARTICLES_MACE_SMASH (read by the client's MaceEffectParticle::spawnSmash)
	static const int SMASH_FLAG_HEAVY = 1;			// fell more than 5 blocks
	static const int SMASH_FLAG_BREACH = 2;			// the weapon has Breach: show armor scraps
	static const int SMASH_FLAG_ARMOR_SHIFT = 2;		// bit (2 + slot) set when the target wears diamond armor in that slot (0 = boots ... 3 = helmet)

	MaceItem(int id);

	virtual bool hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker);
	virtual bool mineBlock(shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, shared_ptr<LivingEntity> owner);
	virtual bool isHandEquipped();
	virtual int getEnchantmentValue();
	virtual bool isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem);
	virtual attrAttrModMap *getDefaultAttributeModifiers();

	// ---- helpers used by Player::attack ----------------------------------------------------------------------------------
	static bool isMace(shared_ptr<ItemInstance> item);

	// The ONLY enchantments a Mace can have: Density, Breach, Smite, Bane of Arthropods, Wind Burst, Fire Aspect, Unbreaking,
	// Mending and Curse of Vanishing. Enchantment::canEnchant and the enchanting-table roll both check this.
	static bool isEnchantmentAllowed(Enchantment *e);

	// Falling far enough to smash? (a player gliding on an elytra never smashes)
	static bool canSmashAttack(shared_ptr<LivingEntity> attacker);

	// Bonus damage for a smash after falling "fallDistance" blocks, including the Density enchantment of the held mace.
	static float getSmashBonusDamage(float fallDistance, shared_ptr<ItemInstance> mace);

	// Server only: the shockwave, sound and dust that go with a smash hit on "target".
	static void doSmashEffects(shared_ptr<Player> attacker, shared_ptr<LivingEntity> target, float fallDistance);

	// Self effects of a smash, for the attacker. Run on BOTH client and server (the client owns its own movement):
	// negates the fall damage and, with Wind Burst, throws the attacker back into the air.
	static void applySmashToAttacker(shared_ptr<Player> attacker, shared_ptr<ItemInstance> mace);

	// Launch speed (blocks/tick) for a Wind Burst level. Tuned for this engine's gravity and drag (about 7, 11 and 17 blocks high).
	static float getWindBurstLaunchSpeed(int level);
};
