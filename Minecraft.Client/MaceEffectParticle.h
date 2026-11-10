#pragma once
#include "Particle.h"

class ParticleEngine;

// ---------------------------------------------------------------------------------------------------------------------------
// neoLegacy - Mace smash effects, ported from the "Mas Effects" Java mod:
//   * SHOCKWAVE: two flat translucent rings that race out over the ground and fade (a big fast one and a small slow one)
//   * FLASH:     a brief white starburst at the point of impact
//   * SPARK:     two small sparks that flick off near the target
//   * SCRAP:     with Breach, bits of diamond armor fly off a target that is wearing it
// All four draw from one texture (misc/mace_effects.png), which is why they live in their own particle texture list
// (ParticleEngine::MACE_TEXTURE). Spawned on the client by LevelEvent::PARTICLES_MACE_SMASH (see LevelRenderer::levelEvent).
// ---------------------------------------------------------------------------------------------------------------------------
class MaceEffectParticle : public Particle
{
public:
	enum Kind { KIND_SHOCKWAVE = 0, KIND_FLASH, KIND_SPARK, KIND_SCRAP };

	// Tuning for the ring size (1.0 = the mod's default). The big ring ends up several blocks wide, fading as it goes.
	static constexpr float SHOCKWAVE_SIZE = 1.0f;

	virtual eINSTANCEOF GetType() { return eType_MACEEFFECTPARTICLE; }

	MaceEffectParticle(Level *level, double x, double y, double z, int kind, float p1, float p2, float p3, int variant);

	virtual void tick();
	virtual void render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2);
	virtual int getParticleTexture();
	virtual int getLightColor(float a);
	virtual float getBrightness(float a);

	// Spawns the whole effect for one smash. (x, y, z) is the point on the ground under the target.
	static void spawnSmash(ParticleEngine *engine, Level *level, double x, double y, double z, int flags);

private:
	int kind;
	int variant;
	float halfSize, prevHalfSize;		// half-size of the quad, in blocks
	float scaler, sizer, alphaCtrl;	// shockwave growth state
	float time;					// spark fade state
	float baseScale;			// spark size
	int maxAge;
};
