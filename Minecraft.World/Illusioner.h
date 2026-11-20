#pragma once
#include "SpellcasterIllager.h"
#include "RangedAttackMob.h"

// neoLegacy - Illusioner: the illusionist illager. 32 health, shoots a bow and casts two spells:
//   * mirror image - turns invisible for a minute; four copies of it dance around where it stands (client side, see IllusionerRenderer),
//   * blindness    - blinds its target (once per target).
class Illusioner : public SpellcasterIllager, public RangedAttackMob
{
public:
	eINSTANCEOF GetType() { return eTYPE_ILLUSIONER; }
	static Entity *create(Level *level) { return new Illusioner(level); }

	// client-only: where the four mirror images stand relative to the real position (previous / current set)
	int clientSideIllusionTicks;
	double illusionOffsets[2][4][3];

	Illusioner(Level *level);

	virtual void aiStep();
	virtual int getCastingSoundId();
	virtual ArmPose getArmPose();
	virtual void performRangedAttack(shared_ptr<LivingEntity> target, float power);
	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0);
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

	// where to draw the four images this frame (interpolated between the previous and current offsets)
	void getRenderLocations(float a, double out[4][3]);

protected:
	virtual void registerAttributes();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();
};
