#pragma once
// neoLegacy - client side of the MAC-10: input, "FPS" feel (aim down sights, recoil, hit marker, sprint rules) and the
// first-person view model that plays the Blockbench glTF animations.
//
// One instance lives inside every LocalPlayer (split-screen players each get their own). The gun's game rules are NOT here:
// they live in Mac10Item (World) and run identically on the client (prediction) and the server (authority).
#include <memory>
#include <vector>
#include <string>
#include "GltfModel.h"

class LocalPlayer;
class Minecraft;
class Player;
class ItemInstance;
class PlayerRenderer;

class Mac10Client
{
public:
    Mac10Client();

    // ---- controller (called by LocalPlayer) --------------------------------------------------------------------------
    void tick(LocalPlayer *p);                                  // LocalPlayer::aiStep, once per game tick
    void onMouseDown(LocalPlayer *p, int button, bool down);    // LocalPlayer::handleMouseDown, every tick
    bool onMouseClick(LocalPlayer *p, int button);              // LocalPlayer::handleMouseClick; true = the gun consumed it

    bool isHoldingGun() const { return holding; }
    bool blocksSprint() const;                                  // trigger held / just fired / aiming
    float getFovFactor(float a) const;                          // multiply the world FOV by this (aim down sights zoom)
    float getAimAmount(float a) const;                          // 0 = hip, 1 = fully aimed

    // ---- HUD ---------------------------------------------------------------------------------------------------------
    // false if no gun is held. reserve == -1 means unlimited (creative).
    bool getHud(int &mag, int &capacity, int &reserve, bool &reloading) const;
    float getHitMarker(float a, bool &headshot) const;          // 0..1 fade of the hit marker

    // Diagnostics line for the HUD: what the client's own hit scan saw and what the server reported back (see MAC10_DEBUG_LOG).
    bool getDebugLine(std::shared_ptr<ItemInstance> held, std::wstring &out) const;

    // ---- third person ------------------------------------------------------------------------------------------------
    // PlayerRenderer holds a player with the gun in the vanilla two-handed aim pose (arms follow the head, correct for every skin) and calls
    // renderGunInHand with the matrix already at the right hand. The gun is the glTF gun parts only; it plays the fire / auto_fire clip and the
    // muzzle flash whenever that player's magazine goes down. Returns false if the model could not be loaded (the vanilla item is drawn instead).
    static bool isModelAvailable(std::shared_ptr<ItemInstance> item);
    static bool renderGunInHand(std::shared_ptr<Player> player, float a);

    // ---- view model --------------------------------------------------------------------------------------------------
    // Draws the gun and arms in camera space. Returns false if the model could not be loaded (caller draws the normal icon).
    bool render(Minecraft *mc, std::shared_ptr<Player> player, float a, float swapHeight, float brightness, float fudgeX, float fudgeY);

private:
    enum Clip { CLIP_DRAW, CLIP_PUT_AWAY, CLIP_IDLE_BOLTOUT, CLIP_IDLE_EMPTY, CLIP_RELOAD_EMPTY, CLIP_RELOAD, CLIP_INSPECT, CLIP_FIRE, CLIP_AUTO_FIRE, CLIP_COUNT };

    // controller
    bool tryFire(LocalPlayer *p, std::shared_ptr<ItemInstance> held);
    void sendAction(LocalPlayer *p, std::shared_ptr<ItemInstance> held, int action, bool aiming);
    void shotEffects(LocalPlayer *p, bool automatic, bool hit, bool headshot, bool killed);
    void beginReloadLocal(LocalPlayer *p, std::shared_ptr<ItemInstance> held, bool wasEmpty);
    void resetTransient();

    // view model
    void playClip(int clip, float speed, bool loop);
    void tickViewModel(LocalPlayer *p);
    void buildPose(GltfModel::Pose &pose, float a) const;
    void reloadTuning();

    // ---- state ----
    bool holding, wasHolding;
    int  heldSlot;
    bool fireHeld;
    int  fireBuffer;          // a click is remembered for a few ticks so it is not lost while the gun is cooling down
    int  sprintOut;           // ticks until the gun is up after a sprint
    int  sinceShot;
    int  dryCooldown;
    bool middleLatch;

    int  mag, capacity, reserve;
    bool reloading, autoFire;

    float ads, adsPrev;
    float sprintBlend, sprintBlendPrev;
    float recoilAccum;
    int   hitTicks, hitTicksPrev;
    bool  hitHeadshot;
    int   dbgCandidates;      // last client-side hit scan (diagnostics)
    float dbgDistance;
    bool  dbgHit;

    // view model
    int   clip;
    float clipTime, clipSpeed;
    bool  clipLoop;
    float kick, kickPrev;
    float swayYaw, swayPitch, swayYawPrev, swayPitchPrev;
    float bobPhase, bobPhasePrev, bobAmp, bobAmpPrev;
    float reloadSeconds;      // length of the running reload in seconds (sound cues)
    bool  cueMagOut, cueMagIn, cueBolt;
    int   configTimer;
    int   curKind;            // GunKind of the gun that is held (picks the model, clip names and tuning)
    bool  burstPending;       // a burst weapon is part-way through its rounds
    float clipVirtualDur;     // length of a procedural (model-less) draw / reload, in seconds

    struct Tuning
    {
        float hipX, hipY, hipZ, hipScale, hipYaw, hipPitch;
        float adsX, adsY, adsZ, adsScale, adsFov;
        float drawSpeed;
        int reloadKey, inspectKey;     // virtual-key codes; letters are their upper-case character
        Tuning(int kind = 0);
    } tuning;
};
