#pragma once

// neoLegacy - how a crossbow lies in the hand.

// ---- third person (mobs and players) -------------------------------------------------------------------------------------
// The crossbow sprite is rolled this many degrees about its own long diagonal so it lies on its side with the muzzle pointing forward.
// If it comes out upside down use -90.0f; to try a different roll use any angle.
#define CROSSBOW_ROLL_DEGREES -90.0f

// ---- Pillager (third person) ---------------------------------------------------------------------------------------------
// A Pillager holds its crossbow like Java: flat on its side, the muzzle pointing the way the arms point, the stock in the main hand.
// The hold is fixed to the arm, so it follows both arm poses (raised and aimed, and lowered across the body while it draws the string).
// The base pose is worked out in HumanoidMobRenderer; these knobs are applied on top of it, in the hand's frame:
//   FORWARD + slides it toward the muzzle (along the arm)    LEFT + slides it to the creature's left    UP + lifts it
//   YAW + turns the muzzle left     PITCH + raises the muzzle     ROLL + rolls it clockwise seen from behind
#define CROSSBOW_TP_FORWARD 0.0f
#define CROSSBOW_TP_LEFT    0.0f
#define CROSSBOW_TP_UP      0.0f
#define CROSSBOW_TP_YAW     0.0f
#define CROSSBOW_TP_PITCH   0.0f
#define CROSSBOW_TP_ROLL    0.0f

// ---- first person --------------------------------------------------------------------------------------------------------
// ItemInHandRenderer::render() lays the crossbow flat on its side, stock at the lower right of the screen, muzzle pointing forward
// and a little up toward the crosshair (the same for the empty, drawing and loaded crossbow). The base pose is fixed in the renderer;
// the knobs below are applied on top of it, in screen directions, so you can nudge it without touching any maths.
//   NUDGE_X  + moves it right       NUDGE_Y  + moves it up        NUDGE_Z  + moves it toward you      (screen units, about 0.1 = a hand's width)
//   YAW      + turns the muzzle left     PITCH  + raises the muzzle     ROLL  + rolls it counter-clockwise seen from behind (180 flips it over)
// All the rotations pivot around the lower right of the screen where the stock sits.
#define CROSSBOW_FP_NUDGE_X 0.0f
#define CROSSBOW_FP_NUDGE_Y 0.0f
#define CROSSBOW_FP_NUDGE_Z 0.0f
#define CROSSBOW_FP_YAW     0.0f
#define CROSSBOW_FP_PITCH   0.0f
#define CROSSBOW_FP_ROLL    0.0f

// While the string is drawn the crossbow slides this far toward you and toward the middle of the screen (screen units).
#define CROSSBOW_FP_PULL_BACK 0.07f
#define CROSSBOW_FP_PULL_LEFT 0.03f

// The held item is drawn in a frame that is yawed 45 degrees and scaled by 0.4 (ItemInHandRenderer::render). This turns a movement in
// screen directions (x right, y up, z toward the player) into that frame.
inline void crossbowViewToHeldFrame(float dx, float dy, float dz, float &lx, float &ly, float &lz)
{
	const float c = 0.70710678f;
	lx = (c * dx - c * dz) / 0.4f;
	ly = dy / 0.4f;
	lz = (c * dx + c * dz) / 0.4f;
}
