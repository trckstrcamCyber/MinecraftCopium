#include "stdafx.h"
#include "../Minecraft.World/Mth.h"
#include "../Minecraft.World/Random.h"
#include "../Minecraft.World/JavaMath.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "../Minecraft.World/net.minecraft.world.item.h"
#include "../Minecraft.World/MaceItem.h"
#include "MaceEffectParticle.h"
#include "ParticleEngine.h"
#include "Tesselator.h"

// Texture atlas layout (misc/mace_effects.png, 256 x 256): shockwave (0,0,90x90), flash (96,0,90x90), sparks (192,0) and (192,32) 32x32,
// diamond scraps (192,64) (208,64) (224,64) 9x9.
static void maceUV(int kind, int variant, float &u0, float &v0, float &u1, float &v1)
{
	float x = 0, y = 0, w = 90, h = 90;
	switch (kind)
	{
	case MaceEffectParticle::KIND_SHOCKWAVE:	x = 0;		y = 0;		w = 90;	h = 90;	break;
	case MaceEffectParticle::KIND_FLASH:		x = 96;		y = 0;		w = 90;	h = 90;	break;
	case MaceEffectParticle::KIND_SPARK:		x = 192;	y = (variant & 1) ? 32.0f : 0.0f;	w = 32;	h = 32;	break;
	case MaceEffectParticle::KIND_SCRAP:		x = 192 + 16.0f * (variant % 3);	y = 64;	w = 9;	h = 9;	break;
	}
	u0 = x / 256.0f;
	v0 = y / 256.0f;
	u1 = (x + w) / 256.0f;
	v1 = (y + h) / 256.0f;
}

MaceEffectParticle::MaceEffectParticle(Level *level, double x, double y, double z, int kind, float p1, float p2, float p3, int variant)
	: Particle(level, x, y, z, 0, 0, 0), kind(kind), variant(variant)
{
	xd = yd = zd = 0;
	gravity = 0;
	noPhysics = true;
	rCol = gCol = bCol = 1.0f;
	alpha = 0.999f;		// always start translucent so ParticleEngine::add files us in the translucent list
	halfSize = prevHalfSize = 0.5f;
	scaler = sizer = alphaCtrl = time = baseScale = 0;
	maxAge = 20;

	switch (kind)
	{
	case KIND_SHOCKWAVE:
		// p1 = growth, p2 = opacity, p3 = size multiplier (the Java mod's xSpeed / ySpeed / zSpeed)
		scaler = p1;
		alphaCtrl = p2;
		sizer = p3 * SHOCKWAVE_SIZE;
		alpha = alphaCtrl > 0.999f ? 0.999f : alphaCtrl;
		maxAge = 40;
		break;
	case KIND_FLASH:
		maxAge = 4;
		alpha = 0.6f;
		halfSize = prevHalfSize = 0.0f;
		break;
	case KIND_SPARK:
		baseScale = p1;
		maxAge = 20;
		alpha = 0.999f;
		halfSize = prevHalfSize = baseScale;
		break;
	case KIND_SCRAP:
		// p1/p2/p3 = outward velocity
		xd = p1 * 0.25f;
		yd = p2 * 0.25f + 0.1f;
		zd = p3 * 0.25f;
		gravity = 1.0f;
		noPhysics = false;
		maxAge = 20;
		halfSize = prevHalfSize = 0.35f;
		break;
	}
	lifetime = maxAge;
}

void MaceEffectParticle::tick()
{
	prevHalfSize = halfSize;

	switch (kind)
	{
	case KIND_SHOCKWAVE:
		halfSize += scaler * sizer;
		scaler -= 1.0f / maxAge;
		alphaCtrl -= 1.0f / maxAge;
		if (scaler < 0) scaler = 0;
		if (scaler > 1) scaler = 1;
		alpha = alphaCtrl < 0 ? 0 : (alphaCtrl > 0.999f ? 0.999f : alphaCtrl);
		if (age++ >= maxAge) remove();
		break;

	case KIND_FLASH:
		if (age++ >= maxAge) remove();
		break;

	case KIND_SPARK:
		{
			float a = (1.0f - time) * 1.5f;
			if (a < 0.2f) a = 0.2f;
			if (a > 1.0f) a = 1.0f;
			alpha = a - 0.2f;
			time += 1.0f / maxAge;
			halfSize = alpha * baseScale;
			if (age++ >= maxAge) remove();
		}
		break;

	case KIND_SCRAP:
		Particle::tick();		// gravity, movement and ageing
		halfSize = (1.0f - static_cast<float>(age) / maxAge) * 0.35f;
		break;
	}
}

void MaceEffectParticle::render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2)
{
	float u0, v0, u1, v1;
	maceUV(kind, variant, u0, v0, u1, v1);

	float px = static_cast<float>(xo + (this->x - xo) * a - xOff);
	float py = static_cast<float>(yo + (this->y - yo) * a - yOff);
	float pz = static_cast<float>(zo + (this->z - zo) * a - zOff);

	float r = prevHalfSize + (halfSize - prevHalfSize) * a;	// half-size
	float alphaNow = alpha;

	if (kind == KIND_FLASH)
	{
		// vanilla firework flash: size 7.1 * sin(...), alpha fading from 0.6
		float f = (age + a - 1.0f) * 0.25f;
		r = 7.1f * Mth::sin(f * PI);
		if (r < 0) r = 0;
		alphaNow = 0.6f - (age + a - 1.0f) * 0.25f * 0.5f;
		if (alphaNow < 0) alphaNow = 0;
	}
	if (r <= 0.0f || alphaNow <= 0.0f) return;

	t->color(rCol, gCol, bCol, alphaNow);

	if (kind == KIND_SHOCKWAVE)
	{
		// flat on the ground, drawn from both sides so it is visible from above and below
		t->vertexUV(px - r, py, pz - r, u0, v1);
		t->vertexUV(px - r, py, pz + r, u0, v0);
		t->vertexUV(px + r, py, pz + r, u1, v0);
		t->vertexUV(px + r, py, pz - r, u1, v1);

		t->vertexUV(px + r, py, pz - r, u1, v1);
		t->vertexUV(px + r, py, pz + r, u1, v0);
		t->vertexUV(px - r, py, pz + r, u0, v0);
		t->vertexUV(px - r, py, pz - r, u0, v1);
		return;
	}

	// camera-facing billboard (same corner order as Particle::render)
	t->vertexUV(px - xa * r - xa2 * r, py - ya * r, pz - za * r - za2 * r, u1, v1);
	t->vertexUV(px - xa * r + xa2 * r, py + ya * r, pz - za * r + za2 * r, u1, v0);
	t->vertexUV(px + xa * r + xa2 * r, py + ya * r, pz + za * r + za2 * r, u0, v0);
	t->vertexUV(px + xa * r - xa2 * r, py - ya * r, pz + za * r - za2 * r, u0, v1);
}

int MaceEffectParticle::getParticleTexture()
{
	return ParticleEngine::MACE_TEXTURE;
}

int MaceEffectParticle::getLightColor(float a)
{
	// flash and sparks glow; the rings and scraps take the light of the place they are in
	if (kind == KIND_FLASH || kind == KIND_SPARK) return 15 << 4 | 15 << 20;
	return Particle::getLightColor(a);
}

float MaceEffectParticle::getBrightness(float a)
{
	if (kind == KIND_FLASH || kind == KIND_SPARK) return 1.0f;
	return Particle::getBrightness(a);
}

void MaceEffectParticle::spawnSmash(ParticleEngine *engine, Level *level, double x, double y, double z, int flags)
{
	if (engine == nullptr || level == nullptr) return;
	Random *rnd = level->random;

	// two shockwaves: big and fast, small and slower (the mod's values)
	engine->add(std::make_shared<MaceEffectParticle>(level, x, y + 0.02, z, KIND_SHOCKWAVE, 0.8f, 0.8f, 1.5f, 0));
	engine->add(std::make_shared<MaceEffectParticle>(level, x, y + 0.03, z, KIND_SHOCKWAVE, 0.4f, 1.0f, 0.35f, 0));

	// two sparks flicking off a random spot near the target
	double rx = (rnd->nextFloat() * 2 - 1) * 0.6;
	double ry = (rnd->nextFloat() * 2 - 1) * 1.2;
	double rz = (rnd->nextFloat() * 2 - 1) * 0.6;
	engine->add(std::make_shared<MaceEffectParticle>(level, x + rx, y + ry + 0.9, z + rz, KIND_SPARK, 0.5f, 0, 0, rnd->nextInt(2)));
	engine->add(std::make_shared<MaceEffectParticle>(level, x + rx, y + ry + 0.9, z + rz, KIND_SPARK, 0.2f, 0, 0, rnd->nextInt(2)));

	// the flash
	engine->add(std::make_shared<MaceEffectParticle>(level, x, y + 0.5, z, KIND_FLASH, 0, 0, 0, 0));

	// Breach: diamond armor chips off. 0-2 scraps per diamond piece, spread over that piece's slice of the body (boots 0 ... helmet 3).
	if (flags & MaceItem::SMASH_FLAG_BREACH)
	{
		for (int slot = 0; slot < 4; ++slot)
		{
			if (!(flags & (1 << (MaceItem::SMASH_FLAG_ARMOR_SHIFT + slot)))) continue;
			int count = rnd->nextInt(3);
			for (int i = 0; i < count; ++i)
			{
				double ox = (rnd->nextFloat() - 0.5) * 0.6;
				double oy = rnd->nextFloat() * 0.45;
				double oz = (rnd->nextFloat() - 0.5) * 0.6;
				engine->add(std::make_shared<MaceEffectParticle>(level, x + ox, y + slot * 0.45 + oy, z + oz, KIND_SCRAP,
					static_cast<float>(ox), static_cast<float>(oy), static_cast<float>(oz), rnd->nextInt(3)));
			}
		}
	}
}
