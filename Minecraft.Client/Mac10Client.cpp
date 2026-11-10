#include "stdafx.h"
#include "Mac10Client.h"
#include "LocalPlayer.h"
#include "Minecraft.h"
#include "MultiPlayerGameMode.h"
#include "Input.h"
#include "Tesselator.h"
#include "Textures.h"
#include "ResourceLocation.h"
#include "Lighting.h"
#include "EntityRenderer.h"
#include "EntityRenderDispatcher.h"
#include "PlayerRenderer.h"
#include "MultiPlayerLocalPlayer.h"
#include "Common/Audio/SoundEngine.h"
#include "../Minecraft.World/Mac10Item.h"
#include "../Minecraft.World/net.minecraft.world.item.h"
#include "../Minecraft.World/net.minecraft.world.entity.player.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "../Minecraft.World/SoundTypes.h"
#include "../Minecraft.World/Mth.h"
#ifdef _WINDOWS64
#include "Windows64/KeyboardMouseInput.h"
#endif
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>

// ===================================================================================================================
// Shared, lazily loaded model
// ===================================================================================================================
namespace
{
    // One loaded model per MODEL (not per gun): the three flintlock variants share one model and differ only in their texture.
    struct GunModel
    {
        GltfModel model;
        bool tried = false, ok = false;
        std::string folder;
        int clip[9];                       // Mac10Client::Clip -> index in the glTF, -1 if the clip is missing
        int baseClip = -1;                 // the clip whose pose is the "ready" hold pose (-1: the end of the draw clip)
        int nodeMuzzle = -1, nodeAmmo = -1, nodeNoAmmo = -1, nodeBullet = -1, nodeRight = -1, nodeLeft = -1;
        int materialRole[32];              // 0 gun, 1 bullet, 2 muzzle flash, 3 arms (player skin)
    };
    struct GunModelDef
    {
        const char *folderName, *file, *cfg;
        const char *clips[9];              // draw, put_away, idle_boltout, idle_empty, reload_empty, reload, inspect, fire, auto_fire
        const char *baseClip;              // null = the end of the draw clip
        const char *muzzleNode;            // group whose scale hides / shows the muzzle flash (null = the model has none)
    };
    const GunModelDef MODEL_DEFS[3] =
    {
        { "mac10", "mac-10.gltf", "mac10_view.cfg",
          { "animation.mac10.draw", "animation.mac10.put_away", "animation.mac10.idle_boltout", "animation.mac10.idle_empty",
            "animation.mac10.reload_empty", "animation.mac10.reload", "animation.mac10.inspect", "animation.mac10.fire", "animation.mac10.auto_fire" },
          nullptr, "muzzle_flash" },
        // The M16 has one reload clip (used for both), no draw / inspect (the draw is a procedural lift, see Mac10Client::render).
        { "m16", "m16.gltf", "m16_view.cfg",
          { nullptr, nullptr, nullptr, nullptr, "animation.gun_m16.reload", "animation.gun_m16.reload", nullptr, "animation.gun_m16.fire", "animation.gun_m16.auto_fire" },
          "animation.gun_m16.idle", "muzzleflash" },
        // The flintlock has a shot, an idle, an empty idle and an inspect; draw and reload are procedural. (Its hammer is animated by the shot clip.)
        { "flintlock", "flintlock-pistol.gltf", "flintlock_view.cfg",
          { nullptr, nullptr, nullptr, "animation.flintlock_idle_empty", nullptr, nullptr, "animation.flintlock_inspect", "animation.flintlock_shot", "animation.flintlock_shot" },
          "animation.flintlock_idle", nullptr },
    };
    GunModel g_models[3];
    GunModel *G = &g_models[0];            // the model of the gun being processed (set by selectKind)
    int g_kind = 0;

    int modelIndexFor(int kind) { return kind == GUN_MAC10 ? 0 : (kind == GUN_M16 ? 1 : 2); }

    ResourceLocation GUN_TEXTURE(TN_ITEM_MAC10_GUN);
    ResourceLocation BULLET_TEXTURE(TN_ITEM_MAC10_BULLET);
    ResourceLocation FLASH_TEXTURE(TN_ITEM_MAC10_FLASH);
    ResourceLocation M16_GUN_TEXTURE(TN_ITEM_M16_GUN);
    ResourceLocation M16_FLASH_TEXTURE(TN_ITEM_M16_FLASH);
    ResourceLocation FLINTLOCK_TEXTURES[3] = { ResourceLocation(TN_ITEM_FLINTLOCK_COPPER), ResourceLocation(TN_ITEM_FLINTLOCK_IRON), ResourceLocation(TN_ITEM_FLINTLOCK_GOLD) };

    // role: 0 gun, 1 bullet, 2 muzzle flash. nullptr = this gun has no such part.
    ResourceLocation *textureFor(int kind, int role)
    {
        if (kind == GUN_MAC10) return role == 0 ? &GUN_TEXTURE : (role == 1 ? &BULLET_TEXTURE : &FLASH_TEXTURE);
        if (kind == GUN_M16) return role == 0 ? &M16_GUN_TEXTURE : (role == 2 ? &M16_FLASH_TEXTURE : nullptr);
        return role == 0 ? &FLINTLOCK_TEXTURES[kind - GUN_FLINTLOCK_COPPER] : nullptr;
    }

    // Rig-space points per model: the grip (hip pivot) and the rear sight (aim pivot).
    struct Anchors { float gx, gy, gz, sx, sy, sz; };
    const Anchors ANCHORS[3] =
    {
        { 0.25f, 0.10f, 0.00f,  0.25f, 0.470f, 0.13f },     // MAC-10
        { 0.45f, 0.05f, 0.25f,  0.44f, 0.340f, 1.05f },     // M16 (a big, stylised rifle: about 3.3 rig units long)
        { -0.02f, 0.10f, -0.05f, -0.02f, 0.216f, 0.15f },   // flintlock pistols
    };

    bool contains(const std::string &s, const char *needle) { return s.find(needle) != std::string::npos; }

    // The game runs with the exe's folder as working directory, and resources are plain files below Common\res\TitleUpdate\res
    // (see DefaultTexturePack::getResourceImplementation). A model is looked up in the same place, then in Common\res\models.
    bool ensureModel()
    {
        GunModel &m = *G;
        if (m.tried) return m.ok;
        m.tried = true;
        const GunModelDef &def = MODEL_DEFS[(int)(G - g_models)];
        std::string error;
        const char *patterns[4] = { "Common\\res\\TitleUpdate\\res\\models\\%s\\", "Common\\res\\models\\%s\\", "Common/res/TitleUpdate/res/models/%s/", "Common/res/models/%s/" };
        for (int i = 0; i < 4 && !m.ok; ++i)
        {
            char folder[160];
            snprintf(folder, sizeof(folder), patterns[i], def.folderName);
            if (m.model.loadFromFile(std::string(folder) + def.file, error)) { m.ok = true; m.folder = folder; }
        }
        if (!m.ok)
        {
            app.DebugPrintf("%s view model: could not load %s (%s). The gun will be drawn as a flat icon.\n", def.folderName, def.file, error.c_str());
            return false;
        }
        for (int i = 0; i < 9; ++i) m.clip[i] = def.clips[i] != nullptr ? m.model.findClip(def.clips[i]) : -1;
        m.baseClip = def.baseClip != nullptr ? m.model.findClip(def.baseClip) : -1;
        auto groupNode = [&](const char *name) -> int
        {
            if (name == nullptr) return -1;
            for (size_t i = 0; i < m.model.nodes.size(); ++i)
                if (m.model.nodes[i].name == name && m.model.nodes[i].mesh < 0) return (int)i;   // the group, not the mesh
            return -1;
        };
        m.nodeMuzzle = groupNode(def.muzzleNode);
        m.nodeBullet = groupNode("bullet");
        m.nodeAmmo = groupNode("ammo");
        m.nodeNoAmmo = groupNode("no_ammo");
        m.nodeRight = m.model.findNode("right");
        m.nodeLeft = m.model.findNode("left");
        for (int i = 0; i < 32; ++i)
        {
            std::string n = m.model.materialTextureName(i);
            int role = 0;
            if (contains(n, "steve") || contains(n, "alex") || contains(n, "skin") || contains(n, "arm")) role = 3;
            else if (contains(n, "flash") || contains(n, "muzzle")) role = 2;
            else if (contains(n, "bullet")) role = 1;
            m.materialRole[i] = role;
        }
        app.DebugPrintf("%s view model: loaded %d nodes, %d clips from %s\n", def.folderName, (int)m.model.nodes.size(), (int)m.model.clips.size(), m.folder.c_str());
        return true;
    }

    // Makes `kind`'s model the current one. Every public entry point calls this before touching G.
    bool selectKind(int kind)
    {
        if (kind < 0 || kind >= GUN_KIND_COUNT) kind = GUN_MAC10;
        g_kind = kind;
        G = &g_models[modelIndexFor(kind)];
        return ensureModel();
    }

    // ---- 4x4 helpers (row-major, m[row*4+col]) ----
    void identity(float m[16]) { memset(m, 0, 16 * sizeof(float)); m[0] = m[5] = m[10] = m[15] = 1.0f; }
    void mul(const float a[16], const float b[16], float out[16])
    {
        float r[16];
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                r[i * 4 + j] = a[i * 4 + 0] * b[0 * 4 + j] + a[i * 4 + 1] * b[1 * 4 + j] + a[i * 4 + 2] * b[2 * 4 + j] + a[i * 4 + 3] * b[3 * 4 + j];
        memcpy(out, r, sizeof(r));
    }
    void translation(float x, float y, float z, float m[16]) { identity(m); m[3] = x; m[7] = y; m[11] = z; }
    void uniformScale(float s, float m[16]) { identity(m); m[0] = m[5] = m[10] = s; }
    void rotationX(float deg, float m[16]) { float a = deg * 3.14159265f / 180.0f, c = cosf(a), s = sinf(a); identity(m); m[5] = c; m[6] = -s; m[9] = s; m[10] = c; }
    void rotationY(float deg, float m[16]) { float a = deg * 3.14159265f / 180.0f, c = cosf(a), s = sinf(a); identity(m); m[0] = c; m[2] = s; m[8] = -s; m[10] = c; }
    void rotationZ(float deg, float m[16]) { float a = deg * 3.14159265f / 180.0f, c = cosf(a), s = sinf(a); identity(m); m[0] = c; m[1] = -s; m[4] = s; m[5] = c; }

    float lerpf(float a, float b, float t) { return a + (b - a) * t; }
    float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
    float smooth(float t) { t = clamp01(t); return t * t * (3.0f - 2.0f * t); }
    float approach(float v, float target, float step) { return v < target ? (v + step > target ? target : v + step) : (v - step < target ? target : v - step); }
    float frand(float range) { return ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * range; }

    // Applies a rigid(+uniform scale) row-major matrix to the GL matrix stack using only glTranslatef / glRotatef / glScalef.
    void applyMatrixToGL(const float m[16])
    {
        float sx = sqrtf(m[0] * m[0] + m[4] * m[4] + m[8] * m[8]);
        if (sx < 1e-6f) sx = 1.0f;
        float r00 = m[0] / sx, r01 = m[1] / sx, r02 = m[2] / sx;
        float r10 = m[4] / sx, r11 = m[5] / sx, r12 = m[6] / sx;
        float r20 = m[8] / sx, r21 = m[9] / sx, r22 = m[10] / sx;
        float c = (r00 + r11 + r22 - 1.0f) * 0.5f;
        if (c > 1.0f) c = 1.0f;
        if (c < -1.0f) c = -1.0f;
        float angle = acosf(c);
        float ax = 0.0f, ay = 1.0f, az = 0.0f;
        if (angle > 1e-4f)
        {
            if (angle > 3.14159265f - 1e-3f)
            {
                ax = sqrtf((r00 + 1.0f) * 0.5f > 0.0f ? (r00 + 1.0f) * 0.5f : 0.0f);
                ay = sqrtf((r11 + 1.0f) * 0.5f > 0.0f ? (r11 + 1.0f) * 0.5f : 0.0f);
                az = sqrtf((r22 + 1.0f) * 0.5f > 0.0f ? (r22 + 1.0f) * 0.5f : 0.0f);
                if (r01 + r10 < 0.0f) ay = -ay;
                if (r02 + r20 < 0.0f) az = -az;
            }
            else
            {
                float s2 = 2.0f * sinf(angle);
                ax = (r21 - r12) / s2; ay = (r02 - r20) / s2; az = (r10 - r01) / s2;
            }
        }
        glTranslatef(m[3], m[7], m[11]);
        if (angle > 1e-4f) glRotatef(angle * 180.0f / 3.14159265f, ax, ay, az);
        glScalef(sx, sx, sx);
    }

    float readCfgNumber(const char *line, const char *key, float current)
    {
        size_t n = strlen(key);
        if (strncmp(line, key, n) == 0 && line[n] == '=') return (float)atof(line + n + 1);
        return current;
    }

    // "key_reload=G" (a single letter / digit) or a raw virtual-key code such as "key_reload=114" (F3).
    int readCfgKey(const char *line, const char *key, int current)
    {
        size_t n = strlen(key);
        if (strncmp(line, key, n) != 0 || line[n] != '=') return current;
        const char *v = line + n + 1;
        while (*v == ' ' || *v == '\t') ++v;
        if (*v >= '0' && *v <= '9' && (v[1] == '\0' || v[1] == '\n' || v[1] == '\r')) return *v;                // a single digit key
        if (*v >= '0' && *v <= '9') return atoi(v);                                                                // numeric virtual-key code
        if (*v >= 'a' && *v <= 'z') return *v - 'a' + 'A';
        if (*v >= 'A' && *v <= 'Z') return *v;
        return current;
    }
}

// Defaults computed from each model's real geometry (see the README); every number can be overridden in <gun>_view.cfg while the game runs.
Mac10Client::Tuning::Tuning(int kind)
    : hipX(0.32f), hipY(-0.36f), hipZ(-0.60f), hipScale(0.62f), hipYaw(-3.0f), hipPitch(0.0f),
      adsX(0.0f), adsY(-0.12f), adsZ(-0.40f), adsScale(0.70f), adsFov(0.84f), drawSpeed(1.5f), reloadKey('G'), inspectKey('F')
{
    adsFov = Mac10Item::statsFor(kind).adsZoom;
    if (kind == GUN_M16)
    {
        hipX = 0.30f; hipY = -0.30f; hipZ = -0.62f; hipScale = 0.26f; hipYaw = -3.0f;
        adsX = 0.0f;  adsY = -0.085f; adsZ = -0.34f; adsScale = 0.34f;
    }
    else if (kind >= GUN_FLINTLOCK_COPPER)
    {
        hipX = 0.26f; hipY = -0.26f; hipZ = -0.55f; hipScale = 1.15f; hipYaw = -3.0f;
        adsX = 0.0f;  adsY = -0.075f; adsZ = -0.34f; adsScale = 1.50f;
    }
}

Mac10Client::Mac10Client()
    : holding(false), wasHolding(false), heldSlot(-1), fireHeld(false), fireBuffer(0), sprintOut(0), sinceShot(1000), dryCooldown(0), middleLatch(false),
      mag(0), capacity(Mac10Item::BASE_MAGAZINE), reserve(0), reloading(false), autoFire(false),
      ads(0), adsPrev(0), sprintBlend(0), sprintBlendPrev(0), recoilAccum(0), hitTicks(0), hitTicksPrev(0), hitHeadshot(false), dbgCandidates(0), dbgDistance(-1.0f), dbgHit(false),
      clip(-1), clipTime(0), clipSpeed(1), clipLoop(false), kick(0), kickPrev(0),
      swayYaw(0), swayPitch(0), swayYawPrev(0), swayPitchPrev(0), bobPhase(0), bobPhasePrev(0), bobAmp(0), bobAmpPrev(0),
      reloadSeconds(2.2f), cueMagOut(false), cueMagIn(false), cueBolt(false), configTimer(0), curKind(0), burstPending(false), clipVirtualDur(0.0f)
{
}

// Optional hot-reloadable tuning file: models\mac10\mac10_view.cfg, "key=value" per line. Re-read every ~2 seconds while the gun is
// held, so the hip / aim-down-sights placement can be adjusted without rebuilding the game.
void Mac10Client::reloadTuning()
{
    if (G->folder.empty()) return;
    std::string path = G->folder + MODEL_DEFS[(int)(G - g_models)].cfg;
    FILE *f = fopen(path.c_str(), "r");
    if (!f) return;
    Tuning t(curKind);
    char line[256];
    while (fgets(line, sizeof(line), f))
    {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        t.hipX = readCfgNumber(line, "hip_x", t.hipX);       t.hipY = readCfgNumber(line, "hip_y", t.hipY);       t.hipZ = readCfgNumber(line, "hip_z", t.hipZ);
        t.hipScale = readCfgNumber(line, "hip_scale", t.hipScale); t.hipYaw = readCfgNumber(line, "hip_yaw", t.hipYaw); t.hipPitch = readCfgNumber(line, "hip_pitch", t.hipPitch);
        t.adsX = readCfgNumber(line, "ads_x", t.adsX);       t.adsY = readCfgNumber(line, "ads_y", t.adsY);       t.adsZ = readCfgNumber(line, "ads_z", t.adsZ);
        t.adsScale = readCfgNumber(line, "ads_scale", t.adsScale); t.adsFov = readCfgNumber(line, "ads_fov", t.adsFov); t.drawSpeed = readCfgNumber(line, "draw_speed", t.drawSpeed);
        t.reloadKey = readCfgKey(line, "key_reload", t.reloadKey); t.inspectKey = readCfgKey(line, "key_inspect", t.inspectKey);
    }
    fclose(f);
    tuning = t;
}

void Mac10Client::resetTransient()
{
    fireHeld = false; fireBuffer = 0; sprintOut = 0; middleLatch = false;
    clip = -1; clipTime = 0; recoilAccum = 0; kick = kickPrev = 0;
    hitTicks = hitTicksPrev = 0;
}

bool Mac10Client::blocksSprint() const
{
    return holding && (fireHeld || sinceShot < 8 || ads > 0.3f);
}

float Mac10Client::getAimAmount(float a) const { return smooth(lerpf(adsPrev, ads, a)); }

float Mac10Client::getFovFactor(float a) const
{
    if (!holding) return 1.0f;
    return lerpf(1.0f, tuning.adsFov, getAimAmount(a));
}

bool Mac10Client::getHud(int &m, int &cap, int &res, bool &isReloading) const
{
    if (!holding) return false;
    m = mag; cap = capacity; res = reserve; isReloading = reloading;
    return true;
}

float Mac10Client::getHitMarker(float a, bool &headshot) const
{
    headshot = hitHeadshot;
    float t = lerpf((float)hitTicksPrev, (float)hitTicks, a) / 6.0f;
    return clamp01(t);
}

// -------------------------------------------------------------------------------------------------------------------
// Input
// -------------------------------------------------------------------------------------------------------------------
void Mac10Client::onMouseDown(LocalPlayer *p, int button, bool down)
{
    if (button == 0) fireHeld = down && holding;
}

bool Mac10Client::onMouseClick(LocalPlayer *p, int button)
{
    if (!holding) return false;
    if (button == 0) { fireBuffer = 3; return true; }   // fire / ADS / reload are driven from tick(); the click just feeds it
    if (button == 1) return true;                       // right button = aim down sights (polled in tick), never "use"
    if (button == 2) { middleLatch = true; return true; }
    return false;
}

void Mac10Client::sendAction(LocalPlayer *p, std::shared_ptr<ItemInstance> held, int action, bool aiming)
{
    Mac10Item *gun = dynamic_cast<Mac10Item *>(held->getItem());
    if (gun == nullptr) return;
    std::shared_ptr<Player> self = std::dynamic_pointer_cast<Player>(p->shared_from_this());
    gun->setPendingAction(held, (Mac10Item::PendingAction)action, aiming);
    Minecraft::GetInstance()->localgameModes[p->GetXboxPad()]->useItem(self, p->level, held);
    gun->clearPendingAction(held);
}

void Mac10Client::beginReloadLocal(LocalPlayer *p, std::shared_ptr<ItemInstance> held, bool wasEmpty)
{
    int ticks = Mac10Item::getReloadTicks(held, wasEmpty);
    reloadSeconds = (float)ticks / 20.0f;
    int id = wasEmpty ? CLIP_RELOAD_EMPTY : CLIP_RELOAD;
    float speed = 1.0f;
    if (selectKind(curKind) && G->clip[id] >= 0 && G->model.clips[G->clip[id]].duration > 0.0f)
        speed = G->model.clips[G->clip[id]].duration / reloadSeconds;
    clipVirtualDur = reloadSeconds;                    // used only when the model has no reload clip (procedural reload)
    playClip(id, speed, false);
    burstPending = false;
    cueMagOut = cueMagIn = cueBolt = false;
    ads = 0.0f; adsPrev = 0.0f;
}

bool Mac10Client::tryFire(LocalPlayer *p, std::shared_ptr<ItemInstance> held)
{
    if (reloading) return false;
    const bool aiming = ads > 0.5f;

    if (mag <= 0)
    {
        if (dryCooldown > 0) return false;
        sendAction(p, held, Mac10Item::ACTION_FIRE, aiming);
        Mac10Item::ShotResult r = Mac10Item::lastResult();
        dryCooldown = 8;
        Minecraft::GetInstance()->soundEngine->playUI(eSoundType_RANDOM_CLICK, 0.6f, 1.7f);
        if (r.reloadStarted) beginReloadLocal(p, held, true);
        return true;
    }
    if (!Mac10Item::canFireNow(p->level, std::dynamic_pointer_cast<Player>(p->shared_from_this()), held)) return false;

    sendAction(p, held, Mac10Item::ACTION_FIRE, aiming);
    Mac10Item::ShotResult r = Mac10Item::lastResult();
    if (!r.fired) return false;
    dbgCandidates = r.candidates; dbgDistance = r.distance; dbgHit = r.hitEntity;
    shotEffects(p, autoFire, r.hitEntity, r.headshot, r.killed);
    // a burst weapon keeps firing its remaining rounds even if the trigger is released (like the real thing)
    burstPending = !autoFire && Mac10Item::statsFor(curKind).burstCount > 0 &&
                   Mac10Item::getBurstLeft(p->level, std::dynamic_pointer_cast<Player>(p->shared_from_this())) > 0;
    return true;
}

void Mac10Client::shotEffects(LocalPlayer *p, bool automatic, bool hit, bool headshot, bool killed)
{
    const bool aimed = ads > 0.5f;
    sinceShot = 0;
    Minecraft *mc = Minecraft::GetInstance();

    const GunStats &gs = Mac10Item::statsFor(curKind);
    // ---- recoil: kick up, wander a little sideways; part of it is recovered once the trigger is released ----
    float up = (automatic ? gs.recoilAutoUp : gs.recoilUp) * (aimed ? 0.62f : 1.0f);
    float side = frand(gs.recoilSide) * (aimed ? 0.6f : 1.0f);
    p->xRot -= up;
    if (p->xRot < -90.0f) p->xRot = -90.0f;
    p->yRot += side;
    recoilAccum += up;
    kick = (std::min)(1.0f, kick + (curKind >= GUN_FLINTLOCK_COPPER ? 1.0f : (automatic ? 0.55f : 0.8f)));

    // ---- sound (layered from existing sounds; there is no dedicated gunshot sample) ----
    if (curKind == GUN_M16)
    {
        mc->soundEngine->playUI(eSoundType_RANDOM_BOW, 0.70f, 0.95f + frand(0.06f));
        mc->soundEngine->playUI(eSoundType_RANDOM_BOW, 0.40f, 1.70f + frand(0.08f));
    }
    else if (curKind >= GUN_FLINTLOCK_COPPER)
    {
        mc->soundEngine->playUI(eSoundType_RANDOM_BOW, 1.00f, 0.55f + frand(0.04f));     // heavy thump
        mc->soundEngine->playUI(eSoundType_RANDOM_BOW, 0.55f, 1.15f + frand(0.06f));     // crack
    }
    else
    {
        mc->soundEngine->playUI(eSoundType_RANDOM_BOW, 0.55f, 1.25f + frand(0.08f));
        mc->soundEngine->playUI(eSoundType_RANDOM_BOW, 0.40f, 2.05f + frand(0.1f));
    }

    // ---- flintlocks have no muzzle-flash mesh: a puff of smoke and a flash of flame at the muzzle instead ----
    if (curKind >= GUN_FLINTLOCK_COPPER && p->level != nullptr)
    {
        double yr = p->yRot * 3.14159265358979 / 180.0, xr = p->xRot * 3.14159265358979 / 180.0;
        double lx = -sin(yr) * cos(xr), ly = -sin(xr), lz = cos(yr) * cos(xr);
        double mx = p->x + lx * 1.1 - cos(yr) * 0.18, my = p->y + ly * 1.1 - 0.12, mz = p->z + lz * 1.1 - sin(yr) * 0.18;
        for (int i = 0; i < 7; ++i)
            p->level->addParticle(eParticleType_smoke, mx + frand(0.05f), my + frand(0.05f), mz + frand(0.05f), lx * 0.08 + frand(0.02f), ly * 0.08 + 0.01 + frand(0.02f), lz * 0.08 + frand(0.02f));
        p->level->addParticle(eParticleType_flame, mx, my, mz, lx * 0.05, ly * 0.05, lz * 0.05);
    }

    // ---- animation ----
    if (automatic)
    {
        if (!(clip == CLIP_AUTO_FIRE && clipLoop)) playClip(CLIP_AUTO_FIRE, 1.0f, true);
    }
    else playClip(CLIP_FIRE, 1.0f, false);

    if (hit)
    {
        hitTicks = 6; hitTicksPrev = 6; hitHeadshot = headshot;
        // (random/bowhit has no sound file in this build - the log said so - so the hit tick is a very high bow sound)
        mc->soundEngine->playUI(eSoundType_RANDOM_BOW, headshot ? 0.45f : 0.30f, headshot ? 3.4f : 2.7f);
    }
}

// -------------------------------------------------------------------------------------------------------------------
// Tick
// -------------------------------------------------------------------------------------------------------------------
void Mac10Client::tick(LocalPlayer *p)
{
    Minecraft *mc = Minecraft::GetInstance();
    std::shared_ptr<ItemInstance> held = (p->inventory != nullptr) ? p->inventory->getSelected() : nullptr;
    wasHolding = holding;
    holding = Mac10Item::isMac10(held) && p->level != nullptr && !p->isSleeping();

    adsPrev = ads; sprintBlendPrev = sprintBlend; kickPrev = kick; hitTicksPrev = hitTicks;
    swayYawPrev = swayYaw; swayPitchPrev = swayPitch; bobPhasePrev = bobPhase; bobAmpPrev = bobAmp;

    if (!holding)
    {
        if (wasHolding) { resetTransient(); heldSlot = -1; }
        ads = approach(ads, 0.0f, 0.25f);
        if (hitTicks > 0) hitTicks--;
        sinceShot = 1000;
        return;
    }

    const int pad = p->GetXboxPad();
    const int kindNow = Mac10Item::getKind(held);
    const bool kindChanged = kindNow != curKind;
    if (kindChanged) { curKind = kindNow; tuning = Tuning(curKind); configTimer = 0; }
    selectKind(curKind);
    const GunStats &gs = Mac10Item::statsFor(curKind);
    if (!wasHolding || heldSlot != p->inventory->selected || kindChanged)
    {
        heldSlot = p->inventory->selected;
        resetTransient();
        burstPending = false;
        ads = adsPrev = 0.0f;
        sinceShot = 1000;
        cueMagOut = cueMagIn = cueBolt = true;
        clipVirtualDur = (float)gs.drawLock / 20.0f;         // only used by guns whose model has no draw clip
        if (G->ok) playClip(CLIP_DRAW, tuning.drawSpeed, false);
    }

    if (--configTimer <= 0) { configTimer = 40; reloadTuning(); }

    // ---- state from the (predicted / server-synced) stack ----
    mag = Mac10Item::getMagazineRounds(held);
    capacity = Mac10Item::getMagazineCapacity(held);
    reserve = Mac10Item::getReserveAmmo(std::dynamic_pointer_cast<Player>(p->shared_from_this()));
    reloading = Mac10Item::isReloading(p->level, std::dynamic_pointer_cast<Player>(p->shared_from_this()));
    autoFire = Mac10Item::hasAutoFire(held);

    // ---- raw input ----
    bool aimInput = false, reloadPressed = false, inspectPressed = false;
    bool inputOk = mc->localgameModes[pad] != nullptr && mc->localgameModes[pad]->isInputAllowed(MINECRAFT_ACTION_USE);
#ifdef _WINDOWS64
    if (pad == 0 && g_KBMInput.IsMouseGrabbed() && g_KBMInput.IsKBMActive())
    {
        reloadPressed = g_KBMInput.IsKeyPressed(tuning.reloadKey);       // default G (R is the crafting / creative inventory key)
        inspectPressed = g_KBMInput.IsKeyPressed(tuning.inspectKey);     // default F
        aimInput = g_KBMInput.IsMouseButtonDown(KeyboardMouseInput::MOUSE_RIGHT);
    }
#endif
    if (InputManager.GetValue(pad, MINECRAFT_ACTION_USE) > 0) aimInput = true;   // controller: the use trigger aims
    if (!inputOk) { aimInput = false; reloadPressed = false; inspectPressed = false; fireBuffer = 0; fireHeld = false; }
    if (middleLatch) { inspectPressed = true; middleLatch = false; }

    // ---- counters ----
    if (sinceShot < 1000) sinceShot++;
    if (dryCooldown > 0) dryCooldown--;
    if (sprintOut > 0) sprintOut--;
    if (hitTicks > 0) hitTicks--;
    kick *= 0.55f;

    // ---- reload / inspect ----
    if (reloadPressed && !reloading && mag < capacity && reserve != 0)
    {
        bool wasEmpty = mag <= 0;
        sendAction(p, held, Mac10Item::ACTION_RELOAD, false);
        if (Mac10Item::lastResult().reloadStarted) beginReloadLocal(p, held, wasEmpty);
    }
    if (inspectPressed && !reloading && sinceShot > 10 && clip != CLIP_DRAW && !p->isSprinting()) playClip(CLIP_INSPECT, 1.0f, false);

    // ---- sprint rules (CoD): can't shoot while sprinting; pressing the trigger lowers the gun first ----
    if (burstPending && (reloading || mag <= 0)) burstPending = false;
    const bool wantsShot = fireBuffer > 0 || (fireHeld && autoFire) || burstPending;
    if (fireBuffer > 0) fireBuffer--;
    if (wantsShot)
    {
        if (p->isSprinting()) { p->setSprinting(false); sprintOut = 3; fireBuffer = 6; }   // remember the click until the gun is up
        else if (sprintOut == 0)
        {
            const bool wasBurst = burstPending;
            if (tryFire(p, held)) { if (!wasBurst) fireBuffer = 0; }
            else if (wasBurst && sinceShot > 6) burstPending = false;                          // never got to fire: drop the burst
        }
    }
    sprintBlend = approach(sprintBlend, (p->isSprinting() && sinceShot > 6) ? 1.0f : 0.0f, 0.2f);

    // ---- aim down sights ----
    const bool busy = reloading || clip == CLIP_RELOAD || clip == CLIP_RELOAD_EMPTY || clip == CLIP_INSPECT || clip == CLIP_DRAW;
    float adsTarget = (aimInput && !p->isSprinting() && !busy) ? 1.0f : 0.0f;
    if (aimInput && clip == CLIP_INSPECT) { clip = -1; }
    ads = approach(ads, adsTarget, 0.18f);
    if (ads > 0.05f && p->input != nullptr)
    {
        // aiming slows you down (hip: full speed)
        float k = lerpf(1.0f, gs.adsMove, smooth(ads));
        p->input->xa *= k; p->input->ya *= k;
    }

    // ---- recoil recovery: ease the view back down a little once the burst is over ----
    if (sinceShot > 3 && recoilAccum > 0.001f)
    {
        float step = (std::min)(recoilAccum, recoilAccum * 0.22f + 0.02f);
        p->xRot += step * 0.6f;
        recoilAccum -= step;
    }
    if (sinceShot > 40) recoilAccum = 0.0f;

    tickViewModel(p);
}

// -------------------------------------------------------------------------------------------------------------------
// View model
// -------------------------------------------------------------------------------------------------------------------
void Mac10Client::playClip(int which, float speed, bool loop)
{
    if (!selectKind(curKind)) return;
    if (which < 0 || which >= CLIP_COUNT) { clip = -1; return; }
    if (G->clip[which] < 0)
    {
        // The model has no such clip. Draw and reload are replaced by a procedural motion (see render()); anything else is simply skipped.
        if (which == CLIP_DRAW || which == CLIP_RELOAD || which == CLIP_RELOAD_EMPTY) { clip = which; clipTime = 0.0f; clipSpeed = 1.0f; clipLoop = false; }
        else clip = -1;
        return;
    }
    clip = which; clipTime = 0.0f; clipSpeed = speed; clipLoop = loop;
}

void Mac10Client::tickViewModel(LocalPlayer *p)
{
    if (!G->ok) return;

    // advance the running clip (a clip the model lacks runs for clipVirtualDur seconds and is drawn procedurally)
    if (clip >= 0)
    {
        const bool real = G->clip[clip] >= 0;
        float duration = real ? G->model.clips[G->clip[clip]].duration : clipVirtualDur;
        clipTime += clipSpeed / 20.0f;
        if (clipLoop)
        {
            if (duration > 0.0f) while (clipTime >= duration) clipTime -= duration;
            if (sinceShot > 4) clip = -1;                     // trigger released: stop the automatic loop
        }
        else if (clipTime >= duration) clip = -1;

        // reload sound cues (magazine out / in / bolt) at fixed fractions of the clip
        if (clip == CLIP_RELOAD || clip == CLIP_RELOAD_EMPTY)
        {
            float f = duration > 0.0f ? clipTime / duration : 1.0f;
            Minecraft *mc = Minecraft::GetInstance();
            if (!cueMagOut && f > 0.28f) { cueMagOut = true; mc->soundEngine->playUI(eSoundType_TILE_PISTON_OUT, 0.35f, 2.2f); }
            if (!cueMagIn && f > 0.62f)  { cueMagIn = true;  mc->soundEngine->playUI(eSoundType_TILE_PISTON_IN, 0.4f, 2.0f); }
            if (!cueBolt && f > 0.88f)   { cueBolt = true;   mc->soundEngine->playUI(eSoundType_RANDOM_CLICK, 0.5f, 1.4f); }
        }
    }

    // sway follows the view's angular velocity, bob follows walking speed
    float dyaw = p->yRot - p->yRotO, dpitch = p->xRot - p->xRotO;
    if (dyaw > 25.0f) dyaw = 25.0f; if (dyaw < -25.0f) dyaw = -25.0f;
    if (dpitch > 25.0f) dpitch = 25.0f; if (dpitch < -25.0f) dpitch = -25.0f;
    swayYaw += (dyaw - swayYaw) * 0.35f;
    swayPitch += (dpitch - swayPitch) * 0.35f;
    double dx = p->x - p->xo, dz = p->z - p->zo;
    float speed = (float)sqrt(dx * dx + dz * dz);
    bobAmp += ((p->onGround && speed > 0.01f ? (std::min)(speed * 7.0f, 1.0f) : 0.0f) - bobAmp) * 0.2f;
    if (p->onGround) bobPhase += speed * 5.5f;
}

// The "ready" pose of the current model: arms up holding the gun, muzzle flash hidden, and the empty-magazine look when mag <= 0.
static void applyBaseline(GltfModel::Pose &pose, int mag)
{
    G->model.resetPose(pose);
    // The rig's rest pose has the arms hanging down. Each model defines its ready pose with a clip (the end of the draw clip for the MAC-10).
    if (G->baseClip >= 0) G->model.applyClip(pose, G->baseClip, 0.0f);
    else if (G->clip[0] >= 0) G->model.applyClip(pose, G->clip[0], 1000.0f);
    auto setScale = [&](int node, float v) { if (node >= 0) pose.s[(size_t)node * 3] = pose.s[(size_t)node * 3 + 1] = pose.s[(size_t)node * 3 + 2] = v; };
    setScale(G->nodeMuzzle, 0.0f);
    if (mag <= 0)
    {
        if (G->clip[3] >= 0) G->model.applyClip(pose, G->clip[3], 0.0f);
        if (G->nodeAmmo >= 0 && G->nodeNoAmmo >= 0) { setScale(G->nodeAmmo, 0.0f); setScale(G->nodeNoAmmo, 1.0f); }   // M16: the magazine shows its empty state
    }
}

void Mac10Client::buildPose(GltfModel::Pose &pose, float a) const
{
    applyBaseline(pose, mag);
    if (clip >= 0 && G->clip[clip] >= 0)
    {
        float duration = G->model.clips[G->clip[clip]].duration;
        float t = clipTime + clipSpeed * a / 20.0f;
        if (clipLoop) { if (duration > 0.0f) while (t >= duration) t -= duration; }
        else if (t > duration) t = duration;
        G->model.applyClip(pose, G->clip[clip], t);
    }
}

// Draws the whole rig (arms + gun + muzzle flash) for the given pose. V maps rig space into the current GL space.
// armOwner == nullptr -> first person view model (local skin); otherwise a third-person render of that player.
static void drawRig(Minecraft *mc, const GltfModel::Pose &pose, const std::vector<float> &world, const float V[16], float br,
                    PlayerRenderer *armRenderer, std::shared_ptr<Player> armOwner)
{
    Lighting::turnOff();                                   // shading is baked into the vertex colours below
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // ---------- arms (the owner's skin, via the vanilla model parts) ----------
    if (armRenderer != nullptr)
    {
        if (armOwner == nullptr)
        {
            // first person: the local player's skin
            glBindTexture(GL_TEXTURE_2D, mc->textures->loadMemTexture(mc->player->customTextureUrl, mc->player->getTexture()));
            mc->textures->clearLastBoundId();
        }
        for (int side = 0; side < 2; ++side)
        {
            int node = side == 0 ? G->nodeRight : G->nodeLeft;
            if (node < 0) continue;
            float M[16];
            mul(V, &world[(size_t)node * 16], M);
            glPushMatrix();
            applyMatrixToGL(M);
            glScalef(-1.0f, -1.0f, 1.0f);                   // model space (y down, x mirrored) -> rig space (y up)
            glTranslatef(side == 0 ? 1.0f / 16.0f : -1.0f / 16.0f, 0.0f, 0.0f);   // the vanilla arm box is not centred on its pivot
            glColor4f(br, br, br, 1.0f);
            armRenderer->renderViewArm(side == 1, armOwner);
            glPopMatrix();
        }
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }

    // ---------- gun parts, one batch per texture ----------
    Tesselator *t = Tesselator::getInstance();
    const float lx = 0.35f, ly = 0.80f, lz = 0.50f;        // fixed key light for the baked shading
    const float ll = sqrtf(lx * lx + ly * ly + lz * lz);
    glDisable(GL_CULL_FACE);                               // the glTF materials are double sided
    for (int role = 0; role <= 2; ++role)
    {
        if (role == 2 && G->nodeMuzzle >= 0 && pose.s[(size_t)G->nodeMuzzle * 3] < 0.01f) continue;     // flash hidden
        ResourceLocation *roleTexture = textureFor(g_kind, role);
        if (roleTexture == nullptr) continue;                // this gun has no bullet / flash part
        mc->textures->bindTexture(roleTexture);
        if (role == 2) { glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE); }
        t->begin();
        for (size_t n = 0; n < G->model.nodes.size(); ++n)
        {
            int meshIndex = G->model.nodes[n].mesh;
            if (meshIndex < 0 || meshIndex >= (int)G->model.meshes.size()) continue;
            float M[16];
            mul(V, &world[n * 16], M);
            for (size_t pi = 0; pi < G->model.meshes[(size_t)meshIndex].size(); ++pi)
            {
                const GltfModel::Primitive &prim = G->model.meshes[(size_t)meshIndex][pi];
                int matRole = (prim.material >= 0 && prim.material < 32) ? G->materialRole[prim.material] : 0;
                if (matRole != role) continue;
                for (size_t i = 0; i + 2 < prim.indices.size(); i += 3)
                {
                    const GltfModel::Vertex *tri[3] = { &prim.vertices[prim.indices[i]], &prim.vertices[prim.indices[i + 1]], &prim.vertices[prim.indices[i + 2]] };
                    float pos[3][3];
                    for (int k = 0; k < 3; ++k)
                    {
                        const GltfModel::Vertex &v = *tri[k];
                        pos[k][0] = M[0] * v.x + M[1] * v.y + M[2] * v.z + M[3];
                        pos[k][1] = M[4] * v.x + M[5] * v.y + M[6] * v.z + M[7];
                        pos[k][2] = M[8] * v.x + M[9] * v.y + M[10] * v.z + M[11];
                    }
                    // face shading from the transformed geometric normal (two-sided: use the absolute lit side)
                    float e1[3] = { pos[1][0] - pos[0][0], pos[1][1] - pos[0][1], pos[1][2] - pos[0][2] };
                    float e2[3] = { pos[2][0] - pos[0][0], pos[2][1] - pos[0][1], pos[2][2] - pos[0][2] };
                    float nx = e1[1] * e2[2] - e1[2] * e2[1], ny = e1[2] * e2[0] - e1[0] * e2[2], nz = e1[0] * e2[1] - e1[1] * e2[0];
                    float nl = sqrtf(nx * nx + ny * ny + nz * nz);
                    float shade = 0.72f;
                    if (nl > 1e-9f) shade = 0.58f + 0.42f * fabsf((nx * lx + ny * ly + nz * lz) / (nl * ll));
                    float c = role == 2 ? 1.0f : shade * br;
                    t->color(c, c, c, 1.0f);
                    // a triangle is submitted as a quad whose last corner repeats (the Tesselator is a quad batcher)
                    t->vertexUV(pos[0][0], pos[0][1], pos[0][2], tri[0]->u, tri[0]->v);
                    t->vertexUV(pos[1][0], pos[1][1], pos[1][2], tri[1]->u, tri[1]->v);
                    t->vertexUV(pos[2][0], pos[2][1], pos[2][2], tri[2]->u, tri[2]->v);
                    t->vertexUV(pos[2][0], pos[2][1], pos[2][2], tri[2]->u, tri[2]->v);
                }
            }
        }
        t->end();
        if (role == 2) { glDisable(GL_BLEND); }
    }
    glEnable(GL_CULL_FACE);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

bool Mac10Client::render(Minecraft *mc, std::shared_ptr<Player> player, float a, float swapHeight, float brightness, float fudgeX, float fudgeY)
{
    if (!selectKind(curKind)) return false;

    // ---------- pose + world matrices ----------
    GltfModel::Pose pose;
    buildPose(pose, a);
    std::vector<float> world;
    G->model.computeWorld(pose, world);

    // ---------- rig -> camera transform ----------
    const float adsE = smooth(lerpf(adsPrev, ads, a));
    const float sprintE = smooth(lerpf(sprintBlendPrev, sprintBlend, a));
    const float kickE = lerpf(kickPrev, kick, a);
    const float yawS = lerpf(swayYawPrev, swayYaw, a), pitchS = lerpf(swayPitchPrev, swayPitch, a);
    const float bobP = lerpf(bobPhasePrev, bobPhase, a), bobA = lerpf(bobAmpPrev, bobAmp, a);
    const float steady = 1.0f - 0.85f * adsE;

    // grip point of the gun in rig space (hip pivot) and the rear-sight point (aim pivot)
    const Anchors &an = ANCHORS[(int)(G - g_models)];
    const float gripX = an.gx, gripY = an.gy, gripZ = an.gz;
    const float sightX = an.sx, sightY = an.sy, sightZ = an.sz;

    float px = lerpf(tuning.hipX, tuning.adsX, adsE) + fudgeX;
    float py = lerpf(tuning.hipY, tuning.adsY, adsE) + fudgeY - (1.0f - swapHeight) * 0.6f;
    float pz = lerpf(tuning.hipZ, tuning.adsZ, adsE);
    float scale = lerpf(tuning.hipScale, tuning.adsScale, adsE);
    float yaw = lerpf(tuning.hipYaw, 0.0f, adsE);
    float pitch = lerpf(tuning.hipPitch, 0.0f, adsE);
    float ax = lerpf(gripX, sightX, adsE), ay = lerpf(gripY, sightY, adsE), az = lerpf(gripZ, sightZ, adsE);

    // sprint pose: lowered, canted and turned in
    px += 0.09f * sprintE; py -= 0.16f * sprintE; pz += 0.05f * sprintE;
    yaw += -26.0f * sprintE; pitch += 14.0f * sprintE;
    // walking bob, view sway, firing shove
    px += cosf(bobP) * 0.012f * bobA * steady - yawS * 0.0035f * steady;
    py += -fabsf(sinf(bobP)) * 0.016f * bobA * steady + pitchS * 0.0035f * steady;
    const bool pistol = curKind >= GUN_FLINTLOCK_COPPER;
    pz += kickE * (pistol ? 0.11f : 0.045f);
    pitch += kickE * (pistol ? 9.0f : 2.2f);

    // draw / reload for models that have no such clip: a procedural motion over clipVirtualDur seconds
    if (clip >= 0 && G->clip[clip] < 0 && clipVirtualDur > 0.0f && (clip == CLIP_DRAW || clip == CLIP_RELOAD || clip == CLIP_RELOAD_EMPTY))
    {
        float t = clamp01((clipTime + clipSpeed * a / 20.0f) / clipVirtualDur);
        if (clip == CLIP_DRAW)
        {
            float e = 1.0f - smooth(t);
            py -= 0.38f * e; pz += 0.10f * e; pitch += 38.0f * e; yaw += 12.0f * e;
        }
        else
        {
            float d = sinf(3.14159265f * t);                 // dips, tilts away and comes back
            py -= 0.30f * d; px -= 0.04f * d; pz += 0.06f * d; pitch += 32.0f * d; yaw += -14.0f * d;
        }
    }

    float V[16], t0[16], r0[16], r1[16], s0[16], t1[16], tmp[16];
    translation(px, py, pz, t0);
    rotationY(yaw, r0);
    rotationX(pitch, r1);
    uniformScale(scale, s0);
    translation(-ax, -ay, -az, t1);
    mul(t0, r0, tmp); mul(tmp, r1, V); mul(V, s0, tmp); mul(tmp, t1, V);

    // ---------- draw ----------
    PlayerRenderer *playerRenderer = static_cast<PlayerRenderer *>(EntityRenderDispatcher::instance->getRenderer(mc->player));
    drawRig(mc, pose, world, V, brightness, playerRenderer, nullptr);
    return true;
}


// -------------------------------------------------------------------------------------------------------------------
// HUD diagnostics
// -------------------------------------------------------------------------------------------------------------------
bool Mac10Client::getDebugLine(std::shared_ptr<ItemInstance> held, std::wstring &out) const
{
#if MAC10_DEBUG_LOG
    if (!holding) return false;
    wchar_t buf[200];
    swprintf(buf, 200, L"C scan: cand=%d hit=%d d=%.1f", dbgCandidates, dbgHit ? 1 : 0, dbgDistance);
    out = buf;
    if (held != nullptr && held->tag != nullptr && held->tag->contains(L"Mac10.DbgCand"))
    {
        swprintf(buf, 200, L"  |  S scan: cand=%d hit=%d d=%.1f hurt=%d hp=%.1f", held->tag->getInt(L"Mac10.DbgCand"), held->tag->getInt(L"Mac10.DbgHit"),
                 held->tag->getFloat(L"Mac10.DbgDist"), held->tag->getInt(L"Mac10.DbgHurt"), held->tag->getFloat(L"Mac10.DbgHp"));
        out += buf;
    }
    else out += L"  |  S scan: (no data yet)";
    return true;
#else
    return false;
#endif
}

// -------------------------------------------------------------------------------------------------------------------
// Third person
// -------------------------------------------------------------------------------------------------------------------
namespace
{
    struct RemoteGun
    {
        int   lastMag = -1;
        float shotTime = -1000.0f;     // in ticks (tickCount + partial) of the last observed shot
        float burstStart = -1000.0f;   // start of the current automatic burst
    };
    std::unordered_map<int, RemoteGun> g_remote;

    // Third-person placement, hot-reloadable from <gun>_view.cfg (keys tp_*). Units are blocks in the right arm's own space:
    // +y runs along the arm toward the hand, +x is across the arm, +z is "down" when the arm is held out straight.
    // gx/gy/gz = the point of the gun (rig space) that is put on the hand.
    struct ThirdPersonTuning { float scale, x, y, z, gx, gy, gz; };
    ThirdPersonTuning g_tp[GUN_KIND_COUNT] =
    {
        { 0.55f, 0.0f, 0.28f, 0.12f,  0.25f, 0.10f,  0.00f },     // MAC-10
        { 0.30f, 0.0f, 0.30f, 0.14f,  0.45f, -0.15f, 1.05f },     // M16 (the pistol grip goes into the hand; the rifle extends forward)
        { 0.90f, 0.0f, 0.14f, 0.10f, -0.02f, 0.02f,  0.10f },     // flintlock copper
        { 0.90f, 0.0f, 0.14f, 0.10f, -0.02f, 0.02f,  0.10f },     // flintlock iron
        { 0.90f, 0.0f, 0.14f, 0.10f, -0.02f, 0.02f,  0.10f },     // flintlock gold
    };
    int g_tpTimer = 0;

    void reloadThirdPersonConfig()
    {
        if (G->folder.empty()) return;
        std::string path = G->folder + MODEL_DEFS[(int)(G - g_models)].cfg;
        FILE *f = fopen(path.c_str(), "r");
        if (!f) return;
        ThirdPersonTuning t = g_tp[g_kind];
        char line[256];
        while (fgets(line, sizeof(line), f))
        {
            if (line[0] == '#') continue;
            t.scale = readCfgNumber(line, "tp_scale", t.scale);
            t.x = readCfgNumber(line, "tp_x", t.x);
            t.y = readCfgNumber(line, "tp_y", t.y);
            t.z = readCfgNumber(line, "tp_z", t.z);
        }
        fclose(f);
        for (int k = 0; k < GUN_KIND_COUNT; ++k)
            if (modelIndexFor(k) == modelIndexFor(g_kind)) { g_tp[k].scale = t.scale; g_tp[k].x = t.x; g_tp[k].y = t.y; g_tp[k].z = t.z; }
    }
}

bool Mac10Client::isModelAvailable(std::shared_ptr<ItemInstance> item)
{
    int kind = Mac10Item::getKind(item);
    return kind >= 0 && selectKind(kind);
}

bool Mac10Client::renderGunInHand(std::shared_ptr<Player> player, float a)
{
    if (player == nullptr) return false;
    std::shared_ptr<ItemInstance> item = player->inventory != nullptr ? player->inventory->getSelected() : nullptr;
    const int kind = Mac10Item::getKind(item);
    if (kind < 0 || !selectKind(kind)) return false;
    Minecraft *mc = Minecraft::GetInstance();
    if (--g_tpTimer <= 0) { g_tpTimer = 150; reloadThirdPersonConfig(); }

    // ---- what the gun is doing: shots are inferred from the magazine going down (the stack is synced to every player) ----
    RemoteGun &rg = g_remote[player->entityId];
    const float now = (float)player->tickCount + a;
    const int mag = Mac10Item::getMagazineRounds(item);
    const bool automatic = Mac10Item::hasAutoFire(item);
    if (rg.lastMag >= 0 && mag < rg.lastMag)
    {
        if (now - rg.shotTime > 5.0f) rg.burstStart = now;
        rg.shotTime = now;
    }
    rg.lastMag = mag;
    if (g_remote.size() > 128) { g_remote.clear(); rg.lastMag = mag; }

    GltfModel::Pose pose;
    applyBaseline(pose, mag);
    const float age = (now - rg.shotTime) / 20.0f;           // seconds since the last shot
    if (automatic && G->clip[CLIP_AUTO_FIRE] >= 0)
    {
        float duration = G->model.clips[G->clip[CLIP_AUTO_FIRE]].duration;
        if (age >= 0.0f && age < 0.25f && duration > 0.0f)
        {
            float t = (now - rg.burstStart) / 20.0f;
            while (t >= duration) t -= duration;
            G->model.applyClip(pose, G->clip[CLIP_AUTO_FIRE], t);
        }
    }
    else if (G->clip[CLIP_FIRE] >= 0)
    {
        float duration = G->model.clips[G->clip[CLIP_FIRE]].duration;
        if (age >= 0.0f && age < duration) G->model.applyClip(pose, G->clip[CLIP_FIRE], age);
    }
    std::vector<float> world;
    G->model.computeWorld(pose, world);

    // ---- rig space -> the right arm's own space (the caller has already moved to the hand) ----
    // rig: +x right, +y up, -z forward.  arm (held straight out forward): +y along the arm, +z down, +x = player's left.
    //   local = (-x, -z, -y)          (a pure rotation: it keeps the gun's up / forward / right directions)
    // The grip point of the gun is put on the hand, the gun is scaled to fit the player, then nudged by the tp_* tuning.
    const ThirdPersonTuning &tp = g_tp[kind];
    const float gripX = tp.gx, gripY = tp.gy, gripZ = tp.gz;
    float A[16]; identity(A);
    A[0] = -1.0f; A[5] = 0.0f; A[6] = -1.0f; A[9] = -1.0f; A[10] = 0.0f;
    float S[16], Tg[16], Tn[16], tmp[16], V[16];
    uniformScale(tp.scale, S);
    translation(-gripX, -gripY, -gripZ, Tg);
    translation(tp.x, tp.y, tp.z, Tn);
    mul(Tn, A, tmp); mul(tmp, S, V); mul(V, Tg, tmp);
    memcpy(V, tmp, sizeof(V));

    glDisable(GL_CULL_FACE);
    drawRig(mc, pose, world, V, 1.0f, nullptr, nullptr);    // gun only: the arms are the vanilla ones
    Lighting::turnOn();                                      // the entity pass runs with lighting on
    glEnable(GL_CULL_FACE);
    return true;
}
