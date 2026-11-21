// mgs3danim 1.0 - 3DS-style over-the-shoulder aiming for METAL GEAR SOLID 3 (Master Collection, PC). An ASI plugin.
//
// While a weapon is raised in third person:
// - the camera sits close behind Snake's right shoulder and looks along the aim;
// - Snake faces where the camera looks and strafes: the stick / keys are hidden from the game's own movement code and
//   the mod moves him through the same "extra movement" vector the game's first-person walk uses (Snake+0x620 /
//   +0x628), so walls still stop him; directional walk, crouch walk and crawl motions are played on his legs;
// - a crosshair is drawn, shots follow it, recoil and camera collision are handled.
// Also: crouch walking outside the shoulder view, a slow crawl, first-person hand-over with a pad, the classic camera
// left alone, and an optional log (mgs3danim.log; [Debug] Log in mgs3danim.ini).
//
// Build (LLVM): see build/build.sh.

typedef unsigned char u8; typedef unsigned short u16; typedef unsigned int u32; typedef unsigned long long u64;
typedef short i16; typedef int i32; typedef long long i64;
typedef void* HANDLE; typedef u32 DWORD; typedef int BOOL;
#define W __declspec(dllimport)
#define API __stdcall

typedef struct { void* BaseAddress; void* AllocationBase; DWORD AllocationProtect; u16 PartitionId; u64 RegionSize; DWORD State, Protect, Type; } MBI;

W HANDLE API GetModuleHandleA(const char*);
W void*  API VirtualAlloc(void*, u64, DWORD, DWORD);
W BOOL   API VirtualProtect(void*, u64, DWORD, DWORD*);
W u64    API VirtualQuery(const void*, MBI*, u64);
W BOOL   API IsBadReadPtr(const void*, u64);
W BOOL   API ReadProcessMemory(HANDLE, const void*, void*, u64, u64*);
W HANDLE API CreateThread(void*, u64, DWORD (API *)(void*), void*, DWORD, DWORD*);
W void   API Sleep(DWORD);
W DWORD  API GetTickCount(void);
typedef struct { i64 q; } LARGE_;
W BOOL   API QueryPerformanceCounter(LARGE_*);
W BOOL   API QueryPerformanceFrequency(LARGE_*);
typedef struct { DWORD attr, c0, c1, a0, a1, w0, w1, sizeHi, sizeLo; } FATTR_;   // WIN32_FILE_ATTRIBUTE_DATA (FILETIMEs as DWORD pairs)
W BOOL   API GetFileAttributesExA(const char*, int, FATTR_*);
W DWORD  API GetModuleFileNameA(HANDLE, char*, DWORD);
W u32    API GetPrivateProfileIntA(const char*, const char*, i32, const char*);
W BOOL   API WritePrivateProfileStringA(const char*, const char*, const char*, const char*);
W HANDLE API CreateFileA(const char*, DWORD, DWORD, void*, DWORD, DWORD, HANDLE);
W BOOL   API WriteFile(HANDLE, const void*, DWORD, DWORD*, void*);
W BOOL   API FlushInstructionCache(HANDLE, const void*, u64);
W HANDLE API GetCurrentProcess(void);
W BOOL   API DisableThreadLibraryCalls(HANDLE);
W int    __cdecl wsprintfA(char*, const char*, ...);
W i16    API GetAsyncKeyState(int);
typedef struct { u32 type; u32 pad; union { struct { i32 dx, dy; u32 data, flags, time, pad2; u64 extra; } mi; struct { u16 vk, scan; u32 flags, time, pad2; u64 extra; } ki; u8 raw[32]; } u; } INPUT_;
W u32    API SendInput(u32, INPUT_*, int);
W HANDLE API GetForegroundWindow(void);
W DWORD  API GetWindowThreadProcessId(HANDLE, DWORD*);
W u32    API MapVirtualKeyA(u32, u32);
W DWORD  API GetCurrentProcessId(void);
W BOOL   API ReadFile(HANDLE, void*, DWORD, DWORD*, void*);
W BOOL   API CloseHandle(HANDLE);
W HANDLE API LoadLibraryA(const char*);
W void*  API GetProcAddress(HANDLE, const char*);
typedef struct { u32 style; u32 pad; void* proc; i32 clsExtra, wndExtra; HANDLE inst, icon, cursor, brush; const char* menu; const char* name; } WNDCLASSA_;
typedef struct { HANDLE hwnd; u32 message; u32 pad; u64 wParam; i64 lParam; u32 time; i32 px, py; u32 priv; } MSG_;
typedef struct { i32 x, y; } POINT_;
typedef struct { i32 l, t, r, b; } RECT_;
typedef struct { u32 size; i32 w, h; u16 planes, bpp; u32 comp, imgSize; i32 xppm, yppm; u32 used, important; u32 colors[4]; } BMI_;
W u16    API RegisterClassA(const WNDCLASSA_*);
W HANDLE API CreateWindowExA(DWORD, const char*, const char*, DWORD, i32, i32, i32, i32, HANDLE, HANDLE, HANDLE, void*);
W i64    API DefWindowProcA(HANDLE, u32, u64, i64);
W int    API GetClassNameA(HANDLE, char*, int);
W BOOL   API ShowWindow(HANDLE, int);
W BOOL   API SetWindowPos(HANDLE, HANDLE, i32, i32, i32, i32, u32);
W BOOL   API UpdateLayeredWindow(HANDLE, HANDLE, POINT_*, POINT_*, HANDLE, POINT_*, u32, u32*, u32);
W BOOL   API GetClientRect(HANDLE, RECT_*);
W BOOL   API ClientToScreen(HANDLE, POINT_*);
W BOOL   API PeekMessageA(MSG_*, HANDLE, u32, u32, u32);
W i64    API DispatchMessageA(const MSG_*);
W BOOL   API IsWindow(HANDLE);
W BOOL   API IsIconic(HANDLE);
W HANDLE API CreateCompatibleDC(HANDLE);
W HANDLE API CreateDIBSection(HANDLE, const BMI_*, u32, void**, HANDLE, DWORD);
W HANDLE API SelectObject(HANDLE, HANDLE);
W BOOL   API DeleteObject(HANDLE);

int _fltused = 0;
void* memcpy(void* d, const void* s, u64 n) { u8* a = (u8*)d; const u8* b = (const u8*)s; while (n--) *a++ = *b++; return d; }
void* memset(void* d, int c, u64 n) { u8* a = (u8*)d; while (n--) *a++ = (u8)c; return d; }
static int eq12(const float* a, const float* b) { return a[0] == b[0] && a[1] == b[1] && a[2] == b[2]; }
static int slen(const char* s) { int n = 0; while (s[n]) n++; return n; }

// ---------------------------------------------------------------- config
static int cfgEnabled = 1, cfgSide = 500, cfgHeight = 80, cfgBack = 1300, cfgLook = 3000, cfgFace = 1, cfgLog = 1, cfgStrafe = 1, cfgSpeed = 25, cfgMoveMode = 1, cfgHold = 1200, cfgLegs = 1, cfgBlend = 8, cfgAimMode = 1, cfgAimKey = 2, cfgLegSpeed = 100, cfgProneSpeed = 8, cfgCrouchH = -300, cfgProneH = 0, cfgPitchDown = 60, cfgPitchMode = 1, cfgProneCrawl = 10, cfgSpine = 1, cfgHotkeys = 0, cfgSpStand = 100, cfgSpCrouch = 100, cfgZoomKey = 0, cfgZoomPct = 200, cfgSpineProne = 1, cfgLeft = 0, cfgPitch = 40, cfgPitchOff = 11, cfgSight = 1, cfgSightScale = 100, cfgSightOpacity = 100, cfgProneFloor = -20, cfgConverge = 1, cfgConvMax = 100000, cfgConvDist = 15000, cfgCrawlRifle = 12, cfgCrawlMask = 6144, cfgPad = 256, cfgPoseStand = -1, cfgPoseCrouch = -1, cfgObstacle = 1, cfgObsRange = 3000, cfgObsNear = 2500, cfgObsMax = 260, cfgEntryPitch = 0, cfgProneBack = 13, cfgProneSide = 80, cfgTilt = 1, cfgTiltSpeed = 150, cfgShots = 1, cfgFov = 100, cfgGhost = 1;
static int cfgWalk[4] = { 204, 205, 206, 207 }, cfgCWalk[4] = { 216, 217, 218, 219 };   // forward, back, left, right
static int cfgProne[2] = { 200, 201 };   // left, right (in the equipped weapon's archive)
static int cfgRun[4] = { 200, 201, 202, 203 }, cfgCRun[4] = { 220, 221, 222, 223 };   // the faster walks: forward, back, left, right
static int cfgProneMain[2] = { 239, 240 };   // lying left / right in Snake's main archive (weapons whose own archive has none)
static int cfgTwoSpeed = 1, cfgFastAbove = 170, cfgSlowKey = 0, cfgRunRoot = 384, cfgCRunRoot = 282, cfgProneWhole = 1;
static int cfgNoOts = 1040449558, cfgNoRaise = 6, cfgSync = 1, cfgAfterFpv = 1, cfgLookDown = 1;
// PoseTurn - how far (degrees) Snake's upper body may twist sideways toward the aim point (the game allows 15);
// PadDeadZone - percent of the left stick's travel below which the shoulder view does not walk him (the game's own:
// about 37)
static int cfgPoseTurn = 45, cfgPadDZ = 24;
// BushFirstPerson - 1 = the game's own automatic first person when Snake lies in a bush (0 = he stays in third person)
static int cfgCollide = 1, cfgFollow = 1, cfgNoWall = 1;
static int cfgCrouchAll = 1, cfgWalkAbove = 90, cfgStalkPct = 60, cfgCrouchArms = 1, cfgBodyScale = 126, cfgBodyEase = 100;
static int cfgThrowMask = 0x1F80008, cfgThrowDown = 10, cfgProneLook = 100, cfgFpvHold = 1;
static int cfgCrouchWalkPct = 120, cfgFpvBackMs = 180, cfgZoomDelay = 200;
static int cfgStandWalkPct = 130, cfgProneSideAnim = 1, cfgRecoil = 1, cfgRecoilPct = 30;
static int cfgRigid = 1, cfgSpeedSwitch = 150, cfgRecoilView = 0;
static int cfgHeadLock = 1, cfgRecoilShake = 0, cfgPadDir = 1, cfgKeyWalk = 1, cfgStalkKey = 16;
static int cfgSightGap = 70, cfgSpreadMask = 0x3E00, cfgCrouchRunPct = 130, cfgBlockCamBtn = 1, cfgPadLookUp = 0, cfgDpadStalk = 1, cfgPerfLog = 1;
static int cfgFastMem = 1, cfgCamRadius = 100, cfgCamMinBack = 300;
static int cfgCamIn = 45, cfgCamOut = 7, cfgCamHold = 8;   // collision smoothing
static int cfgCw = 1, cfgCwWalkRate = 360, cfgCwStalkRate = 300, cfgCwCamoPct = 100, cfgCwCamoAdd = 0, cfgCwWalkCamo = -5;   // built-in crouch walking
static int cfgCwStalkCamo = 0, cfgCwNormCamo = -10;   // on top of the game's own: crouchnormal -10, crouchwalk -5, crouchstalk 0
static int cfgSnd = 1, cfgSndStandFast = 5, cfgSndStandSlow = 4, cfgSndCrouchFast = 4, cfgSndCrouchSlow = 3, cfgSndProne = 12;   // footstep sounds
static int cfgCrawlSnd = 1;   // the game's crawl sounds / grass marks in the shoulder view
static int cfgStandStalkPct = 78, cfgSndStandStalk = 3, cfgMoveMarks = 1, cfgGameStalk = 1, cfgPadWake = 1, cfgLearnAim = 1;   // standing stalk in the shoulder view; the game's walk / run / stalk marks
static int cfgFpvKey = 0, cfgLoadTps = 1;   // the first-person key (learned), loads always in third person
static int cfgFpvYaw = 1;   // back from first person, the shoulder view looks where first person looked
// PoseFix - the barrel is brought onto the crosshair spot vertically (closed loop, see PoseFix); PoseFixGain percent of
// the error a frame; PoseFixMax degrees at most; PoseFixCheck degrees - how well a hand bone must predict the shots;
// PoseDown - how far (degrees) the plugin lets the arms point down (the game's own limit is 50)
static int cfgPoseFix = 0, cfgPoseFixGain = 20, cfgPoseFixMax = 45, cfgPoseFixCheck = 3, cfgPoseDown = 70;
// HeadHoldUnits - firing, the camera ignores at most this much of his head's movement (the kick); ShotWalkHoldMs - the
// walk chosen before a shot is kept this long after it; NoCrosshairWeapons (2^type, 1152 = EZ gun 7 + Scorpion 10),
// HideWhileCrawling, CrawlHideMs; CollisionFlags (0 = the game camera's own) / CollisionSkipFlags for the camera's
// tests
static int cfgHeadHoldU = 24, cfgShotWalkMs = 400, cfgNoSightMask = 1152, cfgSightCrawl = 1, cfgCrawlHideMs = 300, cfgCamFlags = 0, cfgCamSkip = 0x800000;
// PoseAim - the arms are turned so that the barrel line, which the game turns about his shoulder (bone 10), runs
// through the crosshair spot (the game measures its angle from his hand: near a wall or a post the gun went past the
// spot by half as much again); PoseAimBarrel - the barrel's height above the hand bone until his shots have measured
// it; PoseAimWeapons (2^type, 5 - 17: the guns); FireTrace - frame-by-frame lines after that many fire presses (0 =
// none)
static int cfgPoseAim = 1, cfgPoseAimBarrel = 50, cfgPoseAimMask = 0x3FFE0, cfgWalkDebounce = 6, cfgFireTrace = 8;
static int cfgFpvKeepL1 = 1, cfgClassicBlock = 1, cfgCrawlWalkPct = 60;
static int cfgFpvRestore = 1;
static int cfgFpvHandBack = 1, cfgR1Keep = 1;
static volatile u32 g_fpHandBackN, g_r1KeepN;
static char g_dir[260], g_ini[300], g_logPath[300];
static HANDLE g_log = (HANDLE)-1;

// the log is not written to disk from the game's own thread: lines go into a memory buffer and the mod's
// background thread writes them out every 30 ms.
static char g_lbuf[2][262144]; static volatile u32 g_lfill[2]; static volatile int g_lcur, g_llock, g_lthread; static volatile u32 g_ldrop;
static volatile u32 g_logCalls;
static int cfgLogLvl = 1;
static int PreIs(const char* s, const char* p) { while (*p) if (*s++ != *p++) return 0; return 1; }
static int Verbose(const char* s)
{
    static const char* v[] = { "t=", "perf:", "frame pacing", "fire trace", "fire press", "speeds", "camo index", "shot ", "shot:", "gun aim", "new scene +",
        "active:", "first person ended - status", "first person ended: statuses", "back to a wall", "shoulder view on - the game", "window in front", "recoil rolled",
        "camera lock:", "crosshair:", "learned crawl", "lying, shoulder view -", "crawling in the shoulder view", "footsteps:", "shoulder view moving", "strafe:",
        "rigid camera", "camera locked", "first-person eye", "over-the-shoulder off: the game's camera height", "back from first person - CHECK", "upper body", 0 };
    if (PreIs(s, "summary")) return 1;
    for (int i = 0; v[i]; i++) if (PreIs(s, v[i])) return 1;
    return 0;
}
static void Log(const char* s)
{
    g_logCalls++;
    if (g_log == (HANDLE)-1) return;
    if (cfgLogLvl < 2 && Verbose(s)) return;
    u32 n = (u32)slen(s);
    if (!g_lthread) { DWORD w; WriteFile(g_log, s, n, &w, 0); WriteFile(g_log, "\r\n", 2, &w, 0); return; }
    while (__sync_lock_test_and_set(&g_llock, 1)) { }
    int c = g_lcur; u32 f = g_lfill[c];
    if (f + n + 2 <= sizeof g_lbuf[0]) { memcpy(g_lbuf[c] + f, s, n); g_lbuf[c][f + n] = '\r'; g_lbuf[c][f + n + 1] = '\n'; g_lfill[c] = f + n + 2; }
    else g_ldrop++;
    __sync_lock_release(&g_llock);
}
static void LogFlush(void)
{
    if (g_log == (HANDLE)-1) return;
    while (__sync_lock_test_and_set(&g_llock, 1)) { }
    int c = g_lcur; u32 f = g_lfill[c];
    if (f) { g_lcur = c ^ 1; g_lfill[c ^ 1] = 0; }
    __sync_lock_release(&g_llock);
    if (f) { DWORD w; WriteFile(g_log, g_lbuf[c], f, &w, 0); }
}


static void LoadConfig(void)
{
    cfgEnabled = (int)GetPrivateProfileIntA("Mod", "Enabled", (int)GetPrivateProfileIntA("Shoulder", "Enabled", 1, g_ini), g_ini);   // [Mod]
    cfgSide    = (int)GetPrivateProfileIntA("Shoulder", "Side", 500, g_ini);
    cfgHeight  = (int)GetPrivateProfileIntA("Shoulder", "Height", 80, g_ini);
    cfgBack    = (int)GetPrivateProfileIntA("Shoulder", "Back", 1300, g_ini);
    cfgLeft    = (int)GetPrivateProfileIntA("Shoulder", "LeftShoulder", 0, g_ini);
    cfgFace    = 1;
    cfgStrafe  = (int)GetPrivateProfileIntA("Shoulder", "Strafe", 1, g_ini);
    cfgSpeed   = (int)GetPrivateProfileIntA("Shoulder", "StrafeSpeed", 25, g_ini);
    if (cfgSpeed < 0) cfgSpeed = 0;
    if (cfgSpeed > 200) cfgSpeed = 200;
    cfgMoveMode = (int)GetPrivateProfileIntA("Shoulder", "MoveMode", 1, g_ini);
    cfgHold    = (int)GetPrivateProfileIntA("Shoulder", "AimHold", 1200, g_ini);
    cfgLegs    = (int)GetPrivateProfileIntA("Legs", "Enabled", 1, g_ini);
    cfgAimMode = (int)GetPrivateProfileIntA("Aim", "Mode", 1, g_ini);
    cfgAimKey  = (int)GetPrivateProfileIntA("Aim", "Key", 2, g_ini);
    if (cfgAimKey < 1 || cfgAimKey > 254) cfgAimKey = 2;
    cfgLegSpeed = (int)GetPrivateProfileIntA("Legs", "Speed", 100, g_ini);
    if (cfgLegSpeed < 20) cfgLegSpeed = 20;
    if (cfgLegSpeed > 300) cfgLegSpeed = 300;
    cfgProneSpeed = (int)GetPrivateProfileIntA("Legs", "ProneSpeed", 8, g_ini);
    if (cfgProneSpeed < 0) cfgProneSpeed = 0;
    if (cfgProneSpeed > 100) cfgProneSpeed = 100;
    cfgProneCrawl = (int)GetPrivateProfileIntA("Legs", "ProneCrawl", 10, g_ini);
    cfgSpine = (int)GetPrivateProfileIntA("Spine", "Enabled", 1, g_ini);
    cfgHotkeys = (int)GetPrivateProfileIntA("Debug", "Hotkeys", 0, g_ini);
    cfgSpStand = (int)GetPrivateProfileIntA("Legs", "SpeedStanding", 100, g_ini);
    cfgSpCrouch = (int)GetPrivateProfileIntA("Legs", "SpeedCrouched", 100, g_ini);
    if (cfgSpStand < 20) cfgSpStand = 20;
    if (cfgSpStand > 300) cfgSpStand = 300;
    if (cfgSpCrouch < 20) cfgSpCrouch = 20;
    if (cfgSpCrouch > 300) cfgSpCrouch = 300;
    cfgZoomKey = (int)GetPrivateProfileIntA("Aim", "ZoomKey", 0, g_ini) & 0xFF;
    cfgZoomPct = (int)GetPrivateProfileIntA("Aim", "ZoomPercent", 200, g_ini);
    if (cfgZoomPct < 10) cfgZoomPct = 10;
    if (cfgZoomPct > 1000) cfgZoomPct = 1000;
    cfgSpineProne = (int)GetPrivateProfileIntA("Spine", "Prone", 1, g_ini);
    cfgProne[0] = (int)GetPrivateProfileIntA("Legs", "ProneLeft", 200, g_ini);
    cfgProne[1] = (int)GetPrivateProfileIntA("Legs", "ProneRight", 201, g_ini);
    cfgBlend   = (int)GetPrivateProfileIntA("Legs", "Blend", 8, g_ini);
    if (cfgBlend < 0) cfgBlend = 0;
    if (cfgBlend > 60) cfgBlend = 60;
    {
        static const char* kn[4] = { "Forward", "Back", "Left", "Right" };
        static const char* kc[4] = { "CrouchForward", "CrouchBack", "CrouchLeft", "CrouchRight" };
        static const int dw[4] = { 204, 205, 206, 207 }, dc[4] = { 216, 217, 218, 219 };
        for (int i = 0; i < 4; i++) {
            cfgWalk[i]  = (int)GetPrivateProfileIntA("Legs", kn[i], dw[i], g_ini);
            cfgCWalk[i] = (int)GetPrivateProfileIntA("Legs", kc[i], dc[i], g_ini);
        }
    }
    if (cfgHold < 0) cfgHold = 0;
    if (cfgHold > 10000) cfgHold = 10000;
    cfgPitchOff = (int)GetPrivateProfileIntA("Shoulder", "PitchOffset", 11, g_ini);
    if (cfgPitchOff < -30) cfgPitchOff = -30;
    if (cfgPitchOff > 30) cfgPitchOff = 30;
    cfgPitch   = (int)GetPrivateProfileIntA("Shoulder", "PitchLimit", 40, g_ini);
    if (cfgPitch < 5) cfgPitch = 5;
    if (cfgPitch > 80) cfgPitch = 80;
    cfgPitchDown = (int)GetPrivateProfileIntA("Shoulder", "PitchDown", 60, g_ini);
    if (cfgPitchDown < 5) cfgPitchDown = 5;
    if (cfgPitchDown > 80) cfgPitchDown = 80;
    cfgPitchMode = (int)GetPrivateProfileIntA("Shoulder", "PitchMode", 1, g_ini);
    cfgCrouchH = (int)GetPrivateProfileIntA("Shoulder", "CrouchHeight", -300, g_ini);
    cfgProneH  = (int)GetPrivateProfileIntA("Shoulder", "ProneHeight", 0, g_ini);
    cfgLog     = (int)GetPrivateProfileIntA("Debug", "Log", 1, g_ini);
    cfgLogLvl = cfgLog < 0 ? 0 : cfgLog > 2 ? 2 : cfgLog;   // 1 = basic, 2 = advanced
    cfgSight   = (int)GetPrivateProfileIntA("Sight", "Enabled", 1, g_ini);
    cfgConverge = (int)GetPrivateProfileIntA("Aim", "Converge", 3, g_ini);
    cfgPoseTurn = (int)GetPrivateProfileIntA("Aim", "PoseTurn", 45, g_ini); if (cfgPoseTurn < 0) cfgPoseTurn = 0; if (cfgPoseTurn > 80) cfgPoseTurn = 80;
    cfgPadDZ = (int)GetPrivateProfileIntA("Legs", "PadDeadZone", 24, g_ini); if (cfgPadDZ < 0) cfgPadDZ = 0; if (cfgPadDZ > 60) cfgPadDZ = 60;
    cfgConvDist = (int)GetPrivateProfileIntA("Aim", "ConvergeDistance", 15000, g_ini);
    if (cfgConvDist < 2000) cfgConvDist = 2000; if (cfgConvDist > 300000) cfgConvDist = 300000;
    cfgCrawlRifle = (int)GetPrivateProfileIntA("Legs", "ProneCrawlRifle", 12, g_ini);
    cfgCrawlMask = (int)GetPrivateProfileIntA("Legs", "ProneCrawlRifleWeapons", 6144, g_ini);
    cfgPad = (int)GetPrivateProfileIntA("Aim", "PadButton", 256, g_ini);
    cfgPoseStand = (int)GetPrivateProfileIntA("Pose", "StandAim", -1, g_ini);
    cfgPoseCrouch = (int)GetPrivateProfileIntA("Pose", "CrouchAim", -1, g_ini);
    cfgConvMax = (int)GetPrivateProfileIntA("Aim", "Range", 100000, g_ini);
    if (cfgConvMax < 3000) cfgConvMax = 3000; if (cfgConvMax > 300000) cfgConvMax = 300000;
    cfgShots = (int)GetPrivateProfileIntA("Aim", "ShotsToCrosshair", 1, g_ini);
    cfgEntryPitch = (int)GetPrivateProfileIntA("Shoulder", "EntryPitch", 0, g_ini);
    if (cfgEntryPitch < -30) cfgEntryPitch = -30; if (cfgEntryPitch > 30) cfgEntryPitch = 30;
    cfgProneBack = (int)GetPrivateProfileIntA("Legs", "ProneBack", 13, g_ini);
    cfgProneSide = (int)GetPrivateProfileIntA("Legs", "ProneSidePercent", 80, g_ini);
    if (cfgProneSide < 10) cfgProneSide = 10; if (cfgProneSide > 300) cfgProneSide = 300;
    cfgTilt = (int)GetPrivateProfileIntA("Sight", "Tilt", 1, g_ini);
    cfgTiltSpeed = (int)GetPrivateProfileIntA("Sight", "TiltSpeed", 150, g_ini);
    if (cfgTiltSpeed < 10) cfgTiltSpeed = 10; if (cfgTiltSpeed > 2000) cfgTiltSpeed = 2000;
    cfgFov = (int)GetPrivateProfileIntA("Sight", "FovPercent", 100, g_ini);
    if (cfgFov < 50) cfgFov = 50; if (cfgFov > 200) cfgFov = 200;
    cfgGhost = (int)GetPrivateProfileIntA("Sight", "Ghost", 1, g_ini);
    cfgSightScale = (int)GetPrivateProfileIntA("Sight", "Scale", 100, g_ini);
    if (cfgSightScale < 20) cfgSightScale = 20; if (cfgSightScale > 400) cfgSightScale = 400;
    cfgSightOpacity = (int)GetPrivateProfileIntA("Sight", "Opacity", 100, g_ini);
    if (cfgSightOpacity < 0) cfgSightOpacity = 0; if (cfgSightOpacity > 200) cfgSightOpacity = 200;
    cfgProneFloor = (int)GetPrivateProfileIntA("Shoulder", "ProneFloor", -20, g_ini);
    cfgObstacle = (int)GetPrivateProfileIntA("Sight", "Obstacle", 1, g_ini);
    cfgObsRange = (int)GetPrivateProfileIntA("Sight", "ObstacleRange", 3000, g_ini);
    if (cfgObsRange < 500) cfgObsRange = 500; if (cfgObsRange > 10000) cfgObsRange = 10000;
    cfgObsNear = (int)GetPrivateProfileIntA("Sight", "ObstacleDistance", 2500, g_ini);
    if (cfgObsNear < 300) cfgObsNear = 300; if (cfgObsNear > 10000) cfgObsNear = 10000;
    cfgObsMax = (int)GetPrivateProfileIntA("Sight", "ObstacleSpread", 260, g_ini);
    if (cfgObsMax < 100) cfgObsMax = 100; if (cfgObsMax > 600) cfgObsMax = 600;
    if (cfgBack < 100) cfgBack = 100;
    if (cfgBack > 5000) cfgBack = 5000;
    {
        static const char* kf[4] = { "FastForward", "FastBack", "FastLeft", "FastRight" };
        static const char* kg[4] = { "CrouchFastForward", "CrouchFastBack", "CrouchFastLeft", "CrouchFastRight" };
        static const int df[4] = { 200, 201, 202, 203 }, dg[4] = { 220, 221, 222, 223 };
        for (int i = 0; i < 4; i++) {
            cfgRun[i]  = (int)GetPrivateProfileIntA("Legs", kf[i], df[i], g_ini);
            cfgCRun[i] = (int)GetPrivateProfileIntA("Legs", kg[i], dg[i], g_ini);
        }
    }
    cfgTwoSpeed = (int)GetPrivateProfileIntA("Legs", "TwoSpeeds", 1, g_ini);
    cfgFastAbove = (int)GetPrivateProfileIntA("Legs", "FastAbove", 170, g_ini);
    if (cfgFastAbove < 30) cfgFastAbove = 30; if (cfgFastAbove > 255) cfgFastAbove = 255;
    cfgSlowKey = (int)GetPrivateProfileIntA("Legs", "SlowKey", 0, g_ini) & 0xFF;
    cfgStandWalkPct = (int)GetPrivateProfileIntA("Legs", "StandWalkPercent", 130, g_ini);
    if (cfgStandWalkPct < 30) cfgStandWalkPct = 30; if (cfgStandWalkPct > 250) cfgStandWalkPct = 250;
    cfgStandStalkPct = (int)GetPrivateProfileIntA("Legs", "StandStalkPercent", 78, g_ini);
    if (cfgStandStalkPct < 20) cfgStandStalkPct = 20; if (cfgStandStalkPct > 250) cfgStandStalkPct = 250;
    cfgProneSideAnim = (int)GetPrivateProfileIntA("Legs", "ProneSideFromAnimation", 1, g_ini);
    cfgRecoil = (int)GetPrivateProfileIntA("Aim", "Recoil", 1, g_ini);
    cfgRecoilPct = (int)GetPrivateProfileIntA("Aim", "RecoilPercent", 30, g_ini);   // 70 % less spread
    if (cfgRecoilPct < 0) cfgRecoilPct = 0; if (cfgRecoilPct > 400) cfgRecoilPct = 400;
    cfgRunRoot = (int)GetPrivateProfileIntA("Legs", "FastSpeed", 384, g_ini);
    if (cfgRunRoot < 50) cfgRunRoot = 50; if (cfgRunRoot > 1500) cfgRunRoot = 1500;
    cfgCRunRoot = (int)GetPrivateProfileIntA("Legs", "CrouchFastSpeed", 282, g_ini);
    if (cfgCRunRoot < 50) cfgCRunRoot = 50; if (cfgCRunRoot > 1500) cfgCRunRoot = 1500;
    cfgProneMain[0] = (int)GetPrivateProfileIntA("Legs", "ProneLeftPlain", 239, g_ini);
    cfgProneMain[1] = (int)GetPrivateProfileIntA("Legs", "ProneRightPlain", 240, g_ini);
    cfgProneWhole = (int)GetPrivateProfileIntA("Legs", "ProneWholeBody", 1, g_ini);
    cfgNoOts = (int)GetPrivateProfileIntA("Aim", "NoShoulderWeapons", 1040449558, g_ini);   // + torch (18)
    cfgCrouchAll = (int)GetPrivateProfileIntA("Legs", "CrouchRunEverywhere", 1, g_ini);
    cfgWalkAbove = (int)GetPrivateProfileIntA("Legs", "WalkAbove", 90, g_ini);
    cfgStalkPct = (int)GetPrivateProfileIntA("Legs", "StalkPercent", 60, g_ini);
    cfgCrouchWalkPct = (int)GetPrivateProfileIntA("Legs", "CrouchWalkPercent", 120, g_ini);   // the shoulder view's crouch walk
    if (cfgCrouchWalkPct < 30) cfgCrouchWalkPct = 30; if (cfgCrouchWalkPct > 250) cfgCrouchWalkPct = 250;
    cfgFpvBackMs = (int)GetPrivateProfileIntA("Aim", "FirstPersonBackMs", 180, g_ini);
    if (cfgFpvBackMs < 0) cfgFpvBackMs = 0; if (cfgFpvBackMs > 1000) cfgFpvBackMs = 1000;
    cfgZoomDelay = (int)GetPrivateProfileIntA("Aim", "ZoomKeyDelay", 200, g_ini);
    if (cfgStalkPct < 10) cfgStalkPct = 10; if (cfgStalkPct > 100) cfgStalkPct = 100;
    cfgCrouchArms = (int)GetPrivateProfileIntA("Legs", "CrouchRunKeepsWeapon", 1, g_ini);
    cfgBodyScale = (int)GetPrivateProfileIntA("Shoulder", "BodyScale", 126, g_ini);
    if (cfgBodyScale < 20) cfgBodyScale = 20; if (cfgBodyScale > 400) cfgBodyScale = 400;
    cfgBodyEase = (int)GetPrivateProfileIntA("Shoulder", "BodyEase", 100, g_ini);
    cfgRigid = (int)GetPrivateProfileIntA("Shoulder", "CameraRigid", 1, g_ini);
    cfgHeadLock = (int)GetPrivateProfileIntA("Shoulder", "LockToHead", 1, g_ini);
    cfgRecoilShake = (int)GetPrivateProfileIntA("Aim", "RecoilShakePercent", 0, g_ini);
    if (cfgRecoilShake < 0) cfgRecoilShake = 0; if (cfgRecoilShake > 400) cfgRecoilShake = 400;
    cfgPadDir = (int)GetPrivateProfileIntA("Legs", "PadDirection", 1, g_ini);
    cfgKeyWalk = (int)GetPrivateProfileIntA("Legs", "KeyboardWalkKeys", 1, g_ini);
    cfgStalkKey = (int)GetPrivateProfileIntA("Legs", "StalkKey", 16, g_ini);
    cfgSightGap = (int)GetPrivateProfileIntA("Sight", "GapPercent", 70, g_ini);
    if (cfgSightGap < 20) cfgSightGap = 20; if (cfgSightGap > 300) cfgSightGap = 300;
    cfgSpreadMask = (int)GetPrivateProfileIntA("Aim", "RecoilSpreadWeapons", 0x3E00, g_ini);
    cfgCrouchRunPct = (int)GetPrivateProfileIntA("Legs", "CrouchRunPercent", 130, g_ini);
    if (cfgCrouchRunPct < 50) cfgCrouchRunPct = 50; if (cfgCrouchRunPct > 200) cfgCrouchRunPct = 200;
    cfgBlockCamBtn = (int)GetPrivateProfileIntA("Aim", "PadBlockCameraButton", 1, g_ini);
    cfgPadLookUp = (int)GetPrivateProfileIntA("Aim", "PadStickUpLooksUp", 0, g_ini);   // off - the game's own camera setting decides
    cfgDpadStalk = (int)GetPrivateProfileIntA("Legs", "PadDpadStalk", 1, g_ini);
    cfgPerfLog = (int)GetPrivateProfileIntA("Debug", "PerfLog", 1, g_ini);
    cfgFastMem = (int)GetPrivateProfileIntA("Debug", "FastMemoryCheck", 1, g_ini);
    cfgCw = (int)GetPrivateProfileIntA("CrouchWalk", "Enabled", 1, g_ini);
    cfgCwWalkRate = (int)GetPrivateProfileIntA("CrouchWalk", "WalkSpeed", 360, g_ini);   // the normal view's crouch walk (walk toggle); full stick stays 6.0
    if (cfgCwWalkRate < 100) cfgCwWalkRate = 100; if (cfgCwWalkRate > 2000) cfgCwWalkRate = 2000;
    cfgCwStalkRate = (int)GetPrivateProfileIntA("CrouchWalk", "StalkSpeed", 300, g_ini);
    if (cfgCwStalkRate < 50) cfgCwStalkRate = 50; if (cfgCwStalkRate > 2000) cfgCwStalkRate = 2000;
    cfgCwCamoPct = (int)GetPrivateProfileIntA("CrouchWalk", "CamoIndexPercent", 100, g_ini);
    if (cfgCwCamoPct < 10) cfgCwCamoPct = 10; if (cfgCwCamoPct > 400) cfgCwCamoPct = 400;
    cfgCwCamoAdd = (int)GetPrivateProfileIntA("CrouchWalk", "CamoIndexAdd", 0, g_ini);
    cfgCwWalkCamo = (int)GetPrivateProfileIntA("CrouchWalk", "CamoCrouchWalk", -5, g_ini);
    cfgCwNormCamo = (int)GetPrivateProfileIntA("CrouchWalk", "CamoCrouchNormal", -10, g_ini);
    cfgCwStalkCamo = (int)GetPrivateProfileIntA("CrouchWalk", "CamoCrouchStalk", 0, g_ini);
    cfgSnd = (int)GetPrivateProfileIntA("Sounds", "Enabled", 1, g_ini);
    cfgSndStandFast = (int)GetPrivateProfileIntA("Sounds", "StandFast", 5, g_ini);
    cfgSndStandSlow = (int)GetPrivateProfileIntA("Sounds", "StandSlow", 4, g_ini);
    cfgSndCrouchFast = (int)GetPrivateProfileIntA("Sounds", "CrouchFast", 4, g_ini);
    cfgSndCrouchSlow = (int)GetPrivateProfileIntA("Sounds", "CrouchSlow", 3, g_ini);
    cfgSndProne = (int)GetPrivateProfileIntA("Sounds", "Prone", 12, g_ini);
    cfgCrawlSnd = (int)GetPrivateProfileIntA("Sounds", "Crawl", 1, g_ini);
    cfgSndStandStalk = (int)GetPrivateProfileIntA("Sounds", "StandStalk", 3, g_ini);
    cfgMoveMarks = (int)GetPrivateProfileIntA("Sounds", "MoveMarks", 1, g_ini);
    cfgGameStalk = (int)GetPrivateProfileIntA("Legs", "GameStalkInput", 1, g_ini);
    cfgPadWake = (int)GetPrivateProfileIntA("Aim", "PadRaisesGunAfterLoad", 1, g_ini);
    cfgLearnAim = (int)GetPrivateProfileIntA("Aim", "LearnAimKey", 0, g_ini);   // 0
    cfgFpvYaw = (int)GetPrivateProfileIntA("Shoulder", "FirstPersonDirection", 1, g_ini);
    cfgFpvKey = (int)GetPrivateProfileIntA("Aim", "FirstPersonKey", 0, g_ini); if (cfgFpvKey < 0 || cfgFpvKey > 254) cfgFpvKey = 0;
    cfgLoadTps = (int)GetPrivateProfileIntA("Aim", "LoadInThirdPerson", 1, g_ini);
    cfgCamRadius = (int)GetPrivateProfileIntA("Shoulder", "WallDistance", 100, g_ini);
    cfgCamIn = (int)GetPrivateProfileIntA("Shoulder", "CollisionInPercent", 45, g_ini);
    if (cfgCamIn < 5) cfgCamIn = 5; if (cfgCamIn > 100) cfgCamIn = 100;
    cfgCamOut = (int)GetPrivateProfileIntA("Shoulder", "CollisionOutPercent", 7, g_ini);
    if (cfgCamOut < 1) cfgCamOut = 1; if (cfgCamOut > 100) cfgCamOut = 100;
    cfgCamHold = (int)GetPrivateProfileIntA("Shoulder", "CollisionHoldFrames", 8, g_ini);
    if (cfgCamHold < 0) cfgCamHold = 0; if (cfgCamHold > 120) cfgCamHold = 120;
    if (cfgCamRadius < 0) cfgCamRadius = 0; if (cfgCamRadius > 300) cfgCamRadius = 300;
    cfgCamMinBack = (int)GetPrivateProfileIntA("Shoulder", "MinBack", 300, g_ini);
    if (cfgCamMinBack < 100) cfgCamMinBack = 100; if (cfgCamMinBack > 1000) cfgCamMinBack = 1000;
    cfgSpeedSwitch = (int)GetPrivateProfileIntA("Legs", "SpeedSwitchMs", 150, g_ini);
    if (cfgSpeedSwitch < 0) cfgSpeedSwitch = 0; if (cfgSpeedSwitch > 1000) cfgSpeedSwitch = 1000;
    cfgRecoilView = (int)GetPrivateProfileIntA("Aim", "RecoilViewPercent", 0, g_ini);   // no AK climb by default
    if (cfgRecoilView < 0) cfgRecoilView = 0; if (cfgRecoilView > 400) cfgRecoilView = 400;
    if (cfgBodyEase < 1) cfgBodyEase = 1; if (cfgBodyEase > 100) cfgBodyEase = 100;
    cfgProneLook = (int)GetPrivateProfileIntA("Shoulder", "ProneLookDownBack", 100, g_ini);
    if (cfgProneLook < 0) cfgProneLook = 0; if (cfgProneLook > 400) cfgProneLook = 400;
    cfgThrowMask = (int)GetPrivateProfileIntA("Aim", "ThrowableWeapons", 0x1F80008, g_ini);   // + cig spray (3)
    cfgThrowDown = (int)GetPrivateProfileIntA("Aim", "ThrowableLookDown", 10, g_ini);
    if (cfgThrowDown < 0) cfgThrowDown = 0; if (cfgThrowDown > 90) cfgThrowDown = 90;
    cfgFpvHold = (int)GetPrivateProfileIntA("Aim", "FirstPersonHandover", 1, g_ini);
    cfgNoRaise = (int)GetPrivateProfileIntA("Aim", "NoRaiseWeapons", 6, g_ini);
    cfgSync = (int)GetPrivateProfileIntA("Aim", "KeepInStep", 1, g_ini);
    cfgAfterFpv = (int)GetPrivateProfileIntA("Aim", "AfterFirstPerson", 1, g_ini);
    cfgLookDown = (int)GetPrivateProfileIntA("Aim", "LookDownPose", 1, g_ini);
    cfgCollide = (int)GetPrivateProfileIntA("Shoulder", "Collision", 1, g_ini);
    cfgFollow = (int)GetPrivateProfileIntA("Shoulder", "FollowBody", 1, g_ini);
    cfgNoWall = (int)GetPrivateProfileIntA("Shoulder", "NoWallPress", 1, g_ini);
    cfgPoseFix = (int)GetPrivateProfileIntA("Aim", "PoseFix", 0, g_ini);   // off (PoseAim instead)
    cfgPoseFixGain = (int)GetPrivateProfileIntA("Aim", "PoseFixGain", 20, g_ini); if (cfgPoseFixGain < 1) cfgPoseFixGain = 1; if (cfgPoseFixGain > 60) cfgPoseFixGain = 60;
    cfgPoseFixMax = (int)GetPrivateProfileIntA("Aim", "PoseFixMax", 45, g_ini); if (cfgPoseFixMax < 1) cfgPoseFixMax = 1; if (cfgPoseFixMax > 60) cfgPoseFixMax = 60;
    cfgPoseFixCheck = (int)GetPrivateProfileIntA("Aim", "PoseFixCheck", 3, g_ini); if (cfgPoseFixCheck < 1) cfgPoseFixCheck = 1; if (cfgPoseFixCheck > 15) cfgPoseFixCheck = 15;
    cfgPoseDown = (int)GetPrivateProfileIntA("Aim", "PoseDown", 70, g_ini); if (cfgPoseDown < 0) cfgPoseDown = 0; if (cfgPoseDown > 85) cfgPoseDown = 85;
    cfgHeadHoldU = (int)GetPrivateProfileIntA("Shoulder", "HeadHoldUnits", 24, g_ini); if (cfgHeadHoldU < 0) cfgHeadHoldU = 0; if (cfgHeadHoldU > 1000) cfgHeadHoldU = 1000;
    cfgShotWalkMs = (int)GetPrivateProfileIntA("Legs", "ShotWalkHoldMs", 400, g_ini); if (cfgShotWalkMs < 0) cfgShotWalkMs = 0; if (cfgShotWalkMs > 2000) cfgShotWalkMs = 2000;
    cfgNoSightMask = (int)GetPrivateProfileIntA("Sight", "NoCrosshairWeapons", 1152, g_ini);
    cfgSightCrawl = (int)GetPrivateProfileIntA("Sight", "HideWhileCrawling", 1, g_ini);
    cfgCrawlHideMs = (int)GetPrivateProfileIntA("Sight", "CrawlHideMs", 300, g_ini); if (cfgCrawlHideMs < 0) cfgCrawlHideMs = 0; if (cfgCrawlHideMs > 3000) cfgCrawlHideMs = 3000;
    cfgCamFlags = (int)GetPrivateProfileIntA("Shoulder", "CollisionFlags", 0, g_ini);
    cfgCamSkip = (int)GetPrivateProfileIntA("Shoulder", "CollisionSkipFlags", 0x800000, g_ini);
    cfgPoseAim = (int)GetPrivateProfileIntA("Aim", "PoseAim", 1, g_ini);
    cfgPoseAimBarrel = (int)GetPrivateProfileIntA("Aim", "PoseAimBarrel", 50, g_ini); if (cfgPoseAimBarrel < -300) cfgPoseAimBarrel = -300; if (cfgPoseAimBarrel > 300) cfgPoseAimBarrel = 300;
    cfgPoseAimMask = (int)GetPrivateProfileIntA("Aim", "PoseAimWeapons", 0x3FFE0, g_ini);
    cfgWalkDebounce = (int)GetPrivateProfileIntA("Legs", "WalkDebounceFrames", 6, g_ini); if (cfgWalkDebounce < 0) cfgWalkDebounce = 0; if (cfgWalkDebounce > 30) cfgWalkDebounce = 30;
    cfgFireTrace = (int)GetPrivateProfileIntA("Debug", "FireTrace", 8, g_ini); if (cfgFireTrace < 0) cfgFireTrace = 0; if (cfgFireTrace > 40) cfgFireTrace = 40;
    cfgFpvKeepL1 = (int)GetPrivateProfileIntA("Aim", "PadFirstPersonKeepsButton", 1, g_ini);
    cfgClassicBlock = (int)GetPrivateProfileIntA("Aim", "NoShoulderInClassicCamera", 1, g_ini);
    cfgFpvRestore = (int)GetPrivateProfileIntA("Aim", "PadFirstPersonRestore", 1, g_ini);
    cfgFpvHandBack = (int)GetPrivateProfileIntA("Aim", "PadFirstPersonHandBack", 1, g_ini);
    cfgR1Keep = (int)GetPrivateProfileIntA("Aim", "PadFirstPersonKeepsRB", 1, g_ini);
    cfgCrawlWalkPct = (int)GetPrivateProfileIntA("Legs", "CrawlWalkPercent", 60, g_ini); if (cfgCrawlWalkPct < 10) cfgCrawlWalkPct = 10; if (cfgCrawlWalkPct > 100) cfgCrawlWalkPct = 100;
}

// ---------------------------------------------------------------- pattern scan (main module .text)
static u8* g_base;
static u8* g_text; static u64 g_textSize;

static int FindText(void)
{
    u8* b = g_base; u32 pe = *(u32*)(b + 0x3C);
    u16 nsec = *(u16*)(b + pe + 6); u16 optsz = *(u16*)(b + pe + 20);
    u8* sec = b + pe + 24 + optsz;
    for (int i = 0; i < nsec; i++, sec += 40)
        if (sec[0] == '.' && sec[1] == 't' && sec[2] == 'e' && sec[3] == 'x' && sec[4] == 't') {
            g_textSize = *(u32*)(sec + 8); g_text = b + *(u32*)(sec + 12); return 1;
        }
    return 0;
}

static int hexv(char c) { return c >= '0' && c <= '9' ? c - '0' : c >= 'A' && c <= 'F' ? c - 'A' + 10 : c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1; }

static u8* Scan(const char* pat)
{
    i16 p[96]; int n = 0;
    for (const char* s = pat; *s && n < 96;) {
        if (*s == ' ') { s++; continue; }
        if (*s == '?') { p[n++] = -1; s++; if (*s == '?') s++; continue; }
        p[n++] = (i16)(hexv(s[0]) * 16 + hexv(s[1])); s += 2;
    }
    u8* found = 0; int count = 0;
    for (u64 i = 0; i + (u64)n <= g_textSize; i++) {
        if (g_text[i] != (u8)p[0]) continue;
        int k = 1;
        for (; k < n; k++) if (p[k] >= 0 && g_text[i + k] != (u8)p[k]) break;
        if (k == n) { if (!found) found = g_text + i; count++; }
    }
    return count == 1 ? found : 0;
}
static u8* Rip(u8* ins, int disp, int len) { return ins ? ins + len + *(i32*)(ins + disp) : 0; }

// ---------------------------------------------------------------- game pointers
static volatile u32* g_bits;   // player status flag table
static volatile u32* p_held;   // held-trigger word
static volatile u32* p_gpad;   // gameplay pad block
static volatile i32* p_cutscene;
static volatile i32* p_scripted;

static int Bit(int n) { return g_bits ? (int)((g_bits[n >> 5] >> (n & 31)) & 1) : 0; }
// is that memory there to read? Now the few bytes are simply copied with ReadProcessMemory, which fails safely when
// they are not there and costs about a microsecond. FastMemoryCheck: 1 = this way (default), 2 = IsBadReadPtr, 0 = the
// old VirtualQuery way.
static volatile u32 g_rdN; static volatile u64 g_rdT;
static int ReadableRaw(const u8* p, u64 n)
{
    if (!n) n = 1;
    if ((u64)p + n < (u64)p) return 0;
    if (cfgFastMem == 2) return !IsBadReadPtr(p, n);
    if (cfgFastMem) {
        u8 tmp[2048]; u64 got = 0;
        if (n <= sizeof tmp) return ReadProcessMemory((HANDLE)(i64)-1, p, tmp, n, &got) && got == n;
        for (u64 o = 0; o < n; o += 4096) if (!ReadProcessMemory((HANDLE)(i64)-1, p + o, tmp, 1, &got) || got != 1) return 0;
        return ReadProcessMemory((HANDLE)(i64)-1, p + n - 1, tmp, 1, &got) && got == 1;
    }
    MBI m; if (!VirtualQuery(p, &m, sizeof m)) return 0;
    return m.State == 0x1000 && (u8*)m.BaseAddress + m.RegionSize >= p + n && !(m.Protect & 0x101);
}
static int Readable(const u8* p, u64 n)
{
    if (!p || (u64)p < 0x10000) return 0;
    LARGE_ a0, a1; QueryPerformanceCounter(&a0);
    int r = ReadableRaw(p, n);
    QueryPerformanceCounter(&a1); g_rdT += (u64)(a1.q - a0.q); g_rdN++;
    return r;
}

// status flags (from in-game tests): 0xC8 weapon raised, 0xCA third-person camera, 7 first-person view, 1 standing, 2
// crouched
enum { ST_FPV = 7, ST_STAND = 1, ST_CROUCH = 2, ST_WEAPON = 0xC8, ST_TPS = 0xCA, ST_WALL = 0xC6 };   // 0xC6: back against a wall

// ---------------------------------------------------------------- small math
static float fabs_(float v) { return v < 0 ? -v : v; }
static float atan2_(float y, float x)
{
    float ax = fabs_(x), ay = fabs_(y);
    float mx = ax > ay ? ax : ay, mn = ax > ay ? ay : ax;
    if (mx < 1e-9f) return 0;
    float a = mn / mx, s = a * a;
    float r = ((-0.0464964749f * s + 0.15931422f) * s - 0.327622764f) * s * a + a;
    if (ay > ax) r = 1.57079637f - r;
    if (x < 0) r = 3.14159274f - r;
    return y < 0 ? -r : r;
}

static float sin_small(float a) { return a - a * a * a / 6.0f + a * a * a * a * a / 120.0f; }   // |a| < ~1.5 rad
// The game's orbit camera moves its look-at point up and down as the view tilts. This table lets the shoulder pivot
// stay at a fixed height.
static float PivotDrift(float s)
{
    static const float ts[] = { -0.93f, -0.84f, -0.70f, -0.62f, -0.48f, -0.29f, -0.14f, 0.0f, 0.22f, 0.37f, 0.46f };
    static const float td[] = { -693.0f, -542.0f, -420.0f, -366.0f, -292.0f, -213.0f, -91.0f, 0.0f, 113.0f, 177.0f, 207.0f };
    if (s <= ts[0]) return td[0];
    for (int i = 1; i < 11; i++) if (s <= ts[i]) return td[i - 1] + (td[i] - td[i - 1]) * (s - ts[i - 1]) / (ts[i] - ts[i - 1]);
    return td[10];
}

// ---------------------------------------------------------------- the camera hook
// NewSphericalCamera work: +0x3D0 desired eye, +0x3E0 desired target (float4 each)
typedef void (*Apply_t)(u8* work);
static Apply_t o_Apply;
static float g_amt;   // 0..1 smoothed blend
static u8* c_work; static float c_oe[3], c_ot[3], c_me[3], c_mt[3]; static int c_valid;
static DWORD g_lastLog; static int g_wasAim;
static u8* volatile g_pl;   // Snake's object, from the movement function
static volatile u16 g_camYaw16; static volatile int g_camValid;   // camera heading from the last camera update
static volatile int g_strafeKey = -1, g_modeKey = -1;   // F6 / F7 overrides (-1 = take it from the ini)
static int Strafe(void) { return g_strafeKey >= 0 ? g_strafeKey : cfgStrafe; }
static int MoveMode(void) { return g_modeKey >= 0 ? g_modeKey : cfgMoveMode; }
static u8* g_after; static u64* g_savedRet;   // return-address swap for the movement function
static volatile u32 g_calls, g_hidden, g_velWrites;
static volatile i32* p_tick;   // game step length (5 or 6)
// input taken away from the game this frame
static volatile int g_have; static volatile u16 g_inMag, g_inYaw; static volatile u32 g_inFl;
static volatile u32 g_stickFixN, g_dzWalkN, g_poseWideN, g_poseLimN, g_gunAngSum, g_gunAngN, g_poseDownN; static volatile int g_poseYawMax;
static volatile u32 g_otsCamoN, g_bushN, g_l1FixN, g_gStalkT, g_gStalkN;
static int g_padPend; static DWORD g_padPendT; static volatile u32 g_aimKeyLearnN;
static int g_alSeen, g_alRises, g_alScore[256];   // aim key learning
static volatile u32 g_msg; static volatile u32 g_actRva;
static char g_phase[40]; static volatile int g_phaseN;   // order of the calls within one frame
static volatile float g_vx, g_vz;
static volatile int g_rawAim;
static HANDLE volatile g_gameWnd; static volatile int g_fgNow, g_fireNow; static volatile DWORD g_applyT;   // for the crosshair
static volatile int g_sightHide; static volatile u32 g_sightCrawlN, g_sightLaserN, g_sightHideLog;   // crosshair hidden (1 crawling, 2 laser sight)
static volatile int g_crouchLatch, g_proneLatch, g_transLatch;
static volatile float g_aimDir[3]; static volatile int g_aimDirOk; static volatile u32 g_spineSets;
static volatile int g_spineKey = -1;
static int Spine(void) { return g_spineKey >= 0 ? g_spineKey : cfgSpine; }
static volatile int g_ex, g_emin, g_emax, g_corr;   // camera elevation value and its limits (x1000), for the log
static int LegPct(void);
static volatile u64 g_noLatch;   // weapon types the game's aim toggle does nothing for (throwables)
static volatile int g_zoom100, g_burst;   // zoom value seen (x100); frames left of every-frame logging
static int Wpn(u8* pl)   // type of the equipped weapon, 0 or less = none
{
    if (!Readable(pl, 0x800)) return -1;
    u8* w = *(u8**)(pl + 0x6F8);
    return Readable(w, 0x28) ? (int)w[0x20] : -1;
}
static u32 g_crawlArc[64]; static i16 g_crawlMot[64];   // crawl motion per weapon type, learned while playing
static volatile u32 g_wpnArc;   // archive of the equipped weapon, last seen
static volatile u32 g_legFrame; static volatile int g_tb;   // last frame a walk motion was forced; playback rate seen (x100)
static volatile u32 g_frame, g_moveFrame;   // frame counter; last frame Snake was pushed
static volatile int g_aimLatch;   // aiming state as seen at the last camera update
static volatile u32 g_aimSeen;   // bit per phase: status flags said "aiming" when it started
static volatile int g_fast, g_fastLeg;   // the faster walk is wanted / is the one playing on his legs
static volatile int g_hidOn; static volatile u16 g_hidMag, g_hidYaw; static volatile u32 g_hidFl;   // stick input taken away where the game reads the pad
static volatile int g_wallFrames;   // frames in a row Snake has had his back to a wall while the shoulder view is on
static volatile u32 g_moveSeen = 0x80000000u;   // frame in which Snake's walking code last ran
static volatile int g_injDown; static volatile DWORD g_injT;   // an aim-key press the plugin made is still down / when it was made
static volatile int g_padConn;   // a controller answers
static volatile int g_hip100 = -1, g_body100, g_sideK100 = 100, g_backK100 = 100;   // for the log
static volatile int g_hipB100 = -1, g_hipG100 = -1, g_hipSrc, g_smooth1000 = -1, g_rigid1000 = -1;
static volatile u32 g_hipBlends, g_headSkips, g_speedSw, g_speedHeld;
static float g_head[3]; static volatile int g_headOk, g_headWhy, g_headLearned, g_headFromAim, g_headUse100;
static float g_hdF = 0.0f, g_hdR = 0.0f, g_hdU = 800.0f;   // his head relative to his position when standing (body frame)
static volatile int g_hF, g_hR, g_hU;   // this frame
static volatile i16 g_lx, g_ly; static volatile DWORD g_stickT;   // the pad's left stick (XInput)
static volatile u16 g_orbitYaw16;   // the game camera's heading, every frame
static volatile u32 g_padDirN, g_kbWalkN[3];
static volatile int g_stickMag, g_lastWalk = 1;
static volatile u16 g_padBtn; static volatile i16 g_ry;   // pad buttons (XInput) and the right stick
static volatile int g_padAny;   // any pad button or trigger held
static volatile u32 g_walkSrc[4], g_padLookFix, g_camBlockN, g_crunN;
static volatile u64 g_perfApply, g_perfApplyMax, g_perfRay, g_perfN, g_perfRayN, g_perfSight, g_perfSightN;
static volatile u64 g_fiSum, g_fiMax; static volatile u32 g_fiN, g_fi25, g_fi40, g_maxTag; static volatile int g_rowNow;   // frame pacing
static volatile u64 g_tm[8]; static volatile int g_wallPush, g_camHead = -1;
static volatile int g_cwOn, g_cwMoving, g_cwSlow, g_cwIgnoreHold, g_cwHijack, g_cwInstalled;   // built-in crouch walking
static volatile int g_seqSlot = -1, g_seqSub = -1, g_seqTold; static volatile u32 g_seqSwaps;   // footsteps for the added motions
static volatile u32 g_fpvYawN;
// The plugin now also learns that relation from the normal camera (g_orbV0: frames where the number equals the view,
// g_orbV8: opposite) and checks every turn a few frames later, turning it round if it still came out wrong.
static volatile u32 g_orbV0, g_orbV8; static int g_orbFixed = -1;
static float g_handGap; static int g_eBackLog; static volatile u32 g_eBackN, g_outHoldN;   // camera height hand-back
static DWORD g_fpvEndT; static u16 g_fpvEndOrb; static volatile u32 g_fpvSkipN, g_fpvHeldN;   // camera turned after first person
static u16 g_fpRot; static int g_fpRotOk; static DWORD g_fpDT;   // Snake's heading in first person; when its camera direction was last read
static u16 g_fpvWant; static u32 g_fpvCheckFrame; static DWORD g_fpvCheckT; static u8* g_fpvWork; static int g_fpvTurned;
static volatile u32 g_fpvOk, g_fpvBad;
static volatile u32 g_crawlSndS, g_lowCrawlN, g_crawlSndF, g_crawlSndB, g_crawlEvN, g_grassN, g_slideN, g_proneOkN, g_proneBadN;   // lying in the shoulder view
static u8* volatile g_moveWork;   // Snake's action work (movement function)
static int Deg16(u16 v) { return (int)((u32)v * 360u / 65536u); }
static int strcmp_(const char* a, const char* b) { while (*a && *a == *b) { a++; b++; } return (u8)*a - (u8)*b; }
static int SoundFor(int slot, int lying, int crouched);
static volatile u32 g_cwFrames, g_cwSwaps, g_cwCamoN;   // time in each hook (1 pad copy, 2 movement, 3 motion requests, 4 upper body, 5 recoil, 6 movement end)
#define PROF0 LARGE_ p0_; QueryPerformanceCounter(&p0_);
#define PROF1(k) { LARGE_ p1_; QueryPerformanceCounter(&p1_); g_tm[k] += (u64)(p1_.q - p0_.q); }
static int StickActive(void); static float StickR(void); static int StickMag(void); static u16 StickYaw(u16 camYaw); static int KeyWalk(u16 mag); static int WalkChoice(u16 mag); static float BodyHipY(u8* pl);
static int NoOts(int w) { return w > 0 && w < 32 && (((u32)cfgNoOts >> w) & 1); }
static volatile int g_band = -1;   // walking: 0 fast, 1 walk (toggle), 2 stalk
// where WalkChoice's answer came from: 0 keyboard (the stick value the game made of the keys), 1 pad stick, 2 d-pad, 3
// the game's stick value, 4 the game's stalk input, 5 the stalk key held; the last shot (declared again further down)
static volatile int g_wcSrc = -1; static volatile DWORD g_shotT; static volatile u32 g_shotWalkN;
// the last press of the fire button (any way it showed: the game's fire bit, the left mouse button, the raised bit
// dropping for a frame - the pad), walk changes from the game's values not taken because they were too short
// (WalkDebounceFrames)
static volatile DWORD g_fireEvT; static volatile u32 g_wdbN;
static volatile int g_fgCache;   // (declared again with the input thread)
static volatile u32 g_btnNow, g_btnFrame = 0x80000000u; static volatile int g_btnOk;   // Snake's buttons as the camera update read them
static volatile u32 g_rqA0, g_rqA1, g_rqF; static volatile int g_rqM0, g_rqM1, g_rqU, g_rqDid;   // the game's last motion request (before the plugin)
static volatile u16 g_padRaw;   // the stick as the game read it, before the plugin changed anything
static volatile u32 g_cwSwap, g_cwRaise, g_cwLower;   // crouch walking outside the shoulder view: fast animation used / stick raised / lowered
static volatile int g_rate100 = -1, g_lock, g_hold, g_throwClamp, g_proneBack100 = 100;
static volatile u32 g_snaps;
// the switch between the shoulder view and first person is flown by the plugin (see hk_Apply)
static volatile int g_fly;   // 0 no, 1 shoulder -> head (going into first person), 2 head -> shoulder (coming back),
                                                                 // 3 back to the shoulder from part of the way to the head (the
                                                                 // game called first person off)
static DWORD g_flyT, g_flyDur, g_lastFpvT;
static float g_fly3K;   // how far toward the head the camera had got when the switch was called off (0..1)
// the aim key and the gun's raised state. g_latchWorked: weapons for which a press of the aim key (the player's or the
// plugin's) has changed the raised state in this session - such a weapon is never written off as "the aim key does
// nothing for it". g_pressPause: the plugin's presses have not worked for this weapon right now - it stops pressing
// until the view is switched again.
static u64 g_latchWorked; static int g_pressPause;
static volatile u32 g_fpCancelN, g_pressFailN, g_maskFreeN, g_noLatchN;
static float g_holdMs = 235.0f;   // how long the game takes from leaving third person to first person (measured each time)
static float g_fpE[3], g_fpD[3]; static int g_fpOk;   // the first-person camera's last eye position and view direction
static float g_hoF[3] = { 80, 80, 80 }, g_hoU[3] = { 130, 130, 60 }, g_hoR[3]; static u8 g_hoOk[3];   // first-person eye relative to the body point, per stance
static volatile int g_pitchSetOk; static volatile float g_pitchSet;   // pitch to take over when coming back from first person
static int Throwable(int w) { return w > 0 && w < 32 && (((u32)cfgThrowMask >> w) & 1); }
static int SlowHeld(void) { return cfgSlowKey && (GetAsyncKeyState(cfgSlowKey) & 0x8000); }
// Snake's body motion layer (the first of the motion controller's layers: +0x10 motion header, +0x18 motion data, +0x48
// clock, +0x4C playback rate, +0x54 archive)
static u8* Layer0(u8* pl)
{
    if (!pl) return 0;
    u8* mc = *(u8**)(pl + 0x2B8); if (!Readable(mc, 0x10)) return 0;
    u8* ly = *(u8**)(mc + 8); return Readable(ly, 0xA0) ? ly : 0;
}

// ---- what is under the crosshair The camera code asks the game's collision system whether anything lies between two
// points; the plugin asks the same question along the view line, so the gun can be pointed at the spot the crosshair
// covers instead of merely parallel to the camera (which put shots beside the crosshair on anything close).
typedef u32 (*ColArea_t)(u32, const float*, const float*);
typedef int (*ColTest_t)(u32, u32, u32, const float*, const float*, float);
typedef void (*ColSort_t)(void); typedef int (*ColNum_t)(void); typedef u32 (*ColFlag_t)(int); typedef void (*ColPoint_t)(int, float*);
static ColArea_t c_area; static ColTest_t c_test; static ColSort_t c_sort; static ColNum_t c_num; static ColFlag_t c_flag; static ColPoint_t c_point;
static volatile u32* c_areaArg;
static volatile float g_aimPt[3]; static volatile int g_aimPtOk, g_convDist;
static volatile u32 g_camSkipN, g_camOnlyN; static volatile int g_camOnlyNow;   // camera tests - hits let through, frames held in by a surface only the camera sees
static volatile float g_pfAim[3]; static volatile int g_pfUse, g_pfAtLim;   // the aim point with the gun's vertical correction (PoseFix); at the arms' limit (1 down, -1 up)
static volatile float g_pitchDeg, g_panDps;   // aim pitch (degrees); how fast the view pans sideways (degrees a second, + = to the right)
static volatile int g_zoomMul = 1000;
static volatile u32 g_shots, g_shotsAimed;   // bullets Snake fired while the plugin was watching / how many it aimed
static volatile int g_obsMm = -1; static volatile u32 g_obsHits;   // how far from the muzzle the shot lands when that is close (-1 = not close)
// Distance to the first thing a bullet would hit between two points, or maxd. Same question, same settings as the
// game's own first-person shot code asks (exe+3749FA: type 0x1F, Snake's area, flags 0x42, no radius).
static float RayRaw(u32 areaBase, const float* from, const float* dir, float maxd);
static float Ray(u32 areaBase, const float* from, const float* dir, float maxd)
{
    LARGE_ a0, a1; QueryPerformanceCounter(&a0);
    float r = RayRaw(areaBase, from, dir, maxd);
    QueryPerformanceCounter(&a1); g_perfRay += (u64)(a1.q - a0.q); g_perfRayN++;
    return r;
}
static float RayRawF(u32 areaBase, const float* from, const float* dir, float maxd, u32 flags, u32 skip)
{
    if (!c_test || !(maxd > 1.0f)) return maxd;
    __attribute__((aligned(16))) float a[4], b[4], h[4];
    for (int i = 0; i < 3; i++) { a[i] = from[i]; b[i] = from[i] + dir[i] * maxd; }
    a[3] = b[3] = 1.0f;
    u32 area = c_area(areaBase, a, b);
    if (!c_test(0x1Fu, area, flags, a, b, 0.0f)) return maxd;
    c_sort();
    int n = c_num(); float best = maxd;
    if (n < 0 || n > 64) return maxd;
    for (int i = 0; i < n; i++) {
        if (skip && c_flag && (c_flag(i) & skip)) continue;
        h[0] = h[1] = h[2] = h[3] = 0; c_point(i, h);
        float d = (h[0] - a[0]) * dir[0] + (h[1] - a[1]) * dir[1] + (h[2] - a[2]) * dir[2];
        if (d > 0 && d < best) best = d;
    }
    return best;
}
static float RayRaw(u32 areaBase, const float* from, const float* dir, float maxd) { return RayRawF(areaBase, from, dir, maxd, 0x42u, 0); }
// the surface flags the camera's own tests use. Now the camera uses the game camera's flags (plus the shots' 0x42, so
// nothing it stopped at before is lost) and leaves out the same hits. CollisionFlags / CollisionSkipFlags in the ini
// override them.
static volatile int g_camMode = 1;
static u32 CamFlags(void) { if (cfgCamFlags) return (u32)cfgCamFlags; return (g_camMode ? 0x840042u : 0x840050u) | 0x42u; }
static float CamRay(u32 areaBase, const float* from, const float* dir, float maxd)
{
    LARGE_ a0, a1; QueryPerformanceCounter(&a0);
    float r = RayRawF(areaBase, from, dir, maxd, CamFlags(), (u32)cfgCamSkip);
    QueryPerformanceCounter(&a1); g_perfRay += (u64)(a1.q - a0.q); g_perfRayN++;
    return r;
}

// ---- where the shot goes (worked out once a frame in the camera hook, used by the bullet hook and the crosshair)
static volatile float g_tEye[3], g_tF[3], g_tR[3], g_tU[3], g_tQ[3];   // shoulder camera position and its forward / right / up axes; Snake's head-height point
static volatile float g_tPt[3];   // the point under the middle of the screen
static volatile u32 g_tFrame = 0x80000000u;   // frame those were worked out in
static volatile int g_tMuzzle;   // 1 = something stands between the gun and the view line: fire from the gun itself
static volatile int g_ctrDist = -1, g_landDist = -1, g_sideClear = 1;
static volatile float g_sx, g_sy; static volatile int g_ghost;   // where the shot lands on screen, from the middle, in picture heights; 1 = it is off the middle
// Where the muzzle sits relative to Snake (ahead / right / up of the point at head height, along the aim). Measured at
// every shot; typical values until the first one with that weapon and stance.
static float g_mo[64][3][3]; static u8 g_moOk[64][3];
// where each gun's muzzle sits (ahead / right / up of Snake, in the aim's axes) is needed to tell whether something is
// in the way of the gun.
static u8 g_moSrc[64][3]; static float g_moSaved[64][3][3];   // 0 none, 1 built in, 3 measured this session
static void MuzzleDefaults(void)
{
    static const short t[][5] = {   // weapon type, stance, ahead, right, up
        { 5, 0, 982, 34, 52 }, { 5, 1, 921, 34, -234 }, { 6, 0, 982, 34, 52 }, { 6, 1, 921, 34, -234 },
        { 7, 0, 764, 33, 54 }, { 8, 0, 413, 221, -318 }, { 9, 0, 715, 92, -223 },
        { 10, 0, 482, 212, -297 }, { 10, 1, 458, 177, -555 }, { 11, 0, 1012, 132, -53 }, { 11, 1, 1020, 130, 64 },
        { 12, 0, 875, 127, -16 }, { 12, 1, 1020, 130, 64 }, { 13, 0, 894, 232, -335 }, { 14, 1, 802, 223, -575 } };
    for (unsigned i = 0; i < sizeof t / sizeof t[0]; i++) {
        int w = t[i][0], st = t[i][1];
        g_mo[w][st][0] = t[i][2]; g_mo[w][st][1] = t[i][3]; g_mo[w][st][2] = t[i][4]; g_moOk[w][st] = 1; g_moSrc[w][st] = 1;
    }
}
// every weapon has a muzzle position from the start - measured (built in / this session) or, for anything not
// measured yet, an estimate from its kind of gun (taken from the measured ones). The obstacle check never waits for a
// shot; shots only make the value more exact.
static volatile int g_mzEst;   // the value in use is an estimate
static const float* MuzzleOff(int w, int st)
{
    g_mzEst = 0;
    if (w > 0 && w < 64 && g_moOk[w][st]) return g_mo[w][st];
    if (w > 0 && w < 64 && st == 1 && g_moOk[w][0]) {   // crouched not measured: the standing value, lower
        static float c[3]; int longGun = w >= 11 && w <= 17;
        c[0] = g_mo[w][0][0]; c[1] = g_mo[w][0][1]; c[2] = g_mo[w][0][2] + (longGun ? 100.0f : -270.0f);
        g_mzEst = 1; return c;
    }
    // standing crouched lying
    static const float pistol[3][3] = { { 950, 35, 50 },   { 900, 35, -230 },  { 900, 40, 0 } };
    static const float smg[3][3]    = { { 480, 210, -300 },{ 460, 180, -550 }, { 700, 120, 0 } };
    static const float rifle[3][3]  = { { 950, 130, -40 }, { 1000, 130, 60 },  { 1000, 120, 0 } };
    static const float shotg[3][3]  = { { 850, 225, -330 },{ 800, 223, -575 }, { 900, 150, 0 } };
    static const float other[3][3]  = { { 900, 130, -60 }, { 900, 130, 40 },   { 900, 120, 0 } };
    const float (*t)[3] = other;
    if (w >= 5 && w <= 9) t = (w == 8) ? smg : pistol;   // the SAA is held low like the Scorpion
    else if (w == 10) t = smg;
    else if (w >= 11 && w <= 13) t = rifle;
    else if (w == 14) t = shotg;
    else if (w >= 15 && w <= 17) t = rifle;
    g_mzEst = 1;
    return t[st < 0 || st > 2 ? 0 : st];
}

static float sin_small(float a);
static float sin16(u16 y)   // sine of a 16-bit angle
{
    float a = (float)(i16)y * (3.14159274f / 32768.0f);   // -pi..pi
    if (a > 1.57079637f) a = 3.14159274f - a; else if (a < -1.57079637f) a = -3.14159274f - a;
    return sin_small(a);
}
// an event scene the cutscene flags miss. The game's pad routine (exe+357B30) gives Snake no stick at all while status
// 0x139 or 0x9B is on or its check exe+99330 says so (the game's state word exe+1E21AB0 with its top bit, or
// exe+1E21ADC): the player has no control of him. Never on while he can be moved, so it cannot get in the way in play.
static int (*f_noCtl)(void); static volatile u32 g_eventN; static volatile int g_eventWhy;
static int NoControl(void)
{
    int w = Bit(0x139) ? 1 : Bit(0x9B) ? 2 : (f_noCtl && f_noCtl()) ? 3 : 0;
    g_eventWhy = w; return w != 0;
}
static int GameEvent(void) { return (p_cutscene && *p_cutscene == 1) || (p_scripted && *p_scripted == 1) || NoControl(); }
static int Aiming(void)
{
    int gameplay = !GameEvent();
    return gameplay && Bit(ST_WEAPON) && Bit(ST_TPS) && !Bit(ST_FPV);
}
#define DIRBITS 0x000F00F0u   // stick and d-pad direction bits in Snake+0x7E8
// the smallest stick value taken as a push.
static volatile u32 g_noiseN;
static int RealPush(u16 mag, u32 fl);

static int PickDir(void)   // 0 forward, 1 back, 2 left, 3 right
{
    int rel = (i16)(u16)(g_inYaw - g_camYaw16);
    // a little stickiness at the borders between the four walk directions, so a stick held near a diagonal does not
    // flip the leg animation back and forth
    static int last = -1;
    int h = 0x500;
    if (last == 0 && rel > -0x3000 - h && rel < 0x3000 + h) return 0;
    if (last == 1 && (rel > 0x5000 - h || rel < -0x5000 + h)) return 1;
    if (last == 2 && rel > 0x3000 - h && rel < 0x5000 + h) return 2;
    if (last == 3 && rel < -0x3000 + h && rel > -0x5000 - h) return 3;
    int d;
    if (rel > -0x3000 && rel < 0x3000) d = 0;
    else if (rel > 0x5000 || rel < -0x5000) d = 1;
    else d = rel > 0 ? 2 : 3;
    last = d; return d;
}
static volatile int g_legsKey = -1, g_legSlot = -1; static volatile u32 g_legSets;
static volatile int g_legDir = -1;   // direction of the leg motion forced last (0 forward, 1 back, 2 left, 3 right)
static int Legs(void) { return g_legsKey >= 0 ? g_legsKey : cfgLegs; }
// Move Snake along the hidden stick direction. While a walk motion plays on his legs the game itself carries him along
// that motion's direction, so the plugin only adds the difference between that and the direction actually pressed;
// without a walk motion it pushes him the whole way (he glides).
// the playback rate of the plugin's walk motions is set every frame. Now the rate is always normal x the band's
// percentage (walk toggle CrouchWalkPercent, stalk StalkPercent), the plugin's diagonal correction uses the same
// percentage, and every direction has the same speed. When the plugin's walk ends the normal rate is put back.
// the slow (walk) crawl forward in the shoulder view
static volatile u32 g_crawlWalkN, g_crawlSlowMarkN;
static int CrawlWalkNow(u8* pl)
{
    return cfgCrawlWalkPct < 100 && g_aimLatch && g_proneLatch && !g_transLatch && g_legDir == 0 && g_band != 0 && g_legSlot >= 0
        && (u32)(g_frame - g_legFrame) <= 2 && (int)*(i16*)(pl + 0x78) == g_legSlot + 1;
}
static float LegFactor(void)
{
    if (g_fastLeg) return 1.0f;
    // 78 % of the motion's speed is the game's stalk.
    if (!g_crouchLatch) return (float)(g_band == 2 ? cfgStandStalkPct : cfgStandWalkPct) / 100.0f;   // the slow walk, a bit faster
    if (g_band == 2) return (float)cfgStalkPct / 100.0f;
    if (g_band == 1) return (float)cfgCrouchWalkPct / 100.0f;
    return 1.0f;
}
static void LegRate(u8* pl)
{
    static int mine; static u8* lyMine; static float wrote;
    u8* ly = Layer0(pl); if (!ly) { g_rate100 = -1; return; }
    float tick = p_tick && *p_tick > 0 && *p_tick < 20 ? (float)*p_tick : 5.0f;
    int want = g_aimLatch && !g_proneLatch && Legs() && (u32)(g_frame - g_legFrame) <= 2 && *(u32*)(ly + 0x54) == 0x6891CC;
    // outside the shoulder view the upright fast crouch walk ("crouchnormal", swapped in by hk_Req) is played
    // CrouchRunPercent faster. He is moved by the animation itself, so he goes faster with it - no sliding.
    int mo = *(i16*)(pl + 0x78);
    int crun = !g_aimLatch && cfgCrouchAll && cfgCrouchRunPct != 100 && cfgCRun[0] >= 0 && Bit(ST_CROUCH) && !Bit(ST_STAND) && !Bit(ST_FPV)
        && *(u32*)(ly + 0x54) == 0x6891CC && *(u32*)(pl + 0x74) == 0x6891CC && mo == cfgCRun[0] + 1;
    // CrawlWalkPercent=100: off.
    int crawlW = CrawlWalkNow(pl) && *(u32*)(ly + 0x54) == *(u32*)(pl + 0x74);
    if (crawlW) {
        *(float*)(ly + 0x4C) = wrote = tick * (float)cfgCrawlWalkPct / 100.0f; mine = 1; lyMine = ly; g_crawlWalkN++;
        static int told; if (!told) { told = 1; char b[160]; wsprintfA(b, "lying in the shoulder view, crawling forward with the walk: the crawl played at %d%% (the slow crawl of the normal view)", cfgCrawlWalkPct); Log(b); }
    } else if (crun) {
        *(float*)(ly + 0x4C) = wrote = tick * (float)cfgCrouchRunPct / 100.0f; mine = 1; lyMine = ly; g_crunN++;
        static int told; if (!told) { told = 1; char b[140]; wsprintfA(b, "crouch walking (normal view), full speed: the upright crouch walk played at %d%% (crouchnormal)", cfgCrouchRunPct); Log(b); }
    } else if (want) {
        *(float*)(ly + 0x4C) = wrote = tick * LegFactor(); mine = 1; lyMine = ly;
        static int told; if (!told && !g_fastLeg) { told = 1; char b[120]; wsprintfA(b, "shoulder view, slow walk: legs played at %d%% (%s, band %d)", (int)(LegFactor() * 100.0f), g_crouchLatch ? "crouched" : "standing", (int)g_band); Log(b); }
    } else if (mine) {
        // put back only if it is still the speed set here - when the crouch walking (or the game) has already
        // given the next motion its own speed (its stalk plays at 3), that is left alone
        float cur = *(float*)(ly + 0x4C);
        if (ly == lyMine && cur - wrote < 0.001f && cur - wrote > -0.001f) *(float*)(ly + 0x4C) = tick;
        mine = 0;
    }
    g_rate100 = (int)(*(float*)(ly + 0x4C) * 100.0f);
}
static void MoveMarks(void);
static void Push(u8* pl)
{
    if (!g_have) { g_vx = g_vz = 0; return; }
    float tick = p_tick && *p_tick > 0 && *p_tick < 20 ? (float)*p_tick : 5.0f;
    float mag = g_inMag ? (float)g_inMag / 255.0f : 1.0f; if (mag > 1.0f) mag = 1.0f;
    if (!g_inMag && StickActive()) { mag = (float)StickMag() / 255.0f; if (mag < 0.19f) mag = 0.19f; }   // no value from the game - the stick's own
    float vx, vz;
    static const u16 card[4] = { 0, 0x8000, 0x4000, 0xC000 };
    int d = PickDir();
    int legs = Legs() && (u32)(g_frame - g_legFrame) <= 2;
    {   // which of the two walks. Stick pushed far (or a direction key) = the faster one; pushed lightly, or
        // with the walk key held, = the slow one. A little slack so it does not flip back and forth at the edge.
        // Crouched: three speeds. Full stick = the fast crouch walk; the game's walk toggle (Ctrl, the stick reads 128)
        // = the low "squatty" crouch walk at its normal speed; stalking (Shift held, or the stick barely pushed) = the
        // same low walk played at StalkPercent of its speed.
        int slow = SlowHeld();
        int pd = g_padConn && (g_padBtn & 0xF);
        g_wcSrc = -1;
        int kw = (g_inMag || StickActive() || pd) ? WalkChoice(g_inMag ? g_inMag : 1) : (g_band == 2 ? 2 : g_fast ? 0 : 1);
        if (kw < 0) kw = g_band == 2 ? 2 : g_fast ? 0 : 1;
        {   // the frame of a shot. The game reports the stick at its stalk value (51) for a frame or two when a shot goes
            // off (255 -> 51 -> 255) and may mark its stalk input; read as "walk slower" that switched the legs from
            // the fast walk to the slow one and back at every shot - the brief stutter of the strafe. From a shot (or
            // while the fire key is held) until ShotWalkHoldMs after it, a change of walk that comes from the game's
            // stick value is not taken; the stalk key, the pad's stick and the d-pad still change it at once.
            static int keep = -1, pend = -1; static u32 pendF;
            DWORD tn = GetTickCount();
            int fireWalk = cfgWalkDebounce > 0;
            int win = cfgShotWalkMs > 0 && g_aimLatch && (g_fireNow || (g_shotT && tn - g_shotT < (DWORD)cfgShotWalkMs)
                || (fireWalk && ((g_fireEvT && tn - g_fireEvT < (DWORD)cfgShotWalkMs) || (g_fgCache && (GetAsyncKeyState(1) & 0x8000)))));
            int soft = g_wcSrc == 0 || g_wcSrc == 3 || g_wcSrc == 4;
            if (win && keep >= 0 && kw != keep && soft) {
                kw = keep; g_shotWalkN++;
                if (g_shotWalkN <= 6) { char b[200]; wsprintfA(b, "strafe: a shot changed the walk the game reports (source %d) - the walk from before the shot is kept (no stutter)", (int)g_wcSrc); Log(b); }
            } else if (fireWalk && keep >= 0 && kw != keep && g_wcSrc == 4) {
                if (kw != pend) { pend = kw; pendF = g_frame; }
                if ((u32)(g_frame - pendF) < (u32)cfgWalkDebounce) {
                    static u32 lastN; if (g_frame != lastN) { g_wdbN++; lastN = g_frame; }
                    static int told; if (told < 6) { told++; char b[220]; wsprintfA(b, "strafe: the game's %s changed the walk (to %s) - taken only if it lasts %d frames (a click of the fire button makes it report its stalk for a frame)",
                        g_wcSrc == 4 ? "stalk input" : "stick value", kw == 0 ? "full speed" : kw == 1 ? "walk" : "stalk", cfgWalkDebounce); Log(b); }
                    kw = keep;
                } else pend = -1;
            } else pend = -1;
            keep = kw;
        }
        int wantF = cfgTwoSpeed && kw == 0;
        // near the edge (a stick held about there) the change has to be wanted for SpeedSwitchMs: the two walks (and
        // their poses) switched back and forth. Keys and a stick pushed clearly one way switch at once.
        {   static DWORD wantT; DWORD tn = GetTickCount();
            int sm = g_stickMag;
            int nearEdge = StickActive() && sm > cfgFastAbove - 35 && sm < cfgFastAbove + 35;
            if (wantF == g_fast) wantT = 0;
            else if (!cfgSpeedSwitch || !nearEdge || slow) { g_fast = wantF; wantT = 0; g_speedSw++; }
            else {
                if (!wantT) wantT = tn ? tn : 1;
                if (tn - wantT >= (DWORD)cfgSpeedSwitch) { g_fast = wantF; wantT = 0; g_speedSw++; } else g_speedHeld++;
            }
        }
        g_band = g_fast ? 0 : kw == 2 ? 2 : 1;   // standing too (it was -1 there: no stalk)
        (void)slow;
    }
    if (g_proneLatch) {
        // forward and backward: the crawl motion carries him by itself (15 and 8.6 units a frame); sideways: the motion
        // carries part of it (about 4.8 units), the plugin adds the rest. ProneSidePercent scales the whole sideways
        // speed.
        if (legs && g_legDir == d && (d < 2 || cfgProneSideAnim)) { vx = vz = 0; }
        else {
            u16 y = (u16)(g_camYaw16 + card[d]);
            float sp = (float)cfgProneSpeed;
            if (d >= 2) { sp = (sp + 4.8f) * (float)cfgProneSide / 100.0f - (legs ? 4.8f : 0.0f); if (sp < 0) sp = 0; }
            sp *= tick / 5.0f;
            vx = sin16(y) * sp; vz = sin16((u16)(y + 0x4000)) * sp;
        }
    } else if (legs) {
        // the same speed in every direction. The walk motion moves him "root" units along one of the four main
        // directions; what is added is exactly the step from there to the pressed direction at the same speed.
        // the faster walks carry him further each frame. Read from the motions themselves (their travel divided by
        // their length, 126 game units to one motion unit): standing 19.7 slow / 38.4 fast, crouched 11.6 slow / 28.2
        // fast.
        float root = (g_fastLeg ? (float)(g_crouchLatch ? cfgCRunRoot : cfgRunRoot) * 0.1f : (g_crouchLatch ? 12.0f : 20.0f)) * tick / 5.0f * (float)LegPct() / 100.0f;
        root *= LegFactor();   // the motion plays at this percentage too
        u16 y = (u16)(g_camYaw16 + card[d]);
        int dl = (i16)(u16)(g_inYaw - y);
        if (dl > -0x200 && dl < 0x200) { vx = vz = 0; }   // within 3 degrees of the main direction
        else {
            vx = (sin16(g_inYaw) - sin16(y)) * root;
            vz = (sin16((u16)(g_inYaw + 0x4000)) - sin16((u16)(y + 0x4000))) * root;
        }
    } else {
        float sp = (float)cfgSpeed * tick / 5.0f * mag;
        vx = sin16(g_inYaw) * sp; vz = sin16((u16)(g_inYaw + 0x4000)) * sp;
    }
    g_vx = vx; g_vz = vz; g_moveFrame = g_frame;
    MoveMarks();   // walking / running / stalking marks, as the game's own walk
    LegRate(pl);
    if (MoveMode() == 2) { if (g_msg == 0x80) { *(float*)(pl + 0x130) += vx; *(float*)(pl + 0x138) += vz; } }
    else { *(float*)(pl + 0x620) = vx; *(float*)(pl + 0x628) = vz; }
    g_velWrites++;
}

// Called from the stub inside the movement function (the per-frame function of Snake's object; it is called three times
// a frame with msg 0x80 = run the current action, 0x20 and 0x10 = follow-up phases).
static void OnMove_(u8* pl, u8* work, u32 msg, u64* retSlot);
static void OnMove(u8* pl, u8* work, u32 msg, u64* retSlot) { PROF0 OnMove_(pl, work, msg, retSlot); PROF1(2) }
static void CwOnMove(void);

static void OnMove_(u8* pl, u8* work, u32 msg, u64* retSlot)
{
    CwOnMove();   // built-in crouch walking
    g_pl = pl; g_msg = msg; g_calls++; g_moveSeen = g_frame; g_moveWork = work;
    if (g_phaseN < 12) { g_phase[g_phaseN++] = msg == 0x80 ? 'A' : msg == 0x20 ? 'B' : msg == 0x10 ? 'C' : '?'; }
    if (retSlot && g_after && *retSlot != (u64)g_after && *retSlot > 0x10000) { *g_savedRet = *retSlot; *retSlot = (u64)g_after; }
    if (Aiming()) g_aimSeen |= msg;
    // the status flags are rebuilt during the frame, so the aiming state is taken from the last camera update

    if (!pl || !cfgEnabled || !g_camValid || !g_aimLatch) return;
    if (msg == 0x80 && work) g_actRva = (u32)(*(u64*)(work + 0x58) - (u64)g_base);
    if (g_wallFrames) return;   // back against a wall: the game's own controls
    *(u16*)(pl + 0x16A) = g_camYaw16;   // target heading
    *(u16*)(pl + 0x162) = g_camYaw16;   // current rotation
    if (!Strafe() || g_transLatch) return;   // stance changes keep the game's own controls
    u16 mag = *(u16*)(pl + 0x7E0); u32 fl = *(u32*)(pl + 0x7E8);
    if (mag && !RealPush(mag, fl)) { *(u16*)(pl + 0x7E0) = 0; mag = 0; g_noiseN++; }   // noise, not a push
    if (mag || (fl & DIRBITS)) {
        g_inMag = mag; g_inYaw = *(u16*)(pl + 0x7E2); g_inFl = fl;
        g_have = 1; g_hidden++;
        *(u16*)(pl + 0x7E0) = 0; *(u32*)(pl + 0x7E8) = fl & ~DIRBITS;
    } else if (g_hidOn) {   // already taken away where the game reads the pad
        g_inMag = g_hidMag; g_inYaw = g_hidYaw; g_inFl = g_hidFl; g_have = 1;
    }
    // a push inside the game's own dead zone. The game (exe+3585CE) gives Snake a stick value only from 48 of 128 -
    // through the PC port's pad layer about half the stick's travel; under that it sets only the direction bits, or
    // nothing at all under about 37%. In the shoulder view the plugin walks him itself, so from PadDeadZone percent on
    // (24 = the usual XInput dead zone) the stick walks him too - slowly, the way it points.
    if (!g_have && cfgPadDir && cfgPadDZ > 0 && StickActive() && StickR() >= (float)cfgPadDZ * 327.67f) {
        g_inMag = 0; g_inFl = 0; g_have = 1;
        if (msg == 0x80) g_dzWalkN++;
    }
    if (g_have && cfgPadDir && StickActive()) {
        // with the pad the direction is taken from the stick itself. The game's own heading for a lightly pushed stick
        // lagged behind (it still pointed the way he walked before), so a slow push walked him the wrong way for a
        // moment.
        // also when the game's stick value is 0. Then the game has only its direction bits and its heading is the one
        // from the last firm push - up to a whole turn stale: pushed left, he walked right (median 71 degrees off the
        // stick; with a stick value 3.6).
        u16 sy = StickYaw(g_camYaw16);
        if (msg == 0x80 && !g_inMag) { int dd = (i16)(u16)(sy - g_inYaw); if (dd > 0x200 || dd < -0x200) g_stickFixN++; }
        g_inYaw = sy; g_padDirN++;
    }
    if (msg == 0x80) Push(pl);
}

// That is decided by another part of his code than the walking, one that runs on its own and reads the same stick
// values, so hiding the stick inside the walking code (above) did not stop it: walking into a wall in the shoulder view
// stuck him to it while the plugin kept turning him to face along the camera. The stick is now hidden right where the
// game copies the pad into Snake (exe+357B30 writes Snake+0x7E0 / 7E2 / 7E8), so nothing of Snake's sees it while the
// shoulder view is on. Before the game's routine runs again the direction bits are put back, so its own "newly pressed"
// bookkeeping stays right.
typedef u64 (*Inp_t)(u8* pl, u8* work, u32 msg, u64 a4);
static Inp_t o_Inp;
// first person entered from the shoulder view with the pad (LB held = the game's camera button L1, bit 0x400 of Snake's
// buttons; RB = first person). Letting go of LB while first person was on let the game see L1 go up in first person: it
// ended first person and started it again at once and the camera was left a few centimetres in front of his head until
// LB or RB was pressed again. Releasing RB first (first person ends with L1 still held) always worked. Now, once first
// person is on with L1 held, the game keeps seeing L1 held until first person has ended - the order that works - and
// only then sees it let go. Snake+0x7E8 = held, +0x7EC / +0x7F0 = pressed this frame.
static volatile u32 g_letGoF, g_fpReN, g_fpReOkN, g_fpReBadN, g_fpEndN; static u8* volatile g_rawPad; static u8* volatile g_camWorkLast;
static void StatusRingLog(const char* why);
static volatile int g_l1Keep, g_l1Was; static volatile u32 g_l1KeepN, g_l1LetGoN, g_fpRestartN, g_l1KeepEndN;
static void L1Keep(u8* pl)
{
    if (!cfgFpvKeepL1) { g_l1Keep = 0; return; }
    u32 held = *(u32*)(pl + 0x7E8);
    int fp = Bit(ST_FPV);
    if (!g_l1Keep) {
        if (fp && (held & 0x400u) && g_padConn && (g_padBtn & (u16)cfgPad)) {
            g_l1Keep = 1; g_l1Was = 0;
            static int told; if (told < 6) { told++; Log("first person with the pad's shoulder-view button held: the game keeps seeing it held until first person ends (letting go of it first no longer restarts first person)"); }
        }
        return;
    }
    if (!fp) {
        g_l1Keep = 0; g_l1Was = 0; g_l1KeepEndN++;
        static int told; if (told < 12) { told++; char b[200]; wsprintfA(b, "first person ended: the shoulder-view button handed back to the game (%s)", (g_padBtn & (u16)cfgPad) ? "still held" : "let go in first person - the game sees it go up now, in third person"); Log(b); }
        return;
    }
    if (!(held & 0x400u)) {
        if (!g_l1Was) { g_l1Was = 1; g_l1LetGoN++; g_letGoF = g_frame ? g_frame : 1; static int told; if (told < 12) { told++; Log("first person: the shoulder-view button was let go - the game keeps seeing it held until first person ends"); } }
        *(u32*)(pl + 0x7E8) = held | 0x400u; g_l1KeepN++;
        *(u32*)(pl + 0x7EC) &= ~0x400u; *(u32*)(pl + 0x7F0) &= ~0x400u;
        return;
    }
    g_l1Was = 0;
    *(u32*)(pl + 0x7EC) &= ~0x400u; *(u32*)(pl + 0x7F0) &= ~0x400u;   // pressed again while kept: not a new press
}
// What ends it is not Snake's button word, so not yet known; every end of first person now writes what the game had
// (statuses, buttons, the pad as the game read it, the camera against the first-person eye) to the log.
static void FpvEndDiag(u8* pl, int recent)
{
    g_fpEndN++;
    if (g_fpEndN > 30 && !recent) return;
    char b[700]; int n = 0;
    u8* fc = *(u8**)(pl + 0x730); u8* w = g_camWorkLast; int dist = -1;
    if (Readable(fc, 0x40) && Readable(w, 0x400)) {
        float* e = (float*)(w + 0x3D0); float* f = (float*)(fc + 0x20);
        float dx = e[0] - f[0], dy = e[1] - f[1], dz = e[2] - f[2]; dist = (int)__builtin_sqrtf(dx * dx + dy * dy + dz * dz);
    }
    u8* cp = *(u8**)(pl + 0x7F8); u8* rp = g_rawPad;
    n = wsprintfA(b, "first person ended%s: statuses BA %d / 8 %d / BD %d / BC %d / BB %d / D5 %d / E1 %d / 13A %d; Snake's buttons %08X pressed %08X/%08X held-over %08X; pad copy %08X; pad as read %08X/%08X/%08X; camera %d from the first-person eye; plugin camera hold %d fly %d amount %d",
        recent ? " (LB let go in first person just before)" : "", Bit(0xBA), Bit(8), Bit(0xBD), Bit(0xBC), Bit(0xBB), Bit(0xD5), Bit(0xE1), Bit(0x13A),
        *(u32*)(pl + 0x7E8), *(u32*)(pl + 0x7EC), *(u32*)(pl + 0x7F0), *(u32*)(pl + 0x7F4), Readable(cp, 4) ? *(u32*)cp : 0,
        Readable(rp, 0x20) ? *(u32*)rp : 0, Readable(rp, 0x20) ? *(u32*)(rp + 0x14) : 0, Readable(rp, 0x20) ? *(u32*)(rp + 0x18) : 0,
        dist, (int)g_hold, (int)g_fly, (int)(g_amt * 100.0f));
    (void)n; Log(b);
    StatusRingLog("first person ended");
}
static void FpvWatch(u8* pl)
{
    static int fpW, stage; static u32 stF;
    int fp = Bit(ST_FPV);
    if (fpW && !fp) {
        int recent = g_letGoF && (u32)(g_frame - g_letGoF) <= 30;
        FpvEndDiag(pl, recent);
        if (cfgFpvRestore && recent && !stage && g_padConn && (g_padBtn & 0x200) && (*(u32*)(pl + 0x7E8) & 0x800u) && !GameEvent()) {
            stage = 1; stF = g_frame; g_fpReN++;
            if (g_fpReN <= 12) Log("first person ended with RB still held, right after LB was let go - RB pressed again for the game (up for 2 frames, then down) to start first person anew");
        }
        g_letGoF = 0;
    }
    fpW = fp;
    if (!stage) return;
    u32 d = g_frame - stF;
    if (d < 2) { *(u32*)(pl + 0x7E8) &= ~0x800u; *(u32*)(pl + 0x7EC) &= ~0x800u; *(u32*)(pl + 0x7F0) &= ~0x800u; }
    else if (d == 2) { if (*(u32*)(pl + 0x7E8) & 0x800u) { *(u32*)(pl + 0x7EC) |= 0x800u; *(u32*)(pl + 0x7F0) |= 0x800u; } }
    else if (fp) { stage = 0; g_fpReOkN++; if (g_fpReOkN <= 12) { char b[120]; wsprintfA(b, "first person back on %u frames after RB was pressed again for the game", d - 2); Log(b); } }
    else if (d > 45 || !(g_padBtn & 0x200)) { stage = 0; g_fpReBadN++; if (g_fpReBadN <= 12) Log(!(g_padBtn & 0x200) ? "first person not back: RB let go meanwhile - left as it is" : "first person NOT back within 45 frames of RB pressed again for the game"); }
}
// While first person is on with RB held, the game now sees R1 held for as long as the pad itself (XInput) says RB is
// held.
static void R1Keep(u8* pl)
{
    static int keep;
    if (!cfgR1Keep || !Bit(ST_FPV) || !g_padConn) { keep = 0; return; }
    u32 held = *(u32*)(pl + 0x7E8);
    if (!keep) { if ((held & 0x800u) && (g_padBtn & 0x200)) keep = 1; return; }
    if (!(g_padBtn & 0x200)) { keep = 0; return; }   // RB really let go: first person ends as usual
    if (!(held & 0x800u)) {
        *(u32*)(pl + 0x7E8) = held | 0x800u; *(u32*)(pl + 0x7EC) &= ~0x800u; *(u32*)(pl + 0x7F0) &= ~0x800u; g_r1KeepN++;
        static int told; if (told < 12) { told++; Log("first person: the game's pad read lost RB for a moment while RB was held - kept held for the game (first person no longer ends there)"); }
    }
}
static u64 hk_Inp_(u8* pl, u8* work, u32 msg, u64 a4);
static u64 hk_Inp(u8* pl, u8* work, u32 msg, u64 a4) { PROF0 u64 r = hk_Inp_(pl, work, msg, a4); PROF1(1) return r; }
static u64 hk_Inp_(u8* pl, u8* work, u32 msg, u64 a4)
{
    int mine = pl && pl == g_pl;
    if (mine && g_hidOn) { *(u16*)(pl + 0x7E0) = g_hidMag; *(u32*)(pl + 0x7E8) |= g_hidFl & DIRBITS; }
    g_hidOn = 0;
    u64 r = o_Inp(pl, work, msg, a4);
    if (mine && cfgEnabled && !g_aimLatch) LegRate(pl);   // crouchnormal's speed, every frame
    if (mine) {
        // the game's own stalk input. Where the game copies the pad into Snake (exe+3586CF) it marks a push from its
        // stalk input with status 0x13B - whatever key or button the player has given it in the game's controls. The
        // plugin takes that as stalking too, next to its own StalkKey and the d-pad.
        if (cfgGameStalk && Bit(0x13B) && *(u16*)(pl + 0x7E0)) {
            g_gStalkT = g_frame ? g_frame : 1; g_gStalkN++;
            static int told; if (!told) { told = 1; Log("stalk: the game's own stalk input (status 0x13B) seen - the shoulder view stalks with it too"); }
        }
        g_padRaw = *(u16*)(pl + 0x7E0);
        int kw = WalkChoice((u16)g_padRaw);   // keys, stick or d-pad choose the walk
        if (kw >= 0) g_padRaw = kw == 0 ? 255 : kw == 1 ? 128 : 51;
    }
    // crouch walking outside the shoulder view (the crouch walking below plays it: stick at 180 or more = its normal low
    // walk, less = the same walk slowed down). Wanted: full stick = the fast crouch walk (swapped in where the motion
    // is requested, hk_Req), the walk toggle (stick 128) = the low walk at normal speed, stalking (Shift) = the low
    // walk slowed down. So the stick the game sees is raised to 180 for the walk toggle and lowered under it while
    // Shift is held.
    if (mine && cfgEnabled && cfgCrouchAll && !g_aimLatch && Bit(ST_CROUCH) && !Bit(ST_STAND) && !Bit(ST_FPV)) {
        u16 m = *(u16*)(pl + 0x7E0);
        int kw = WalkChoice(m);
        // stalk = the very value the game's own stalk gives (51), so whatever the game ties to stalking sees a stalk;
        // walk = 180 (the mod's low walk at normal speed); full = 255 (the fast crouch walk, hk_Req)
        if (m) {
            if (kw == 2) { if (m != 51) { *(u16*)(pl + 0x7E0) = 51; g_cwLower++; } }
            else if (kw == 1) { if (m != 180) { *(u16*)(pl + 0x7E0) = 180; g_cwRaise++; } }
            else if (kw == 0) { if (m != 255) *(u16*)(pl + 0x7E0) = 255; }
        }
    }
    if (mine && cfgEnabled) { R1Keep(pl); L1Keep(pl); FpvWatch(pl); }
    // Only while his walking code is the one in charge (it ran this frame or the last): on a ladder, hanging, swimming
    // or already against a wall other code moves him and needs the stick, as before.
    if (mine && cfgEnabled && cfgNoWall && g_camValid && g_aimLatch && Strafe() && !g_transLatch && !g_wallFrames
        && (u32)(g_frame - g_moveSeen) <= 1) {
        u16 mag = *(u16*)(pl + 0x7E0); u32 fl = *(u32*)(pl + 0x7E8);
        if (mag && !RealPush(mag, fl)) { *(u16*)(pl + 0x7E0) = 0; mag = 0; g_noiseN++; }   // noise, not a push
        if (mag || (fl & DIRBITS)) {
            g_hidMag = mag; g_hidYaw = *(u16*)(pl + 0x7E2); g_hidFl = fl;
            *(u16*)(pl + 0x7E0) = 0; *(u32*)(pl + 0x7E8) = fl & ~DIRBITS;
            *(u32*)(pl + 0x7F0) &= ~DIRBITS;   // "pressed this frame"
            g_hidOn = 1; g_hidden++;
        }
    }
    return r;
}

// Called right after the movement function returns.
static void OnMoveEnd_(void);
static void OnMoveEnd(void) { PROF0 OnMoveEnd_(); PROF1(6) }
// Lying in the shoulder view: the game's own crawl (exe+366FC0 forward, exe+366C40 backward) does more than play the
// crawl motion. Every frame it marks Snake as moving (status set 14, 3, [4 backward,] 0x10C - the same marks the crouch
// walk sets for "walking sounds in grass"; they make grass and bushes rustle and count for the enemies' hearing and the
// camouflage), and it plays the crawl sounds at set frames of the motion (exe+36A070: cloth at two frames, the rustle
// at frames 24 and 49, plus a noise the guards can hear). The shoulder view hides the stick from that code and plays
// the crawl motions itself, so all of it was missing. The plugin now does both while its crawl motions play: the marks
// every frame, and the game's own sound routine with the motion that is playing.
typedef i64 (*CwStSet_t)(int a, i64 b, i64 c, i64 d); static CwStSet_t f_CwStSet;
typedef i64 (*StSet5_t)(int a, i64 b, i64 c, i64 d, i64 e);
typedef void (*CrawlSnd_t)(u8* pl, u8* mw, int f1, int f2);
static StSet5_t f_StSet5; static CrawlSnd_t f_CrawlSnd;
static void CrawlSounds(u8* pl)
{
    if (!cfgCrawlSnd || g_msg != 0x80 || !g_proneLatch || !g_have || !g_inMag) return;
    if ((u32)(g_frame - g_legFrame) > 2 || g_legDir < 0 || g_legSlot < 0) return;
    if ((int)*(i16*)(pl + 0x78) != g_legSlot + 1) return;   // the plugin's crawl is the one playing
    int back = g_legDir == 1;
    if (f_CwStSet && f_StSet5) {
        // the slow forward crawl marks status 4 as well, as the game's forward crawl does when it crawls slowly
        int slowF = !back && g_legDir == 0 && CrawlWalkNow(pl);
        if (back || slowF) f_StSet5(14, 3, 4, 0x10C, (i64)0xFFFFFFFFu); else f_CwStSet(14, 3, 0x10C, (i64)0xFFFFFFFFu);
        if (slowF) g_crawlSlowMarkN++;
        g_grassN++;
    }
    if (!f_CrawlSnd) return;
    // sideways too: the same routine, its cloth sounds put at frames 60 and 75 of the roll (the rustles stay at 24 and
    // 49)
    int side = g_legDir >= 2;
    u8* mw = g_moveWork; u8* mc = *(u8**)(pl + 0x2B8);
    if (!Readable(mw, 0x80) || !Readable(mc, 0x10)) return;
    u8* ly = *(u8**)(mc + 8); if (!Readable(ly, 0x50)) return;
    u32 arc = *(u32*)(pl + 0x74); int sel = -1;   // which of Snake's archives the motion is in
    if (arc == *(u32*)(pl + 0xC0)) sel = 0; else if (arc == *(u32*)(pl + 0xC8)) sel = 1; else if (arc == *(u32*)(pl + 0xC4)) sel = 2; else if (arc == *(u32*)(pl + 0xCC)) sel = 3;
    if (sel < 0) return;
    u32 h = ((u32)sel << 14) | ((u32)*(i16*)(pl + 0x78) & 0x3FFu);
    int m0 = *(int*)(ly + 0x3C);
    u32 keep = *(u32*)(mw + 0x60);
    *(u32*)(mw + 0x60) = h;   // the routine checks the action's own motion
    f_CrawlSnd(pl, mw, side ? 60 : back ? 2 : 1, side ? 75 : back ? 0x2B : 0x1C);   // the frames the game's crawls use
    *(u32*)(mw + 0x60) = keep;
    if (side) g_crawlSndS++; else if (back) g_crawlSndB++; else g_crawlSndF++;
    if (*(int*)(ly + 0x3C) != m0) {
        g_crawlEvN++;
        static int told[3]; int kd = side ? 2 : back; if (told[kd] < 2) { told[kd]++; char b[200]; wsprintfA(b, "crawling in the shoulder view (%s, motion %X/%d): a crawl sound of the game's own was played (frame %d)",
            side ? "sideways" : back ? "backward" : "forward", arc, (int)*(i16*)(pl + 0x78), *(int*)(ly + 0x3C) / 5); Log(b); }
    }
}
// walking upright or crouched in the shoulder view. The game's own walking code (exe+36A890; with a weapon exe+387E00)
// marks Snake every frame it moves: "moving" (status 0x0E), "grass" (0x10C) and one of walk (0x0B), run (0x0C, stick at
// 150 or more) or stalk (0x0D, the stalk input). Those marks are what the rest of the game goes by: the camouflage
// index (exe+A8070: no movement penalty without 0x0E; the stalk / run rows from 0x0D / 0x0C), the footstep sound and
// the noise the guards hear (exe+37DE00: run, walk or the quiet stalk step), the rustle of grass. The shoulder view
// hides the stick from that code and walks him itself, so none of it was set: the camo index stayed at the standing-
// still value and every speed sounded the same. The plugin now sets the game's own marks for the walk it plays:
// standing full speed = run, slow walk = walk, stalk = stalk; crouched = the marks the crouch walk of the normal view
// sets (walk, moving, grass), so the camo index is the crouching one, with the same changes on top ([CrouchWalk]
// CamoCrouchNormal / Walk / Stalk).
static volatile u32 g_markFrame, g_markN[4]; static volatile int g_markKind = -1;
static void MoveMarks(void)
{
    if (!cfgMoveMarks || !f_CwStSet || g_msg != 0x80 || g_proneLatch || !g_have || g_transLatch) return;
    int k = g_crouchLatch ? 3 : g_band == 0 ? 0 : g_band == 2 ? 2 : 1;   // 0 run, 1 walk, 2 stalk, 3 crouched
    static const int st[4] = { 0x0C, 0x0B, 0x0D, 0x0B };
    f_CwStSet(st[k], 0x0E, 0x10C, (i64)0xFFFFFFFFu);
    g_markFrame = g_frame; g_markN[k]++;
    static int told; static int lastK = -1;
    if (k != lastK && told < 16) {
        told++; static const char* nm[4] = { "standing, full speed: run (0x0C)", "standing, slow walk: walk (0x0B)", "standing, stalk: stalk (0x0D - quiet steps, the guards do not hear them)", "crouched: walk (0x0B), as the crouch walk of the normal view" };
        char b[200]; wsprintfA(b, "shoulder view moving, the game's own marks set: %s, moving (0x0E), grass (0x10C)", nm[k]); Log(b);
    }
    lastK = k; g_markKind = k;
}
static int OtsCrouchMoving(void) { return g_markKind == 3 && (u32)(g_frame - g_markFrame) <= 2 && g_aimLatch && g_crouchLatch && !g_proneLatch; }
// is the crawl going where it was pushed? Every 45 frames of one crawl motion the way Snake actually moved is compared
// with the way pressed. The sideways motions wait about a quarter second before they move him, so less distance counts
// for them.
static void ProneCheck(u8* pl)
{
    static int n; static float x0, z0; static u32 key; static int told[48]; static u32 toldKey[48]; static int toldN, bad;
    if (g_msg != 0x80) return;
    int on = g_aimLatch && g_proneLatch && !g_transLatch && g_have && g_inMag;
    int legs = (u32)(g_frame - g_legFrame) <= 2 && g_legSlot >= 0 && (int)*(i16*)(pl + 0x78) == g_legSlot + 1;
    static int run;
    if (on && !legs) {
        if (++run == 12) {   // a dozen frames in a row: not just the start of a move
            g_slideN++;
            static int t2; if (t2 < 8) { t2++; char b[200]; wsprintfA(b, "lying in the shoulder view: PROBLEM - moving without a crawl motion (the game plays %X/%d) - pushed along", *(u32*)(pl + 0x74), (int)*(i16*)(pl + 0x78)); Log(b); }
        }
    } else run = 0;
    if (!on || !legs) { n = 0; return; }
    u32 k = (*(u32*)(pl + 0x74) << 10) ^ (u32)*(i16*)(pl + 0x78) ^ ((u32)g_legDir << 28);
    float x = *(float*)(pl + 0x130), z = *(float*)(pl + 0x138);
    if (n == 0 || k != key) { n = 1; key = k; x0 = x; z0 = z; return; }
    if (++n < 45) return;
    n = 0;
    float dx = x - x0, dz = z - z0, d = __builtin_sqrtf(dx * dx + dz * dz);
    u16 got = (u16)(int)(atan2_(dx, dz) / 6.2831853f * 65536.0f);
    int off = (i16)(u16)(got - g_inYaw); if (off < 0) off = -off; int deg = off * 360 / 65536;
    int ok = d >= (g_legDir >= 2 ? 30.0f : 80.0f) && deg <= 50;
    if (ok) g_proneOkN++; else g_proneBadN++;
    int i; for (i = 0; i < toldN; i++) if (toldKey[i] == k) break;
    if (i == toldN && toldN < 48) { toldKey[toldN] = k; told[toldN] = 0; toldN++; }
    if (i < 48 && (!told[i] || (!ok && bad < 30))) {
        static const char* dn[4] = { "forward", "backward", "left", "right" };
        char b[240]; wsprintfA(b, "lying, shoulder view - %s %s: motion %X/%d (weapon %d) moved him %d units in 45 frames, %d degrees off the pressed direction",
            ok ? "OK" : "PROBLEM", dn[g_legDir & 3], *(u32*)(pl + 0x74), (int)*(i16*)(pl + 0x78), Wpn(pl), (int)d, deg);
        Log(b); told[i] = 1; if (!ok) bad++;
    }
}
static void OnMoveEnd_(void)
{
    u8* pl = g_pl;
    if (!pl || !cfgEnabled || !g_camValid || !Strafe() || !g_aimLatch || g_transLatch || g_wallFrames) return;
    *(u16*)(pl + 0x16A) = g_camYaw16;
    *(u16*)(pl + 0x162) = g_camYaw16;
    if (g_msg == 0x80) Push(pl);
    if (g_msg == 0x80 && g_proneLatch) { CrawlSounds(pl); ProneCheck(pl); }
}

static u8* AllocNear(u8* target);
// ---- leg animation The game applies Snake's animation requests in one routine: request+0x0C = motion archive, +0x10 =
// motion number (1-based), +0x12 = blend frames, +0x18 = bone mask for the body layer (the upper-body layer that holds
// the weapon follows at +0x34). While the plugin is moving Snake, the body layer's request is replaced by a directional
// walk from Snake's main archive (the added motions in the 3DS style), and put back afterwards so the game's own
// state is untouched.
typedef void (*Req_t)(u8* pl, u8* req);
static Req_t o_Req;
// weapon archives that carry the added lying left / right motions (slots 200 / 201). Checked against the
// archives themselves; the two spray-type archives (27EF8, 201199) and a few others have none, and asking for slot 200
// there played nothing sensible - those weapons use Snake's own lying left / right instead.
static int HasProneSide(u32 a)
{
    static const u32 k[] = { 0x23291, 0x25BB7, 0x2614B, 0x27945, 0x215251, 0x46E2CB, 0x482B59, 0x48E32B, 0x4963A5, 0x4C4AF2, 0x51525F, 0x5EA57B,
                             0x6BC12E, 0x6C745A, 0x914F09, 0xAD99CA, 0xBAD929, 0xC8B59B, 0xCCBFF6, 0xD14E46, 0xD791EA, 0xF8E660, 0xFC64D1 };
    for (unsigned i = 0; i < sizeof k / sizeof k[0]; i++) if (k[i] == a) return 1;
    return 0;
}
static void hk_Req_(u8* pl, u8* req);
static void hk_Req(u8* pl, u8* req) { PROF0 hk_Req_(pl, req); PROF1(3) }
static void hk_Req_(u8* pl, u8* req)
{
    static int was, cwWas;
    int did = 0, pose = 0, whole = 0, cw = 0, cwArms = 0; u32 sa = 0; i16 sm = 0;
    u64 wm0 = 0, wm1 = 0; u32 wa1 = 0; i16 wn1 = 0, wb1 = 0;
    if (pl && req && pl == g_pl && cfgEnabled) {
        g_rqA0 = *(u32*)(req + 0x0C); g_rqM0 = *(i16*)(req + 0x10); g_rqA1 = *(u32*)(req + 0x34); g_rqM1 = *(i16*)(req + 0x38); g_rqU = *(u64*)(req + 0x40) != 0; g_rqF = g_frame;
        int moving = g_aimLatch && Strafe() && Legs() && (u32)(g_frame - g_moveFrame) <= 1;
        {   u32 a0 = *(u32*)(req + 0x0C), a1 = *(u32*)(pl + 0x9C);
            if (a0 && a0 != 0x6891CC) g_wpnArc = a0; else if (a1 && a1 != 0x6891CC) g_wpnArc = a1; }
        // Only replace the game's resting poses. With the weapon raised the body layer holds that weapon's aiming pose,
        // whose number differs per weapon (1, 15, 16, 59 ... standing). Anything else is a stance change, a shot or a
        // reload, and replacing it would stop it from finishing.
        int rm = *(i16*)(req + 0x10);
        {   // Lying down, each weapon has its own crawl motion and the number differs between them. The plugin
            // notes the one the game itself plays when the player crawls normally (shoulder view off) and uses that for
            // crawling forward in the shoulder view.
            static u32 la; static int lm, cnt; int w = Wpn(pl);
            // Also while the shoulder view is on but the gun is lowered: the plugin then leaves the motion to the game,
            // and moving forward the game plays its own crawl. Only weapon archives, and never the lying poses (12, 17
            // - 21, 61, 62).
            // the lying poses are never taken as a crawl.
            int pose = rm == 12 || (rm >= 17 && rm <= 21) || rm == 61 || rm == 62 || rm == 15;
            int ownFwd = g_aimLatch && !g_rawAim && Strafe() && (u32)(g_frame - g_moveFrame) <= 1 && PickDir() == 0
                && *(u32*)(req + 0x0C) != 0x6891CC;
            // never the backward crawl (Snake's main archive, motion 14).
            int backC = *(u32*)(req + 0x0C) == 0x6891CC && cfgProneBack >= 0 && rm == cfgProneBack + 1;
            if (backC && g_proneLatch && !g_aimLatch && *(u16*)(pl + 0x7E0) > 100 && w > 0 && w < 64) {
                static u64 told; if (!((told >> w) & 1)) { told |= 1ull << w; char b[160]; wsprintfA(b, "crawl for weapon %d: the game crawls him backward (6891CC/%d) - not taken as the forward crawl", w, rm); Log(b); }
            }
            if (((!g_aimLatch && *(u16*)(pl + 0x7E0) > 100) || ownFwd) && !pose && !backC && g_proneLatch && !g_transLatch && w > 0 && w < 64 && *(u32*)(req + 0x0C)) {
                u32 a0 = *(u32*)(req + 0x0C);
                if (a0 == la && rm == lm) { if (cnt < 1000) cnt++; } else { la = a0; lm = rm; cnt = 0; }
                if (cnt == 20 && (g_crawlArc[w] != a0 || g_crawlMot[w] != rm)) {
                    g_crawlArc[w] = a0; g_crawlMot[w] = (i16)rm;
                    char b[120]; wsprintfA(b, "learned crawl for weapon %d: %X/%d", w, a0, rm); Log(b);
                }
            } else cnt = 0;
        }
        // While a long gun fires, cycles or reloads on his feet, the game plays that motion on both layers at once
        // (same archive and number, the upper-body layer with its own mask) and the raised flag flickers with every
        // shot. That pair counts too, so the legs keep walking steadily under the firing arms instead of being switched
        // back and forth several times a second.
        u32 ra = *(u32*)(req + 0x0C), rb = *(u32*)(req + 0x34);
        int both = !g_proneLatch && ra && ra != 0x6891CC && rb == ra && *(i16*)(req + 0x38) == rm && *(u64*)(req + 0x40) != 0;
        // lying in the shoulder view the game now and then lowers the gun by itself and asks for its own lying motion
        // 10 / 11 or its crawl; that request was left alone, no crawl motion played and the plugin pushed him along:
        // sliding. Those requests are now replaced like the others.
        int w2 = Wpn(pl);
        int crawlReq = g_proneLatch && g_aimLatch && !g_rawAim && ((ra != 0x6891CC && (rm == 10 || rm == 11)) || (ra == 0x6891CC && (rm == 13 || rm == 14))
            || (w2 > 0 && w2 < 64 && g_crawlArc[w2] && g_crawlArc[w2] == ra && g_crawlMot[w2] == rm));
        int idle = g_rawAim || both || crawlReq || (g_proneLatch ? (rm == 12 || (rm >= 17 && rm <= 21) || rm == 61 || rm == 62) : (rm == 1 || rm == 3));
        if (crawlReq && moving && !g_rawAim) {
            g_lowCrawlN++;
            static int told; if (told < 4) { told++; char b[200]; wsprintfA(b, "lying in the shoulder view, gun lowered by the game: its motion %X/%d is replaced by the crawl motions too", ra, rm); Log(b); }
        }
        if (moving && idle && !g_transLatch && *(u64*)(req + 0x18) != 0) {
            int d = PickDir();
            int slot = -1; u32 arc = 0x6891CC;
            if (g_proneLatch) {
                // lying down: left / right live in the equipped weapon's archive
                u32 wa = g_wpnArc;
                if (d >= 2) {
                    if (wa && wa != 0x6891CC && HasProneSide(wa)) { slot = cfgProne[d - 2]; arc = wa; }
                    else if (cfgProneMain[d - 2] >= 0) { slot = cfgProneMain[d - 2]; arc = 0x6891CC; }
                }
                // Backward: the game's own backward crawl, motion 14 of Snake's main archive. Read from the archive
                // itself: it moves him 690 units backward over 80 frames, lying height, the only motion there that
                // travels backward on the ground; like the forward crawl (motion 13) it loops.
                else if (d == 1) { if (cfgProneBack >= 0) { slot = cfgProneBack; arc = 0x6891CC; } }
                else if (d == 0) {   // forward: the game's own crawl motion for this weapon
                    int w = Wpn(pl);
                    // the rifles named in the ini first, then a learned crawl
                    if (w > 0 && w < 32 && ((u32)cfgCrawlMask >> w & 1) && cfgCrawlRifle >= 0) { slot = cfgCrawlRifle; arc = 0x6891CC; }   // these keep their crawl in Snake's main archive
                    else if (w > 0 && w < 64 && g_crawlArc[w]) { slot = g_crawlMot[w] - 1; arc = g_crawlArc[w]; }
                    else if (cfgProneCrawl >= 0 && wa && wa != 0x6891CC) { slot = cfgProneCrawl; arc = wa; }
                    // never the backward crawl as "forward".
                    if (arc == 0x6891CC && slot == cfgProneBack) slot = cfgCrawlRifle >= 0 ? cfgCrawlRifle : 12;
                }
            } else {
                // two walks each, as on the 3DS (stick pushed lightly / fully). If a faster slot is switched off in the
                // ini (-1) the slow one is used and the plugin keeps to its speed.
                int fs = g_fast ? (g_crouchLatch ? cfgCRun[d] : cfgRun[d]) : -1;
                if (fs >= 0) { slot = fs; g_fastLeg = 1; } else { slot = g_crouchLatch ? cfgCWalk[d] : cfgWalk[d]; g_fastLeg = 0; }
            }
            if (slot >= 0 && slot < 1000) {
                sa = *(u32*)(req + 0x0C); sm = *(i16*)(req + 0x10);
                *(u32*)(req + 0x0C) = arc; *(i16*)(req + 0x10) = (i16)(slot + 1);
                if (*(u32*)(pl + 0x74) != arc || *(i16*)(pl + 0x78) != (i16)(slot + 1)) { *(i16*)(req + 0x12) = (i16)cfgBlend; g_legSets++; }
                g_legSlot = slot; did = 1; g_legFrame = g_frame; g_legDir = d;
                // Lying down: some weapons (EZ gun, knife ...) are held with a second, upper-body layer on top of the
                // plain lying pose, where the Mk22 and the long guns use one whole-body motion. The crawl motions are
                // whole-body motions: with the upper layer left on, only his legs crawled and the arms stayed frozen in
                // the aiming pose. While the plugin plays a crawl the request is made the way the Mk22's is: whole
                // body, no upper layer.
                if (g_proneLatch && cfgProneWhole && *(u64*)(req + 0x40) != 0) {
                    wm0 = *(u64*)(req + 0x18); wa1 = *(u32*)(req + 0x34); wn1 = *(i16*)(req + 0x38); wb1 = *(i16*)(req + 0x3A); wm1 = *(u64*)(req + 0x40);
                    *(u64*)(req + 0x18) = 0x01FFFFFFFFFFFFFFull;
                    *(u32*)(req + 0x34) = 0x6891CC; *(i16*)(req + 0x38) = 0; *(u64*)(req + 0x40) = 0;
                    if (*(u32*)(pl + 0xAC) || *(u32*)(pl + 0xA8)) *(i16*)(req + 0x3A) = (i16)cfgBlend;
                    whole = 1;
                }
            }
        }
        // crouch walking outside the shoulder view, stick pushed fully: the fast crouch walk (the one the shoulder view
        // uses) instead of the low walk. The crouch walking asks for motion 5 ("stand move stalk") and turns it into
        // its low walk further down; asking for main-archive motion 221 instead leaves that alone. Holding a weapon,
        // the original motion is kept on the upper-body layer so the gun stays in his hands.
        if (!did && cfgCrouchAll && !g_aimLatch && cfgCRun[0] >= 0 && Bit(ST_CROUCH) && !Bit(ST_STAND) && !Bit(ST_FPV)
            && !g_transLatch && (rm == 5 || rm == 4) && ra && (o_Inp ? g_padRaw : *(u16*)(pl + 0x7E0)) >= cfgFastAbove && !SlowHeld()) {
            sa = ra; sm = (i16)rm;
            i16 nm = (i16)(cfgCRun[0] + 1);
            *(u32*)(req + 0x0C) = 0x6891CC; *(i16*)(req + 0x10) = nm;
            if (*(u32*)(pl + 0x74) != 0x6891CC || *(i16*)(pl + 0x78) != nm) *(i16*)(req + 0x12) = (i16)cfgBlend;
            cw = 1;
            if (cfgCrouchArms && ra != 0x6891CC && *(u64*)(req + 0x40) == 0) {
                wa1 = *(u32*)(req + 0x34); wn1 = *(i16*)(req + 0x38); wb1 = *(i16*)(req + 0x3A); wm1 = *(u64*)(req + 0x40);
                *(u32*)(req + 0x34) = ra; *(i16*)(req + 0x38) = (i16)rm; *(u64*)(req + 0x40) = 0x1FFFFFFFF807FFEull;
                if (*(u32*)(pl + 0x9C) != ra || *(i16*)(pl + 0xA0) != (i16)rm) *(i16*)(req + 0x3A) = (i16)cfgBlend;
                cwArms = 1;
            }
            if (!cwWas) {
                g_cwSwap++;
                static int told; if (!told) { told = 1; char b[140]; wsprintfA(b, "crouch walking (normal view): fast crouch animation 6891CC/%d instead of %X/%d%s", (int)nm, ra, rm, cwArms ? ", weapon kept on the upper body" : ""); Log(b); }
            }
        } else if (cwWas && !did && !g_aimLatch) *(i16*)(req + 0x12) = (i16)cfgBlend;
        cwWas = cw;
        if (!did && !cw && g_aimLatch && g_rawAim && !g_transLatch && !g_proneLatch && ra && ra != 0x6891CC) {
            // optional: another aiming pose from the weapon's own archive (the added high slots)
            int ps = g_crouchLatch ? (rm == 3 ? cfgPoseCrouch : -1) : (rm == 1 ? cfgPoseStand : -1);
            if (ps >= 0 && ps < 1000) {
                sa = ra; sm = (i16)rm; *(i16*)(req + 0x10) = (i16)(ps + 1);
                if (*(i16*)(pl + 0x78) != (i16)(ps + 1)) *(i16*)(req + 0x12) = (i16)cfgBlend;
                pose = 1;
            }
        }
        if (!did && was) { *(i16*)(req + 0x12) = (i16)cfgBlend; g_legSlot = -1; }   // blend back to the game's pose
        was = did;
        g_rqDid = did | (cw << 1) | (pose << 2) | (whole << 3);   // for the fire trace
    }
    if (did || cw) {   // footsteps for an added motion
        int ns = *(i16*)(req + 0x10) - 1;
        int sub = SoundFor(ns, g_proneLatch, g_crouchLatch || cw);
        // lying sideways the weapons' own archives are used (slots 200 / 201 there) - also lying sounds
        if (did && g_proneLatch && *(u32*)(req + 0x0C) != 0x6891CC && ns >= 199) sub = cfgSnd ? cfgSndProne : -1;
        if (sub >= 0) { g_seqSlot = ns; g_seqSub = sub; }
    }
    o_Req(pl, req);
    g_seqSlot = -1;
    if (pose) { *(u32*)(req + 0x0C) = sa; *(i16*)(req + 0x10) = sm; }
    if (cw) {
        *(u32*)(req + 0x0C) = sa; *(i16*)(req + 0x10) = sm;
        if (cwArms) { *(u32*)(req + 0x34) = wa1; *(i16*)(req + 0x38) = wn1; *(i16*)(req + 0x3A) = wb1; *(u64*)(req + 0x40) = wm1; }
    }
    if (whole) { *(u64*)(req + 0x18) = wm0; *(u32*)(req + 0x34) = wa1; *(i16*)(req + 0x38) = wn1; *(i16*)(req + 0x3A) = wb1; *(u64*)(req + 0x40) = wm1; }
    if (did) {
        *(u32*)(req + 0x0C) = sa; *(i16*)(req + 0x10) = sm;
        // playback step of the body layer, for the log
        LegRate(pl);
        u8* mc = *(u8**)(pl + 0x2B8);
        if (Readable(mc, 0x10)) {
            u8* mt = *(u8**)(mc + 8);
            if (Readable(mt, 0x50)) g_tb = (int)(*(float*)(mt + 0x4C) * 100.0f);
        }
    }
}
// the game walk whose footstep sequence an added motion borrows (-1 = none). Lengths: the standing fast walks (200 -
// 203) are 40 frames like the run (5); the slow ones (204 - 207) 60 like the stalk walk (4); the low crouch walks (216
// - 219) 112 like the slow walk (3); the fast crouch walks (220 - 223) 50 - 70, near the stalk walk (4).
static int SoundFor(int slot, int lying, int crouched)
{
    if (!cfgSnd || slot < 199) return -1;
    if (lying) return cfgSndProne;
    if (slot >= 200 && slot <= 203) return cfgSndStandFast;
    if (slot >= 204 && slot <= 207) return !crouched && g_band == 2 && cfgSndStandStalk >= 0 ? cfgSndStandStalk : cfgSndStandSlow;   // stalk steps
    if (slot >= 216 && slot <= 219) return cfgSndCrouchSlow;
    if (slot >= 220 && slot <= 223) return cfgSndCrouchFast;
    return crouched ? cfgSndCrouchSlow : cfgSndStandSlow;
}
static int HookReq(u8* target)
{
    static const u8 expect[8] = { 0x48, 0x8B, 0xC4, 0x53, 0x48, 0x83, 0xEC, 0x70 };   // mov rax,rsp; push rbx; sub rsp,70h
    for (int i = 0; i < 8; i++) if (target[i] != expect[i]) return 0;
    u8* m = AllocNear(target); if (!m) return 0;
    m[0] = 0xFF; m[1] = 0x25; *(u32*)(m + 2) = 0; *(u64*)(m + 6) = (u64)(void*)&hk_Req;
    u8* tr = m + 16;
    memcpy(tr, target, 8);
    tr[8] = 0xFF; tr[9] = 0x25; *(u32*)(tr + 10) = 0; *(u64*)(tr + 14) = (u64)(target + 8);
    o_Req = (Req_t)(void*)tr;
    DWORD old;
    if (!VirtualProtect(target, 8, 0x40, &old)) return 0;
    u8 patch[8]; patch[0] = 0xE9; *(i32*)(patch + 1) = (i32)(m - (target + 5)); patch[5] = patch[6] = patch[7] = 0x90;
    memcpy(target, patch, 8);
    VirtualProtect(target, 8, old, &old);
    FlushInstructionCache(GetCurrentProcess(), target, 8);
    return 1;
}

static volatile u32 g_rearms;   // times the plugin pressed the aim key for the player
// ---- upper body follows the aim The game already bends Snake's upper body and arms toward a point when it wants him
// to aim at something (lock-on, first-person view): each frame some code stores a world position at Snake+0x520 and a
// weight at Snake+0x52C, and this routine turns that into bone rotations and clears the weight again. While the
// shoulder view is on, the plugin supplies a point far along the aim direction just before the routine runs.
typedef void (*AimAdj_t)(u8* pl, u8* st);
static AimAdj_t o_AimAdj;
static int PoseAim(u8* pl, float* A, float* P, float* H, float* qOut, float* q14);
static void PoseAimAfter(u8* pl, int solved, const float* P, const float* H, float q, const float* q14);
static void PaIdle(void);
static void hk_AimAdj_(u8* pl, u8* st);
static void hk_AimAdj(u8* pl, u8* st) { PROF0 hk_AimAdj_(pl, st); PROF1(4) }
static void hk_AimAdj_(u8* pl, u8* st)
{
    int chg = 0, solved = 0; i16 l0 = 0, l1 = 0, l2 = 0;
    float paP[3], paH[3], paQ = 0, paQ14[4];   // his shoulder and hand before the game's turn, the turn asked for
    if (pl && pl == g_pl && cfgEnabled && Spine() && g_aimLatch && g_aimDirOk && (cfgSpineProne || !g_proneLatch) && !Bit(ST_FPV)) {
        float d0 = g_aimDir[0], d1 = g_aimDir[1], d2 = g_aimDir[2];
        *(float*)(pl + 0x520) = *(float*)(pl + 0x130) + d0 * 150000.0f;
        *(float*)(pl + 0x524) = *(float*)(pl + 0x134) + 1000.0f + d1 * 150000.0f;
        *(float*)(pl + 0x528) = *(float*)(pl + 0x138) + d2 * 150000.0f;
        if (g_aimPtOk) {
            // the point raised or lowered so that the barrel, turned about his shoulder, runs through the spot
            // (PoseAim)
            float A[3];
            if (PoseAim(pl, A, paP, paH, &paQ, paQ14)) { *(float*)(pl + 0x520) = A[0]; *(float*)(pl + 0x524) = A[1]; *(float*)(pl + 0x528) = A[2]; solved = 1; }
            // with the gun's vertical correction (PoseFix) the point is the same one raised or lowered
            else if (g_pfUse) { *(float*)(pl + 0x520) = g_pfAim[0]; *(float*)(pl + 0x524) = g_pfAim[1]; *(float*)(pl + 0x528) = g_pfAim[2]; }
            else { *(float*)(pl + 0x520) = g_aimPt[0]; *(float*)(pl + 0x524) = g_aimPt[1]; *(float*)(pl + 0x528) = g_aimPt[2]; }
        }
        *(float*)(pl + 0x52C) = -1.0f;
        g_spineSets++;
        // the routine turns his upper body toward the point by at most +-15 degrees sideways (its limits at Snake+0x53A
        // / +0x53E; up and down -80 to +50 at +0x538 / +0x53C, set by the game per situation, exe+9AB2A). Beyond that
        // it only eases toward the limit (exe+BAC50), so a near point beside him - the post next to him, which needs
        // about 30 - left him pointing straight ahead. For this one call the sideways limits are PoseTurn degrees; the
        // game's own values are put back right after, so nothing else of the game sees them.
        // likewise downward: the game stops the arms at 50 degrees down (+0x538), so looking down at the ground near
        // him the gun stayed too level. For this call the limit is PoseDown (70) degrees.
        if (cfgConverge == 3 && g_aimPtOk) {
            l0 = *(i16*)(pl + 0x538); l1 = *(i16*)(pl + 0x53A); l2 = *(i16*)(pl + 0x53E);
            i16 w = (i16)(cfgPoseTurn * 65536 / 360), dw = (i16)(cfgPoseDown * 65536 / 360);
            if (cfgPoseTurn > 15) {
                // how far he has to turn: the heading from his position to the point against his own (the routine
                // measures it the same way, exe+118430 from Snake+0x130)
                float vx = g_aimPt[0] - *(float*)(pl + 0x130), vz = g_aimPt[2] - *(float*)(pl + 0x138);
                int need = (i16)(u16)((u16)(int)(atan2_(vx, vz) * (32768.0f / 3.14159265f)) - *(u16*)(pl + 0x162));
                int needA = need < 0 ? -need : need;
                if (needA * 360 / 65536 > g_poseYawMax) g_poseYawMax = needA * 360 / 65536;
                if (l1 > 0 && l2 < 0 && l1 < w && -l2 < w) {
                    if (needA > l1 && needA > -l2) g_poseWideN++;
                    if (needA > w) g_poseLimN++;
                    *(i16*)(pl + 0x53A) = w; *(i16*)(pl + 0x53E) = (i16)-w;
                    chg |= 1;
                    static int told; if (!told) { told = 1; char b[160]; wsprintfA(b, "upper body: may twist %d degrees sideways toward the aim point in the shoulder view (the game's own limit %d / %d)", cfgPoseTurn, l1 * 360 / 65536, l2 * 360 / 65536); Log(b); }
                }
            }
            if (cfgPoseDown > 0 && l0 > 0 && l0 < dw) {
                *(i16*)(pl + 0x538) = dw; chg |= 2; g_poseDownN++;
                static int told; if (!told) { told = 1; char b[160]; wsprintfA(b, "upper body: the arms may point %d degrees down toward the aim point in the shoulder view (the game's own limit %d)", cfgPoseDown, l0 * 360 / 65536); Log(b); }
            }
        }
    }
    o_AimAdj(pl, st);
    if (chg & 1) { *(i16*)(pl + 0x53A) = l1; *(i16*)(pl + 0x53E) = l2; }
    if (chg & 2) *(i16*)(pl + 0x538) = l0;
    if (pl && pl == g_pl && cfgEnabled && g_aimLatch && cfgPoseAim) PoseAimAfter(pl, solved, paP, paH, paQ, paQ14);   // did the arm turn as asked?
    if (!solved && pl == g_pl) PaIdle();
}
// Changing it would only have changed the cigar spray, so it is not done. What makes the game go into first person in a
// bush is not known yet: each time first person (or the end of the third-person state) comes by itself while he lies
// down - nothing pressed - the plugin writes which of the game's status marks changed in the second before, so the next
// log shows it.
static u32 g_stRing[32][12]; static u32 g_stRingAct[32]; static int g_stRingPos; static volatile u32 g_autoFpN, g_autoTpN;
static void StatusRing(void)
{
    if (!g_bits) return;
    int k = g_stRingPos++ & 31;
    for (int w = 0; w < 12; w++) g_stRing[k][w] = g_bits[w];
    u8* mw = g_moveWork; g_stRingAct[k] = Readable(mw, 0x60) ? (u32)(*(u64*)(mw + 0x58) - (u64)g_base) : 0;
}
static void StatusRingLog(const char* why)
{
    if (!g_bits) return;
    char b[460]; int n = wsprintfA(b, "%s - status marks changed in the 30 frames before (frames ago: +on / -off):", why);
    int cur = (g_stRingPos - 1) & 31, cnt = 0;
    for (int a = 1; a < 31 && cnt < 40; a++) {
        int f1 = (g_stRingPos - 1 - a) & 31, f0 = (f1 + 1) & 31;   // f1 older, f0 the frame after it
        for (int w = 0; w < 12 && cnt < 40; w++) {
            u32 d = g_stRing[f1][w] ^ g_stRing[f0][w];
            for (int t = 0; t < 32 && d && cnt < 40; t++) if (d >> t & 1) {
                int on = (g_stRing[f0][w] >> t) & 1;
                if (n < 400) n += wsprintfA(b + n, " %d:%c%X", a - 1, on ? '+' : '-', w * 32 + t);
                cnt++;
            }
        }
    }
    if (n < 420) n += wsprintfA(b + n, "; action exe+%X (30 frames ago exe+%X)", g_stRingAct[cur], g_stRingAct[(g_stRingPos - 30) & 31]);
    Log(b);
}
static int HookAimAdj(u8* target)
{
    static const u8 expect[5] = { 0x4C, 0x8B, 0xDC, 0x55, 0x56 };   // mov r11,rsp; push rbp; push rsi
    for (int i = 0; i < 5; i++) if (target[i] != expect[i]) return 0;
    u8* m = AllocNear(target); if (!m) return 0;
    m[0] = 0xFF; m[1] = 0x25; *(u32*)(m + 2) = 0; *(u64*)(m + 6) = (u64)(void*)&hk_AimAdj;
    u8* tr = m + 16;
    memcpy(tr, target, 5);
    tr[5] = 0xFF; tr[6] = 0x25; *(u32*)(tr + 7) = 0; *(u64*)(tr + 11) = (u64)(target + 5);
    o_AimAdj = (AimAdj_t)(void*)tr;
    DWORD old;
    if (!VirtualProtect(target, 5, 0x40, &old)) return 0;
    u8 patch[5]; patch[0] = 0xE9; *(i32*)(patch + 1) = (i32)(m - (target + 5));
    memcpy(target, patch, 5);
    VirtualProtect(target, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), target, 5);
    return 1;
}

static int LegPct(void) { return cfgLegSpeed * (g_crouchLatch ? cfgSpCrouch : cfgSpStand) / 100; }   // walk speed in percent
static float l_px, l_pz; static int l_have;
// The game's third-person camera: its elevation number (work+0x340, 0..1) against the pitch the camera really ends up
// with, in degrees (+ = up). Measured from three logs (60 settled pairs, the same standing, crouched and lying). Used
// so that the shoulder view and the normal view hand each other the same direction.
static const float kElev[] = { 0.150f, 0.280f, 0.306f, 0.347f, 0.388f, 0.437f, 0.456f, 0.497f, 0.533f, 0.569f, 0.601f, 0.637f,
                               0.664f, 0.677f, 0.692f, 0.709f, 0.744f, 0.784f, 0.848f, 0.859f, 0.900f, 0.950f, 1.000f };
static const float kPit[]  = { 40.1f, 30.0f, 28.3f, 23.9f, 16.6f, 8.75f, 6.0f, 0.8f, -3.2f, -6.7f, -9.6f, -12.4f,
                               -14.2f, -16.8f, -20.0f, -23.9f, -30.9f, -38.8f, -49.7f, -51.3f, -59.0f, -66.0f, -67.7f };
#define NPIT ((int)(sizeof kElev / sizeof kElev[0]))
static float ElevOfPitch(float p)
{
    if (p >= kPit[0]) return kElev[0];
    for (int i = 1; i < NPIT; i++)
        if (p >= kPit[i]) return kElev[i - 1] + (kElev[i] - kElev[i - 1]) * (kPit[i - 1] - p) / (kPit[i - 1] - kPit[i]);
    return kElev[NPIT - 1];
}

// Every shot of Snake's guns runs exe+379290 (Snake, the gun's work), which rolls, by gun and stance (standing most,
// lying down least; the Patriot and the Scorpion much less):
// - a random kick of the gun (work+0xC4 up/down, +0xC6 sideways, 1/65536 of a turn), which loses an eighth each frame
//   and is added to the gun's aim every frame (exe+37A5B9): in first person the shots spray around the crosshair and
//   the gun jumps about - the "random pattern";
// - a jolt of the view (Snake+0x7A0 / +0x7A2, first person adds it to its view), random each shot;
// - for the AK-47 a climb of the view (Snake+0x7C0, which first person adds to its view once and clears). The plugin
//   watches that routine and does the same in the shoulder view: the shot leaves the gun turned by the kick (so it
//   sprays around the crosshair exactly like in first person), the view jolts and the AK climbs. A gun for which the
//   game rolls nothing has no recoil in first person and none in the shoulder view either.
static volatile u32 g_fpShotFrame = 0x80000000u, g_fpShotN;   // last frame Snake fired in first person
static volatile float g_recUp, g_recTmpP, g_recTmpY;   // view climb to add to the aim pitch / fading jolt (degrees)
static volatile int g_kP, g_kY;   // the gun's kick as the game rolled it, fading as the game fades it
static volatile DWORD g_shotT; static volatile u32 g_shotHold;   // last shoulder-view shot; frames the head was held
static volatile u32 g_headDragN; static volatile int g_dragMax, g_dragMaxAll;   // frames the camera followed his head past the kick while firing; largest
static volatile u32 g_rkN, g_rkFpv, g_rkOts, g_rkFrame, g_rkSpread, g_rkEmu, g_otsShotFrame = 0x80000000u, g_otsShots, g_rkAtShot;
static volatile u64 g_rkSeen;   // guns the game rolled a recoil for (this session)
static volatile int g_rkLastP, g_rkLastY, g_rkLastJP, g_rkLastJY, g_rkLastC;
static u32 g_rng = 0x12345u;
static u32 Rnd16(void) { g_rng = g_rng * 0x5D588B65u + 1u; return g_rng; }
// the game's fade (exe+37A5B9): v minus v halved up to three times (stopping before 0)
static int Fade8(int v)
{
    int r = v;
    for (int k = 0; k < 3; k++) { int e = r / 2; if (!e) break; r = e; }
    return (int)(i16)(v - r);
}
typedef u64 (*Recoil_t)(u8* pl, u8* work);
static Recoil_t o_Recoil;
static void RecoilLog(const char* where, int w, int st, int kp, int ky, int jp, int jy, int cl)
{
    static u32 told[64][2];
    int fp = where[0] == 'f';
    if (w <= 0 || w >= 64) return;
    u32 c = ++told[w][fp];
    if (c > 4 && (c % 50) != 0) return;
    char b[260];
    wsprintfA(b, "recoil rolled by the game (%s): weapon %d stance %d - gun kick %d,%d  view jolt %d,%d  climb %d  (182 = 1 degree; shot %u with this gun)",
        where, w, st, kp, ky, jp, jy, cl, c);
    Log(b);
}
static u64 hk_Recoil_(u8* pl, u8* work);
static u64 hk_Recoil(u8* pl, u8* work) { PROF0 u64 r = hk_Recoil_(pl, work); PROF1(5) return r; }
static u64 hk_Recoil_(u8* pl, u8* work)
{
    u8* me = g_pl;
    int mine = pl && pl == me && Readable(pl, 0x800) && Readable(work, 0xD0);
    i16 c0 = 0, a0 = 0, a2 = 0;
    if (mine) { c0 = *(i16*)(pl + 0x7C0); a0 = *(i16*)(pl + 0x7A0); a2 = *(i16*)(pl + 0x7A2); }
    u64 r = o_Recoil(pl, work);
    if (!mine) return r;
    int kp = *(i16*)(work + 0xC4), ky = *(i16*)(work + 0xC6);
    int jp = (i16)(*(i16*)(pl + 0x7A0) - a0), jy = (i16)(*(i16*)(pl + 0x7A2) - a2), cl = (i16)(*(i16*)(pl + 0x7C0) - c0);
    int w = Wpn(pl), fp = Bit(ST_FPV), st = Bit(3) ? 2 : Bit(ST_CROUCH) ? 1 : 0;
    g_rkN++; g_rkFrame = g_frame;
    if (w > 0 && w < 64) g_rkSeen |= 1ull << w;
    g_rkLastP = kp; g_rkLastY = ky; g_rkLastJP = jp; g_rkLastJY = jy; g_rkLastC = cl;
    if (fp) { g_rkFpv++; RecoilLog("first person", w, st, kp, ky, jp, jy, cl); }
    else if (g_aimLatch) {
        g_rkOts++;
        RecoilLog("shoulder view", w, st, kp, ky, jp, jy, cl);
        if (cfgRecoil) {
            g_kP = kp; g_kY = ky;
            float u = 360.0f / 65536.0f * (float)cfgRecoilView / 100.0f, us = 360.0f / 65536.0f * (float)cfgRecoilShake / 100.0f;
            g_recTmpP += -(float)jp * us; g_recTmpY += (float)jy * us;   // the jolt (screen shake) is off by default
            g_recUp += -(float)cl * u;   // the AK-47's climb stays
        }
        *(i16*)(pl + 0x7C0) = c0;   // the shoulder view has shown the climb: it is not saved up for the next first person
    }
    return r;
}
// if the game does not roll a recoil for shoulder-view shots of a gun it rolls one for in first person, the plugin
// rolls one the same way (same numbers as exe+379290)
static void RecoilEmulate(int w)
{
    if (!cfgRecoil || w <= 0 || w >= 32 || !((g_rkSeen >> w) & 1) || !((u32)cfgSpreadMask >> w & 1)) return;
    int prone = Bit(3), crouch = Bit(ST_CROUCH) && !prone, lowR = prone || crouch;
    u32 range, shake;
    if (w == 9 || w == 10) { range = prone ? 0xF : crouch ? 0x1F : 0xFF; shake = lowR ? 0x40 : 0x80; }
    else { range = prone ? 0xFF : crouch ? 0x3FF : 0xFFFF; shake = lowR ? 0x40 : 0x100; }
    float u = 360.0f / 65536.0f * (float)cfgRecoilView / 100.0f;
    if (w == 12 && !prone) { int c = crouch ? -(0x24 + (int)(Rnd16() & 0x3F)) : -(0x5B + (int)(Rnd16() & 0x7F)); g_recUp += -(float)c * u; }
    int jp = (int)(i16)(u16)((Rnd16() & (shake - 1)) - (shake >> 1)), jy = (int)(i16)(u16)((Rnd16() & (shake - 1)) - (shake >> 1));
    float us = 360.0f / 65536.0f * (float)cfgRecoilShake / 100.0f;
    g_recTmpP += -(float)jp * us; g_recTmpY += (float)jy * us;
    int op = g_kP, oy = g_kY;
    int np = (int)(i16)(u16)((Rnd16() & range) - (range >> 1)), ny = (int)(i16)(u16)((Rnd16() & range) - (range >> 1));
    if (np - op > 0x16C) np = op + 0x16C; if (np - op < -0x16C) np = op - 0x16C;
    if (ny - oy > 0x16C) ny = oy + 0x16C; if (ny - oy < -0x16C) ny = oy - 0x16C;
    g_kP = np; g_kY = ny; g_rkEmu++;
    static int told; if (!told) { told = 1; char b[140]; wsprintfA(b, "recoil: the game rolls none for shoulder-view shots - the plugin rolls it the same way (weapon %d)", w); Log(b); }
}
// For the log: the height of Snake's head bone above his position, from the game's own bone routine (exe+372800 (out,
// bone, Snake+0x2B0, Snake+0x120); bone 4 is the one the first-person camera uses)
typedef void (*Bone_t)(float* out, int bone, u8* pair, u8* ctrl);
static Bone_t f_Bone;
// the camera is locked to Snake's head, as on the 3DS (measured from the 3DS capture: his head stays on the same pixel,
// within 2, through standing, crouching and walking). The head's world position comes from the game's own bone routine
// (exe+372800 with Snake's model pair +0x2B0 and his place +0x120 - the game calls it the same way, e.g. at
// exe+3723A6); bone 4 is the one the first-person camera sits on.
static int HeadWorld(u8* pl, float* out)
{
    if (!f_Bone) { g_headWhy = 1; return 0; }
    if (!Readable(pl, 0x800)) { g_headWhy = 2; return 0; }
    if (*(u32*)(pl + 0x24C) & 0x80000) {   // his model stands on the motion's own height then (exe+B8860)
        u8* q = *(u8**)(pl + 0x250);
        if (!Readable(q, 0x10) || !Readable(*(u8**)(q + 8), 0x80)) { g_headWhy = 3; return 0; }
    }
    u8* model = *(u8**)(pl + 0x2B0); u8* mc = *(u8**)(pl + 0x2B8);
    if (!Readable(model, 0xC0) || !Readable(mc, 0x60)) { g_headWhy = 4; return 0; }
    u8* sk = *(u8**)(model + 0xB8); u8* q = *(u8**)(mc + 0x50); int nb = *(int*)(mc + 0x14);
    if (nb < 5 || nb > 256 || !Readable(sk, 0x50 * 5) || !Readable(q, 16 * nb)) { g_headWhy = 5; return 0; }
    for (int bn = 4, k = 0; bn > 0; k++) { int p = *(int*)(sk + bn * 0x50 + 8); if (k > 8 || p >= bn || p < -1) { g_headWhy = 6; return 0; } bn = p; }
    __attribute__((aligned(16))) float o[4] = { 0, 0, 0, 1 };
    f_Bone(o, 4, pl + 0x2B0, pl + 0x120);
    float dx = o[0] - *(float*)(pl + 0x130), dy = o[1] - *(float*)(pl + 0x134), dz = o[2] - *(float*)(pl + 0x138);
    if (!(dx * dx + dz * dz < 1500.0f * 1500.0f) || !(dy > -1500.0f && dy < 2500.0f)) { g_headWhy = 7; return 0; }
    out[0] = o[0]; out[1] = o[1]; out[2] = o[2]; g_headWhy = 0;
    return 1;
}
static void HeadTrack(void)
{
    static int told, toldWhy, still; static float lx, lz;
    u8* pl = g_pl; float H[3];
    g_headOk = cfgHeadLock ? HeadWorld(pl, H) : 0;
    if (!g_headOk) {
        if (cfgHeadLock && g_headWhy && !toldWhy && g_pl) { toldWhy = 1; char b[120]; wsprintfA(b, "camera lock: Snake's head cannot be read (reason %d) - the camera follows his hips instead", (int)g_headWhy); Log(b); }
        return;
    }
    if (!told) {   // once: which bones lead to the head and to the two bones the aim turns (for the next step)
        told = 1;
        u8* sk = *(u8**)(*(u8**)(pl + 0x2B0) + 0xB8); int nb = *(int*)(*(u8**)(pl + 0x2B8) + 0x14);
        char b[300]; int n = wsprintfA(b, "camera lock: head bone found (%d bones); chains:", nb);
        int bones[3] = { 4, 9, 14 };
        for (int i = 0; i < 3; i++) {
            int bn = bones[i]; if (bn >= nb || !Readable(sk, 0x50 * (bn + 1))) continue;
            n += wsprintfA(b + n, " %d", bn);
            for (int k = 0; k < 12 && bn > 0; k++) { int p = *(int*)(sk + bn * 0x50 + 8); if (p < 0 || p >= nb || !Readable(sk, 0x50 * (p + 1))) break; n += wsprintfA(b + n, "<%d", p); bn = p; }
            n += wsprintfA(b + n, ";");
        }
        Log(b);
    }
    memcpy(g_head, H, 12);
    u16 by = *(u16*)(pl + 0x162);
    float fx = sin16(by), fz = sin16((u16)(by + 0x4000)), rx = -fz, rz = fx;
    float ox = H[0] - *(float*)(pl + 0x130), oy = H[1] - *(float*)(pl + 0x134), oz = H[2] - *(float*)(pl + 0x138);
    float F = ox * fx + oz * fz, R = ox * rx + oz * rz;
    g_hF = (int)F; g_hR = (int)R; g_hU = (int)oy;
    // learn where his head is when he stands still (preferably with the gun raised in the shoulder view)
    float mv = (*(float*)(pl + 0x130) - lx) * (*(float*)(pl + 0x130) - lx) + (*(float*)(pl + 0x138) - lz) * (*(float*)(pl + 0x138) - lz);
    lx = *(float*)(pl + 0x130); lz = *(float*)(pl + 0x138);
    int standing = Bit(ST_STAND) && !Bit(ST_CROUCH) && !Bit(3) && !Bit(6) && !Bit(ST_FPV) && !Bit(ST_WALL) && Bit(ST_TPS)
        && *(u16*)(pl + 0x7E0) == 0 && !g_have && mv < 1.0f;
    float hy = standing ? BodyHipY(pl) : -1.0f;
    still = (standing && hy > 7.6f && hy < 8.6f) ? still + 1 : 0;
    if (still >= 30 && oy > 200.0f && oy < 2000.0f && F > -500.0f && F < 500.0f && R > -500.0f && R < 500.0f) {
        int aim = g_aimLatch != 0;
        if (!g_headLearned || (aim && !g_headFromAim)) {
            g_hdF = F; g_hdR = R; g_hdU = oy; g_headLearned = 1; g_headFromAim = aim;
            char b[200]; wsprintfA(b, "camera lock: Snake's head learned standing still%s: %d ahead, %d right, %d above his position - the camera now follows his head",
                aim ? " in the shoulder view" : "", (int)F, (int)R, (int)oy); Log(b);
        } else if (aim || !g_headFromAim) {
            g_hdF += (F - g_hdF) * 0.02f; g_hdR += (R - g_hdR) * 0.02f; g_hdU += (oy - g_hdU) * 0.02f;
        }
    }
}
// for the log: tenths, signed (wsprintfA has no floats)
static char* Tenths(char* o, float v) { int t = (int)(v * 10.0f + (v < 0 ? -0.5f : 0.5f)); int a = t < 0 ? -t : t; wsprintfA(o, "%s%d.%d", t < 0 ? "-" : "", a / 10, a % 10); return o; }
// ---------------------------------------------------------------- the gun's vertical aim (Build 40, wish 1)
// How the game turns Snake's arms toward the aim point (exe+3700A0, read from the exe): it measures the angle up / down
// from his right-hand bone (bone 14 - placed by the bone routine exe+372800) to the point at Snake+0x520 and the angle
// sideways from his position, limits them (down 50 / up 80 degrees at Snake+0x538 / +0x53C, sideways +0x53A / +0x53E)
// and turns both arms (and the head) by them at the shoulders: one rotation, in his model's axes, for bones 5 / 6 and
// 10 / 11 and everything below them (lying down two other bones, half each). That points the hand's line at the point -
// but the barrel only follows it if the aiming motion holds the gun level when nothing is added, and it does not. So
// the plugin now looks at where the barrel really points, every frame, and moves the aim point up or down until the
// barrel points at the spot the crosshair covers:
// - the barrel: at every shot in the shoulder view the game's bullet matrix says exactly where the muzzle is and where
//   the barrel points (third row). The plugin notes both in the axes of a few hand bones. A bone's axes are the game's
//   own: Snake's motion controller (+0x2B8) keeps one quaternion per bone at +0x50 in his model's axes; the game's
//   exe+754240 turns it into a matrix and his model matrix (exe+B8860, Snake+0x120) places it in the world - exactly
//   what exe+372800 does. The bone the gun is fixed to keeps the same barrel in its own axes shot after shot, the
//   others do not: each bone's prediction of the next shot is scored, and one that predicts it within PoseFixCheck (3)
//   degrees over 3 shots (and shots at different angles) is used.
// - every frame the barrel and muzzle are worked out from that bone, the angle between the barrel and the line from the
//   muzzle to the crosshair spot is measured, and PoseFixGain (20) percent of it is added to a correction (at most
//   PoseFixMax degrees). The correction turns the aim point up or down as seen from bone 14 - the point the game
//   measures from - so the game's arms go up or down by it. Only its height changes: the sideways turn and the shots
//   are untouched;
// If the barrel does not follow the correction (90 frames at the limit with the error still there) it is switched off
// for that gun and the log says so. A shot is never moved: they still come from the middle of the screen.
typedef void (*QMat_t)(float* out, const float* q); typedef void (*MMat_t)(u8* place, float* out);
static QMat_t f_QMat; static MMat_t f_MMat;
#define PF_NB 6
#define PF_NW 32
static const int kPfBone[PF_NB] = { 14, 13, 9, 8, 12, 7 };
typedef struct { float b[3], o[3], w, err; i32 n, pad; } PfCand;   // barrel / muzzle in the bone's axes (sums), weight, prediction error, shots
typedef struct { PfCand c[PF_NB]; float pMin, pMax; } PfWpn;   // per weapon type; the barrel pitch range of the shots learned from
static PfWpn g_pf[PF_NW];
static u8 g_pfIdx[PF_NW], g_pfDead[PF_NW];   // candidate in use + 1 (0 = none); switched off this session
static float g_pfC[3];   // the correction now, per stance (radians)
static volatile float g_pfPredB[3]; static volatile u32 g_pfPredFrame = 0x80000000u; static volatile int g_pfPredW = -1;
static float g_pfCamErr[PF_NW], g_pfLastCam = -1, g_pfLastHand = -1; static int g_pfCamN[PF_NW];   // frame check per weapon
static volatile int g_pfState, g_pfEv10, g_pfC10, g_pfBone = -1;
static volatile u32 g_pfFrames, g_pfActive, g_pfLearnN, g_pfShotN[2], g_pfDeadN, g_pfNoRespN, g_pfSatN; static volatile float g_pfShotErr[2], g_pfEvSum;

// the arms' motions (upper-body layer, Snake+0x9C archive / +0xA0 number) seen while aiming: at a shot, or 30 frames in
// a row with the barrel within 6 degrees of the spot. In those the correction may go to 45 degrees; in any other motion
// (a reload) only up to 20 degrees off is taken as aiming.
static u32 g_pfPose[16]; static int g_pfPoseN;
static u32 PoseKey(u8* pl) { return Readable(pl, 0x800) ? (*(u32*)(pl + 0x9C) ^ ((u32)*(u16*)(pl + 0xA0) << 20)) : 0; }
static int PoseKnown(u32 k) { for (int i = 0; i < g_pfPoseN; i++) if (g_pfPose[i] == k) return 1; return 0; }
static void PoseNote(u8* pl, u32 k, const char* how)
{
    if (!k || PoseKnown(k)) return;
    if (g_pfPoseN < 16) g_pfPose[g_pfPoseN++] = k; else { for (int i = 1; i < 16; i++) g_pfPose[i - 1] = g_pfPose[i]; g_pfPose[15] = k; }
    static int told; if (told < 16) { told++; char b[160]; wsprintfA(b, "gun pose: arms' motion %X/%d noted as an aiming pose (%s)", *(u32*)(pl + 0x9C), (int)*(i16*)(pl + 0xA0), how); Log(b); }
}
static float sqrt_(float v) { return v > 0 ? __builtin_sqrtf(v) : 0; }
static int Norm3(float* v) { float l = sqrt_(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]); if (!(l > 1e-6f && l < 1e12f)) return 0; v[0] /= l; v[1] /= l; v[2] /= l; return 1; }
static float Pitch3(const float* v) { return atan2_(v[1], sqrt_(v[0] * v[0] + v[2] * v[2])); }
static float AngDeg(const float* a, const float* b)   // angle between two unit vectors, degrees
{
    float c = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    float x = a[1] * b[2] - a[2] * b[1], y = a[2] * b[0] - a[0] * b[2], z = a[0] * b[1] - a[1] * b[0];
    return atan2_(sqrt_(x * x + y * y + z * z), c) * 57.29578f;
}
static void LocalToWorld(const float* R, const float* v, float* o) { for (int j = 0; j < 3; j++) o[j] = v[0] * R[j] + v[1] * R[3 + j] + v[2] * R[6 + j]; }
static void WorldToLocal(const float* R, const float* v, float* o)
{
    float s2 = R[0] * R[0] + R[1] * R[1] + R[2] * R[2];
    for (int i = 0; i < 3; i++) o[i] = (v[0] * R[i * 3] + v[1] * R[i * 3 + 1] + v[2] * R[i * 3 + 2]) / s2;
}
// a bone's world position (the game's bone routine), with the same checks as the head lock
static int BonePos(u8* pl, int bone, float* P)
{
    if (!f_Bone || !Readable(pl, 0x800)) return 0;
    if (*(u32*)(pl + 0x24C) & 0x80000) { u8* q = *(u8**)(pl + 0x250); if (!Readable(q, 0x10) || !Readable(*(u8**)(q + 8), 0x80)) return 0; }
    u8* model = *(u8**)(pl + 0x2B0); u8* mc = *(u8**)(pl + 0x2B8);
    if (!Readable(model, 0xC0) || !Readable(mc, 0x60)) return 0;
    u8* sk = *(u8**)(model + 0xB8); u8* qs = *(u8**)(mc + 0x50); int nb = *(int*)(mc + 0x14);
    if (bone < 1 || nb <= bone || nb > 256 || !Readable(sk, 0x50 * (bone + 1)) || !Readable(qs, 16 * nb)) return 0;
    for (int bn = bone, k = 0; bn > 0; k++) { int p = *(int*)(sk + bn * 0x50 + 8); if (k > 16 || p >= bn || p < -1) return 0; bn = p; }
    __attribute__((aligned(16))) float o[4] = { 0, 0, 0, 1 };
    f_Bone(o, bone, pl + 0x2B0, pl + 0x120);
    float dx = o[0] - *(float*)(pl + 0x130), dy = o[1] - *(float*)(pl + 0x134), dz = o[2] - *(float*)(pl + 0x138);
    if (!(dx * dx + dz * dz < 2500.0f * 2500.0f) || !(dy > -1500.0f && dy < 3000.0f)) return 0;
    P[0] = o[0]; P[1] = o[1]; P[2] = o[2];
    return 1;
}
// ... and its axes in the world (row vectors: world = local x R; R carries the model's scale)
static int BoneFrame(u8* pl, int bone, float* R, float* P)
{
    if (!f_QMat || !f_MMat || !BonePos(pl, bone, P)) return 0;
    const float* q = (const float*)(*(u8**)(*(u8**)(pl + 0x2B8) + 0x50) + 16 * bone);
    float qq = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];
    if (!(qq > 0.5f && qq < 2.0f)) return 0;
    __attribute__((aligned(16))) float mq[16], mm[16];
    f_QMat(mq, q); f_MMat(pl + 0x120, mm);
    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) R[i * 3 + j] = mq[i * 4] * mm[j] + mq[i * 4 + 1] * mm[4 + j] + mq[i * 4 + 2] * mm[8 + j];
    float s2 = R[0] * R[0] + R[1] * R[1] + R[2] * R[2];
    return s2 > 1e-6f && s2 < 1e6f;
}
static void PoseChoose(int w, int quiet);
static void PoseChoose(int w, int quiet)
{
    PfWpn* Wp = &g_pf[w];
    int best = -1; float be = 1e9f;
    for (int c = 0; c < PF_NB; c++) { PfCand* k = &Wp->c[c]; if (k->n >= 3 && k->w > 0 && k->err < be) { be = k->err; best = c; } }
    int ok = best >= 0 && be <= (float)cfgPoseFixCheck && (Wp->pMax - Wp->pMin >= 3.0f || Wp->c[best].n >= 6);
    int ni = ok ? best + 1 : 0;
    if (ni != g_pfIdx[w] && !quiet) {
        char b[260], e[16]; int t = (int)(be * 10.0f + 0.5f);
        wsprintfA(e, "%d.%d", t / 10, t % 10);
        if (ni) wsprintfA(b, "gun pose: weapon %d - the gun is held by bone %d (it predicts the shots within %s degrees) - the barrel is now brought onto the crosshair spot", w, kPfBone[best], e);
        else if (best >= 0) wsprintfA(b, "gun pose: weapon %d - no hand bone predicts the shots well enough yet (best bone %d: %s degrees, shot pitches %d..%d) - no correction", w, kPfBone[best], e, (int)Wp->pMin, (int)Wp->pMax);
        else wsprintfA(b, "gun pose: weapon %d - not enough shots yet to find the bone holding the gun - no correction", w);
        Log(b);
    }
    g_pfIdx[w] = (u8)ni;
}
// at a shot in the shoulder view: note the barrel and muzzle in each candidate bone's axes, score the predictions
static void PoseLearn(u8* pl, int w, const float* m)
{
    if (w <= 0 || w >= PF_NW || !f_QMat || !f_MMat) return;
    static u32 lastF = 0xFFFFFFFFu;
    if (lastF == g_frame) return;   // one sample a frame (the shotgun's pellets are one shot)
    float b[3] = { m[8], m[9], m[10] }; if (!Norm3(b)) return;
    lastF = g_frame;
    float M[3] = { m[12], m[13], m[14] };
    g_pfLastCam = -1;
    if (g_pfPredW == w && (u32)(g_frame - g_pfPredFrame) <= 2) {   // last frame's prediction against the real barrel
        float pb[3] = { g_pfPredB[0], g_pfPredB[1], g_pfPredB[2] };
        float a = AngDeg(pb, b); g_pfLastCam = a;
        g_pfCamErr[w] = g_pfCamN[w] ? g_pfCamErr[w] + (a - g_pfCamErr[w]) * 0.3f : a; g_pfCamN[w]++;
        if (g_pfCamN[w] == 3 && g_pfCamErr[w] > 6.0f) { char t1[16], bb[220]; wsprintfA(bb, "gun pose: weapon %d - the barrel worked out each frame is %s degrees off the real one at the shots (frame check) - no correction for it", w, Tenths(t1, g_pfCamErr[w])); Log(bb); }
    }
    PfWpn* Wp = &g_pf[w];
    float pit = Pitch3(b) * 57.29578f;
    int any = 0; for (int c = 0; c < PF_NB; c++) if (Wp->c[c].n) any = 1;
    if (!any) { Wp->pMin = Wp->pMax = pit; } else { if (pit < Wp->pMin) Wp->pMin = pit; if (pit > Wp->pMax) Wp->pMax = pit; }
    g_pfLastHand = -1;
    for (int c = 0; c < PF_NB; c++) {
        float R[9], Pb[3];
        if (!BoneFrame(pl, kPfBone[c], R, Pb)) continue;
        PfCand* k = &Wp->c[c];
        float bl[3], ol[3], d[3] = { M[0] - Pb[0], M[1] - Pb[1], M[2] - Pb[2] };
        WorldToLocal(R, b, bl); if (!Norm3(bl)) continue;
        WorldToLocal(R, d, ol);
        if (k->n > 0 && k->w > 0) {
            float mb[3] = { k->b[0], k->b[1], k->b[2] }, pw[3];
            if (Norm3(mb)) { LocalToWorld(R, mb, pw); if (Norm3(pw)) { float a = AngDeg(pw, b); k->err = k->n == 1 ? a : k->err + (a - k->err) * 0.3f;
                if (g_pfIdx[w] == c + 1) g_pfLastHand = a; } }
        }
        for (int i = 0; i < 3; i++) { k->b[i] = k->b[i] * 0.85f + bl[i]; k->o[i] = k->o[i] * 0.85f + ol[i]; }
        k->w = k->w * 0.85f + 1.0f; if (k->n < 1000000) k->n++;
    }
    g_pfLearnN++;
    PoseNote(pl, PoseKey(pl), "a shot");
    PoseChoose(w, 0);
}
// the barrel's direction and the muzzle now, from the bone the gun is held by
static int GunNow(u8* pl, int w, float* b, float* M)
{
    if (w <= 0 || w >= PF_NW || !g_pfIdx[w]) return 0;
    int c = g_pfIdx[w] - 1; PfCand* k = &g_pf[w].c[c];
    if (!(k->w > 0)) return 0;
    float R[9], P[3];
    if (!BoneFrame(pl, kPfBone[c], R, P)) return 0;
    float bl[3] = { k->b[0], k->b[1], k->b[2] }, ol[3] = { k->o[0] / k->w, k->o[1] / k->w, k->o[2] / k->w };
    if (!Norm3(bl)) return 0;
    LocalToWorld(R, bl, b); if (!Norm3(b)) return 0;
    LocalToWorld(R, ol, M); for (int i = 0; i < 3; i++) M[i] += P[i];
    return 1;
}
// every frame in the shoulder view (Converge=3), right after the aim point is worked out
static void PoseDead(int w, const char* why, float a, float b)
{
    g_pfDead[w] = 1; g_pfDeadN++;
    char t1[16], t2[16], bb[260];
    wsprintfA(bb, "gun pose: weapon %d - %s (%s / %s) - switched off for this gun until the game is restarted", w, why, Tenths(t1, a), Tenths(t2, b)); Log(bb);
}
static __attribute__((noinline)) void PoseFix(u8* pl)
{
    g_pfUse = 0; g_pfEv10 = 0;
    int st = g_proneLatch ? 2 : g_crouchLatch ? 1 : 0;
    int w = Wpn(pl);
    static int lastW = -999, armed, sat, steady, fails, atLimWas, good; static float pb0, ps0, phiWas, psiWas; static u32 lastL1;
    if (w != lastW) { g_pfC[0] = g_pfC[1] = g_pfC[2] = 0; lastW = w; armed = 0; sat = 0; fails = 0; }
    float* c = &g_pfC[st];
    int wv = w > 0 && w < PF_NW;
    int state = !cfgPoseFix ? 5 : wv && g_pfDead[w] ? 4 : 0;
    float b[3], M[3];
    int model = cfgPoseFix && cfgConverge == 3 && wv && !Throwable(w) && g_pfIdx[w] && !g_pfDead[w] && (g_pfCamN[w] < 3 || g_pfCamErr[w] <= 6.0f);
    if (model && !GunNow(pl, w, b, M)) model = 0;
    if (!model && state == 0 && wv) { for (int k = 0; k < PF_NB; k++) if (g_pf[w].c[k].n) state = 1; }
    u32 l1 = PoseKey(pl);
    steady = l1 == lastL1 ? (steady < 1000 ? steady + 1 : steady) : 0; lastL1 = l1;
    if (!steady) good = 0;
    // The correction turns the aim point up or down as seen from his chest (bone 2): a point the game's arm turn never
    // moves (it turns bones 5 / 6 and 10 / 11 and below, also lying down), so the plugin's own correction cannot move
    // the point it is measured from. (Measured from his hand, which the turn does move, the correction partly cancelled
    // itself.)
    float S[3], hS = 0, phi = 0; int haveS = g_aimPtOk && BonePos(pl, 2, S);
    if (haveS) {
        float v[3] = { g_aimPt[0] - S[0], g_aimPt[1] - S[1], g_aimPt[2] - S[2] };
        hS = sqrt_(v[0] * v[0] + v[2] * v[2]); phi = atan2_(v[1], hS);
        if (hS < 200.0f) haveS = 0;
    }
    // the angle of the point last handed to the game (with the correction), seen from the chest: the barrel should
    // follow it
    float psi = 0;
    if (haveS) psi = phi + *c;   // (the correction as it was handed to the game last)
    int integ = 0;
    if (model) {
        state = 3;
        for (int i = 0; i < 3; i++) g_pfPredB[i] = b[i];
        g_pfPredFrame = g_frame; g_pfPredW = w;
        float d[3] = { g_tPt[0] - M[0], g_tPt[1] - M[1], g_tPt[2] - M[2] };
        float L = sqrt_(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
        if (L > 300.0f && haveS && (u32)(g_frame - g_tFrame) <= 1) {
            float pb = Pitch3(b), ev = Pitch3(d) - pb;   // + = the barrel has to go up
            g_pfEv10 = (int)(ev * 572.9578f);
            DWORD tn = GetTickCount();
            int quiet = !g_fireNow && !(g_shotT && tn - g_shotT < 400) && !g_transLatch && steady >= 15;
            float evA = fabs_(ev);
            if (quiet && evA < 0.105f) { if (++good == 30) PoseNote(pl, l1, "the barrel on the spot"); } else good = 0;
            if (quiet && (evA < 0.349f || (evA < 0.785f && PoseKnown(l1)))) {
                state = 2; integ = 1;
                // does the barrel follow the point handed to the game? Once that point's angle (seen from the chest)
                // has moved 5 degrees - by the correction or by the view - the barrel must have moved at least 20 % of
                // it the same way. If not, the plugin is not seeing the game's turn: the first time the correction is
                // taken back to zero, the second time in a row it is switched off for this gun. Not counted at the
                // arms' limit (they cannot follow then) or while the view swings fast (the arms are a frame behind).
                if (atLimWas || fabs_(phi - phiWas) > 0.0175f || fabs_(psi - psiWas) > 0.035f) armed = 0;
                if (!armed) { armed = 1; pb0 = pb; ps0 = psi; }
                else if (fabs_(psi - ps0) >= 0.0873f) {
                    float resp = (pb - pb0) / (psi - ps0);
                    if (resp < 0.2f) {
                        g_pfNoRespN++;
                        if (++fails >= 2) { PoseDead(w, "the barrel did not follow the point given to the game (that point moved / the barrel moved, degrees)", (psi - ps0) * 57.29578f, (pb - pb0) * 57.29578f); fails = 0; }
                        else { *c = 0; ev = 0; }   // start again from no correction
                    } else fails = 0;
                    armed = 0;
                }
                if (!g_pfDead[w] && !(g_pfAtLim > 0 && ev < 0) && !(g_pfAtLim < 0 && ev > 0)) {
                    *c += ev * (float)cfgPoseFixGain / 100.0f;
                    float mx = (float)cfgPoseFixMax * 0.0174532925f;
                    if (*c > mx) *c = mx; if (*c < -mx) *c = -mx;
                    if (fabs_(*c) >= mx * 0.95f && evA > 0.07f) { if (!sat) g_pfSatN++; sat = 1; } else sat = 0;
                }
                g_pfFrames++; g_pfEvSum += evA * 57.29578f;
            }
        }
    }
    if (!integ) armed = 0;
    phiWas = phi; psiWas = psi; atLimWas = g_pfAtLim;
    if (!model || (wv && g_pfDead[w])) { *c *= 0.9f; if (fabs_(*c) < 0.0005f) *c = 0; sat = 0; if (state == 3) state = 4; }
    g_pfState = state; g_pfC10 = (int)(*c * 572.9578f); g_pfBone = model ? kPfBone[g_pfIdx[w] - 1] : -1;
    g_pfAtLim = 0;
    if (*c != 0.0f && haveS) {
        float phi2 = phi + *c;
        if (phi2 > 1.40f) phi2 = 1.40f; if (phi2 < -1.40f) phi2 = -1.40f;
        float sn = sin_small(phi2), cs = sqrt_(1.0f - sn * sn);
        if (cs < 0.05f) return;
        g_pfAim[0] = g_aimPt[0]; g_pfAim[1] = S[1] + hS * sn / cs; g_pfAim[2] = g_aimPt[2];
        {   // at the game's limits? (its angle is measured from the hand, bone 14: down + at Snake+0x538 - PoseDown for the
            // plugin's call - and up at +0x53C). There the correction does not wind up further.
            float B[3];
            if (BonePos(pl, 14, B)) {
                float hb = sqrt_((g_pfAim[0] - B[0]) * (g_pfAim[0] - B[0]) + (g_pfAim[2] - B[2]) * (g_pfAim[2] - B[2]));
                float ang = atan2_(g_pfAim[1] - B[1], hb);   // + = up
                i16 l0 = *(i16*)(pl + 0x538), l3 = *(i16*)(pl + 0x53C);
                float dn = l0 > 0 ? (float)l0 * (6.2831853f / 65536.0f) : 1.40f, up = l3 < 0 ? -(float)l3 * (6.2831853f / 65536.0f) : 1.40f;
                float pd = (float)cfgPoseDown * 0.0174532925f;
                if (l0 > 0 && pd > dn) dn = pd;
                if (-ang >= dn - 0.01f) g_pfAtLim = 1; else if (ang >= up - 0.01f) g_pfAtLim = -1;
            }
        }
        g_pfUse = 1; g_pfActive++;
    }
}
// Turned about the shoulder, the hand itself rises or drops with the turn, so for a near point the barrel ends up
// pointing past it - up when looking up, down when looking down, more the nearer the spot. Here the turn is worked out
// the other way round. The barrel line runs some distance c above (or below) the shoulder, and turning about the
// shoulder never changes that distance; the turn that lays the line through the spot is q = (the spot's angle seen from
// the shoulder) - asin(c / the spot's distance from the shoulder), and the point handed to the game is raised or
// lowered so that the game, measuring from the hand, arrives at exactly q. c is the line through his hand (as the
// aiming motion holds it this frame, before the turn) plus how far the barrel runs above the hand (k: PoseAimBarrel
// until his shots have measured it - the game's bullet matrix is the gun's own, and c does not change with the turn, so
// each shot gives c, and c less the hand's line gives k), per weapon and stance (standing / crouched; a stance with no
// shots yet takes the other one's). So is the barrel's own slant in the aiming motion; with b the turn is q = angle - b
// - asin(c / distance). Lying down the game turns the arms by half the angle and other bones too - left as it was. The
// sideways turn and the shots are not touched.
typedef struct { float c, v, b, bv; i32 n, bn, pad[2]; } PaW;   // c: the barrel above the hand's line (units) and its scatter (squared), shots;
                                                                      // b: the barrel's slant (radians, + = up) and its scatter,
                                                                      // shots
static PaW g_pa[PF_NW][2];

static volatile int g_paState, g_paC, g_paQ10, g_paG10, g_paTurn10 = -9999;   // for the rows: 0 off, 1 c not measured yet, 2 measured; c; turn asked / the game's own / done (tenths of a degree)
static volatile u32 g_paFrames, g_paLearnN, g_paShotN[2], g_paSteadyN, g_paNoBone, g_paLimN; static volatile float g_paShotErr[2], g_paTurnErr, g_paLineErr, g_paTurnMax;
static float g_paCcur, g_paBcur; static float g_paP[3]; static volatile u32 g_paPFrame = 0x80000000u, g_paUseFrame = 0x80000000u;
static float g_paHpre[3], g_paPpre[3]; static volatile u32 g_paHFrame = 0x80000000u;   // his hand and shoulder before the turn (for the shots)
static float g_paTurnRad; static volatile u32 g_paTurnF = 0x80000000u;   // the turn the routine gave the arm, and the frame
// how much of the angle handed to it the routine really turns the arm, per stance (standing: all of it; lying it is
// half - crouched it is checked here too): measured while the view stands still, used once 40 frames agree; and the
// pivot check
static float g_paGain[2] = { 1.0f, 1.0f }; static int g_paGainN[2]; static volatile float g_paPivotErr, g_paPivotMove;
static float asin_(float s) { if (s > 0.999f) s = 0.999f; if (s < -0.999f) s = -0.999f; return atan2_(s, sqrt_(1.0f - s * s)); }
static float tan_(float a)   // |a| < 1.4
{
    float a2 = a * a;
    float sn = a * (1.0f - a2 / 6.0f * (1.0f - a2 / 20.0f * (1.0f - a2 / 42.0f * (1.0f - a2 / 72.0f))));
    float cs = 1.0f - a2 / 2.0f * (1.0f - a2 / 12.0f * (1.0f - a2 / 30.0f * (1.0f - a2 / 56.0f)));
    return cs > 0.01f ? sn / cs : sn / 0.01f;
}
static void PaIdle(void) { g_paState = 0; }
static int PoseAimW(int w) { return w > 0 && w < 32 && (((u32)cfgPoseAimMask >> w) & 1) && !Throwable(w); }
static int BoneQuat(u8* pl, int bone, float* q)   // a bone's rotation as it is now (model axes)
{
    u8* mc = *(u8**)(pl + 0x2B8); if (!Readable(mc, 0x60)) return 0;
    u8* qs = *(u8**)(mc + 0x50); int nb = *(int*)(mc + 0x14);
    if (bone >= nb || nb > 256 || !Readable(qs + 16 * bone, 16)) return 0;
    for (int i = 0; i < 4; i++) q[i] = ((const float*)(qs + 16 * bone))[i];
    float qq = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];
    return qq > 0.5f && qq < 2.0f;
}
static float PaSlant(int w, int st) { PaW* k = &g_pa[w][st]; PaW* o = &g_pa[w][st ^ 1]; return k->bn >= 2 ? k->b : o->bn >= 2 ? o->b : 0.0f; }
// the line through the hand bone along the barrel's slant b: how far it runs above the shoulder (in the upright plane
// along the horizontal direction dx, dz)
static float PaHandLine(const float* P, const float* H, float dx, float dz, float b)
{
    float sb = sin_small(b), cb = sqrt_(1.0f - sb * sb), hu = (H[0] - P[0]) * dx + (H[2] - P[2]) * dz;
    return cb * (H[1] - P[1]) - sb * hu;
}
// before the game's routine: the point to hand to it (A), or 0 = leave the point at the crosshair spot
static int PoseAim(u8* pl, float* A, float* P, float* H, float* qOut, float* q14)
{
    g_paState = 0;
    if (!cfgPoseAim || cfgConverge != 3 || !g_aimPtOk || g_proneLatch) return 0;
    int w = Wpn(pl); if (!PoseAimW(w)) return 0;
    if (!BonePos(pl, 10, P) || !BonePos(pl, 14, H)) { g_paNoBone++; return 0; }
    float T[3] = { g_aimPt[0], g_aimPt[1], g_aimPt[2] };
    float tx = T[0] - P[0], tz = T[2] - P[2], u = sqrt_(tx * tx + tz * tz), v = T[1] - P[1];
    float hx = T[0] - H[0], hz = T[2] - H[2], hA = sqrt_(hx * hx + hz * hz);
    if (u < 300.0f || hA < 200.0f) return 0;   // the spot right at his hands: as before
    int sg = g_crouchLatch ? 1 : 0;
    PaW* k = &g_pa[w][sg]; PaW* o = &g_pa[w][sg ^ 1];
    float bs = PaSlant(w, sg);   // the barrel's own slant (measured: about 0)
    float kk = k->n > 0 ? k->c : o->n > 0 ? o->c : (float)cfgPoseAimBarrel;
    float c = PaHandLine(P, H, tx / u, tz / u, bs) + kk;   // the barrel line above the shoulder
    for (int i = 0; i < 3; i++) { g_paHpre[i] = H[i]; g_paPpre[i] = P[i]; }
    g_paHFrame = g_frame;
    float d = sqrt_(u * u + v * v), s = c / d;
    if (s > 0.7f) s = 0.7f; if (s < -0.7f) s = -0.7f;
    float q = atan2_(v, u) - bs - asin_(s);   // + = up
    if (g_paGainN[sg] >= 40) q /= g_paGain[sg];   // the routine turns the arm by only part of its angle
    if (q > 1.3f) q = 1.3f; if (q < -1.3f) q = -1.3f;
    A[0] = T[0]; A[2] = T[2]; A[1] = H[1] + hA * tan_(q);   // seen from the hand at the same distance: angle q
    *qOut = q; g_paCcur = c; g_paBcur = bs;
    if (!BoneQuat(pl, 14, q14)) q14[3] = 9.0f;
    g_paState = k->n > 0 || o->n > 0 ? 2 : 1; g_paC = (int)c; g_paQ10 = (int)(q * 572.9578f); g_paG10 = (int)(atan2_(T[1] - H[1], hA) * 572.9578f);
    g_paFrames++; g_paUseFrame = g_frame;
    static int told; if (!told) { told = 1; char b[300]; wsprintfA(b, "gun aim: the arms' turn is worked out about his shoulder (bone 10) so the barrel line runs through the crosshair spot - weapon %d, barrel line %d units %s the shoulder, %d above his hand (%s)", w, (int)(c < 0 ? -c : c), c < 0 ? "below" : "above", (int)kk, k->n > 0 || o->n > 0 ? "measured from his shots" : "PoseAimBarrel until a shot measures it"); Log(b); }
    return 1;
}
// after the game's routine: his shoulder for the shots; how far the arm really turned (bone 14 before / after) against
// the turn asked for, and whether the barrel line now runs through the spot (both only while nothing moves: the game
// eases the arms toward the angle over a few frames)
static void PoseAimAfter(u8* pl, int solved, const float* P, const float* H, float q, const float* q14)
{
    float P2[3];
    if (g_proneLatch || !BonePos(pl, 10, P2)) { g_paTurn10 = -9999; return; }
    for (int i = 0; i < 3; i++) g_paP[i] = P2[i];
    g_paPFrame = g_frame;
    if (!solved || !f_QMat || q14[3] > 5.0f) { g_paTurn10 = -9999; return; }
    float qa[4];
    if (!BoneQuat(pl, 14, qa)) { g_paTurn10 = -9999; return; }
    __attribute__((aligned(16))) float m0[16], m1[16], qb[4] = { q14[0], q14[1], q14[2], q14[3] }, qc[4] = { qa[0], qa[1], qa[2], qa[3] };
    f_QMat(m0, qb); f_QMat(m1, qc);
    // the turn the routine put on the hand, applied to his forward axis (model z): row vectors, world = local x M
    float y[3] = { m0[2], m0[6], m0[10] }, f[3];
    for (int j = 0; j < 3; j++) f[j] = m1[j] * y[0] + m1[4 + j] * y[1] + m1[8 + j] * y[2];
    float turn = atan2_(f[1], sqrt_(f[0] * f[0] + f[2] * f[2]));   // + = up
    g_paTurn10 = (int)(turn * 572.9578f); g_paTurnRad = turn; g_paTurnF = g_frame;
    static float qWas; static int steady; static u32 lastF;
    if (g_frame != lastF + 1) steady = 0;
    lastF = g_frame;
    if (fabs_(q - qWas) < 0.0035f && !g_transLatch) { if (steady < 100000) steady++; } else steady = 0;
    qWas = q;
    {   // at the arms' limits the turn cannot be what was asked: not counted (up -80 at +0x53C; down PoseDown, at least 50)
        float up = -(float)*(i16*)(pl + 0x53C) * (6.2831853f / 65536.0f), dn = (float)(cfgPoseDown > 50 ? cfgPoseDown : 50) * 0.0174532925f;
        if (q > up - 0.02f || q < -dn + 0.02f) { g_paLimN++; steady = 0; }
    }
    if (steady >= 20 && fabs_(q) > 0.105f) {   // how much of the angle the arm turns (6 degrees or more asked)
        float r = turn / q; int sg = g_crouchLatch ? 1 : 0;
        if (r > 0.4f && r < 1.6f) {   // (q is the angle handed to the game: r is the routine's own share)
            g_paGain[sg] += (r - g_paGain[sg]) * 0.05f;
            if (g_paGainN[sg] < 100000) g_paGainN[sg]++;
            if (g_paGain[sg] < 0.4f) g_paGain[sg] = 0.4f; if (g_paGain[sg] > 1.6f) g_paGain[sg] = 1.6f;
            static int told[2]; if (g_paGainN[sg] == 40 && !told[sg]) { told[sg] = 1; char b[200], t1[16]; wsprintfA(b, "gun aim: %s, the game's routine turns the arm by %s%% of the angle handed to it%s", sg ? "crouched" : "standing",
                Tenths(t1, g_paGain[sg] * 100.0f), g_paGain[sg] > 0.95f && g_paGain[sg] < 1.05f ? " - as expected" : " - the angle handed to it is scaled to match"); Log(b); }
        }
    }
    {   // the pivot: turned about the shoulder, the hand keeps its distance from it
        float H2[3];
        if (steady >= 20 && BonePos(pl, 14, H2)) {
            float a0 = 0, a1 = 0, mv = 0;
            for (int i = 0; i < 3; i++) { a0 += (H[i] - P[i]) * (H[i] - P[i]); a1 += (H2[i] - P2[i]) * (H2[i] - P2[i]); mv += (H2[i] - H[i]) * (H2[i] - H[i]); }
            float dd = sqrt_(a1) - sqrt_(a0); if (dd < 0) dd = -dd;
            if (mv > 40.0f * 40.0f) { g_paPivotErr += (dd - g_paPivotErr) * 0.05f; g_paPivotMove += (sqrt_(mv) - g_paPivotMove) * 0.05f; }
        }
    }
    if (steady >= 20) {
        int sg = g_crouchLatch ? 1 : 0; float qw = q * (g_paGainN[sg] >= 40 ? g_paGain[sg] : 1.0f);   // the turn wanted
        float e = (turn - qw) * 57.29578f; if (e < 0) e = -e;
        float tx = g_aimPt[0] - P2[0], tz = g_aimPt[2] - P2[2], u = sqrt_(tx * tx + tz * tz), v = g_aimPt[1] - P2[1], d = sqrt_(u * u + v * v);
        float le = d > 300.0f ? (turn - (atan2_(v, u) - g_paBcur - asin_(g_paCcur / d))) * 57.29578f : 0;   // + = the line passes above the spot
        g_paTurnErr = g_paSteadyN ? g_paTurnErr + (e - g_paTurnErr) * 0.05f : e;
        g_paLineErr = g_paSteadyN ? g_paLineErr + (fabs_(le) - g_paLineErr) * 0.05f : fabs_(le);
        if (e > g_paTurnMax) g_paTurnMax = e;
        g_paSteadyN++;
        if (g_paSteadyN == 60 || g_paSteadyN == 3000) {
            char b[420], t1[16], t2[16], t3[16], t4[16];
            wsprintfA(b, "gun aim check: the game turned his arm %s deg (asked %s) - off by %s deg on average while the view stands still; the barrel line passes the spot %s deg off (seen from his shoulder);",
                Tenths(t1, turn * 57.29578f), Tenths(t2, qw * 57.29578f), Tenths(t3, g_paTurnErr), Tenths(t4, g_paLineErr));
            wsprintfA(b + slen(b), " the hand moved %d units with the turn and its distance from the shoulder changed %d (0 = it turns about the shoulder)", (int)g_paPivotMove, (int)g_paPivotErr);
            Log(b);
        }
    }
}
// at each of his shots in the shoulder view: how far the barrel line (the game's bullet matrix) runs above his shoulder
static void PaLearn(u8* pl, int w, const float* m)
{
    (void)pl;
    if (!PoseAimW(w) || g_proneLatch || (u32)(g_frame - g_paPFrame) > 3) return;
    static u32 lastF = 0xFFFFFFFFu;
    if (lastF == g_frame) return;   // the shotgun's pellets are one shot
    float b[3] = { m[8], m[9], m[10] }; if (!Norm3(b)) return;
    float hb = sqrt_(b[0] * b[0] + b[2] * b[2]); if (hb < 0.3f) return;
    float M[3] = { m[12], m[13], m[14] };
    float u = ((M[0] - g_paP[0]) * b[0] + (M[2] - g_paP[2]) * b[2]) / hb, v = M[1] - g_paP[1];
    float c = hb * v - b[1] * u;   // + = above the shoulder
    if (!(c > -600.0f && c < 600.0f) || !(u > -300.0f && u < 2000.0f) || (u32)(g_frame - g_paHFrame) > 2) return;
    lastF = g_frame;
    int st = g_crouchLatch ? 1 : 0; PaW* k = &g_pa[w][st];
    if ((u32)(g_frame - g_paTurnF) <= 1) {   // the barrel's own slant: its angle less the arm's turn
        float bb = atan2_(b[1], hb) - g_paTurnRad;
        if (bb > -0.35f && bb < 0.35f) {
            float ab = k->bn < 4 ? 1.0f / (float)(k->bn + 1) : 0.25f, db = bb - k->b;
            if (k->bn == 0) { k->b = bb; k->bv = 0; } else { k->b += db * ab; k->bv += (db * db - k->bv) * ab; }
            if (k->bn < 1000000) k->bn++;
        }
    }
    c -= PaHandLine(g_paPpre, g_paHpre, b[0] / hb, b[2] / hb, PaSlant(w, st));   // the barrel above the hand's line
    float a = k->n < 4 ? 1.0f / (float)(k->n + 1) : 0.25f, dv = c - k->c;
    float was = k->c; int n0 = k->n;
    if (k->n == 0) { k->c = c; k->v = 0; } else { k->c += dv * a; k->v += (dv * dv - k->v) * a; }
    if (k->n < 1000000) k->n++;
    g_paLearnN++;
    static int told; float mv = k->c - was;
    if (told < 24 && (n0 < 2 || mv > 25.0f || mv < -25.0f)) {
        told++; char b2[300];
        char t1[16], t2[16];
        wsprintfA(b2, "gun aim: weapon %d %s - his shot puts the barrel %d units %s his hand's line (now %d after %d shots, scatter %d); the barrel's own slant %s deg (scatter %s, %d shots)", w, st ? "crouched" : "standing",
            (int)(c < 0 ? -c : c), c < 0 ? "below" : "above", (int)k->c, k->n, (int)sqrt_(k->v), Tenths(t1, k->b * 57.29578f), Tenths(t2, sqrt_(k->bv) * 57.29578f), k->bn);
        Log(b2);
    }
}
static volatile u32 g_fpN, g_fpLat[2], g_fpLatN[2], g_fpLatFr[2], g_fpLatMax[2], g_fpNoShot[2];
static volatile DWORD g_fpT; static volatile u32 g_fpFrame; static volatile int g_fpPending, g_fpMoving, g_fpKind, g_fpW, g_ftLeft, g_ftN, g_ftIdx;
static const char* FireKind(int k) { return k == 1 ? "fire key" : k == 2 ? "left mouse" : "raised bit dropped (pad)"; }
static void FirePress(int kind)
{
    DWORD tn = GetTickCount();
    if (g_fpPending && tn - g_fpT < 150) return;   // the same press, seen another way
    int moving = g_aimLatch && (u32)(g_frame - g_moveFrame) <= 2;   // pushed along by the plugin this frame or the last two
    g_fpPending = 1; g_fpT = tn ? tn : 1; g_fpFrame = g_frame; g_fpMoving = moving; g_fpKind = kind; g_fpW = Wpn(g_pl); g_fpN++;
    if (cfgFireTrace && g_ftN < cfgFireTrace && (moving || g_ftN < 2)) {
        g_ftLeft = 24; g_ftN++; g_ftIdx = g_ftN;
        char b[220]; wsprintfA(b, "fire trace %d: %s while %s (weapon %d, walk %s) - one line a frame follows", g_ftIdx, FireKind(kind), moving ? "strafing" : "standing",
            g_fpW, g_band == 0 ? "full speed" : g_band == 1 ? "walk" : g_band == 2 ? "stalk" : "-");
        Log(b);
    }
}
static void FireShot(void)   // one of his shoulder-view bullets
{
    if (!g_fpPending) return;
    DWORD tn = GetTickCount(); u32 ms = tn - g_fpT, fr = g_frame - g_fpFrame;
    g_fpPending = 0;
    if (ms > 1000) return;
    int k = g_fpMoving ? 1 : 0;
    g_fpLat[k] += ms; g_fpLatFr[k] += fr; g_fpLatN[k]++; if (ms > g_fpLatMax[k]) g_fpLatMax[k] = ms;
    static int told; if (told < 24) { told++; char b[200]; wsprintfA(b, "fire press (%s) while %s: the shot came %u frame(s) / %u ms later (weapon %d)", FireKind(g_fpKind), k ? "strafing" : "standing", fr, ms, g_fpW); Log(b); }
}
static void FireTick(u8* pl)   // once a frame, from the camera update
{
    {   // a press of the fire button, whichever way it shows (see FirePress)
        static int fireW, latchW, mouseW;
        int okq; u32 btn;
        if (g_btnFrame == g_frame && pl == g_pl) { okq = g_btnOk; btn = g_btnNow; } else { okq = 0; btn = 0; }   // (AimMode=0: the mouse only)
        int latch = (btn & 0x8000u) != 0, fire = (btn & 0x100000u) != 0, mouse = g_fgCache && (GetAsyncKeyState(1) & 0x8000) != 0, ev = 0;
        if (g_aimLatch) { if (okq && fire && !fireW) ev = 1; else if (mouse && !mouseW) ev = 2; else if (okq && latchW && !latch && !(g_shotT && GetTickCount() - g_shotT < 250)) ev = 3; }
        if (g_aimLatch && (fire || mouse)) { DWORD tn = GetTickCount(); g_fireEvT = tn ? tn : 1; }   // (not the raised bit: it drops when the gun is lowered too)
        if (ev) FirePress(ev);
        fireW = fire; mouseW = mouse; if (okq) latchW = latch;
    }
    if (g_fpPending && GetTickCount() - g_fpT > 1000) {
        g_fpPending = 0;
        if (g_aimLatch) {   // (the raised bit drops when the gun is lowered, too)
            int k = g_fpMoving ? 1 : 0; g_fpNoShot[k]++;
            static int told; if (told < 12) { told++; char b[200]; wsprintfA(b, "fire press (%s) while %s: no shot within a second (weapon %d - empty, reloading, or not a shot)", FireKind(g_fpKind), k ? "strafing" : "standing", g_fpW); Log(b); }
        }
    }
    if (g_ftLeft <= 0 || !Readable(pl, 0x800)) return;
    g_ftLeft--;
    char b[400];
    wsprintfA(b, "fire trace %d +%u: btn=%08X held=%08X stick=%u game=%u stalk13B=%d walk src=%d band=%d fast=%d leg=%d L0=%X/%d L1=%X/%d req(f-%u)=%X/%d|%X/%d up=%d did=%d shots=%u",
        g_ftIdx, g_frame - g_fpFrame, *(u32*)(pl + 0x7E8), *(u32*)(pl + 0x7F4), (unsigned)*(u16*)(pl + 0x7E0), (unsigned)g_padRaw, Bit(0x13B) ? 1 : 0, (int)g_wcSrc, (int)g_band, (int)g_fast, (int)g_legSlot,
        *(u32*)(pl + 0x74), (int)*(i16*)(pl + 0x78), *(u32*)(pl + 0x9C), (int)*(i16*)(pl + 0xA0), g_frame - g_rqF, g_rqA0, g_rqM0, g_rqA1, g_rqM1, g_rqU, g_rqDid, g_shots);
    Log(b);
}
static void LogState(int aiming, float camYaw, const float* eye, const float* tgt)
{
    g_rowNow = 1;   // for the frame pacing line
    char b[2048];
    u8* pl = g_pl;
    int ok = Readable(pl, 0x800);
    u32 w0 = g_bits ? g_bits[0] : 0, w1 = g_bits ? g_bits[1] : 0, w6 = g_bits ? g_bits[6] : 0, w9 = g_bits ? g_bits[9] : 0;
    int cy = (int)(camYaw / 6.2831853f * 65536.0f) & 0xFFFF;
    float px = ok ? *(float*)(pl + 0x130) : 0, pz = ok ? *(float*)(pl + 0x138) : 0;
    float mx = px - l_px, mz = pz - l_pz;   // how Snake moved since the last line
    int md = l_have ? (int)__builtin_sqrtf(mx * mx + mz * mz) : 0;
    int my = (l_have && md > 20) ? ((int)(atan2_(mx, mz) / 6.2831853f * 65536.0f) & 0xFFFF) : 0xFFFFF;
    l_px = px; l_pz = pz; l_have = 1;
    g_phase[g_phaseN < 12 ? g_phaseN : 12] = 0;
    wsprintfA(b, "t=%u aim=%d raw=%d strafe=%d mode=%d bits0=%08X bits1=%08X bits6=%08X bits9=%08X camYaw=%04X moveYaw=%05X moveDist=%d  head=%04X rot=%04X in=%d mag=%u yaw=%04X fl=%08X now7E0=%u now7E8=%08X push=%d,%d v620=%d,%d d140=%d,%d pos=%d,%d,%d act=%X seen=%X leg=%d legSets=%u tb=%d prone=%d rearm=%u elev=%d/%d/%d corr=%d spine=%u aimw=%d lim=%04X,%04X,%04X,%04X wpn=%d zoom=%d amt=%d L0=%X/%d/%X%08X L1=%X/%d/%X%08X order=%s calls=%u hid=%u pw=%u tick=%d  eye=%d,%d,%d tgt=%d,%d,%d",
        GetTickCount(), aiming, (int)g_rawAim, Strafe(), MoveMode(), w0, w1, w6, w9, cy, my, md,
        ok ? *(u16*)(pl + 0x16A) : 0, ok ? *(u16*)(pl + 0x162) : 0,
        (int)g_have, g_inMag, g_inYaw, g_inFl,
        ok ? *(u16*)(pl + 0x7E0) : 0, ok ? *(u32*)(pl + 0x7E8) : 0,
        (int)g_vx, (int)g_vz,
        ok ? (int)*(float*)(pl + 0x620) : 0, ok ? (int)*(float*)(pl + 0x628) : 0,
        ok ? (int)*(float*)(pl + 0x140) : 0, ok ? (int)*(float*)(pl + 0x148) : 0,
        (int)px, ok ? (int)*(float*)(pl + 0x134) : 0, (int)pz,
        g_actRva, g_aimSeen, (int)g_legSlot, g_legSets, (int)g_tb, (int)g_proneLatch, g_rearms, (int)g_ex, (int)g_emin, (int)g_emax, (int)g_corr, g_spineSets,
        ok ? (int)*(float*)(pl + 0x51C) : 0, ok ? *(u16*)(pl + 0x538) : 0, ok ? *(u16*)(pl + 0x53A) : 0, ok ? *(u16*)(pl + 0x53C) : 0, ok ? *(u16*)(pl + 0x53E) : 0,
        Wpn(pl), (int)g_zoom100, (int)(g_amt * 100.0f),
        ok ? *(u32*)(pl + 0x74) : 0, ok ? (int)*(i16*)(pl + 0x78) : 0, ok ? *(u32*)(pl + 0x84) : 0, ok ? *(u32*)(pl + 0x80) : 0,
        ok ? *(u32*)(pl + 0x9C) : 0, ok ? (int)*(i16*)(pl + 0xA0) : 0, ok ? *(u32*)(pl + 0xAC) : 0, ok ? *(u32*)(pl + 0xA8) : 0,
        g_phase, g_calls, g_hidden, g_velWrites, p_tick ? *p_tick : -1,
        (int)eye[0], (int)eye[1], (int)eye[2], (int)tgt[0], (int)tgt[1], (int)tgt[2]);
    // second part (wsprintfA stops at 1024 characters)
    wsprintfA(b + slen(b), " conv=%d near=%d pan=%d ctr=%d land=%d fromgun=%d side=%d sight=%d,%d zmul=%d shots=%u/%u legdir=%d",
        g_aimPtOk ? (int)g_convDist : -1, (int)g_obsMm, (int)g_panDps, (int)g_ctrDist, (int)g_landDist, (int)g_tMuzzle, (int)g_sideClear,
        (int)(g_sx * 1000.0f), (int)(g_sy * 1000.0f), (int)g_zoomMul, g_shotsAimed, g_shots, (int)g_legDir);
    wsprintfA(b + slen(b), " hip=%d body=%d lock=%d sideK=%d backK=%d wall=%d fast=%d/%d band=%d rate=%d padraw=%u cw=%u/%u/%u hid=%d inj=%d hold=%d flights=%u throw=%d proneback=%d",
        (int)g_hip100, (int)g_body100, (int)g_lock, (int)g_sideK100, (int)g_backK100, (int)g_wallFrames, (int)g_fast, (int)g_fastLeg, (int)g_band, (int)g_rate100,
        (unsigned)g_padRaw, g_cwSwap, g_cwRaise, g_cwLower, (int)g_hidOn, (int)g_injDown, (int)g_hold, g_snaps, (int)g_throwClamp, (int)g_proneBack100);
    {   int w = Wpn(pl), st = g_proneLatch ? 2 : g_crouchLatch ? 1 : 0;
        (void)w;
        wsprintfA(b + slen(b), " rec=%u/%u/%u kick=%d,%d spread=%u emu=%u", g_rkN, g_rkFpv, g_rkOts, (int)g_kP, (int)g_kY, g_rkSpread, g_rkEmu);
        wsprintfA(b + slen(b), " hipb=%d blends=%u smooth=%d/%d head=%d/%d,%d,%d use=%d learned=%d,%d,%d/%d headskip=%u spd=%u/%u stick=%d,%d/%d kbwalk=%u paddir=%u walksrc=%u/%u/%u/%u lookfix=%u camblock=%u crun=%u smag=%d dpad=%X walk=%d noise=%u headhold=%u wallpush=%d camhead=%d cw=%d/%d/%d/%u/%u/%u steps=%u fpvyaw=%u",
            (int)g_hipB100, g_hipBlends, (int)g_smooth1000, (int)g_rigid1000, g_headOk ? 1 : -(int)g_headWhy, (int)g_hF, (int)g_hR, (int)g_hU, (int)g_headUse100,
            (int)g_hdF, (int)g_hdR, (int)g_hdU, (int)g_headLearned + (int)g_headFromAim, g_headSkips, g_speedSw, g_speedHeld,
            (int)g_lx, (int)g_ly, StickActive(), g_kbWalkN[1] + g_kbWalkN[2], g_padDirN, g_walkSrc[0], g_walkSrc[1], g_walkSrc[2], g_walkSrc[3], g_padLookFix, g_camBlockN, g_crunN, g_stickMag, (unsigned)(g_padBtn & 0xF), g_lastWalk, g_noiseN, g_shotHold, g_wallPush, g_camHead, (int)g_cwOn, (int)g_cwMoving, (int)g_cwSlow, g_cwFrames, g_cwSwaps, g_cwCamoN, g_seqSwaps, g_fpvYawN);
        wsprintfA(b + slen(b), " fly=%d mz=%d btn=%d%d", (int)g_fly, (w > 0 && w < 64) ? (g_moOk[w][st] ? (int)g_moSrc[w][st] : 0) : -1,
            (GetAsyncKeyState(1) & 0x8000) ? 1 : 0, (GetAsyncKeyState(2) & 0x8000) ? 1 : 0);
        wsprintfA(b + slen(b), " ev=%d", g_eventWhy);   // event scene (1 / 2 / 3 = why, 0 = play)
        // gun pose (vertical error of the barrel in tenths of a degree, + = it must go up / correction / bone / state:
        // 0 nothing learned, 1 learning, 2 correcting, 3 held still, 4 switched off, 5 PoseFix=0), crosshair hidden (1
        // crawling, 2 laser sight), camera held in by a surface only it sees (fence nets), the camera following his
        // head while firing
        wsprintfA(b + slen(b), " pf=%d/%d/%d/%d sighthide=%d camonly=%u headfollow=%u", (int)g_pfEv10, (int)g_pfC10, (int)g_pfBone, (int)g_pfState, (int)g_sightHide, g_camOnlyN, g_headDragN);
        // gun aim (state 0 off / 1 barrel line not measured yet / 2 measured, the line above the shoulder, turn asked /
        // the game's own angle / turn done, tenths of a degree), walk changes not taken (too short), fire presses
        wsprintfA(b + slen(b), " pa=%d/%d/%d/%d/%d wdb=%u fp=%u", (int)g_paState, (int)g_paC, (int)g_paQ10, (int)g_paG10, (int)g_paTurn10, g_wdbN, g_fpN); }
    Log(b);
}

// XInput pad, first controller only. Button values: 256 LB, 512 RB, 4096 A, 8192 B, 16384 X, 32768 Y, 65536 left
// trigger, 131072 right trigger; 0 = no pad button.
typedef struct { u32 packet; u16 buttons; u8 lt, rt; i16 lx, ly, rx, ry; } XSTATE_;
// XInputGetState can take milliseconds (known to be slow on some systems); the pad is now read on a thread of the
// plugin's own, about 250 times a second, and the camera code only looks at the last reading (PadHeld).
static volatile int g_padHeldNow, g_fgCache; static HANDLE volatile g_fgWnd; static volatile u64 g_padReadT, g_padReadN;
static int PadRead(DWORD now);
static int PadHeld(DWORD now) { (void)now; return g_padHeldNow; }
static void InputPoll(void)
{
    DWORD now = GetTickCount();
    LARGE_ a0, a1; QueryPerformanceCounter(&a0);
    g_padHeldNow = PadRead(now);
    QueryPerformanceCounter(&a1); g_padReadT += (u64)(a1.q - a0.q); g_padReadN++;
    HANDLE fw = GetForegroundWindow(); DWORD pid = 0;
    GetWindowThreadProcessId(fw, &pid);
    int fg = pid == GetCurrentProcessId();
    // windows of the game's process that are not the game: console windows, dialogs, the mod's own
    // overlay. Only the game's own window counts as "the game in front" - for the crosshair and for the aim key.
    if (fg) {
        static HANDLE seen = (HANDLE)(i64)-1; static int seenOk;
        if (fw != seen) {
            char cn[64]; cn[0] = 0; GetClassNameA(fw, cn, 64);
            RECT_ rc; int big = GetClientRect(fw, &rc) && rc.r - rc.l >= 320 && rc.b - rc.t >= 240;
            int other = !strcmp_(cn, "ConsoleWindowClass") || !strcmp_(cn, "mgs3danimSight") || !strcmp_(cn, "#32770") || !big;
            seen = fw; seenOk = !other;
            static int told; if (told < 6) { told++; char b[200]; wsprintfA(b, "window in front: \"%s\" %dx%d - %s", cn, big ? rc.r - rc.l : 0, big ? rc.b - rc.t : 0,
                other ? "not the game (no crosshair, aim key ignored)" : "the game"); Log(b); }
        }
        if (!seenOk) fg = 0;
    }
    if (fg) g_fgWnd = fw;
    g_fgCache = fg;
}
static DWORD API InputThread(void* arg)
{
    (void)arg;
    for (;;) { InputPoll(); Sleep(4); }
    return 0;
}
static int PadRead(DWORD now)
{
    static u32 (API *get)(u32, XSTATE_*); static int tried; static DWORD retry;
    if (!tried) {
        tried = 1;
        HANDLE h = LoadLibraryA("xinput1_4.dll"); if (!h) h = LoadLibraryA("xinput1_3.dll"); if (!h) h = LoadLibraryA("xinput9_1_0.dll");
        if (h) get = (u32 (API *)(u32, XSTATE_*))GetProcAddress(h, "XInputGetState");
        Log(get ? "pad: XInput ready" : "pad: XInput not available");
    }
    if (!get || (retry && now - retry < 2000)) return 0;
    XSTATE_ st; memset(&st, 0, sizeof st);
    if (get(0, &st) != 0) { retry = now ? now : 1; g_padConn = 0; g_padAny = 0; return 0; }   // no pad: ask again in two seconds
    retry = 0; g_padConn = 1;
    g_lx = st.lx; g_ly = st.ly; g_stickT = now ? now : 1;   // the stick's own direction for walking
    g_padBtn = st.buttons; g_ry = st.ry;   // the d-pad (stalk) and the right stick (look)
    u32 b = st.buttons | (st.lt > 100 ? 0x10000u : 0) | (st.rt > 100 ? 0x20000u : 0);
    g_padAny = b != 0;
    return cfgPad && (b & (u32)cfgPad) != 0;   // the stick is read even without a shoulder-view button
}
// the pad's left stick is being used (outside its dead zone, read in the last half second)
static int StickActive(void)
{
    if (!g_padConn || !g_stickT || GetTickCount() - g_stickT > 500) return 0;
    int x = g_lx, y = g_ly;
    return x * x + y * y > 3000 * 3000;
}
// how far the stick is pushed, 0..255 (255 = all the way, the XInput dead zone counted as 0)
static int StickMag(void)
{
    float x = (float)g_lx, y = (float)g_ly, r = __builtin_sqrtf(x * x + y * y);
    float t = (r - 3000.0f) / (31000.0f - 3000.0f);
    if (t < 0) t = 0; if (t > 1) t = 1;
    return (int)(t * 255.0f + 0.5f);
}
// how far the stick is pushed, in XInput units (0 .. 32767)
static float StickR(void) { float x = (float)g_lx, y = (float)g_ly; return __builtin_sqrtf(x * x + y * y); }
// the stick's direction as a world heading (the camera's heading plus the stick's angle; left = +0x4000)
static u16 StickYaw(u16 camYaw)
{
    float a = atan2_(-(float)g_lx, (float)g_ly);
    return (u16)(camYaw + (u16)(i16)(int)(a * (32768.0f / 3.14159265f)));
}
// with the keyboard the walk is chosen by the keys, not by the stick value the game makes of them: the game gives odd
// values, so straight lines got the stalk speed and diagonals the walk speed. Keyboard: no key = full speed (0), walk
// toggle on = walk (1), stalk key (Shift) held = stalk (2) - the same in every direction. -1 = not the keyboard (the
// pad stick is in use).
static int KeyWalk(u16 mag)
{
    if (!cfgKeyWalk || !mag || StickActive()) return -1;
    int r = (cfgStalkKey && (GetAsyncKeyState(cfgStalkKey) & 0x8000)) ? 2 : mag >= 250 ? 0 : 1;
    g_kbWalkN[r]++; g_wcSrc = r == 2 ? 5 : 0;
    return r;
}
// which walk is asked for, from any device: 0 = full speed (crouched: the upright fast crouch walk, "crouchnormal"), 1
// = walk (crouched: the low squat walk, "crouchwalk"), 2 = stalk ("crouchstalk"), -1 = none.
// - keyboard: the direction keys = 0, walk toggle (Ctrl) on = 1, stalk key (Shift) held = 2;
// - pad: the d-pad = 2 (in every direction, also diagonals - as the game's own standing stalk); the stick = 1 when
//   pushed part way, 0 when pushed past FastAbove. The stick never gives the stalk.
static int RealPush(u16 mag, u32 fl)
{
    if (mag >= 16 || (fl & DIRBITS) || StickActive()) return 1;
    if (g_padConn && g_stickT && GetTickCount() - g_stickT < 500 && (g_padBtn & 0xF)) return 1;
    static int told; if (!told) { told = 1; char b[200]; wsprintfA(b, "stick value %u from the game with no direction and the pad's stick at rest: ignored", (unsigned)mag); Log(b); }
    return 0;
}
static int WalkChoice(u16 mag)
{
    if (!mag) return -1;
    if (g_gStalkT && (u32)(g_frame - g_gStalkT) <= 2) { g_walkSrc[2]++; g_wcSrc = 4; return g_lastWalk = 2; }   // the game's stalk input
    int dpad = cfgDpadStalk && g_padConn && g_stickT && GetTickCount() - g_stickT < 500 && (g_padBtn & 0xF);
    if (dpad) {
        g_walkSrc[2]++; g_wcSrc = 2;
        static int told; if (!told) { told = 1; Log("pad: d-pad walking = stalk (crouched: crouchstalk); the stick gives the walk and the full speed, never the stalk"); }
        return g_lastWalk = 2;
    }
    if (!StickActive() && mag < 16) return -1;   // noise (see RealPush)
    if (StickActive()) {
        // the stick's own push (XInput), not the value the game made of it: that one lagged and now and then read 0
        // while he was moving, so the same push gave the fast walk one moment and the slow one the next
        int sm = StickMag(); g_stickMag = sm;
        g_walkSrc[1]++; g_wcSrc = 1; return g_lastWalk = (sm >= (g_lastWalk == 0 ? cfgFastAbove - 25 : cfgFastAbove) ? 0 : 1);
    }
    int kw = KeyWalk(mag);
    if (kw >= 0) { g_walkSrc[0]++; return g_lastWalk = kw; }
    g_walkSrc[3]++; g_wcSrc = 3;
    return g_lastWalk = (mag >= cfgFastAbove ? 0 : 1);
}
// press / release any key or mouse button (the learned first-person key)
static void KeyEvent(int k, int down)
{
    INPUT_ in; memset(&in, 0, sizeof in);
    if (k == 1 || k == 2 || k == 4) { in.type = 0; in.u.mi.flags = k == 1 ? (down ? 0x2u : 0x4u) : k == 2 ? (down ? 0x8u : 0x10u) : (down ? 0x20u : 0x40u); }
    else if (k == 5 || k == 6) { in.type = 0; in.u.mi.flags = down ? 0x80u : 0x100u; in.u.mi.data = (u32)(k - 4); }
    else { in.type = 1; in.u.ki.vk = (u16)k; in.u.ki.scan = (u16)MapVirtualKeyA((u32)k, 0); in.u.ki.flags = down ? 0u : 0x2u; }
    SendInput(1, &in, (int)sizeof in);
}
static volatile int g_fkDown; static volatile DWORD g_fkT;   // the first-person key the plugin pressed
// keys held right now (not the generic shift / ctrl / alt codes); returns how many, fills up to 8
static int HeldKeys(int* out)
{
    int n = 0;
    for (int vk = 1; vk < 255; vk++) {
        if (vk == 0x10 || vk == 0x11 || vk == 0x12) continue;
        if (GetAsyncKeyState(vk) & 0x8000) { if (n < 8) out[n] = vk; n++; }
    }
    return n;
}
static void AimKeyEvent(int down)   // press or release the aim key for the player
{
    INPUT_ in; memset(&in, 0, sizeof in);
    int k = cfgAimKey;
    if (k == 1 || k == 2 || k == 4) {
        in.type = 0;
        in.u.mi.flags = k == 1 ? (down ? 0x2u : 0x4u) : k == 2 ? (down ? 0x8u : 0x10u) : (down ? 0x20u : 0x40u);
    } else {
        in.type = 1; in.u.ki.vk = (u16)k; in.u.ki.scan = (u16)MapVirtualKeyA((u32)k, 0);
        in.u.ki.flags = down ? 0u : 0x2u;
    }
    SendInput(1, &in, (int)sizeof in);
}
static void AimDiag(char* b, u8* pl)
{
    u8* pw = Readable(pl, 0x800) ? *(u8**)(pl + 0x7F8) : 0;
    int cp = Readable(pw, 0x2C) && pw != (u8*)p_gpad;
    wsprintfA(b + slen(b), " [input word %08X, pad block %08X, Snake reads %08X (state %d/%d), held-over mask %08X, buttons %08X, status 78/85/138 %d/%d/%d, pad %s %04X, game in front %d]",
        p_held && Readable((const u8*)p_held, 4) ? *p_held : 0, p_gpad && Readable((const u8*)p_gpad, 4) ? *p_gpad : 0,
        Readable(pw, 4) ? *(u32*)pw : 0, cp ? *(int*)(pw + 0x24) : -1, cp ? *(int*)(pw + 0x28) : -1,
        Readable(pl, 0x800) ? *(u32*)(pl + 0x7F4) : 0, Readable(pl, 0x800) ? *(u32*)(pl + 0x7E8) : 0,
        Bit(0x78), Bit(0x85), Bit(0x138), g_padConn ? "connected" : "none", (unsigned)g_padBtn, g_fgNow);
}
// the game called first person off while the camera was flying to his head (it never started): fly back to the shoulder
// from the share of the way reached (g_fly3K, the last one used), taking as long as the way there took
static void FlyBackCancelled(DWORD nowT, DWORD holdT)
{
    if (g_fly != 1) return;
    DWORD d = nowT - g_flyT; if (d < 80) d = 80; if (d > 400) d = 400;
    g_fly = 3; g_flyT = nowT ? nowT : 1; g_flyDur = d; g_fpCancelN++;
    char b[220]; wsprintfA(b, "first person called off by the game after %d ms (it never started): the camera flies back to the shoulder from %d%% of the way to his head, over %d ms",
        (int)(nowT - holdT), (int)(g_fly3K * 100.0f), (int)d); Log(b);
}
// how high Snake's hips are in the motion his body is playing, in the motion's own units (8.0 standing,
// 4.2 crouched, 5.1 in the low crouch walk, 7.0 in the fast crouch walk, 1.4 lying; one unit is about 126 game units).
// Every motion carries this as a small track of half-floats (the same track that moves him along while walking).
// Returns -1 .. -4 when it cannot.
static float Half(u16 h)
{
    u32 sg = (u32)(h & 0x8000u) << 16, e = (h >> 10) & 0x1Fu, m = h & 0x3FFu, f;
    if (e == 0) f = sg;   // (the game treats tiny values as 0 as well)
    else if (e == 31) f = sg | 0x7F800000u | (m << 13);
    else f = sg | ((e + 112u) << 23) | (m << 13);
    float r; memcpy(&r, &f, 4); return r;
}
static u64 RegionLeft(const u8* p)
{
    MBI m; if (!p || !VirtualQuery(p, &m, sizeof m)) return 0;
    if (m.State != 0x1000 || (m.Protect & 0x101)) return 0;
    return (u64)((const u8*)m.BaseAddress + m.RegionSize - p);
}
static const u8* g_hipHdr; static int g_hipN; static u16 g_hipT[512]; static float g_hipY[512];
static float BodyHipY(u8* pl)
{
    u8* ly = Layer0(pl); if (!ly) return -1.0f;
    const u8* hdr = *(const u8**)(ly + 0x10);
    if (!Readable(hdr, 0x40)) return -2.0f;
    u32 frames = *(u32*)(hdr + 0x0C), step = *(u32*)(hdr + 0x08);
    if (hdr != g_hipHdr) {
        g_hipHdr = hdr; g_hipN = 0;
        const u8* data = *(const u8**)(ly + 0x18);
        if (!Readable(data, 2)) data = hdr + *(u32*)(hdr + 0x10);
        if ((*(u32*)(hdr + 4) & 0x20) && frames > 0 && frames < 100000) {
            const u8* tr = data + 2 * (i64)*(i32*)(hdr + 0x38);
            // what is readable after it, without a system call when there is plenty (16 KB covers 512 keys)
            u64 words = (cfgFastMem && Readable(tr, 16384)) ? 8192 : RegionLeft(tr) / 2;
            if (words >= 4) {
#define HW(k) ((u64)((k) < words ? *(const u16*)(tr + 2 * (k)) : 0))
                g_hipT[0] = 0; g_hipY[0] = Half((u16)HW(1)); g_hipN = 1;
                u64 bit = 48; u32 t = 0;
                while (g_hipN < 512 && t < frames) {
                    u64 k = bit >> 4; if (k + 8 >= words) break;
                    u32 sh = (u32)(bit & 15);
                    #define GETB(n) ((u32)(((HW(bit >> 4) | HW((bit >> 4) + 1) << 16 | HW((bit >> 4) + 2) << 32) >> (bit & 15)) & ((1u << (n)) - 1)))
                    (void)sh;
                    u32 du = GETB(8); bit += 8;
                    bit += 16;   // x
                    u32 yv = GETB(16); bit += 16;
                    bit += 16;   // z
                    if (!du) break;
                    t += du; g_hipT[g_hipN] = (u16)(t > 65535 ? 65535 : t); g_hipY[g_hipN] = Half((u16)yv); g_hipN++;
                    #undef GETB
                }
#undef HW
            }
        }
    }
    if (g_hipN <= 0) return -3.0f;
    if (!step) return g_hipY[0];
    float f = *(float*)(ly + 0x48) / (float)step;
    if (!(f >= 0.0f)) f = 0.0f;
    if (frames) { int guard = 0; while (f >= (float)frames && guard++ < 64) f -= (float)frames; }
    int i = 0; while (i + 1 < g_hipN && (float)g_hipT[i + 1] <= f) i++;
    float y = g_hipY[i];
    if (i + 1 < g_hipN) { float t0 = g_hipT[i], t1 = g_hipT[i + 1]; if (t1 > t0) y += (g_hipY[i + 1] - y) * (f - t0) / (t1 - t0); }
    return (y > 0.3f && y < 14.0f) ? y : -4.0f;
}
// the hip height the game itself shows. When the motion changes the game blends the old pose into the new one over a
// few frames. Snake's motion controller (Snake+0x2B8) keeps that blend: +0x18 bit 0x4000 = blending, +0x1C frames left,
// +0x20 weight 0..1, +0x70 hip height when the blend started, +0x74 the blended hip height (exe+74FF39; the pose itself
// is blended with the same weight, exe+750BDA). The game's value is used once it has matched the plugin's own reading
// (while no blend runs); otherwise the plugin blends between the motions itself (linearly over Blend frames).
static float HipBlended(u8* pl, float hy)
{
    static float from, last = -1.0f; static const u8* hdrWas; static int n, N, act, good, bad, told, toldBad, valid; static u32 lastFrame;
    if (g_frame - lastFrame > 2) { hdrWas = g_hipHdr; act = 0; last = -1.0f; }   // not called for a while (shoulder view was off): start fresh
    lastFrame = g_frame;
    u8* mc = Readable(pl, 0x800) ? *(u8**)(pl + 0x2B8) : 0;
    int okm = Readable(mc, 0x80);
    u32 fl = okm ? *(u32*)(mc + 0x18) : 0;
    int blending = okm && (fl & 0x4000);
    float g = okm ? *(float*)(mc + 0x74) : -1.0f;
    g_hipG100 = (g > -100.0f && g < 100.0f) ? (int)(g * 100.0f) : -9999;
    if (hy > 0 && okm && !blending) {
        float d = g - hy; if (d < 0) d = -d;
        if (d < 0.06f) { if (good < 1000000) good++; bad = 0; }
        else if (d > 0.25f) { if (bad < 1000000) bad++; }
    }
    int was = valid;
    valid = good >= 30 && bad < 30;
    if (valid && !was && !told) { told = 1; char b[200]; wsprintfA(b, "camera body lock: the game's own blended hip height matches the motion data (%d frames) - the camera now follows Snake's pose changes exactly", good); Log(b); }
    if (!valid && was && !toldBad) { toldBad = 1; Log("camera body lock: the game's blended hip height stopped matching - the plugin blends between motions itself"); }
    // the plugin's own blend (used when the game's value is not confirmed)
    if (g_hipHdr != hdrWas) {
        if (hdrWas && last > 0 && hy > 0) { from = last; n = 0; N = cfgBlend > 0 ? cfgBlend : 1; act = 1; g_hipBlends++; }
        hdrWas = g_hipHdr;
    }
    float out = hy;
    if (valid && g > 0.3f && g < 14.0f) { out = g; g_hipSrc = 1; act = 0; }
    else if (hy > 0) {
        g_hipSrc = 2;
        if (act) { n++; float t = (float)n / (float)N; if (t >= 1.0f) { t = 1.0f; act = 0; } out = from + (hy - from) * t; }
    } else g_hipSrc = 0;
    last = out;
    return out;
}
static u32 g_surfSeen[24]; static int g_surfN;
static void SurfaceNote(u32 fl, int skipped)
{
    if (!c_flag) return;
    u32 key = fl ^ (skipped ? 0x80000000u : 0);
    for (int i = 0; i < g_surfN; i++) if (g_surfSeen[i] == key) return;
    if (g_surfN >= 24) return;
    g_surfSeen[g_surfN++] = key;
    char b[240];
    wsprintfA(b, "camera collision: surface kind %08X %s", fl, skipped ? "let through (flag 800000 - the game's own camera lets it through too)"
        : (fl & 0x42u) ? "stops the camera (the shots' test sees it too - as before)" : "stops the camera - the shots' test does not see it (a see-through surface such as a fence net; the game's own camera stops at it too)");
    Log(b);
}
// is there something between two points that the camera may not pass through? The same question the game's own third-
// person camera asks (exe+2139B4: a ball swept from Snake to the camera; kind 0xF / 0x1F and flags 0x840050 / 0x840042
// depending on the camera's mode at work+0x60; things flagged 0x800000 do not count). Returns how far along the way the
// first one is, or -1 for a clear way.
static float CamEase(float k, float t, float hard, int* outN)
{
    if (t < k - 0.004f) {   // something in the way: in, over a few frames
        *outN = 0;
        float a = (float)cfgCamIn / 100.0f; if (k - t > 0.3f && a < 0.65f) a = 0.65f;   // a big jump: faster
        k += (t - k) * a; if (k - t < 0.003f) k = t;
    } else if (t > k + (t >= 0.999f ? 0.0005f : 0.015f)) {   // clear: out only after a while, and slowly (all the way when fully clear)
        if (++*outN >= cfgCamHold) {
            float st = (t - k) * (float)cfgCamOut / 100.0f; if (st > 0.03f) st = 0.03f; if (st < 0.002f) st = 0.002f;
            k += st; if (k > t) k = t;
        }
    } else *outN = 0;   // about the same: stay
    if (k > hard) k = hard;   // never behind the surface itself (no seeing through)
    return k;
}
static float Sweep(int mode, const float* from, const float* to, float minY)
{
    if (!c_test || !c_areaArg) return -1.0f;
    __attribute__((aligned(16))) float a[4], b[4], h[4];
    float dir[3], len = 0;
    for (int i = 0; i < 3; i++) { a[i] = from[i]; b[i] = to[i]; dir[i] = to[i] - from[i]; len += dir[i] * dir[i]; }
    len = __builtin_sqrtf(len); if (!(len > 1.0f)) return -1.0f;
    for (int i = 0; i < 3; i++) dir[i] /= len;
    a[3] = b[3] = 1.0f;
    (void)mode;
    u8* plc = g_pl; if (!Readable(plc, 0x800)) return -1.0f;
    u32 area = c_area(*(u32*)(plc + 0x120), a, b);
    u32 skip = (u32)cfgCamSkip;
    if (!c_test(0x1Fu, area, CamFlags(), a, b, 0.0f)) return -1.0f;
    c_sort();
    int n = c_num(); float best = -1.0f; u32 bestF = 0;
    if (n < 0 || n > 64) return -1.0f;
    for (int i = 0; i < n; i++) {
        u32 fl = c_flag ? c_flag(i) : 0;
        if (skip && (fl & skip)) { g_camSkipN++; SurfaceNote(fl, 1); continue; }
        h[0] = h[1] = h[2] = h[3] = 0; c_point(i, h);
        if (h[1] < minY) continue;   // the ground under a lying Snake
        float d = (h[0] - a[0]) * dir[0] + (h[1] - a[1]) * dir[1] + (h[2] - a[2]) * dir[2];
        if (d < 0) d = 0;
        if (d > len + 100.0f) continue;
        if (best < 0 || d < best) { best = d; bestF = fl; }
    }
    if (best >= 0) { SurfaceNote(bestF, 0); if (c_flag && !(bestF & 0x42u)) g_camOnlyNow = 1; }
    return best;
}
// the game's camera button (the work's button mask at +0xC4, set from +0x7C when the camera is made, exe+1402123AD).
// Pressed, it turns the camera to where Snake faces (exe+140212C90); with the gun up it turns it toward the target the
// game's auto-aim picked (Snake+0xA60, exe+140212BF0), and held it keeps turning it there (exe+140212CE0). On the pad
// that button is LB - the shoulder-view button - so pressing LB swung the view to his facing (item 2) and, near
// enemies, kept it turned onto them so it could not be moved sideways (item 4). That is the game's own behaviour; with
// a pad in use the plugin takes the button away from the camera.
static void CamButton(u8* work)
{
    static u8* zw; static u32 saved;
    if (!Readable(work, 0x100)) return;
    int want = cfgBlockCamBtn && cfgPad && g_padConn;
    u32 cur = *(u32*)(work + 0xC4);
    if (want) {
        if (cur) {
            saved = cur; zw = work; *(u32*)(work + 0xC4) = 0; g_camBlockN++;
            static int told; if (!told) { told = 1; char b[200]; wsprintfA(b, "pad: the game's camera button (mask %X) no longer turns the camera to Snake's facing or onto enemies - that button is the shoulder-view button", cur); Log(b); }
        }
    } else if (zw == work && !cur && saved) { *(u32*)(work + 0xC4) = saved; zw = 0; }
}
// how fast Snake really moves, per stance, view and walk (units a second, measured from his position), for the log:
// shows whether crouchnormal is 30 % faster than crouchwalk and slower than running, and the stalk slowest
static double s_spd[2][2][3], s_spdT[2][2][3]; static u32 s_spdN[2][2][3]; static int s_spdR[2][2][3];
static void SpeedTrack(void)
{
    static float lx, lz; static int have; static LARGE_ lt, fq;
    u8* pl = g_pl; LARGE_ t; QueryPerformanceCounter(&t);
    if (!fq.q) QueryPerformanceFrequency(&fq);
    if (!Readable(pl, 0x800) || !fq.q) { have = 0; return; }
    float x = *(float*)(pl + 0x130), z = *(float*)(pl + 0x138);
    double dt = have ? (double)(t.q - lt.q) / (double)fq.q : 0;
    float dx = x - lx, dz = z - lz, d = __builtin_sqrtf(dx * dx + dz * dz);
    lx = x; lz = z; lt = t;
    int was = have; have = 1;
    if (!was || dt <= 0 || dt > 0.1 || d < 0.5f || d > 300.0f || Bit(ST_FPV) || Bit(3)) return;
    int crouch = Bit(ST_CROUCH) && !Bit(ST_STAND), stand = Bit(ST_STAND) && !Bit(ST_CROUCH);
    if (!crouch && !stand) return;
    int ots = g_aimLatch && g_amt > 0.5f ? 1 : 0, wk = g_lastWalk < 0 || g_lastWalk > 2 ? 1 : g_lastWalk;
    s_spd[ots][crouch][wk] += d; s_spdT[ots][crouch][wk] += dt; s_spdN[ots][crouch][wk]++; s_spdR[ots][crouch][wk] = g_rate100;
    static DWORD lastT; DWORD nw = GetTickCount();
    if (!lastT) lastT = nw;
    if (nw - lastT < 10000) return;
    lastT = nw;
    static const char* vn[2] = { "normal view", "shoulder view" }; static const char* sn[2] = { "standing", "crouched" };
    static const char* wn[2][3] = { { "full", "walk", "stalk" }, { "crouchnormal", "crouchwalk", "crouchstalk" } };
    char b[1100]; int n = 0, any = 0;
    n += wsprintfA(b + n, "speeds (units a second while moving; animation rate):");
    for (int v = 0; v < 2; v++) for (int c = 0; c < 2; c++) for (int k = 0; k < 3; k++) {
        if (s_spdN[v][c][k] >= 15 && s_spdT[v][c][k] > 0) {
            n += wsprintfA(b + n, " %s %s %s %d (%d)", vn[v], sn[c], wn[c][k], (int)(s_spd[v][c][k] / s_spdT[v][c][k]), s_spdR[v][c][k]);
            any = 1;
        }
        s_spd[v][c][k] = s_spdT[v][c][k] = 0; s_spdN[v][c][k] = 0;
    }
    if (any) Log(b);
}
// the pad's right stick moved up or down this moment (XInput, read by PadHeld)
static int PadLookOn(void) { return g_padConn && g_stickT && GetTickCount() - g_stickT < 500 && (g_ry > 7849 || g_ry < -7849); }
// the normal view: the game's elevation (work+0x340, smaller = camera lower = looking up) is moved by the stick at
// exe+140212AA0, the way round given by the game's camera setting (work+0x5C, from the settings at [exe+ACDE98]+0xB bit
// 0, exe+140211E95). With a pad the plugin makes stick up look up (PadStickUpLooksUp=1; 2 = stick up looks down; 0 =
// the game decides): a frame where the game moved it the other way is mirrored.
static float g_ePrev; static int g_ePrevOk;
static void PadLookNormal(u8* work)
{
    if (!cfgPadLookUp || !g_ePrevOk || g_amt > 0.001f || g_hold || g_fly || !PadLookOn() || !Readable(work, 0x400)) return;
    float e = *(float*)(work + 0x340), de = e - g_ePrev, emin = *(float*)(work + 0x98), emax = *(float*)(work + 0x9C);
    if (!(emax - emin > 0.05f && emax - emin < 100.0f) || de > 0.2f || de < -0.2f) return;
    float want = (g_ry > 0) == (cfgPadLookUp == 1) ? -1.0f : 1.0f;   // the way the elevation should go
    if (de * want > 0.00001f) { static int ok; if (!ok) { ok = 1; Log("pad: the game's camera already moves the way the stick is pushed (stick up looks up)"); } }
    if (de * want < -0.00001f) {
        float n = g_ePrev - de; if (n < emin) n = emin; if (n > emax) n = emax;
        *(float*)(work + 0x340) = n; g_padLookFix++;
        static int told; if (!told) { told = 1; char b[200]; wsprintfA(b, "pad: the game's camera moved the other way to the stick (its camera setting: vertical %s) - stick %s now looks up, in the normal view and the shoulder view", *(int*)(work + 0x5C) ? "reversed" : "normal", cfgPadLookUp == 1 ? "up" : "down"); Log(b); }
    }
}
// the shoulder view takes its heading from the game camera (its orbit, work+0x338, 16 bits, pointing from Snake to the
// camera). First person has a camera of its own, so after looking around in first person the game camera still pointed
// the old way, and the shoulder view came back facing where first person had started. Now, when the shoulder view comes
// on within 1.5 s of first person, the game camera is turned to first person's heading (as the game's own camera reset
// does: exe+140212CC4 - the orbit set, the turn at +0x33C cleared).
static u16 OrbOff(void)   // add to a view heading to get the camera number
{
    if (g_orbFixed >= 0) return (u16)g_orbFixed;
    return (g_orbV8 > g_orbV0 + 30) ? 0x8000 : 0;   // 0 unless the game's camera says otherwise
}
static void FpvHeading(u8* work, DWORD nowT)
{
    if (!cfgFpvYaw || !g_lastFpvT || nowT - g_lastFpvT > 1500 || !Readable(work, 0x400)) return;
    // only when the normal camera has not been turned since first person ended. Now the camera's own direction wins.
    if (g_fpvEndT && nowT - g_fpvEndT > 300 && !Bit(ST_FPV)) {
        int d = (i16)(u16)(*(u16*)(work + 0x338) - g_fpvEndOrb); if (d < 0) d = -d;
        if (d > 0x1C7) {   // 10 degrees
            g_fpvSkipN++;
            char b[200]; wsprintfA(b, "back from first person: the normal camera was turned %d degrees since then - the shoulder view looks where the camera looks (first person's direction not used)", d * 360 / 65536);
            Log(b); return;
        }
    }
    // where first person looked: its own camera's direction, checked against Snake's heading
    int haveD = 0; u16 dy = 0;
    if (g_fpOk && g_fpDT && nowT - g_fpDT < 1600) {   // read during this first person
        float h = __builtin_sqrtf(g_fpD[0] * g_fpD[0] + g_fpD[2] * g_fpD[2]);
        if (h >= 0.2f) { dy = (u16)(i32)(atan2_(g_fpD[0], g_fpD[2]) * (32768.0f / 3.14159265f)); haveD = 1; }
    }
    u16 view; const char* src;
    if (haveD && g_fpRotOk) {
        int df = (i16)(u16)(dy - g_fpRot); if (df < 0) df = -df;
        view = dy; src = df < 0x2000 ? "its camera, Snake's heading agrees" : "its camera (Snake's heading differs - lying down he turns less)";
    } else if (g_fpRotOk) { view = g_fpRot; src = "Snake's heading"; }
    else if (haveD) { view = dy; src = "its camera"; }
    else return;
    u16 off = OrbOff();
    u16 was = *(u16*)(work + 0x338);
    *(u16*)(work + 0x338) = (u16)(view + off); *(u32*)(work + 0x33C) = 0;
    g_fpvYawN++;
    g_fpvWant = view; g_fpvCheckFrame = g_frame + 3; g_fpvCheckT = nowT ? nowT : 1; g_fpvWork = work; g_fpvTurned = 0;
    int turn = (int)((float)(i16)(u16)(view + off - was) * 360.0f / 65536.0f);
    char b[260]; wsprintfA(b, "back from first person: the shoulder view is turned to where first person looked - heading %d degrees (from %s), camera turned %d degrees",
        Deg16(view), src, turn); Log(b);
}
// learn how the game's camera number relates to where its camera looks, and check the turn above a few frames later
static void OrbitWatch(u8* work, float camYaw, float hl)
{
    if (!Readable(work, 0x400)) return;
    u16 vy = (u16)(int)(camYaw / 6.2831853f * 65536.0f);
    {   // the camera number when first person ended (followed for 300 ms while the game hands its camera back)
        static int fpPrev; DWORD t = GetTickCount(); int fp = Bit(ST_FPV);
        if (fpPrev && !fp) { g_fpvEndT = t ? t : 1; g_fpvEndOrb = *(u16*)(work + 0x338); }
        else if (!fp && g_fpvEndT && t - g_fpvEndT <= 300) g_fpvEndOrb = *(u16*)(work + 0x338);
        if (fp) g_fpvEndT = 0;
        fpPrev = fp;
    }
    if (hl > 1.0f && !Bit(ST_FPV) && !g_fly && !g_hold) {
        int d = (i16)(u16)(vy - *(u16*)(work + 0x338));
        if (d > -0x800 && d < 0x800) { if (g_orbV0 < 1000000) g_orbV0++; }
        else if (d > 0x7800 || d < -0x7800) { if (g_orbV8 < 1000000) g_orbV8++; }
        static int told; if (!told && g_orbV0 + g_orbV8 >= 300) {
            told = 1; char b[200]; int rv = g_orbV8 > g_orbV0 + 30;
            wsprintfA(b, "camera: the game's camera number points %s its camera looks (%u frames say so, %u the other way)",
                rv ? "the opposite way to where" : "where", rv ? g_orbV8 : g_orbV0, rv ? g_orbV0 : g_orbV8); Log(b);
        }
    }
    if (g_fpvCheckT && (i32)(g_frame - g_fpvCheckFrame) >= 0) {
        DWORD nt = GetTickCount();
        if (work == g_fpvWork && hl > 1.0f && !Bit(ST_FPV)) {
            int df = (i16)(u16)(vy - g_fpvWant), ad = df < 0 ? -df : df, deg = ad * 360 / 65536;
            char b[260];
            if (deg <= 25) {
                g_fpvOk++; g_orbFixed = OrbOff();
                wsprintfA(b, "back from first person - CHECK OK: first person looked at %d degrees, the shoulder view looks at %d degrees (%d off)", Deg16(g_fpvWant), Deg16(vy), deg);
                Log(b); g_fpvCheckT = 0;
            } else if (deg >= 150 && !g_fpvTurned) {
                u16 now = (u16)(OrbOff() ^ 0x8000); g_orbFixed = now; g_fpvTurned = 1;
                *(u16*)(work + 0x338) = (u16)(*(u16*)(work + 0x338) + 0x8000);
                wsprintfA(b, "back from first person - CHECK WRONG: first person looked at %d degrees, the shoulder view came back at %d (%d off) - turned round; from now on the camera number is taken %s",
                    Deg16(g_fpvWant), Deg16(vy), deg, now ? "+180" : "as it is");
                Log(b); g_fpvBad++; g_fpvCheckFrame = g_frame + 3;
            } else {
                wsprintfA(b, "back from first person - CHECK: first person looked at %d degrees, the shoulder view looks at %d degrees (%d off%s)",
                    Deg16(g_fpvWant), Deg16(vy), deg, g_fpvTurned ? " after turning it round" : "");
                Log(b); if (deg > 25) g_fpvBad++; g_fpvCheckT = 0;
            }
        } else if (nt - g_fpvCheckT > 1500) { Log("back from first person - check not done (the camera was elsewhere)"); g_fpvCheckT = 0; }
    }
}
// the classic (overhead) camera. With it the shoulder view should not come on. No flag of the game's for it has been
// found yet, so it is told by what the camera does: the normal camera always looks the way its camera number
// (work+0x338) says; a camera that looks elsewhere for 30 frames in a row while the game has the camera to itself is
// taken as the classic one, until it agrees again for 30 frames. Every switch, and every time the shoulder view is
// turned on or refused, writes the camera's other fields to the log so the game's own flag can be found from them.
static volatile int g_classic; static volatile u32 g_classicN, g_classicBlockN, g_classicFrames;
// The game's own switch was found instead: R3 flips the camera style (exe+1D2DC04, exe+214860 - when the options allow
// it), and exe+214C10 then marks the classic camera in bit 0x1000000 of exe+1E21AB0 (set when the style is 0 or the
// area has no 3D camera - work+68 = 0 - and cleared again for the 3D camera). That bit is what the plugin goes by now.
static volatile u32* g_camStyle; static volatile u32* g_camFlags;
static int CamFields(char* o, u8* work)
{
    if (!Readable(work, 0x400)) { o[0] = 0; return 0; }
    float* e = (float*)(work + 0x3D0); float* t = (float*)(work + 0x3E0);
    float dx = t[0] - e[0], dy = t[1] - e[1], dz = t[2] - e[2], h = __builtin_sqrtf(dx * dx + dz * dz);
    int pit = h > 1.0f ? (int)(atan2_(dy, h) * 57.29578f) : 0, dist = (int)__builtin_sqrtf(dx * dx + dy * dy + dz * dz);
    u16 vy = (u16)(int)((h > 1.0f ? atan2_(dx, dz) : 0) / 6.2831853f * 65536.0f);
    return wsprintfA(o, "view %d deg, camera number %d deg, pitch %d, distance %d, work+74 %X +318 %X +31C %X +2F0 %X +68 %X, elevation %d (%d..%d), statuses 3B/3D %d%d; camera style %d, classic mark %d",
        Deg16(vy), Deg16(*(u16*)(work + 0x338)), pit, dist, *(u32*)(work + 0x74), *(u32*)(work + 0x318), *(u32*)(work + 0x31C), *(u32*)(work + 0x2F0), *(u32*)(work + 0x68),
        (int)(*(float*)(work + 0x340) * 1000.0f), (int)(*(float*)(work + 0x98) * 1000.0f), (int)(*(float*)(work + 0x9C) * 1000.0f), Bit(0x3B), Bit(0x3D),
        g_camStyle ? (int)*g_camStyle : -1, g_camFlags ? ((*g_camFlags & 0x1000000u) ? 1 : 0) : -1);
}
static void ClassicWatch(u8* work, float camYaw, float hl)
{
    static int on, off;
    if (!cfgClassicBlock || !work) { g_classic = 0; on = off = 0; return; }   // (work is read up to +0x3EC in this frame already)
    if (g_camFlags) {   // the game's own mark
        int c = g_camStyle && *g_camStyle == 1 && !(*g_camFlags & 0x1000000u);
        if (c != g_classic) {
            g_classic = c; if (c) g_classicN++;
            static int told; if (told < 40) { told++; char b[600]; int n = wsprintfA(b, c ? "camera: the classic (overhead) camera is on (the game's mark) - no shoulder view while it is: " : "camera: the 3D camera again (the game's mark) - shoulder view allowed: "); CamFields(b + n, work); Log(b); }
        }
        if (g_classic) g_classicFrames++;
        (void)camYaw; (void)hl; return;
    }
    // not with his back to a wall (the game's wall camera looks along the wall) or for 30 frames after it
    static int wallGap; if (Bit(ST_WALL)) wallGap = 30; else if (wallGap > 0) wallGap--;
    int gameCam = !g_aimLatch && g_amt <= 0.001f && !g_hold && !g_fly && !Bit(ST_FPV) && hl > 1.0f && !GameEvent() && !wallGap;
    if (!gameCam) { on = off = 0; return; }   // kept as it was
    u16 vy = (u16)(int)(camYaw / 6.2831853f * 65536.0f);
    int d = (i16)(u16)(vy - *(u16*)(work + 0x338)); if (d < 0) d = -d; if (d > 0x4000) d = 0x8000 - d;   // off from the number or its reverse
    if (d > 0x1200) { on++; off = 0; } else { off++; on = 0; }   // 25 degrees
    if (g_classic) g_classicFrames++;
    int flip = (!g_classic && on >= 30) || (g_classic && off >= 30);
    if (!flip) return;
    g_classic = !g_classic; if (g_classic) g_classicN++;
    on = off = 0;
    if (g_classicN <= 20 || !g_classic) {
        char b[600]; int n = wsprintfA(b, g_classic ? "camera: the classic (overhead) camera taken to be on - the game's camera looks away from its camera number; no shoulder view while it is (NoShoulderInClassicCamera): "
                                                     : "camera: the normal camera again (it looks where its camera number says) - shoulder view allowed: ");
        CamFields(b + n, work); Log(b);
    }
}
static int ClassicRefuse(u8* work, const char* what)
{
    if (!cfgClassicBlock || !g_classic) return 0;
    g_classicBlockN++;
    if (g_classicBlockN <= 20) { char b[600]; int n = wsprintfA(b, "%s: no shoulder view - the classic (overhead) camera is on (NoShoulderInClassicCamera=1); camera: ", what); CamFields(b + n, work); Log(b); }
    return 1;
}
static void CamFieldsOn(u8* work)   // the camera's fields when the shoulder view comes on
{
    static int n; if (!cfgClassicBlock || n >= 20) return; n++;
    char b[600]; int k = wsprintfA(b, "shoulder view on - the game's camera before it: "); CamFields(b + k, work); Log(b);
}
static char* FrameCheck(char* o)
{
    int n = 0; o[0] = '-'; o[1] = 0;
    for (int w = 1; w < PF_NW && n < 2; w++) if (g_pfCamN[w]) { char t[16]; wsprintfA(o + (n ? slen(o) : 0), "%sw%d %s", n ? " / " : "", w, Tenths(t, g_pfCamErr[w])); n++; }
    return o;
}
static __attribute__((noinline)) void LogSumFpv(void)
{
    static u32 last[14];
    u32 cur[14] = { g_fpEndN, g_fpReN, g_fpReOkN, g_fpReBadN, g_l1KeepN, g_l1LetGoN, g_fpHandBackN, g_r1KeepN, g_classicN, g_classicBlockN, g_classicFrames, (u32)g_classic,
        g_crawlWalkN, g_crawlSlowMarkN };
    int ch = 0; for (int i = 0; i < 14; i++) if (cur[i] != last[i]) ch = 1;
    if (!ch) return;
    for (int i = 0; i < 14; i++) last[i] = cur[i];
    char b[700];
    wsprintfA(b, "summary (first person, camera, crawl): first person - ended %u times, RB pressed again for the game %u times (first person back %u / not %u), LB kept held for the game %u frames (let go in first person %u times), "
        "camera handed back with LB let go %u times, RB kept held for the game %u frames; classic camera - on %u times (now %s, %u frames), shoulder view refused %u times; lying - slow crawl frames %u (status 4 marked %u)",
        cur[0], cur[1], cur[2], cur[3], cur[4], cur[5], cur[6], cur[7], cur[8], cur[11] ? "on" : "off", cur[10], cur[9], cur[12], cur[13]);
    Log(b);
}
static __attribute__((noinline)) void LogSumAim(void)
{
    static u32 last[12];
    u32 cur[12] = { g_paFrames, g_paLearnN, g_paShotN[0], g_paShotN[1], g_paSteadyN, g_wdbN, g_fpN, g_fpLatN[0], g_fpLatN[1], g_fpNoShot[0] + g_fpNoShot[1], g_paNoBone, g_paLimN };
    int ch = 0; for (int i = 0; i < 12; i++) if (cur[i] != last[i]) ch = 1;
    if (!ch) return;
    for (int i = 0; i < 12; i++) last[i] = cur[i];
    int nm = 0; for (int w = 0; w < PF_NW; w++) for (int st = 0; st < 2; st++) if (g_pa[w][st].n) nm++;
    char b[900], t1[16], t2[16], t3[16], t4[16];
    wsprintfA(b, "summary (gun aim): gun aim - frames turned about his shoulder %u (bones missing %u, at the arms' limit %u), barrel lines measured from %u shots (%d weapon / stance pairs); "
        "shots at spots within 30 m with it %u (barrel %s deg off the spot vertically on average) / without %u (%s deg); arm turn check %u frames - off by %s deg, line %s deg off the spot; ",
        cur[0], cur[10], cur[11], cur[1], nm, cur[3], Tenths(t1, cur[3] ? g_paShotErr[1] / (float)cur[3] : 0.0f), cur[2], Tenths(t2, cur[2] ? g_paShotErr[0] / (float)cur[2] : 0.0f),
        cur[4], Tenths(t3, g_paTurnErr), Tenths(t4, g_paLineErr));
    wsprintfA(b + slen(b), "strafe - walk changes from the game too short to take %u frames; fire presses %u - shot after standing %u ms / %u frames on average (%u, slowest %u ms), strafing %u ms / %u frames (%u, slowest %u ms), no shot %u",
        cur[5], cur[6], g_fpLatN[0] ? g_fpLat[0] / g_fpLatN[0] : 0, g_fpLatN[0] ? g_fpLatFr[0] / g_fpLatN[0] : 0, g_fpLatN[0], g_fpLatMax[0],
        g_fpLatN[1] ? g_fpLat[1] / g_fpLatN[1] : 0, g_fpLatN[1] ? g_fpLatFr[1] / g_fpLatN[1] : 0, g_fpLatN[1], g_fpLatMax[1], cur[9]);
    Log(b);
}
static __attribute__((noinline)) void LogSumPose(void)
{
    static u32 last[12];
    u32 cur[12] = { g_pfLearnN, g_pfFrames, g_pfShotN[0], g_pfShotN[1], g_pfDeadN, g_headDragN, g_shotWalkN, g_sightCrawlN, g_sightLaserN, g_camOnlyN, g_camSkipN, g_poseDownN };
    int ch = 0; for (int i = 0; i < 12; i++) if (cur[i] != last[i]) ch = 1;
    if (!ch) return;
    for (int i = 0; i < 12; i++) last[i] = cur[i];
    int ready = 0; for (int w = 0; w < PF_NW; w++) if (g_pfIdx[w]) ready++;
    char b[900], t1[16], t2[16], t3[16], t4[48];
    wsprintfA(b, "summary (gun pose): gun pose - shots learned from %u, weapons ready %d, frames corrected %u (vertical error then %s deg on average), shots with the correction on %u (barrel %s deg off the spot vertically on average) / off %u (%s deg), frame check %s deg, switched off %u; "
        "arms let point further down %u frames; camera kept to his head while firing - followed past the kick %u frames (largest %d units); strafe - walk kept through a shot %u frames; crosshair hidden crawling %u / laser sight %u frames; "
        "camera - frames held in by a surface only it stops at (nets) %u, hits let through (800000) %u",
        cur[0], ready, cur[1], Tenths(t1, g_pfFrames ? g_pfEvSum / (float)g_pfFrames : 0.0f), cur[3], Tenths(t2, cur[3] ? g_pfShotErr[1] / (float)cur[3] : 0.0f), cur[2], Tenths(t3, cur[2] ? g_pfShotErr[0] / (float)cur[2] : 0.0f),
        FrameCheck(t4), cur[4], cur[11], cur[5], (int)g_dragMaxAll, cur[6], cur[7], cur[8], cur[9], cur[10]);
    Log(b);
}
static void hk_Apply(u8* work)
{
    if (!work || !cfgEnabled) { o_Apply(work); return; }
    LARGE_ t0; QueryPerformanceCounter(&t0);   // how long the plugin takes
    {   // time since the last camera update = the game's frame time
        static LARGE_ prev, fq; if (!fq.q) QueryPerformanceFrequency(&fq);
        if (prev.q && fq.q) {
            u64 d = (u64)(t0.q - prev.q);
            if (d < (u64)fq.q) {   // pauses, menus, loading (a second or more) are not frames
                g_fiN++; g_fiSum += d; if (d > g_fiMax) g_fiMax = d;
                if (d * 40 > (u64)fq.q) g_fi25++; if (d * 25 > (u64)fq.q) g_fi40++;
            }
        }
        prev = t0;
    }
    u32 lc0 = g_logCalls, ray0 = (u32)g_perfRayN;
    CamButton(work);
    PadLookNormal(work);
    float* eye = (float*)(work + 0x3D0);
    float* tgt = (float*)(work + 0x3E0);
    g_camWorkLast = work;

    // undo last frame's change if the game is re-applying without recomputing
    if (c_valid && c_work == work && eq12(eye, c_me) && eq12(tgt, c_mt)) { memcpy(eye, c_oe, 12); memcpy(tgt, c_ot, 12); }
    c_valid = 0;

    // The weapon-raised flag drops the moment a shot is fired (releasing the button fires) and flickers for single
    // frames afterwards. The over-the-shoulder state therefore needs the flag for 3 frames in a row to start, and then
    // stays on for AimHold ms after the flag was last seen steadily.
    static int consec, held; static DWORD lastSeen;
    int raw = Aiming();
    // not with his back against a wall either (if he gets there anyway the shoulder view steps aside until he leaves
    // the wall, instead of the plugin turning him against the wall pose)
    int stay = !Bit(ST_FPV) && Bit(ST_TPS) && !Bit(ST_WALL) && !GameEvent();   // event scenes too
    DWORD nowT = GetTickCount();
    consec = raw ? consec + 1 : 0;
    int aiming;
    if (cfgAimMode == 0) {
        // old behaviour: follow the game's weapon-raised state, with a hold time
        if (consec >= 3 && !held && cfgClassicBlock && g_classic) { static DWORD rt; if (nowT - rt > 2000) { rt = nowT; ClassicRefuse(work, "gun raised"); } }
        else if (consec >= 3) { held = 1; lastSeen = nowT; }
        if (held && (!stay || nowT - lastSeen > (DWORD)cfgHold)) held = 0;
        aiming = held;
    } else {
        // The aim key (right mouse button by default) switches the over-the-shoulder state on and off. The game has its
        // own aim toggle on the same key, visible as bit 0x8000 of Snake's button word, and every shot switches that
        // toggle off. While over-the-shoulder is on, the plugin presses the aim key again as soon as the game's toggle
        // is seen off, so the weapon button counts as held again before the firing animation has finished and Snake
        // does not lower the gun in between.
        // the plugin's state and the game's own aim toggle are two switches on one key, and they could get out of step:
        // after first-person aiming, a loading screen or a cutscene the game's toggle was still on while the shoulder
        // view was off, so the next press switched the shoulder view on and the gun DOWN (or left Snake aiming in the
        // game's normal view). Now:
        // - a new scene (loading, cutscene, the camera not running for a second) always starts with the shoulder view
        //   off and, if the game still has the gun raised, lowers it: plain third person;
        // - whenever the gun is found raised in the normal view with the shoulder view off, the plugin lowers it
        //   (KeepInStep), so that state cannot be sat in and one press always leads into the shoulder view;
        // - leaving first-person view with the gun raised returns to the shoulder view (AfterFirstPerson); the same
        //   when the key raised the gun at a moment the view could not switch;
        // - knife and fork (NoRaiseWeapons) are never "pressed" by the plugin - for them the aim key is the attack,
        //   which is what made Snake stab in a loop - and the handkerchief (NoShoulderWeapons) has no shoulder view at
        //   all.
        static int ots, prevKey, mism, fails, backFpv, cutWas, holdFpv; static DWORD exitT, lastRun, freshT, pendT, pressT, holdT; static u8* lastPl;
        static int bySrc, padNow;   // 1 = the aim key switched the view on, 2 = the pad button
        int fg = g_fgCache;   // from the input thread
        if (fg && g_fgWnd) g_gameWnd = g_fgWnd;
        g_fgNow = fg;
        u8* plq = g_pl; int okq = Readable(plq, 0x800);
        u32 btn = okq ? *(u32*)(plq + 0x7E8) : 0;
        int latch = (btn & 0x8000u) != 0, fire = (btn & 0x100000u) != 0;
        g_fireNow = fire;
        g_btnNow = btn; g_btnOk = okq; g_btnFrame = g_frame;   // for FireTick (no second read)
        if (g_injDown && nowT - g_injT >= 40) { AimKeyEvent(0); g_injDown = 0; }
        int key = (GetAsyncKeyState(cfgAimKey) & 0x8000) != 0;
        int cut = GameEvent();   // event scenes the cutscene flags miss, too
        {   static int evWas; int ev = g_eventWhy;
            if (ev && !evWas) { g_eventN++; if (g_eventN <= 20) { char b[160]; wsprintfA(b, "event scene (%s): no shoulder view, no crosshair", ev == 1 ? "status 0x139" : ev == 2 ? "status 0x9B" : "the game's no-control check"); Log(b); } }
            evWas = ev;
        }
        int wpn = Wpn(plq), armed = wpn > 0 && !NoOts(wpn);
        {
            // state: the key works for this weapon (a press made by the plugin shows up as the key going down too)
            static int latchW, wpnW, keyW; static DWORD pressW;
            if (key && !keyW) pressW = nowT ? nowT : 1;
            keyW = key;
            if (okq && wpn == wpnW && latch != latchW && pressW && nowT - pressW < 400 && wpn > 0 && wpn < 64 && !((g_latchWorked >> wpn) & 1) && !Throwable(wpn)) {
                g_latchWorked |= 1ull << wpn;
                if (g_noLatch & (1ull << wpn)) { g_noLatch &= ~(1ull << wpn); g_noLatchN++; Log("aim key: it changes this weapon's raised state after all - the plugin presses it for this weapon again"); }
            }
            if (okq) { latchW = latch; wpnW = wpn; }
        }
        {
            int restart = !lastRun || (lastRun && nowT - lastRun > 1000) || (cutWas && !cut) || (okq && lastPl && plq != lastPl);
            if (restart) {
                if (ots) Log("over-the-shoulder off (new scene)");
                // also forget a press made before
                if (pendT || pressT || latch) { char b[140]; wsprintfA(b, "new scene: plain third person (gun %s, earlier press %s)", latch ? "raised - lowering it" : "down", (pendT || pressT) ? "forgotten" : "none"); Log(b); }
                ots = 0; backFpv = 0; mism = fails = 0; exitT = 0; prevKey = key; freshT = nowT ? nowT : 1; g_hold = 0;
                pendT = 0; pressT = 0; g_fly = 0; holdFpv = 0; g_pressPause = 0; g_padPend = 0;
            }
            lastRun = nowT; cutWas = cut; if (okq) lastPl = plq;
        }
        {   // controller: the shoulder view is on while the chosen pad button is held
            static int padWas; int pad = PadHeld(nowT); padNow = pad;
            // after loading a save, the continue screen or an event the game is in neither first nor third person until
            // the gun is raised once: the fire button got the view going. The keyboard's aim key is the game's own aim
            // key, so it raised the gun and the view followed; the pad button is not, so nothing happened. Now a press
            // in that state raises the gun with the aim key (once per press) and the view comes on as soon as the game
            // allows it, while the button is still held. Only then: on foot, gun down, no first person, no wall, no
            // event scene, a weapon with the shoulder view that the aim key raises.
            {   int nl = wpn > 0 && wpn < 64 && (((g_noLatch >> wpn) & 1) || (wpn < 32 && (((u32)cfgNoRaise >> wpn) & 1)));
                int almost = cfgPadWake && fg && okq && armed && !ots && !nl && !latch && !Bit(ST_TPS) && !Bit(ST_FPV) && !Bit(ST_WALL) && !cut
                    && (Bit(ST_STAND) || Bit(ST_CROUCH) || Bit(3)) && !g_injDown && nowT - g_injT >= 250 && !(cfgClassicBlock && g_classic);
                if (pad && !padWas && almost) {
                    AimKeyEvent(1); g_injDown = 1; g_injT = nowT ? nowT : 1; g_padPend = 1; g_padPendT = nowT ? nowT : 1; freshT = 0; pendT = 0; pressT = 0; g_l1FixN++;
                    if (g_l1FixN <= 20) Log("pad: shoulder-view button pressed while the game is in neither first nor third person (new scene, gun down) - the aim key pressed once to raise the gun; the view comes on when the game allows it");
                }
                if (g_padPend) {
                    if (!pad || ots || !armed || cut) g_padPend = 0;
                    else if (stay && ClassicRefuse(work, "pad (after raising the gun)")) g_padPend = 0;
                    else if (stay) {
                        g_padPend = 0; ots = 1; CamFieldsOn(work); mism = 0; fails = 0; bySrc = 2; freshT = 0;
                        char b[120]; wsprintfA(b, "pad: over-the-shoulder on (after raising the gun, %d ms)", (int)(nowT - g_padPendT)); Log(b); FpvHeading(work, nowT);
                    } else if (nowT - g_padPendT > 1500) {
                        g_padPend = 0; Log("pad: the game did not go into third person within 1.5 s of raising the gun - left as it is");
                    }
                }
            }
            if (pad != padWas && fg && stay) {
                if (pad && !ots && armed && ClassicRefuse(work, "pad")) { }
                else if (pad && !ots && armed) { ots = 1; mism = 0; fails = 0; bySrc = 2; freshT = 0; Log("pad: over-the-shoulder on"); CamFieldsOn(work); FpvHeading(work, nowT); }
                else if (!pad && ots) { ots = 0; exitT = nowT ? nowT : 1; Log("pad: over-the-shoulder off"); }
            }
            // switched on by the pad button and the button is no longer held (let go while the game was in the
            // background or the view could not switch): off - the pad never leaves the view on by itself
            if (ots && bySrc == 2 && !pad && fg && stay) { ots = 0; exitT = nowT ? nowT : 1; Log("pad: over-the-shoulder off (button no longer held)"); }
            if (ots && stay && cfgClassicBlock && g_classic && g_camFlags) { ots = 0; exitT = nowT ? nowT : 1; Log("over-the-shoulder off: the classic (overhead) camera came on"); }
            padWas = pad;
        }
        int synthetic = g_injT && nowT - g_injT < 200;
        {   // the pause in the plugin's presses lasts until the view is switched again (or another weapon is taken)
            static int otsP, wpnP; if (ots != otsP || wpn != wpnP) g_pressPause = 0; otsP = ots; wpnP = wpn;
        }
        int nolatch = wpn > 0 && wpn < 64 && (((g_noLatch >> wpn) & 1) || (wpn < 32 && (((u32)cfgNoRaise >> wpn) & 1)));
        if (key && !prevKey && !synthetic && fg && wpn > 0 && NoOts(wpn) && !ots) {
            static u32 told; if (wpn < 32 && !((told >> wpn) & 1)) { told |= 1u << wpn; char b[120]; wsprintfA(b, "weapon type %d: no shoulder view for it (NoShoulderWeapons)", wpn); Log(b); }
        }
        if (key && !prevKey && !synthetic && fg) {
            if (stay && !ots && armed && ClassicRefuse(work, "aim key")) { }   // the game's own aiming goes ahead
            else if (stay && (ots || armed)) {   // no shoulder view with empty hands
                ots = !ots; if (ots) CamFieldsOn(work); mism = 0; fails = 0; backFpv = 0; pendT = 0; bySrc = 1;
                if (!ots) exitT = nowT ? nowT : 1;
                else FpvHeading(work, nowT);
                g_fly = 0;
                Log(ots ? "aim key: over-the-shoulder on" : "aim key: over-the-shoulder off");
            } else if (!stay && armed && !freshT) pressT = nowT ? nowT : 1;   // the view cannot switch now; the key still raises / lowers the gun
            freshT = 0;   // the player has taken over after the new scene
        }
        if (pressT && nowT - pressT >= 150) {   // what that press did, once the game has taken it in
            if (latch) pendT = nowT ? nowT : 1; else { pendT = 0; backFpv = 0; }
            pressT = 0;
        }
        if (!synthetic) prevKey = key;
        if (ots && !stay) {
            ots = 0; exitT = 0; backFpv = !cut; pendT = 0;
            // Now the shoulder camera simply stays where it is until the game switches to first person, so the switch
            // starts from the shoulder.
            // and it does not just wait there - it flies from the shoulder to Snake's head in the time the game takes
            // to switch (measured each time, about a quarter second), arriving as first person starts.
            if (cfgFpvHold && !cut && !Bit(ST_WALL) && (Bit(ST_FPV) || !Bit(ST_TPS))) {
                g_hold = 1; holdT = nowT ? nowT : 1; holdFpv = 0;
                g_fly = 1; g_flyT = holdT; g_flyDur = (DWORD)(g_holdMs * 0.9f); if (g_flyDur < 60) g_flyDur = 60; g_fly3K = 0;
            }
            if (g_hold) { char b[120]; wsprintfA(b, "over-the-shoulder off (first person): camera flies from the shoulder to the head over %d ms", (int)g_flyDur); Log(b); }
            else Log("over-the-shoulder off (view changed)");
        }
        if (g_hold) {
            if (Bit(ST_FPV) && !holdFpv) {
                holdFpv = 1; float ms = (float)(nowT - holdT);
                if (ms > 50.0f && ms < 1000.0f) g_holdMs += (ms - g_holdMs) * 0.5f;
                char b[120]; wsprintfA(b, "first person took over after %d ms (the flight to the head took %d ms)", (int)ms, (int)g_flyDur); Log(b);
            }
            if (ots || cut || (stay && !backFpv) || (!Bit(ST_FPV) && !holdFpv && nowT - holdT > 1500)) {
                g_hold = 0;
                if (g_fly == 1) {
                    // the game called first person off before it started and the shoulder view is on again. Now it
                    // flies back from where it got to.
                    if (ots && !holdFpv && !cut) FlyBackCancelled(nowT, holdT);
                    else g_fly = 0;
                }
            }
            // putting the weapon away in first person (or first person ending any other way than straight back into the
            // shoulder view) left the held camera parked behind Snake's head while the game did something else. The
            // camera now goes straight back to the game in those cases.
            else if (!armed || (holdFpv && !Bit(ST_FPV) && !(stay && backFpv && cfgAfterFpv && !(cfgFpvHandBack && bySrc == 2 && !padNow)))) {
                // also when the shoulder view is not coming back because the pad's shoulder-view button is no longer
                // held (LB let go in first person, then RB, gun still raised): the view stayed off, so the held camera
                // was left parked at Snake's head - the camera stuck behind his head. Lowering the gun first (L3) made
                // "stay" false and the camera went back to the game - why that order worked.
                int lbUp = armed && stay && backFpv && cfgAfterFpv && bySrc == 2 && !padNow;
                g_hold = 0; g_fly = 0; g_amt = 0; if (lbUp) { backFpv = 0; g_fpHandBackN++; }
                Log(!armed ? "first person: weapon put away - camera handed back to the game" : lbUp ? "first person ended with the shoulder-view button let go: camera handed back to the game (the gun stays raised in the game's own view)" : "first person ended: camera handed back to the game");
            }
        }
        if (ots && okq && !armed) { ots = 0; exitT = 0; Log(wpn > 0 ? "over-the-shoulder off (not used with this weapon)" : "over-the-shoulder off (no weapon)"); }
        if (!armed) { backFpv = 0; pendT = 0; pressT = 0; }
        if (pendT && nowT - pendT > 20000) pendT = 0;
        if (!ots && stay && fg && okq && !pressT && !freshT && (backFpv || (pendT && latch)) && !(backFpv && bySrc == 2 && !padNow) && ClassicRefuse(work, backFpv ? "back from first person" : "gun raised while the view could not switch")) { backFpv = 0; pendT = 0; }
        if (!ots && stay && fg && okq && !pressT && !freshT && (backFpv || (pendT && latch)) && !(backFpv && bySrc == 2 && !padNow)) {
            int fromKey = !backFpv;
            backFpv = 0; pendT = 0;
            if (cfgAfterFpv) {
                ots = 1; mism = 0; fails = 0; if (fromKey) bySrc = 1;
                FpvHeading(work, nowT);
                // back from first person the camera flies from the head out to the shoulder (FirstPersonBackMs),
                // keeping the pitch first person had; no fade-in from the normal camera position (that was the zoom out
                // and in when leaving first person)
                // only while the camera is still at his head: parked there by the plugin, or first person ended a
                // moment ago (150 ms). When first person had ended earlier (gun lowered in first person, first person
                // left, then the aim key) the camera is already in the normal view, and flying from his head made it
                // jump into his head for a frame: it swings in from where it is instead.
                int recentFp = g_lastFpvT && nowT - g_lastFpvT < 800;
                int fromFp = recentFp && (g_hold || nowT - g_lastFpvT < 150);
                if (recentFp && !fromFp) { g_headSkips++; Log("over-the-shoulder on shortly after first person: the camera was already back in the normal view - it swings in from there (no move from his head)"); }
                if (cfgFpvHold && fromFp) {
                    g_amt = 1.0f; g_hold = 0; g_snaps++;
                    if (g_fpOk) { float h = __builtin_sqrtf(g_fpD[0] * g_fpD[0] + g_fpD[2] * g_fpD[2]); g_pitchSet = atan2_(g_fpD[1], h) * 57.29578f; g_pitchSetOk = 1; }
                    if (cfgFpvBackMs > 0) { g_fly = 2; g_flyT = nowT ? nowT : 1; g_flyDur = (DWORD)cfgFpvBackMs; } else g_fly = 0;
                }
                char b[160]; wsprintfA(b, "over-the-shoulder on (%s)%s", fromKey ? "the gun was raised while the view could not switch" : "back from first person",
                    (cfgFpvHold && fromFp) ? ": camera flies from the head to the shoulder" : ""); Log(b);
                if (!fromFp && g_hold && !holdFpv) FlyBackCancelled(nowT, holdT);   // turned round in this same frame
            }
        }
        if (freshT && nowT - freshT > 4000) freshT = 0;   // 4 s
        if (exitT && nowT - exitT > 1500) exitT = 0;
        // the game's toggle is wanted on while the shoulder view is on and off otherwise
        int sync = cfgSync && !g_padConn;
        int want = stay ? ots : 0;
        // right after a new scene with a pad connected, first person with the gun raised was left alone (a pad player
        // may be holding the first-person view) - so after loading a save the game sat in first person. In the first
        // seconds of a scene, with no pad button held, it is lowered too: plain third person.
        int freshFpv = freshT && Bit(ST_FPV) && latch && !(g_padConn && g_padAny);
        int enforce = okq && fg && !fire && !key && armed && !nolatch && !synthetic && !pressT && !g_pressPause && !g_padPend
            && (stay ? (ots || exitT || sync || freshT) : ((sync || !Bit(ST_FPV) || freshFpv) && freshT && !cut));   // a new scene, with a pad too
        if (enforce && stay && !ots && !exitT && !freshT && cfgClassicBlock && g_classic) enforce = 0;   // the classic camera - the game's own aiming
        // the gun is wanted up for the shoulder view and the game's own input already says "raised" (bit 0x8000 of the
        // word Snake reads, Snake+0x7F8), but Snake's "held over from before" mask (Snake+0x7F4) hides it. The game
        // fills that mask with every button held at certain moments (status 0x138) and clears a bit only once the
        // button is let go - which a raised-state that stays on never is. The plugin lets it through.
        if (enforce && stay && ots && want && !latch) {
            u8* pw = *(u8**)(plq + 0x7F8);
            u32 inw = Readable(pw, 4) ? *(u32*)pw : 0, mk = *(u32*)(plq + 0x7F4);
            if ((inw & 0x8000u) && (mk & 0x8000u)) {
                *(u32*)(plq + 0x7F4) = mk & ~0x8000u; g_maskFreeN++;
                if (g_maskFreeN <= 10) { char b[200]; wsprintfA(b, "shoulder view: the game's input says the gun is raised but Snake was holding that back (mask %08X, from before) - let through", mk); Log(b); }
                mism = 0;
            }
        }
        {   // the first 6 seconds of every scene go to the log: first person, gun, view switch, pad
            static DWORD sceneT, lastFresh; static int lastSt = -1;
            if (freshT && freshT != lastFresh) { sceneT = freshT; lastSt = -1; lastFresh = freshT; }
            if (sceneT && nowT - sceneT < 6000) {
                int st = (Bit(ST_FPV) ? 1 : 0) | (latch ? 2 : 0) | ((btn & 0x800u) ? 4 : 0) | (Bit(ST_TPS) ? 8 : 0) | (stay ? 16 : 0) | (ots ? 32 : 0) | (g_padConn ? 64 : 0) | (g_padAny ? 128 : 0);
                if (st != lastSt) {
                    char b[260]; wsprintfA(b, "new scene +%d ms: first person %s, gun %s, first-person button bit %s, third-person flag %s, view can switch %s, shoulder view %s, pad %s%s%s",
                        (int)(nowT - sceneT), (st & 1) ? "ON" : "off", (st & 2) ? "raised" : "down", (st & 4) ? "on" : "off", (st & 8) ? "on" : "off", (st & 16) ? "yes" : "no",
                        (st & 32) ? "on" : "off", (st & 64) ? "connected" : "none", (st & 128) ? " (a button held)" : "", freshFpv ? " - the plugin lowers the gun" : "");
                    Log(b); lastSt = st;
                }
            } else if (sceneT && nowT - sceneT >= 6000) sceneT = 0;
        }
        // loading a save into first person. In the first 5 seconds of a scene such a switch-on is now held off: the bit
        // is added to the game's own "button held over from before" mask for Snake (Snake+0x7F4; the game clears it by
        // itself the moment the button is really off), so Snake stays in third person. If the player's first-person key
        // is known (learned from his own presses, kept in the ini as [Aim] FirstPersonKey), the plugin also presses it
        // once so the toggle is really off again; otherwise his next press of it only switches the toggle off.
        {
            static u32 lastBtn; static DWORD sT, lastF, chkT; static int held, score[256], rises;
            if (freshT && freshT != lastF) { sT = freshT; lastF = freshT; }
            u8* pp = okq ? *(u8**)(plq + 0x7F8) : 0;
            u32 padw = Readable(pp, 4) ? *(u32*)pp : 0;
            int rose = okq && (btn & 0x800u) && !(lastBtn & 0x800u);
            if (rose) {
                int hk[8]; int nk = HeldKeys(hk);
                int mine = g_fkT && nowT - g_fkT < 600;
                if (cfgLoadTps && sT && nowT - sT < 5000 && !nk && !(g_padConn && g_padAny) && !mine) {
                    *(u32*)(plq + 0x7F4) |= 0x800u; held = 1; g_fpvHeldN++;
                    char b[240]; wsprintfA(b, "new scene +%d ms: the game's first-person toggle switched itself on (nothing pressed) - held off, Snake stays in third person", (int)(nowT - sT)); Log(b);
                    if (cfgFpvKey && !g_fkDown) { KeyEvent(cfgFpvKey, 1); g_fkDown = cfgFpvKey; g_fkT = nowT ? nowT : 1; chkT = nowT ? nowT : 1;
                        wsprintfA(b, "new scene: pressed the first-person key (code %02X) once to switch the toggle back off", cfgFpvKey); Log(b); }
                } else if (nk && !mine && !(g_padConn && g_padAny)) {
                    // the player's own press: learn which key it is (the key held at every switch-on)
                    rises++; for (int i = 0; i < nk && i < 8; i++) score[hk[i]]++;
                    int best = 0; for (int v = 1; v < 255; v++) if (score[v] > score[best]) best = v;
                    if (best && score[best] >= 2 && score[best] * 10 >= rises * 8 && best != cfgFpvKey) {
                        cfgFpvKey = best; char v[8]; wsprintfA(v, "%d", best);
                        WritePrivateProfileStringA("Aim", "FirstPersonKey", v, g_ini);
                        char b[160]; wsprintfA(b, "first-person key learned: code %02X (held at %d of %d switch-ons) - saved as [Aim] FirstPersonKey", best, score[best], rises); Log(b);
                    }
                }
            }
            if (held && okq) {
                if (!(padw & 0x800u)) { held = 0; Log(chkT && nowT - chkT < 1500 ? "first-person toggle: off again (the plugin's key press worked) - nothing held any more" : "first-person toggle: off again - nothing held any more"); chkT = 0; }
                else *(u32*)(plq + 0x7F4) |= 0x800u;   // keep it held off while the toggle stays on
                if (chkT && nowT - chkT > 1500) { chkT = 0; Log("first-person toggle: the plugin's key press did not switch it off - it stays held off until you press your first-person key"); }
            }
            lastBtn = btn;
        }
        {   // players who gave aiming another key in the game's controls. The plugin listens to its aim key ([Aim]
            // Key, the right mouse button) and presses it to raise the gun; if the game's aim is on another key, the
            // view switched on a key that does nothing in the game and the gun never came up. Each time the gun goes up
            // by the player's own key press (no pad button, not a press of the plugin's) the keys held are noted; if
            // [Aim] Key was never among them and one key always was (3 times or more), that key is taken as the aim key
            // and saved to the ini. Once [Aim] Key is seen raising the gun nothing is learned (LearnAimKey=0: never).
            // Only keys pressed just then count: keys already held a moment before (walking keys) are left out - every
            // 8th frame with the gun down the plugin notes which keys are held.
            static int latchL, sampN; static u32 heldBefore[8];
            if (cfgLearnAim && !g_alSeen && fg && armed && !latch && (++sampN & 7) == 0) {
                for (int w = 0; w < 8; w++) heldBefore[w] = 0;
                for (int vk = 1; vk < 255; vk++) if (GetAsyncKeyState(vk) & 0x8000) heldBefore[vk >> 5] |= 1u << (vk & 31);
            }
            if (cfgLearnAim && okq && fg && latch && !latchL && !synthetic && !(g_padConn && g_padAny) && !g_alSeen) {
                if (key) g_alSeen = 1;
                else {
                    int hk0[8], hk[8], nk = 0; int n0 = HeldKeys(hk0);
                    for (int i = 0; i < n0 && i < 8; i++) if (!((heldBefore[hk0[i] >> 5] >> (hk0[i] & 31)) & 1)) hk[nk++] = hk0[i];
                    if (nk > 0 && nk <= 3) {
                        g_alRises++; for (int i = 0; i < nk; i++) g_alScore[hk[i]]++;
                        int best = 0; for (int v = 1; v < 255; v++) if (g_alScore[v] > g_alScore[best]) best = v;
                        if (best && g_alScore[best] >= 3 && g_alScore[best] * 10 >= g_alRises * 8 && best != cfgAimKey && best != cfgFpvKey) {
                            char v[8]; wsprintfA(v, "%d", best); WritePrivateProfileStringA("Aim", "Key", v, g_ini);
                            char b[220]; wsprintfA(b, "aim key learned: the game raises the gun on key code %02X (held at %d of %d raises), not on [Aim] Key %02X - saved as [Aim] Key", best, g_alScore[best], g_alRises, cfgAimKey); Log(b);
                            cfgAimKey = best; g_aimKeyLearnN++; g_alSeen = 1; prevKey = (GetAsyncKeyState(cfgAimKey) & 0x8000) != 0;
                        }
                    }
                }
            }
            if (okq) latchL = latch;
        }
        if (enforce && latch != want) mism++; else { mism = 0; if (latch == want) fails = 0; }
        static int fpvTries;   // presses made for first person right after a new scene
        if (!freshT) fpvTries = 0;
        if (freshFpv && fpvTries >= 2) mism = 0;   // tried twice: leave it (and never blame the weapon for it)
        if (mism >= ((want || exitT) ? 2 : 6) && !g_injDown && nowT - g_injT >= 250) {
            if (freshFpv) { AimKeyEvent(1); g_injDown = 1; g_injT = nowT ? nowT : 1; g_rearms++; mism = 0; fpvTries++;
                char b[120]; wsprintfA(b, "new scene: first person with the gun raised - aim key pressed to lower it (try %d)", fpvTries); Log(b); }
            else if (fails >= 6) {
                int worked = wpn > 0 && wpn < 64 && ((g_latchWorked >> wpn) & 1) && !Throwable(wpn);
                char b[600];
                if (!worked) {
                    // the aim key does nothing for this weapon (grenades and other throwables): keep the shoulder view
                    // and stop pressing the key for it
                    if (wpn > 0 && wpn < 64) g_noLatch |= 1ull << wpn;
                    wsprintfA(b, "the aim key does not change this weapon's raised state (weapon %d): the plugin stops pressing it", wpn);
                } else {
                    // it has worked for this weapon before. Something in the game is not taking the presses right now:
                    // stop pressing until the view is switched again, and log what the game's input looks like.
                    g_pressPause = 1; g_pressFailN++;
                    wsprintfA(b, "the plugin's aim key presses did not raise the gun this time (weapon %d; they did before) - it stops pressing until the view is switched again", wpn);
                }
                if (g_pressFailN <= 12 || !worked) { AimDiag(b, plq); Log(b); }
                exitT = 0; fails = 0;
            }
            else {
                AimKeyEvent(1); g_injDown = 1; g_injT = nowT ? nowT : 1; fails++; g_rearms++; mism = 0;
                if (fails == 3 && want) {   // half way - what the game's input looks like
                    static int told3; if (told3 < 12) { told3++; char b[600]; wsprintfA(b, "aim key: 2 presses by the plugin did not raise the gun (weapon %d) - pressing again", wpn); AimDiag(b, plq); Log(b); }
                }
            }
        }
        aiming = ots;
    }
    g_rawAim = raw;
    g_aimLatch = aiming; g_applyT = nowT;
    g_crouchLatch = Bit(ST_CROUCH) && !Bit(ST_STAND);
    g_proneLatch = Bit(3) && !Bit(ST_STAND) && !Bit(ST_CROUCH);
    g_transLatch = Bit(6);   // changing stance (standing up, lying down ...)
    g_wallFrames = (aiming && Bit(ST_WALL)) ? (g_wallFrames < 1000000 ? g_wallFrames + 1 : g_wallFrames) : 0;
    {   // no crosshair while he crawls lying down (forward, back, left or right - as on the 3DS) and for a moment after,
        // and none at all with the guns that have a laser sight of their own (NoCrosshairWeapons: the EZ gun and the
        // Scorpion)
        static DWORD crawlT; static int was = -1;
        int w = aiming && cfgNoSightMask ? Wpn(g_pl) : 0;
        int laser = aiming && w > 0 && w < 32 && (((u32)cfgNoSightMask >> w) & 1);
        int crawling = aiming && cfgSightCrawl && g_proneLatch && (u32)(g_frame - g_moveFrame) <= 2;
        if (crawling) crawlT = nowT ? nowT : 1;
        if (!aiming || !g_proneLatch) crawlT = 0;
        int crawlHide = cfgSightCrawl && crawlT && nowT - crawlT < (DWORD)cfgCrawlHideMs + 1;
        int why = laser ? 2 : crawlHide ? 1 : 0;
        g_sightHide = why;
        if (why == 1) g_sightCrawlN++; else if (why == 2) g_sightLaserN++;
        if (why != was && why && g_sightHideLog < 12) {
            g_sightHideLog++; char b[200];
            if (why == 1) wsprintfA(b, "crosshair: hidden while Snake crawls lying down (back %d ms after he stops)", cfgCrawlHideMs);
            else wsprintfA(b, "crosshair: hidden - weapon %d has a laser sight of its own (NoCrosshairWeapons)", w);
            Log(b);
        }
        was = why;
    }
    {   // the moment first person starts: what was held (keys, pad), for the log
        static int fpWas; int fpNow = Bit(ST_FPV);
        static DWORD fpEndT;   // first person started again right after it ended
        if (!fpNow && fpWas) fpEndT = nowT ? nowT : 1;
        if (fpNow && !fpWas && fpEndT && nowT - fpEndT < 400) {
            g_fpRestartN++;
            if (g_fpRestartN <= 12) { char b[160]; wsprintfA(b, "first person started again %d ms after it ended", (int)(nowT - fpEndT)); Log(b); }
        }
        if (fpNow && !fpWas) {
            static int told; if (told < 12) {
                told++; char b[300]; int n = wsprintfA(b, "first person started: Snake's buttons %08X", g_pl && Readable(g_pl, 0x800) ? *(u32*)(g_pl + 0x7E8) : 0);
                // the first-person key is learned where the toggle switches on, not here
                if (g_padConn) wsprintfA(b + n, "; pad buttons %04X", (unsigned)g_padBtn);
                Log(b);
            }
        }
        fpWas = fpNow;
    }
    {   // first person / the end of third person coming by itself while he lies down (see StatusRing)
        StatusRing();
        static int fpW = -1, tpW = -1; int fp = Bit(ST_FPV), tp = Bit(ST_TPS);
        u32 bt = g_pl && Readable(g_pl, 0x800) ? *(u32*)(g_pl + 0x7E8) : 0;
        int lying = Bit(3) && !Bit(ST_STAND) && !Bit(ST_CROUCH), byHand = (bt & 0x800u) || (g_fkT && nowT - g_fkT < 600);
        if (fpW >= 0 && lying && !byHand && !GameEvent()) {
            if (fp && !fpW && g_autoFpN < 8) { g_autoFpN++; StatusRingLog("lying: first person started by itself (no first-person button)"); }
            else if (!tp && tpW && !fp && g_autoTpN < 8) { g_autoTpN++; StatusRingLog("lying: the game's third-person state ended by itself"); }
        }
        if (fp && !fpW && lying && !byHand) g_bushN++;
        fpW = fp; tpW = tp;
    }
    if (Bit(ST_FPV)) {   // where the first-person camera is (Snake+0x730: its camera; +0x20 eye, +0x30 look-at)
        g_lastFpvT = nowT ? nowT : 1;
        u8* plf = g_pl; u8* fc = Readable(plf, 0x800) ? *(u8**)(plf + 0x730) : 0;
        if (fc) { g_fpRot = *(u16*)(plf + 0x162); g_fpRotOk = 1; }
        if (Readable(fc, 0x40)) {
            float* E = (float*)(fc + 0x20); float* T = (float*)(fc + 0x30);
            float ex = E[0] - *(float*)(plf + 0x130), ey = E[1] - *(float*)(plf + 0x134), ez = E[2] - *(float*)(plf + 0x138);
            float dx0 = T[0] - E[0], dy0 = T[1] - E[1], dz0 = T[2] - E[2], dl0 = __builtin_sqrtf(dx0 * dx0 + dy0 * dy0 + dz0 * dz0);
            if (ex * ex + ez * ez < 600.0f * 600.0f && ey > -1200.0f && ey < 1500.0f && dl0 > 1.0f) {   // an eye in his head, not a camera behind him
                for (int i = 0; i < 3; i++) g_fpE[i] = E[i];
                g_fpD[0] = dx0 / dl0; g_fpD[1] = dy0 / dl0; g_fpD[2] = dz0 / dl0; g_fpOk = 1; g_fpDT = nowT ? nowT : 1;
            }
        }
    }
    HeadTrack();
    if (aiming && Bit(6) && g_burst < 30) g_burst = 30;   // every frame of a stance change goes to the log
    int camOn = aiming || g_hold;   // the camera stays on the shoulder through the switch to first person
    {   // leaving after looking up or down, the blend back waits two frames: the frame the view ends (the game's
        // camera is handed its new height in it) and the next (the game moves its camera there). Until then the view
        // stays fully on the shoulder, so the game's camera changing height is never seen. Without looking up or down
        // there is nothing to hand over and nothing changes.
        static int prevOn, outHold;
        if (prevOn && !camOn && g_handGap > 0.01f && !Bit(ST_FPV)) { outHold = 2; g_outHoldN++; }
        if (camOn) outHold = 0;
        prevOn = camOn;
        if (outHold > 0) { outHold--; g_amt = 1.0f; }
        else g_amt += ((camOn ? 1.0f : 0.0f) - g_amt) * 0.22f;
    }
    if (g_fly && camOn) g_amt = 1.0f;
    if (!camOn) g_fly = 0;
    if (g_amt < 0.002f) g_amt = 0;

    float fx = tgt[0] - eye[0], fz = tgt[2] - eye[2];
    float hl = __builtin_sqrtf(fx * fx + fz * fz);
    float camYaw = hl > 1.0f ? atan2_(fx, fz) : 0;

    g_orbitYaw16 = (u16)(int)(camYaw / 6.2831853f * 65536.0f);   // the game camera's heading, for keyboard diagonals
    OrbitWatch(work, camYaw, hl);
    ClassicWatch(work, camYaw, hl);

    if (cfgLog) {
        DWORD now = GetTickCount();
        {   static int movWas; int mov = aiming && g_pl && Readable(g_pl, 0x800) && (g_have || *(u16*)(g_pl + 0x7E0));
            if (mov && !movWas && g_burst < 20) g_burst = 20; movWas = mov; }
        if (aiming != g_wasAim) g_burst = 40;   // log every frame for a moment after switching
        if (g_burst > 0) g_burst--;
        {
            // changes, so the log shows what the game did with the press
            static int mb; int w = Wpn(g_pl);
            int m = ((GetAsyncKeyState(1) & 0x8000) ? 1 : 0) | ((GetAsyncKeyState(2) & 0x8000) ? 2 : 0);
            if (m != mb && w > 0 && NoOts(w)) g_burst = 60;
            mb = m;
        }
        if (cfgLogLvl >= 2 && (aiming != g_wasAim || g_burst > 0 || (aiming && now - g_lastLog >= 250))) { g_lastLog = now; LogState(aiming, camYaw, eye, tgt); }
    }
    FireTick(g_pl);
    g_wasAim = aiming;

    if (g_amt > 0 && hl > 1.0f) {
        memcpy(c_oe, eye, 12); memcpy(c_ot, tgt, 12);
        // view direction of the game's orbit camera
        float dx = tgt[0] - eye[0], dy = tgt[1] - eye[1], dz = tgt[2] - eye[2];
        float dl = __builtin_sqrtf(dx * dx + dy * dy + dz * dz);
        dx /= dl; dy /= dl; dz /= dl;
        static float stance;   // smoothed extra height for the current stance
        stance += ((float)(g_proneLatch ? cfgProneH : g_crouchLatch ? cfgCrouchH : 0) - stance) * 0.15f;
        float pivotY = tgt[1] - PivotDrift(dy) + (float)cfgHeight + stance;   // fixed height, whatever the view pitch
        // The height is taken from Snake himself. The game's look-at point sits 695 above his position standing,
        // 493 crouched and 296 lying, but it also wanders with the view pitch, and lying down it does so in a way the
        // table above does not describe - the camera sank into the ground looking up.
        static float baseH, posYs, proneF; static int baseOk; static float posYsOut;
        float floorY = -1e9f;
        float ancX = tgt[0], ancZ = tgt[2];   // the point the camera hangs from (his head, or his position)
        {   u8* plp = g_pl;
            if (Readable(plp, 0x800)) {
                float want = g_proneLatch ? 296.0f : g_crouchLatch ? 493.0f : 695.0f;
                float py0 = *(float*)(plp + 0x134);
                if (!baseOk || g_amt < 0.01f) { baseH = want; posYs = py0; proneF = g_proneLatch ? 1.0f : 0.0f; baseOk = 1; }
                if (!g_transLatch) baseH += (want - baseH) * 0.15f;
                // his position is followed exactly (CameraRigid);
                if (cfgRigid || py0 - posYs > 500.0f || py0 - posYs < -500.0f) posYs = py0; else posYs += (py0 - posYs) * 0.4f;
                proneF += ((g_proneLatch ? 1.0f : 0.0f) - proneF) * 0.15f;
                float body = baseH + stance;   // height above his position, from the stance flags
                {   // the camera is locked to a point on Snake's body, as on the 3DS: his hips' height in the
                    // motion playing (standing 8.0, crouched 4.2, the fast crouch walk 7.0, the low crouch walk 5.1
                    // motion units) times BodyScale, so standing, crouching, the two crouch walks and the change in
                    // between all set the height by themselves - no separate crouch offset. Height is added on top.
                    static int settle, lastSt = -1, bad, odd, toldS, toldC; static float hyS;
                    int stNow = g_proneLatch ? 2 : g_crouchLatch ? 1 : 0;
                    settle = (stNow == lastSt && !g_transLatch) ? settle + 1 : 0; lastSt = stNow;
                    float hy = (cfgFollow && !bad) ? BodyHipY(plp) : -9.0f;
                    g_hip100 = (int)(hy * 100.0f);
                    float hb = cfgRigid ? HipBlended(plp, hy) : hy;   // as the game blends it
                    g_hipB100 = (int)(hb * 100.0f);
                    int forced = Legs() && (u32)(g_frame - g_legFrame) <= 2;
                    // self-check while standing still (every standing pose has its hips near 8.0): a reading far from
                    // that for three seconds means the motion data is not what it is taken for
                    if (hy > 0 && settle >= 30 && !forced && !g_have && stNow < 2 && !g_wallFrames) {
                        int okv = stNow == 0 ? (hy > 5.5f && hy < 10.5f) : (hy > 1.0f && hy < 9.0f);
                        if (okv) {
                            odd = 0;
                            char b[160];
                            if (stNow == 0 && !toldS) { toldS = 1; wsprintfA(b, "camera locked to Snake's body: standing, hip height %d/100 -> camera %d above his position", (int)(hy * 100.0f), (int)(695.0f + (hy - 8.0f) * (float)cfgBodyScale) + cfgHeight); Log(b); }
                            if (stNow == 1 && !toldC) { toldC = 1; wsprintfA(b, "camera locked to Snake's body: crouched, hip height %d/100 -> camera %d above his position", (int)(hy * 100.0f), (int)(695.0f + (hy - 8.0f) * (float)cfgBodyScale) + cfgHeight); Log(b); }
                        } else if (stNow == 0 && ++odd > 180) { bad = 1; hy = -9.0f; Log("body height reading does not look right: the camera height follows the stance only"); }
                    }
                    if (hy > 0 && hb > 0) {
                        float ease = cfgRigid ? 100.0f : (float)cfgBodyEase;   // rigid = no easing at all
                        if (!g_lock || g_amt < 0.01f) hyS = hb; else hyS += (hb - hyS) * ease / 100.0f;
                        float lb = 695.0f + (hyS - 8.0f) * (float)cfgBodyScale;
                        body = lb * (1.0f - proneF) + body * proneF;
                        g_lock = 1;
                    } else g_lock = 0;
                    {   // locked to his head (see HeadTrack). Standing still this gives exactly the point above:
                        // his head minus where his head is then. Lying down keeps the old behaviour (proneF).
                        static float w, uF, uR, uU;
                        float want = (cfgHeadLock && g_headOk && g_headLearned) ? 1.0f : 0.0f;   // until learned: his hips
                        if (g_amt < 0.01f) { w = want; uF = g_hdF; uR = g_hdR; uU = g_hdU; }
                        else {   // only when reading his head starts / stops, or when what was learned changes
                            w += (want - w) * 0.25f;
                            uF += (g_hdF - uF) * 0.1f; uR += (g_hdR - uR) * 0.1f; uU += (g_hdU - uU) * 0.1f;
                        }
                        float k = w * (1.0f - proneF);
                        if (k > 0.001f) {
                            u16 by = *(u16*)(plp + 0x162);
                            float bfx = sin16(by), bfz = sin16((u16)(by + 0x4000)), brx = -bfz, brz = bfx;
                            // firing, the gun's kick in his firing motion moves his head.
                            float hx, hy2, hz;
                            {   static float rF, rR, rU, fz; static int haveR; static DWORD logT, ranT; static float sx, sz, sF, sR, sU;
                                float px0 = *(float*)(plp + 0x130), pz0 = *(float*)(plp + 0x138);
                                if (GetTickCount() - ranT > 200) { fz = 0; haveR = 0; logT = 0; }
                                ranT = GetTickCount();
                                float ox = g_head[0] - px0, oz = g_head[2] - pz0;
                                float cF = ox * bfx + oz * bfz, cR = ox * brx + oz * brz, cU = g_head[1] - posYs;
                                DWORD tn = GetTickCount();
                                int firing = g_aimLatch && (g_fireNow || (g_shotT && tn - g_shotT < 600));
                                if (firing && haveR) { if (fz < 1.0f && !logT) { logT = tn ? tn : 1; sx = px0; sz = pz0; sF = cF; sR = cR; sU = cU; g_dragMax = 0; } fz = 1.0f; g_shotHold++; }
                                else if (fz > 0.0f) { fz -= 0.08f; if (fz < 0.0f) fz = 0.0f; }
                                if (fz <= 0.0f) { rF = cF; rR = cR; rU = cU; haveR = 1; }
                                // the hold only takes out the kick. The kick moves his head at most about 16 forward
                                // and 9 - 23 down; the place kept now never differs from his head's real place by more
                                // than HeadHoldUnits (24) - anything beyond that, the camera follows at once.
                                if (fz > 0.0f && haveR) {
                                    float B = (float)cfgHeadHoldU, dr = 0;
                                    float lo, e;
                                    lo = cF - B; if (rF < lo) { e = lo - rF; if (e > dr) dr = e; rF = lo; } lo = cF + B; if (rF > lo) { e = rF - lo; if (e > dr) dr = e; rF = lo; }
                                    lo = cR - B; if (rR < lo) { e = lo - rR; if (e > dr) dr = e; rR = lo; } lo = cR + B; if (rR > lo) { e = rR - lo; if (e > dr) dr = e; rR = lo; }
                                    lo = cU - B; if (rU < lo) { e = lo - rU; if (e > dr) dr = e; rU = lo; } lo = cU + B; if (rU > lo) { e = rU - lo; if (e > dr) dr = e; rU = lo; }
                                    if (dr > 0.5f) { g_headDragN++; if ((int)dr > g_dragMax) g_dragMax = (int)dr; if ((int)dr > g_dragMaxAll) g_dragMaxAll = (int)dr; }
                                }
                                if (logT && tn - logT >= 500) {
                                    char b[260]; float mx = px0 - sx, mz = pz0 - sz;
                                    wsprintfA(b, "shot: in half a second Snake moved %d units, his head moved %d forward / %d right / %d up relative to him - the camera ignored at most %d of it (the kick) and followed the rest%s",
                                        (int)__builtin_sqrtf(mx * mx + mz * mz), (int)(cF - sF), (int)(cR - sR), (int)(cU - sU), cfgHeadHoldU, g_dragMax ? " (it did follow here)" : "");
                                    static int nl; if (nl < 12) { nl++; Log(b); }
                                    logT = 0;
                                }
                                float dF = (rF - cF) * fz, dR = (rR - cR) * fz;
                                hx = g_head[0] + bfx * dF + brx * dR; hz = g_head[2] + bfz * dF + brz * dR; hy2 = g_head[1] + (rU - cU) * fz;
                            }
                            float bx = hx - bfx * uF - brx * uR, bz = hz - bfz * uF - brz * uR;
                            float bodyH = (hy2 - uU - posYs) + 695.0f;
                            body += (bodyH - body) * k;
                            ancX += (bx - ancX) * k; ancZ += (bz - ancZ) * k;
                            g_lock = 2;
                        }
                        g_headUse100 = (int)(k * 100.0f);
                    }
                    g_body100 = (int)body;
                }
                pivotY = posYs + body + (float)cfgHeight;
                posYsOut = posYs;
                floorY = posYs + (float)cfgProneFloor;
            } else baseOk = 0;
        }
        {   // aim pitch = orbit pitch raised by PitchOffset (the orbit camera rests looking slightly down), then limited
            float so = sin_small((float)cfgPitchOff * 0.0174532925f), co = __builtin_sqrtf(1.0f - so * so);
            float h = __builtin_sqrtf(dx * dx + dz * dz);
            float s2 = dy * co + h * so;   // from the orbit camera's geometry
            float lim = sin_small((float)cfgPitch * 0.0174532925f), limD = sin_small((float)cfgPitchDown * 0.0174532925f);
            // The game also keeps the camera's elevation as one number (work+0x340, limits at +0x98 / +0x9C, 0.667
            // when the camera rests). Near the ground the orbit camera stops moving although that number still changes,
            // which felt like a dead zone at the top; mapping the number itself onto the aim pitch uses the whole
            // range.
            float ex = *(float*)(work + 0x340), emin = *(float*)(work + 0x98), emax = *(float*)(work + 0x9C);
            g_ex = (int)(ex * 1000.0f); g_emin = (int)(emin * 1000.0f); g_emax = (int)(emax * 1000.0f);
            if (cfgPitchMode == 1 && emax - emin > 0.05f && emax - emin < 100.0f && ex >= emin - 0.01f && ex <= emax + 0.01f) {
                float e0 = 0.6666667f;
                if (!(e0 > emin + 0.01f && e0 < emax - 0.01f)) e0 = emin + (emax - emin) * 0.6666667f;
                // smaller number = camera lower = looking up. The aim pitch is the plugin's own number, moved by how
                // much the game's elevation changes each frame. The game's value itself is left alone (it is only
                // pulled back to the middle when it reaches an end, so it can keep moving, and put back when the
                // shoulder view ends), because writing to it made the game's camera swing at the sky or the ground for
                // a moment when entering and leaving.
                // (1) the shoulder view starts at the pitch the game's camera had; (2) while it is on, the game's own
                // elevation is held still at one value and only its per-frame change is read. (3) on leaving, the
                // game's camera is handed the pitch the player ended at, so the normal view carries on from there.
                // the shoulder view starts at the pitch the game's camera really shows at that moment (taken from its
                // own eye and look-at points), and on leaving the game's camera is given the elevation that shows the
                // pitch the player ended at (table above).
                static int wasAiming; static float myDeg, xPin;
                float K = (float)cfgPitchDown / (emax - e0);
                float rng = emax - emin;
                int std = emin > -0.01f && emin < 0.01f && emax > 0.99f && emax < 1.01f;
                if (camOn && !wasAiming) {
                    myDeg = atan2_(dy, h) * 57.29578f + (float)cfgEntryPitch;
                    xPin = ex;
                    if (xPin < emin + 0.15f * rng) xPin = emin + 0.15f * rng;
                    if (xPin > emax - 0.15f * rng) xPin = emax - 0.15f * rng;
                    *(float*)(work + 0x340) = xPin; ex = xPin;
                }
                if (camOn && g_pitchSetOk) { myDeg = g_pitchSet + (float)cfgEntryPitch; g_pitchSetOk = 0; }
                if (aiming && (g_recUp > 0.001f || g_recUp < -0.001f)) { float a = g_recUp * 0.5f; myDeg += a; g_recUp -= a; }   // AK-47 climb
                if (!aiming) g_recUp = 0;
                if (camOn) {
                    float dl = xPin - ex;
                    // the pad's right stick: up looks up, whatever the game's own camera setting says (the game's
                    // normal camera moves the other way round, which in the shoulder view felt upside down)
                    if (cfgPadLookUp && PadLookOn() && (dl > 0.0001f || dl < -0.0001f)) {
                        float want = (g_ry > 0) == (cfgPadLookUp == 1) ? 1.0f : -1.0f;
                        if ((dl > 0 ? 1.0f : -1.0f) != want) { dl = -dl; g_padLookFix++; }
                    }
                    if (aiming && dl < 0.25f && dl > -0.25f) myDeg += dl * K;
                    if (myDeg > (float)cfgPitch) myDeg = (float)cfgPitch;
                    if (myDeg < -(float)cfgPitchDown) myDeg = -(float)cfgPitchDown;
                    *(float*)(work + 0x340) = xPin; ex = xPin;
                    {   // how far the game's camera height will have to move when the view ends
                        float xw = std ? ElevOfPitch(myDeg - (float)cfgEntryPitch) : e0 - myDeg / K;
                        if (xw < emin) xw = emin; if (xw > emax) xw = emax;
                        g_handGap = xw - xPin; if (g_handGap < 0) g_handGap = -g_handGap;
                    }
                } else if (wasAiming) {
                    float xo = std ? ElevOfPitch(myDeg - (float)cfgEntryPitch) : e0 - myDeg / K;
                    if (xo < emin) xo = emin; if (xo > emax) xo = emax;
                    // written in one go again. Now the blend waits (the view stays fully on the shoulder) until the
                    // game's camera is there - see outHold below.
                    *(float*)(work + 0x340) = xo; g_eBackN++;
                    { int jump = (int)((xo - ex) * 1000.0f); if (jump < 0) jump = -jump;
                      if (jump > 10 && g_eBackLog < 12) { g_eBackLog++; char b[220]; wsprintfA(b, "over-the-shoulder off: the game's camera height set from %d to %d while the view is still fully on the shoulder - the blend back starts once it is there", (int)(ex * 1000.0f), (int)(xo * 1000.0f)); Log(b); } }
                }
                wasAiming = camOn; g_corr = (int)myDeg; g_pitchDeg = myDeg;
                s2 = sin_small((myDeg + (aiming ? g_recTmpP : 0.0f)) * 0.0174532925f);   // plus the fading part of the kick
            }
            if (s2 > lim) s2 = lim; else if (s2 < -limD) s2 = -limD;
            float h2 = __builtin_sqrtf(1.0f - s2 * s2);
            dx = fx / hl * h2; dz = fz / hl * h2; dy = s2;
            if (aiming && (g_recTmpY > 0.001f || g_recTmpY < -0.001f)) {   // sideways jump of the recoil
                float sy = sin_small(g_recTmpY * 0.0174532925f), cy = __builtin_sqrtf(1.0f - sy * sy);
                float ndx = dx * cy + dz * sy, ndz = dz * cy - dx * sy; dx = ndx; dz = ndz;
            }
            g_recTmpP *= 0.5f; g_recTmpY *= 0.5f;   // a jolt, as in first person
            if (!aiming) { g_recTmpP = 0; g_recTmpY = 0; }
        }
        float rx = -fz / hl, rz = fx / hl;   // screen-right on the ground plane
        float side = (float)cfgSide * (cfgLeft ? -1.0f : 1.0f);
        // pivot: beside Snake's shoulder; the camera sits Back behind it and looks straight ahead
        float backNow = (float)cfgBack;
        {   // Lying down: looking down, the camera swings further out (and so higher) the further down the view
            // goes, so the ground around Snake comes into view, like the game's own camera does when it looks down.
            // From 10 degrees down to the lowest pitch it reaches ProneLookDownBack percent more distance.
            float k = 1.0f;
            if (proneF > 0.01f && cfgProneLook > 0 && g_pitchDeg < -10.0f && cfgPitchDown > 10) {
                float w = (-g_pitchDeg - 10.0f) / ((float)cfgPitchDown - 10.0f); if (w > 1.0f) w = 1.0f;
                k = 1.0f + (float)cfgProneLook / 100.0f * w * proneF;
                static int told; if (!told && w > 0.5f) { told = 1; Log("lying down, looking down: camera swings out to show the ground around Snake"); }
            }
            backNow *= k; g_proneBack100 = (int)(k * 100.0f);
        }
        {   // the camera does not go through walls, trees or the ground. Two stretches are checked each
            // frame: from above Snake sideways to the shoulder point, and from there back to the camera.
            // smooth, like the game's own camera.
            // Now: the tests run from points that do not depend on the result; the camera comes in quickly but over a
            // few frames (CollisionInPercent of the way each frame), goes back out only once the way has been clear for
            // CollisionHoldFrames frames in a row, and then slowly (CollisionOutPercent); small changes are ignored.
            // Pulled in close, the camera still comes toward the middle so Snake stays in the picture, but only as a
            // last step - it does not feed back into the tests.
            static float sideK = 1.0f, backK = 1.0f; static int sideOut, backOut;
            float ks = 1.0f, kb = 1.0f;
            float kMin = (float)cfgCamMinBack / (backNow > 1.0f ? backNow : 1.0f); if (kMin > 1.0f) kMin = 1.0f; if (kMin < 0.1f) kMin = 0.1f;
            if (cfgCollide && c_test && c_areaArg && g_amt > 0.2f) {
                int mode = *(u32*)(work + 0x60) != 0;
                g_camMode = mode; g_camOnlyNow = 0;   // the game camera's mode picks its surface flags
                float minY = proneF > 0.5f ? floorY + 60.0f : -1e9f;
                float A[3] = { ancX, pivotY, ancZ }, P[3], E[3];
                float as = side < 0 ? -side : side, hs = 1.0f, hb = 1.0f;
                P[0] = A[0] + rx * side; P[1] = pivotY; P[2] = A[2] + rz * side;
                if (as > 1.0f) {
                    float d = Sweep(mode, A, P, minY);
                    if (d >= 0) {
                        ks = (d - 120.0f) / as; if (ks < 0) ks = 0; if (ks > 1.0f) ks = 1.0f;
                        hs = (d - 30.0f) / as; if (hs < 0) hs = 0; if (hs > 1.0f) hs = 1.0f;
                    }
                }
                sideK = CamEase(sideK, ks, hs, &sideOut);
                P[0] = A[0] + rx * side * sideK; P[2] = A[2] + rz * side * sideK;
                E[0] = P[0] - dx * backNow; E[1] = P[1] - dy * backNow; E[2] = P[2] - dz * backNow;
                float d = Sweep(mode, P, E, minY);
                if (d >= 0) {
                    kb = (d - 120.0f) / backNow; if (kb < kMin) kb = kMin; if (kb > 1.0f) kb = 1.0f;
                    hb = (d - 30.0f) / backNow; if (hb < kMin) hb = kMin; if (hb > 1.0f) hb = 1.0f;
                }
                backK = CamEase(backK, kb, hb, &backOut);
            } else { sideK = CamEase(sideK, 1.0f, 1.0f, &sideOut); backK = CamEase(backK, 1.0f, 1.0f, &backOut); }
            // pulled in close (a wall behind him) the camera comes toward the middle so Snake stays in the picture
            float g = backK < 0.6f ? backK / 0.6f : 1.0f; if (g < 0.3f) g = 0.3f;
            side *= sideK * g; backNow *= backK;
            g_sideK100 = (int)(sideK * 100.0f); g_backK100 = (int)(backK * 100.0f);
            if (g_camOnlyNow && (sideK < 0.995f || backK < 0.995f)) g_camOnlyN++;   // frames held in by a surface only the camera sees
        }
        float px = ancX + rx * side, py = pivotY, pz = ancZ + rz * side;
        float ne[3], nt[3];
        ne[0] = px - dx * backNow; ne[1] = py - dy * backNow; ne[2] = pz - dz * backNow;
        nt[0] = ne[0] + dx * (float)cfgLook; nt[1] = ne[1] + dy * (float)cfgLook; nt[2] = ne[2] + dz * (float)cfgLook;
        {   // a wall beside the camera (a narrow passage, the camera along a wall) came closer than the picture's
            // near edge and showed through. The camera keeps WallDistance from walls to its left and right (two short
            // tests, the same kind the shots use); between two close walls it stays in the middle.
            static float pushS;
            float push = 0;
            if (cfgCollide && cfgCamRadius > 0 && c_test && g_amt > 0.2f && Readable(g_pl, 0x800)) {
                u32 ab = *(u32*)(g_pl + 0x120); float R = (float)cfgCamRadius;
                float rr[3] = { rx, 0, rz }, ll[3] = { -rx, 0, -rz };
                float hr = CamRay(ab, ne, rr, R), hl = CamRay(ab, ne, ll, R);   // the camera's own flags (fence nets too)
                if (hr < R - 0.5f && hl < R - 0.5f) push = (hr - hl) * 0.5f;
                else if (hr < R - 0.5f) push = -(R - hr);
                else if (hl < R - 0.5f) push = R - hl;
            }
            // eased as well, small changes ignored
            {   static int pushOut; float dp = push - pushS, ap = push < 0 ? -push : push, as_ = pushS < 0 ? -pushS : pushS;
                if (ap > as_ + 4.0f) { pushS += dp * (float)cfgCamIn / 100.0f; pushOut = 0; }
                else if (ap < as_ - (push == 0 ? 0.01f : 6.0f)) {
                    if (++pushOut >= cfgCamHold) { float st = dp * (float)cfgCamOut / 100.0f; if (st > -1.0f && st < 1.0f) st = dp < 0 ? (dp < -1.0f ? -1.0f : dp) : (dp > 1.0f ? 1.0f : dp); pushS += st; }
                }
                else pushOut = 0;
                if (push == 0 && pushS > -0.5f && pushS < 0.5f) pushS = 0;
            }
            ne[0] += rx * pushS; ne[2] += rz * pushS; nt[0] += rx * pushS; nt[2] += rz * pushS;
            g_wallPush = (int)pushS;
            float hx = ne[0] - g_head[0], hy = ne[1] - g_head[1], hz = ne[2] - g_head[2];
            g_camHead = g_headOk ? (int)__builtin_sqrtf(hx * hx + hy * hy + hz * hz) : -1;
        }
        if (proneF > 0.01f && ne[1] < floorY) { float lift = (floorY - ne[1]) * proneF; ne[1] += lift; nt[1] += lift; }   // lying down: keep the camera above the ground
        for (int i = 0; i < 3; i++) { ne[i] = eye[i] + (ne[i] - eye[i]) * g_amt; nt[i] = tgt[i] + (nt[i] - tgt[i]) * g_amt; }
        g_aimDir[0] = dx; g_aimDir[1] = dy; g_aimDir[2] = dz; g_aimDirOk = 1;
        {   // ---- where the shot goes
            float pv[3], dv[3], rv[3], uv[3];
            pv[0] = px; pv[1] = ne[1] + dy * backNow; pv[2] = pz; dv[0] = dx; dv[1] = dy; dv[2] = dz;
            rv[0] = rx; rv[1] = 0; rv[2] = rz;   // screen right
            uv[0] = -rz * dy; uv[1] = rz * dx - rx * dz; uv[2] = rx * dy;   // screen up
            u8* plp = g_pl;
            static int blk, blkN, clrN; static float dist;
            float farD = -1.0f;   // distance from the pivot to what the crosshair covers
            if (aiming && g_amt > 0.9f && c_test && (cfgShots || cfgSight) && baseOk && Readable(plp, 0x800)) {
                // The middle of the screen is a line from the camera. Everything the player aims at lies on it, so that
                // line is what a shot has to follow (the game's own first-person shots work the same way). The gun is
                // beside that line, so three things are checked each frame: 1. what the line hits first, starting level
                // with the muzzle -> the point aimed at 2. whether something stands between the muzzle and that point
                // -> the shot lands there instead 3. whether something stands between the muzzle and the line beside it
                // (a wall at his shoulder)
                int w = Wpn(plp), st = g_proneLatch ? 2 : g_crouchLatch ? 1 : 0;
                const float* mo = MuzzleOff(w, st);
                float q[3], M[3], S[3], T[3];
                q[0] = *(float*)(plp + 0x130); q[1] = posYs + baseH; q[2] = *(float*)(plp + 0x138);
                for (int i = 0; i < 3; i++) M[i] = q[i] + dv[i] * mo[0] + rv[i] * mo[1] + uv[i] * mo[2];
                {   // an estimated muzzle is never put below the ground (that would make every shot look
                    // blocked); lying down without a measured value it sits just above the ground
                    u32 abg = *(u32*)(plp + 0x120);
                    float down[3] = { 0, -1.0f, 0 };
                    float gd = Ray(abg, q, down, 3000.0f);
                    if (gd < 2999.0f) {
                        float gy = q[1] - gd;
                        if (g_mzEst && st == 2) M[1] = gy + 150.0f;
                        else if (M[1] < gy + 60.0f) M[1] = gy + 60.0f;
                    }
                }
                float dm = (M[0] - ne[0]) * dx + (M[1] - ne[1]) * dy + (M[2] - ne[2]) * dz;   // how far along the line the muzzle is
                if (dm < backNow * 0.5f) dm = backNow * 0.5f;
                if (dm > backNow + 2000.0f) dm = backNow + 2000.0f;
                for (int i = 0; i < 3; i++) S[i] = ne[i] + dv[i] * dm;
                u32 ab = *(u32*)(plp + 0x120);
                float range = (float)cfgConvMax;
                float dC;
                {   static float farD_, lastS[3], lastV[3]; static int age;
                    float nearR = range < 8000.0f ? range : 8000.0f;
                    dC = Ray(ab, S, dv, nearR);
                    if (dC >= nearR - 1.0f && range > nearR) {
                        float mv = (S[0] - lastS[0]) * (S[0] - lastS[0]) + (S[1] - lastS[1]) * (S[1] - lastS[1]) + (S[2] - lastS[2]) * (S[2] - lastS[2]);
                        float dd = dv[0] * lastV[0] + dv[1] * lastV[1] + dv[2] * lastV[2];
                        --age;
                        if (age <= 0 || (age <= 2 && (mv > 150.0f * 150.0f || dd < 0.99985f))) {   // at most every 2nd frame, at least every 4th
                            float S2[3] = { S[0] + dv[0] * nearR, S[1] + dv[1] * nearR, S[2] + dv[2] * nearR };
                            farD_ = nearR + Ray(ab, S2, dv, range - nearR); age = 4;
                            for (int i = 0; i < 3; i++) { lastS[i] = S[i]; lastV[i] = dv[i]; }
                        }
                        dC = farD_;
                    } else age = 0;
                }
                for (int i = 0; i < 3; i++) T[i] = S[i] + dv[i] * dC;
                g_ctrDist = dC < range - 1.0f ? (int)dC : -1;
                farD = dC + dm - backNow;
                float gv[3] = { T[0] - M[0], T[1] - M[1], T[2] - M[2] };
                float L = __builtin_sqrtf(gv[0] * gv[0] + gv[1] * gv[1] + gv[2] * gv[2]);
                float dH = L; int blocked = 0, side = 1;
                // 2 and 3 need the real muzzle position, which is measured at each shot: until the first shot with this
                // weapon in this stance the shot simply follows the view line
                int known = 1;   // always (estimated until measured)
                if (L > 1.0f && known) {
                    gv[0] /= L; gv[1] /= L; gv[2] /= L;
                    float Lg = L < 8000.0f ? L : 8000.0f;   // only what is near the gun can block it
                    dH = Ray(ab, M, gv, Lg); if (dH >= Lg - 1.0f) dH = L;
                    float tol = L * 0.03f; if (tol < 200.0f) tol = 200.0f;
                    blocked = dH < L - tol;
                    float sv[3] = { S[0] - M[0], S[1] - M[1], S[2] - M[2] };
                    float sl = __builtin_sqrtf(sv[0] * sv[0] + sv[1] * sv[1] + sv[2] * sv[2]);
                    if (sl > 30.0f) { sv[0] /= sl; sv[1] /= sl; sv[2] /= sl; side = Ray(ab, M, sv, sl) >= sl - 1.0f; }
                }
                // two frames in a row before the answer changes, so one odd frame cannot make the marker flicker
                if (blocked) { blkN++; clrN = 0; } else { clrN++; blkN = 0; }
                if (blkN >= 2) blk = 1; else if (clrN >= 2) blk = 0;
                float sx = 0, sy = 0; int off = 0;
                if (blk && blocked) {
                    // the shot stops short: show where, through the camera
                    float hx = M[0] + gv[0] * dH - ne[0], hy = M[1] + gv[1] * dH - ne[1], hz = M[2] + gv[2] * dH - ne[2];
                    float Z = hx * dx + hy * dy + hz * dz;
                    if (Z > 200.0f) {
                        u8* cam = *(u8**)(work + 0xE0);
                        float zm = Readable(cam, 0x80) ? *(float*)(cam + 0x7C) : *(float*)(work + 0xD8);
                        if (!(zm > 0.3f && zm < 30.0f)) zm = 1.9f;
                        float k = zm * 0.5f * (float)cfgFov / 100.0f / Z;
                        sx = (hx * rx + hz * rz) * k; sy = -(hx * uv[0] + hy * uv[1] + hz * uv[2]) * k;
                        if (sx > 0.8f) sx = 0.8f; if (sx < -0.8f) sx = -0.8f;
                        if (sy > 0.45f) sy = 0.45f; if (sy < -0.45f) sy = -0.45f;
                        off = 1;
                    }
                }
                for (int i = 0; i < 3; i++) { g_tEye[i] = ne[i]; g_tF[i] = dv[i]; g_tR[i] = rv[i]; g_tU[i] = uv[i]; g_tQ[i] = q[i]; g_tPt[i] = T[i]; }
                g_tMuzzle = blk || !side; g_sideClear = side; g_tFrame = g_frame;
                g_sx = sx; g_sy = sy; g_ghost = off;
                float ld = blk && blocked ? dH : L;
                g_landDist = (int)ld;
                // close in front of the gun: the 3DS crosshair opens wide (about 2.6 times with the gun at a trunk)
                // the ground he is looking down at is not an obstacle - lying down and looking down, the shot lands
                // close in front of the gun and the crosshair kept opening wider the further down he looked
                float ly = blk && blocked ? M[1] + gv[1] * dH : T[1];
                int ground = dy < -0.17f && ly < M[1] - 100.0f;
                g_obsMm = cfgObstacle && !ground && ld < (float)cfgObsRange ? (int)ld : -1;
                if (g_obsMm >= 0) g_obsHits++;
            } else { blk = blkN = clrN = 0; g_sx = g_sy = 0; g_ghost = 0; g_obsMm = -1; g_ctrDist = g_landDist = -1; g_tMuzzle = 0; }
            // where Snake's upper body points. The shot itself does not depend on this, so it only has to look right:
            // 1 = a steady point ConvergeDistance along the view line, 2 = what the crosshair covers (between 4 and 40
            // m, eased so that the arms never jerk), 0 = parallel to the camera.
            // 3 = as on the 3DS - exactly what the crosshair covers, near or far: the shot comes from the middle of the
            // screen, and his body and gun turn to point at the very spot it lands on. A post right beside him turns
            // him toward it. Eased like 2. His upper body may twist up to PoseTurn degrees for it (the game itself
            // allows 15 - see hk_AimAdj).
            if (aiming && cfgConverge && g_amt > 0.9f) {
                float d = (float)cfgConvDist;
                if (cfgConverge == 2 && farD > 0) { d = farD; if (d < 4000.0f) d = 4000.0f; if (d > 40000.0f) d = 40000.0f; }
                if (cfgConverge == 3) { d = farD > 0 ? farD : (float)cfgConvMax; if (d < 600.0f) d = 600.0f; if (d > 40000.0f) d = 40000.0f; }
                // looking down, the 3DS Snake turns his upper body and the gun to the right, toward the spot on the
                // ground the crosshair covers (the view line runs beside him, so a near spot on it lies well to his
                // right). The fixed far point above cannot show that, so as the view tilts below 15 degrees down the
                // point slides in to what the crosshair really covers (fully from 40 degrees down, never nearer than
                // 1.5 m). It is what his body aims at, so it works with every gun.
                int ease = cfgConverge >= 2;
                if (cfgLookDown && cfgConverge == 1 && farD > 0 && g_pitchDeg < -15.0f) {
                    float w = (-g_pitchDeg - 15.0f) / 25.0f; if (w > 1.0f) w = 1.0f;
                    float dn = farD; if (dn < 1500.0f) dn = 1500.0f; if (dn > d) dn = d;
                    d = 1.0f / ((1.0f - w) / d + w / dn); ease = 1;
                }
                // throwables (grenades, magazine ...) - the upper body does not follow the view down past
                // ThrowableLookDown degrees: bending down with one of them in hand pushed his arms out behind him and
                // looked stiff. No look-down turn for them either.
                float ax = dx, ay = dy, az = dz;
                g_throwClamp = 0;
                if (Throwable(Wpn(g_pl))) {
                    float lim = -sin_small((float)cfgThrowDown * 0.0174532925f);
                    if (ay < lim) {
                        float h0 = __builtin_sqrtf(1.0f - ay * ay), h1 = __builtin_sqrtf(1.0f - lim * lim);
                        if (h0 > 0.01f) { ax *= h1 / h0; az *= h1 / h0; } ay = lim; g_throwClamp = 1;
                        static int told; if (!told) { told = 1; Log("throwable in hand, looking down: upper body kept level"); }
                    }
                    d = (float)cfgConvDist; ease = 0;
                }
                if (d < (cfgConverge == 3 && !g_proneLatch ? 600.0f : 1500.0f)) d = cfgConverge == 3 && !g_proneLatch ? 600.0f : 1500.0f;
                static int eased;
                if (!g_aimPtOk || !(ease || eased)) dist = d; else dist = 1.0f / (1.0f / dist + (1.0f / d - 1.0f / dist) * (cfgConverge == 2 ? 0.12f : 0.2f));
                eased = ease || (eased && (dist < d * 0.98f));
                g_aimPt[0] = pv[0] + ax * dist; g_aimPt[1] = pv[1] + ay * dist; g_aimPt[2] = pv[2] + az * dist;
                g_aimPtOk = 1; g_convDist = (int)dist;
                PoseFix(g_pl);   // the barrel onto the spot, vertically
            } else { g_aimPtOk = 0; g_pfUse = 0; }
        }
        if (g_fly && Readable(g_pl, 0x800)) {
            // the switch to and from first person. The point the first-person camera uses is learned each time first
            // person is on (per stance, relative to Snake's body point); until then a guess just above it.
            u8* plh = g_pl;
            int stx = g_proneLatch ? 2 : g_crouchLatch ? 1 : 0;
            float fwx = fx / hl, fwz = fz / hl;   // ahead, level
            float B[3] = { *(float*)(plh + 0x130), posYsOut + (float)g_body100, *(float*)(plh + 0x138) };
            float H[3] = { B[0] + fwx * g_hoF[stx] + rx * g_hoR[stx], B[1] + g_hoU[stx], B[2] + fwz * g_hoF[stx] + rz * g_hoR[stx] };
            DWORD nowF = GetTickCount();
            float u = g_flyDur ? (float)(nowF - g_flyT) / (float)g_flyDur : 1.0f; if (u < 0) u = 0; if (u > 1.0f) u = 1.0f;
            float sm = u * u * (3.0f - 2.0f * u);
            float d3[3] = { dx, dy, dz };
            if (g_fly == 1 || g_fly == 3) {
                // stop 120 short of the eye point, behind the head: first person takes over from there
                // mode 3 is the same path walked back, from the share of the way reached (g_fly3K) to the shoulder
                float E1[3] = { H[0] - d3[0] * 120.0f, H[1] - d3[1] * 120.0f, H[2] - d3[2] * 120.0f };
                float k = g_fly == 1 ? sm : g_fly3K * (1.0f - sm);
                if (g_fly == 1) g_fly3K = sm;   // the share reached, for a return from here
                for (int i = 0; i < 3; i++) { ne[i] += (E1[i] - ne[i]) * k; nt[i] = ne[i] + d3[i] * (float)cfgLook; }
                if (g_fly == 3 && u >= 1.0f) g_fly = 0;
            } else {
                float S0[3], D0[3];
                for (int i = 0; i < 3; i++) { S0[i] = g_fpOk ? g_fpE[i] : H[i]; D0[i] = g_fpOk ? g_fpD[i] : d3[i]; }
                {   // start 450 units out of his head along the way
                    float L = __builtin_sqrtf((ne[0] - S0[0]) * (ne[0] - S0[0]) + (ne[1] - S0[1]) * (ne[1] - S0[1]) + (ne[2] - S0[2]) * (ne[2] - S0[2]));
                    if (L > 1.0f) { float k = 450.0f / L; if (k > 0.4f) k = 0.4f; for (int i = 0; i < 3; i++) S0[i] += (ne[i] - S0[i]) * k; }
                }
                float Dm[3], dlm = 0;
                for (int i = 0; i < 3; i++) { Dm[i] = D0[i] + (d3[i] - D0[i]) * sm; dlm += Dm[i] * Dm[i]; }
                dlm = __builtin_sqrtf(dlm); if (dlm < 0.001f) dlm = 1.0f;
                for (int i = 0; i < 3; i++) { ne[i] = S0[i] + (ne[i] - S0[i]) * sm; nt[i] = ne[i] + Dm[i] / dlm * (float)cfgLook; }
                if (u >= 1.0f) g_fly = 0;
            }
            *(u32*)(work + 0x320) |= 1u;   // the game's camera follows exactly this frame (no smoothing lag)
        }
        if (Bit(ST_FPV) && g_fpOk && Readable(g_pl, 0x800) && g_body100 > 0) {
            // learn where the first-person eye sits relative to Snake's body point (for the next flight)
            u8* plh = g_pl; int stx = g_proneLatch ? 2 : g_crouchLatch ? 1 : 0;
            float hx = g_fpD[0], hz = g_fpD[2], hh = __builtin_sqrtf(hx * hx + hz * hz);
            if (hh > 0.2f) {
                hx /= hh; hz /= hh;
                float ox = g_fpE[0] - *(float*)(plh + 0x130), oy = g_fpE[1] - (posYsOut + (float)g_body100), oz = g_fpE[2] - *(float*)(plh + 0x138);
                float F = ox * hx + oz * hz, R = ox * hz - oz * hx;   // right = (fz, -fx)
                float k = g_hoOk[stx] ? 0.1f : 1.0f;
                g_hoF[stx] += (F - g_hoF[stx]) * k; g_hoU[stx] += (oy - g_hoU[stx]) * k; g_hoR[stx] += (R - g_hoR[stx]) * k;
                if (!g_hoOk[stx]) { g_hoOk[stx] = 1; char b[120]; wsprintfA(b, "first-person eye learned (stance %d): ahead %d, up %d, right %d of the body point", stx, (int)F, (int)oy, (int)R); Log(b); }
            }
        }
        memcpy(eye, ne, 12); memcpy(tgt, nt, 12);
        memcpy(c_me, ne, 12); memcpy(c_mt, nt, 12); c_work = work; c_valid = 1;

        u8* pl = g_pl;
        u16 yaw = (u16)(int)(camYaw / 6.2831853f * 65536.0f);
        g_camYaw16 = yaw; g_camValid = 1;
        {   // how fast the view pans sideways, for the crosshair (the 3DS sight tilts and opens while the view pans;
            // looking up and down does neither). Turning right makes the yaw number smaller.
            static u16 py; static float tr; static int have;
            if (have && aiming) {
                float v = -(float)(i16)(u16)(yaw - py) * (360.0f / 65536.0f) * 60.0f;
                if (v > 2000.0f) v = 2000.0f; if (v < -2000.0f) v = -2000.0f;
                tr += (v - tr) * 0.3f;
            } else tr = 0;
            py = yaw; have = 1; g_panDps = tr;
        }
        if (aiming && !g_wallFrames && Readable(pl, 0x800)) {
            *(u16*)(pl + 0x16A) = yaw;   // Snake faces where the camera looks
            if (Strafe()) *(u16*)(pl + 0x162) = yaw;
        }
    } else { g_camValid = 0; g_aimDirOk = 0; g_aimPtOk = 0; g_pfUse = 0; }
    {   // zoom: while the zoom key is held in the shoulder view the camera's zoom value is scaled
        static float zf, zLast, zBase; static int zWrote;
        float z = *(float*)(work + 0xD8);
        if (!zWrote || z != zLast) zBase = z;   // the game's own value (it changed it, or the plugin is not writing)
        // the zoom key only counts once held for ZoomKeyDelay ms, never during a switch, and the zoom drops at once
        // when the shoulder view ends.
        static DWORD zDown;
        int zraw = cfgZoomKey && g_aimLatch && !g_hold && !g_fly && (GetAsyncKeyState(cfgZoomKey) & 0x8000);
        DWORD zn = GetTickCount();
        if (zraw) { if (!zDown) zDown = zn ? zn : 1; } else zDown = 0;
        int zk = zraw && zn - zDown >= (DWORD)cfgZoomDelay;
        if (!g_aimLatch || g_hold || g_fly) zf = 0;
        zf += ((zk ? 1.0f : 0.0f) - zf) * 0.25f;
        g_zoom100 = (int)(zBase * 100.0f);
        float mult = 1.0f + ((float)cfgZoomPct / 100.0f - 1.0f) * zf;
        // the game scales its zoom as its own camera tilts down past the resting angle (exe+2131FB: zoom x (1 +
        // (elevation - 2/3) x 3 x (work+0x94 - 1))). In the shoulder view that camera is parked at whatever angle it
        // had when aiming started, so the view came out a little different each time. The factor is taken out again
        // while the shoulder view is on.
        float k94 = *(float*)(work + 0x94), exz = *(float*)(work + 0x340);
        if (g_amt > 0 && k94 > 0.2f && k94 < 5.0f && exz > 0.6666667f) {
            float t = (exz - 0.6666667f) * 3.0f; if (t > 1.0f) t = 1.0f;
            float comp = 1.0f + t * (k94 - 1.0f);
            if (comp > 0.2f) mult /= 1.0f + (comp - 1.0f) * g_amt;
        }
        g_zoomMul = (int)(mult * 1000.0f);
        if (mult > 1.0005f || mult < 0.9995f) { zLast = zBase * mult; *(float*)(work + 0xD8) = zLast; zWrote = 1; }
        else if (zWrote) { *(float*)(work + 0xD8) = zBase; zWrote = 0; }
    }
    {   // end of frame: give the game its input back (it normally rewrites it anyway) and start a new frame
        u8* pl = g_pl;
        if (g_have && !g_hidOn && Readable(pl, 0x800) && *(u16*)(pl + 0x7E0) == 0 && !(*(u32*)(pl + 0x7E8) & DIRBITS)) {
            *(u16*)(pl + 0x7E0) = g_inMag; *(u32*)(pl + 0x7E8) = g_inFl;
        }
        g_have = 0; g_phaseN = 0; g_aimSeen = 0;
        // the gun's kick fades as the game fades it (exe+37A5B9, once a frame); a shoulder-view shot the game rolled no
        // recoil for gets one rolled by the plugin
        g_kP = g_aimLatch ? Fade8(g_kP) : 0; g_kY = g_aimLatch ? Fade8(g_kY) : 0;
        if (g_otsShotFrame != 0x80000000u) {
            if (g_rkN != g_rkAtShot || g_rkFrame == g_otsShotFrame || g_rkFrame + 1 == g_otsShotFrame) g_otsShotFrame = 0x80000000u;   // rolled (before or after the bullet)
            else if (g_frame != g_otsShotFrame) { RecoilEmulate(Wpn(g_pl)); g_otsShotFrame = 0x80000000u; }
        }
        g_frame++;
    }
    {   // the game moves its camera only part of the way to where it should be each frame (work+0xA0, exe+213160),
        // so in the shoulder view the camera trailed Snake by a few frames - he seemed to lag behind the camera and
        // then catch up. While the shoulder view is on that smoothing is lifted (gradually as the view swings in, so
        // there is no jump); the game's value is put back right after.
        float k0 = *(float*)(work + 0xA0);
        int kw = cfgRigid && g_amt > 0.001f && k0 > 0.0f && k0 < 1.0f;
        g_smooth1000 = (k0 > -10.0f && k0 < 10.0f) ? (int)(k0 * 1000.0f) : -9999;
        if (kw) {
            float t = g_amt >= 0.97f ? 1.0f : g_amt;
            float k = k0 + (1.0f - k0) * t;
            *(float*)(work + 0xA0) = k; g_rigid1000 = (int)(k * 1000.0f);
            static int told; if (!told && t >= 1.0f) { told = 1; char b[160]; wsprintfA(b, "rigid camera: the game's camera smoothing (%d/1000 per frame) is lifted in the shoulder view - the camera is locked to Snake", (int)(k0 * 1000.0f)); Log(b); }
        } else g_rigid1000 = g_smooth1000;
        LARGE_ t1; QueryPerformanceCounter(&t1);
        u64 dt = (u64)(t1.q - t0.q); g_perfApply += dt; g_perfN++;
        if (dt > g_perfApplyMax) { g_perfApplyMax = dt; g_maxTag = (g_rowNow ? 1 : 0) | (g_logCalls - lc0 > (g_rowNow ? 1u : 0u) ? 2 : 0) | ((u32)g_perfRayN != ray0 ? 4 : 0) | (g_amt > 0.01f ? 8 : 0); }
        g_rowNow = 0;
        o_Apply(work);
        if (kw) *(float*)(work + 0xA0) = k0;
        g_ePrev = *(float*)(work + 0x340); g_ePrevOk = g_amt <= 0.001f && !g_hold && !g_fly;
        SpeedTrack();
    }
    if (cfgPerfLog) {   // how much time the plugin takes, every 5 seconds (microseconds)
        static DWORD pt; DWORD nw = GetTickCount();
        if (!pt) pt = nw;
        if (nw - pt >= 5000 && g_perfN) {
            static LARGE_ fq; if (!fq.q) QueryPerformanceFrequency(&fq);
            double us = 1000000.0 / (double)(fq.q ? fq.q : 1);
            double fr = (double)g_perfN;
            int cam = (int)((double)g_perfApply / fr * us), hk[7];
            int all = cam;
            for (int k = 1; k < 7; k++) { hk[k] = (int)((double)g_tm[k] / fr * us); all += hk[k]; g_tm[k] = 0; }
            char b[520];
            wsprintfA(b, "perf: the plugin takes %d us a frame in all - camera %d (max %d), pad copy %d, movement %d+%d, motion requests %d, upper body %d, recoil %d; "
                "memory checks %d a frame taking %d us (%s); collision tests %d a frame taking %d us; pad read %d us (own thread); crosshair picture updates %d a second taking %d us each; frames %d a second; log lines dropped %u",
                all, cam, (int)((double)g_perfApplyMax * us), hk[1], hk[2], hk[6], hk[3], hk[4], hk[5],
                (int)(g_rdN / g_perfN), (int)((double)g_rdT / fr * us), cfgFastMem == 2 ? "IsBadReadPtr" : cfgFastMem ? "copy" : "VirtualQuery",
                (int)(g_perfRayN / g_perfN), (int)((double)g_perfRay / fr * us),
                g_padReadN ? (int)((double)g_padReadT / (double)g_padReadN * us) : -1,
                (int)(g_perfSightN * 1000 / (nw - pt)), g_perfSightN ? (int)((double)g_perfSight / (double)g_perfSightN * us) : 0,
                (int)(g_perfN * 1000 / (nw - pt)), g_ldrop);
            Log(b);
            {   // how evenly the game's frames come (time between two camera updates), and the plugin's slowest frame
                int avg = g_fiN ? (int)((double)g_fiSum / (double)g_fiN * us / 100.0) : 0;
                wsprintfA(b, "frame pacing: %u frames, average %d.%d ms, longest %d ms; %u over 25 ms, %u over 40 ms (game hitches); the plugin's slowest frame %d us%s%s%s%s",
                    g_fiN, avg / 10, avg % 10, (int)((double)g_fiMax * us / 1000.0), g_fi25, g_fi40, (int)((double)g_perfApplyMax * us),
                    (g_maxTag & 1) ? " (wrote a log row)" : "", (g_maxTag & 2) ? " (log lines)" : "", (g_maxTag & 4) ? " (collision tests)" : "", (g_maxTag & 8) ? " (shoulder view on)" : "");
                Log(b);
                g_fiN = 0; g_fiSum = 0; g_fiMax = 0; g_fi25 = g_fi40 = 0; g_maxTag = 0;
            }
            {
                static u32 last[36]; u32 cur[36] = { g_crawlSndF, g_crawlSndB, g_crawlSndS, g_crawlEvN, g_grassN, g_lowCrawlN, g_slideN, g_proneOkN, g_proneBadN, g_fpvOk, g_fpvBad, g_fpvSkipN, g_eBackN, g_outHoldN, g_fpvHeldN, g_cwCamoN,
                    g_fpCancelN, g_pressFailN, g_maskFreeN, g_noLatchN, g_stickFixN, g_dzWalkN, g_poseWideN, g_poseLimN, (u32)g_poseYawMax, g_gunAngN,
                    g_markN[0], g_markN[1], g_markN[2], g_markN[3], g_otsCamoN, g_gStalkN, g_l1FixN, g_bushN, g_eventN, g_aimKeyLearnN };
                int ch = 0; for (int i = 0; i < 36; i++) if (cur[i] != last[i]) ch = 1;
                if (ch) {
                    wsprintfA(b, "summary (moves): lying - crawl sound routine %u forward / %u backward / %u sideways frames, crawl sounds played %u, grass marks %u, gun-lowered requests replaced %u, slides %u, "
                        "direction checks OK %u / PROBLEM %u; back from first person - OK %u / wrong %u / camera turned since, not used %u; leaving the shoulder view - camera height handed over %u times, blend waited for it %u times;"
                        " first-person toggle held off after a load %u; camo changes %u",
                        cur[0], cur[1], cur[2], cur[3], cur[4], cur[5], cur[6], cur[7], cur[8], cur[9], cur[10], cur[11], cur[12], cur[13], cur[14], cur[15]);
                    Log(b);
                    // the new checks on a line of their own (the buffer is kept small: this is the largest stack frame)
                    wsprintfA(b, "summary (aim key): first person called off by the game - camera flown back %u times; aim key presses that did not raise the gun %u times (paused, weapon not written off); raised state let through the held-over mask %u times; weapons taken back from \"aim key does nothing\" %u",
                        cur[16], cur[17], cur[18], cur[19]);
                    Log(b);
                    {   u32 an = g_gunAngN, as = g_gunAngSum; int avg = an ? (int)(as / an) : -1;
                        wsprintfA(b, "summary (pad stick): pad stick - walked the stick's own way where the game's heading was stale %u frames, walked by a push inside the game's dead zone %u frames; upper body twisted past the game's 15 degrees %u frames (needed up to %u degrees), held at PoseTurn %u frames; shots %u, gun pointing off the shot's spot by %d.%d degrees on average",
                            cur[20], cur[21], cur[22], cur[24], cur[23], cur[25], avg < 0 ? -1 : avg / 10, avg < 0 ? 0 : avg % 10);
                    }
                    Log(b);
                    wsprintfA(b, "summary (shoulder view): shoulder view moving with the game's marks - run %u / walk %u / stalk %u / crouched %u frames; camo index read or changed in the shoulder view %u times;"
                        " the game's stalk input %u frames; pad button raised the gun after a load %u times; first person started by itself while lying %u times; event scenes %u; aim key learned %u",
                        cur[26], cur[27], cur[28], cur[29], cur[30], cur[31], cur[32], cur[33], cur[34], cur[35]);
                    Log(b); for (int i = 0; i < 36; i++) last[i] = cur[i];
                }
            }
            LogSumPose();
            LogSumAim();
            LogSumFpv();
            g_perfApply = g_perfApplyMax = g_perfRay = g_perfN = g_perfRayN = g_perfSight = g_perfSightN = 0; pt = nw;
            g_rdN = 0; g_rdT = 0; g_padReadT = g_padReadN = 0;
        }
    }
}

// ---------------------------------------------------------------- crosshair
// The 3DS game's aiming sight as it appears on screen (measured from captures of the 3DS game, not from its layout
// file, whose 45-degree turn and triangle sizes the game overrides): four small white squares with a thin grey edge, in
// a plus. It is shown in small click-through windows on top of the game, so no part of the renderer is touched.
// sharp squares instead of soft dots; the plus turns toward an X while the view pans sideways; the sight sits where the
// shot will land, and when that is not the middle of the screen a dim copy stays in the middle (all three as on the
// 3DS).
static void SightPut(u32* px, int S, int x, int y, float a, float c)   // a = opacity, c = brightness (1 = white)
{
    if (x < 0 || y < 0 || x >= S || y >= S || a <= 0.0f) return;
    if (a > 1.0f) a = 1.0f;
    u32 A = (u32)(a * 255.0f + 0.5f);
    if ((px[y * S + x] >> 24) >= A) return;
    u32 C = (u32)(a * c * 255.0f + 0.5f); if (C > A) C = A;
    px[y * S + x] = A << 24 | C << 16 | C << 8 | C;   // premultiplied
}
static void SightDraw(u32* px, int S, float u, float r, float tilt, float op)
{
    // each mark: a white square 3.6 units across with a grey edge half a unit wide (one unit = 1/640 of the picture
    // height). Standing upright the edges are put exactly on pixel boundaries, so nothing is blurred.
    static const float ox[4] = { 0, 1, 0, -1 }, oy[4] = { -1, 0, 1, 0 };
    static int pb[4][4], pbS;
    if (pbS != S) { for (int i = 0; i < S * S; i++) px[i] = 0; pbS = S; }
    else for (int k = 0; k < 4; k++) for (int y = pb[k][1]; y <= pb[k][3]; y++) for (int x = pb[k][0]; x <= pb[k][2]; x++)
        if (x >= 0 && y >= 0 && x < S && y < S) px[y * S + x] = 0;
    float sn = sin_small(tilt), cs = __builtin_sqrtf(1.0f - sn * sn);
    int straight = tilt > -0.004f && tilt < 0.004f;
    float hs = 1.8f * u, bw = 0.5f * u, c = (float)(S / 2);
    if (bw < 1.0f) bw = 1.0f;
    if (straight) { hs = (float)(int)(hs + 0.5f); if (hs < 2.0f) hs = 2.0f; bw = (float)(int)(bw + 0.5f); sn = 0; cs = 1.0f; }
    int rad = (int)((hs + bw) * 1.45f) + 2;
    for (int k = 0; k < 4; k++) {
        float bx = ox[k] * r, by = oy[k] * r;
        float cx = c + bx * cs - by * sn, cy = c + bx * sn + by * cs;   // turned clockwise by tilt
        if (straight) { cx = (float)(int)(cx + 0.5f); cy = (float)(int)(cy + 0.5f); }
        int x0 = (int)cx - rad, y0 = (int)cy - rad, x1 = (int)cx + rad, y1 = (int)cy + rad;
        pb[k][0] = x0; pb[k][1] = y0; pb[k][2] = x1; pb[k][3] = y1;
        for (int y = y0; y <= y1; y++) for (int x = x0; x <= x1; x++) {
            float dx = (float)x + 0.5f - cx, dy = (float)y + 0.5f - cy;
            float lx = dx * cs + dy * sn, ly = dy * cs - dx * sn;   // in the square's own axes
            if (lx < 0) lx = -lx; if (ly < 0) ly = -ly;
            float d = (lx > ly ? lx : ly) - hs;   // distance outside the white part
            float core = 0.5f - d, edge = 0.5f - (d - bw);
            if (core < 0) core = 0; if (core > 1.0f) core = 1.0f;
            if (edge < 0) edge = 0; if (edge > 1.0f) edge = 1.0f;
            float ring = (edge - core) * 0.65f;   // the edge: mid grey, a little see-through
            float a = core + ring;
            if (a > 0.003f) SightPut(px, S, x, y, a * op, (core + ring * 0.42f) / a);
        }
    }
}
static DWORD API SightThread(void* arg)
{
    (void)arg;
    Log("crosshair: 3DS-style squares");
    WNDCLASSA_ wc; memset(&wc, 0, sizeof wc);
    wc.proc = (void*)&DefWindowProcA; wc.inst = GetModuleHandleA(0); wc.name = "mgs3danimSight";
    RegisterClassA(&wc);
    // layered + click-through + topmost + never activated + not in the taskbar. Two windows showing the same picture:
    // the sight itself, and its dim copy for the middle of the screen.
    HANDLE wnd = CreateWindowExA(0x00080000u | 0x20u | 0x8u | 0x08000000u | 0x80u, wc.name, "", 0x80000000u, 0, 0, 16, 16, 0, 0, wc.inst, 0);
    HANDLE wndG = CreateWindowExA(0x00080000u | 0x20u | 0x8u | 0x08000000u | 0x80u, wc.name, "", 0x80000000u, 0, 0, 16, 16, 0, 0, wc.inst, 0);
    HANDLE dc = CreateCompatibleDC(0);
    if (!wnd || !dc) { Log("crosshair: could not create the overlay window"); return 0; }
    Log("crosshair: overlay window ready");
    HANDLE bmp = 0; u32* px = 0; int S = 0, shown = 0, shownG = 0, lastKey = -1, lastX = -99999, lastY = 0, lastGX = -99999, lastGY = 0, said = 0, redrawn = 0;
    float f = 0, fo = 1.0f, tilt = 0, ddx = 0, ddy = 0; DWORD lastT = GetTickCount();
    for (;;) {
        MSG_ m; while (PeekMessageA(&m, 0, 0, 0, 1)) DispatchMessageA(&m);
        DWORD now = GetTickCount(); float dt = (float)(now - lastT) * 0.001f; lastT = now; if (dt > 0.1f) dt = 0.1f;
        HANDLE gw = g_gameWnd;
        int want = cfgEnabled && cfgSight && g_aimLatch && g_fgNow && g_amt > 0.6f && now - g_applyT < 250 && gw && IsWindow(gw) && !IsIconic(gw)
            && GetForegroundWindow() == gw   // only while the game's own window is in front
            && !g_sightHide;   // crawling lying down / a laser-sight gun
        RECT_ rc; POINT_ org = { 0, 0 };
        if (want && !(GetClientRect(gw, &rc) && ClientToScreen(gw, &org) && rc.b - rc.t >= 200)) want = 0;
        if (!want) {
            if (shown) { ShowWindow(wnd, 0); shown = 0; }
            if (shownG) { ShowWindow(wndG, 0); shownG = 0; }
            f = 0; fo = 1.0f; tilt = 0; ddx = ddy = 0; Sleep(40); continue;
        }
        // It opens to 72 while Snake moves and to about 152 with the gun against a tree trunk.
        float H = (float)(rc.b - rc.t);
        float u = H / 640.0f * (float)cfgSightScale / 100.0f;
        int busy = (u32)(g_frame - g_moveFrame) <= 2;
        float mt = busy ? 72.0f / 59.0f : 1.0f;
        if (g_fireNow) f = 1.0f; else { f -= dt * 3.0f; if (f < 0) f = 0; }
        if (1.0f + 0.5f * f > mt) mt = 1.0f + 0.5f * f;
        float fmax = (float)cfgObsMax / 100.0f;
        int om = g_obsMm;
        if (om >= 0) { float ft = (float)cfgObsNear / (float)(om > 50 ? om : 50); if (ft > fmax) ft = fmax; if (ft > mt) mt = ft; }
        fo += (mt - fo) * (mt > fo ? (dt * 10.0f > 1.0f ? 1.0f : dt * 10.0f) : (dt * 6.0f > 1.0f ? 1.0f : dt * 6.0f));
        if (fo < 1.0f) fo = 1.0f;
        // Tilt: while the view pans sideways the plus turns the same way, up to a full X (45 degrees) at TiltSpeed
        // degrees a second, and settles back in about a quarter of a second when the pan stops; it also opens a little
        // as it turns. Looking up and down does neither.
        float tt = 0;
        if (cfgTilt) { tt = g_panDps / (float)cfgTiltSpeed; if (tt > 1.0f) tt = 1.0f; if (tt < -1.0f) tt = -1.0f; tt *= 0.7853982f; }
        {   float away = (tt > 0 ? tt : -tt) > (tilt > 0 ? tilt : -tilt) && tt * tilt >= 0;
            float k = dt * (away ? 14.0f : 4.0f); if (k > 1.0f) k = 1.0f;
            tilt += (tt - tilt) * k;
            if (tt == 0 && tilt > -0.003f && tilt < 0.003f) tilt = 0;
        }
        float tq = tilt / 0.7853982f;
        float gap = 59.0f * (float)cfgSightGap / 100.0f;
        float rr = gap * fo * (1.0f + 0.55f * tq * tq);
        if (rr > gap * 3.0f) rr = gap * 3.0f;
        float r = rr * u;
        int need = ((int)(rr * u + 8.0f * u) + 4) * 2; need = (need + 63) & ~63; if (need > 2400) need = 2400;
        if (need < S && need >= S - 64) need = S;   // do not shrink by one step and back
        if (need != S) {
            if (bmp) { DeleteObject(bmp); bmp = 0; }
            BMI_ bi; memset(&bi, 0, sizeof bi); bi.size = 40; bi.w = need; bi.h = -need; bi.planes = 1; bi.bpp = 32;
            void* bits = 0; bmp = CreateDIBSection(dc, &bi, 0, &bits, 0, 0);
            if (!bmp || !bits) { Log("crosshair: could not create its picture"); return 0; }
            SelectObject(dc, bmp); px = (u32*)bits; S = need; lastKey = -1;
            for (int i = 0; i < S * S; i++) px[i] = 0;
        }
        // where it sits: the middle of the screen, or where the shot stops when something is in the way of the gun
        float tx = g_sx * H, ty = g_sy * H;
        {   float k = dt * 30.0f; if (k > 1.0f || !shown) k = 1.0f;
            ddx += (tx - ddx) * k; ddy += (ty - ddy) * k;
            if (tx == 0 && ty == 0 && ddx > -0.5f && ddx < 0.5f && ddy > -0.5f && ddy < 0.5f) ddx = ddy = 0;
        }
        int key = ((int)(r + 0.5f) * 512 + (int)(tilt * 120.0f) + 256) * 256 + (cfgSightOpacity & 255);   // whole pixels
        int cx0 = org.x + (rc.r - rc.l) / 2 - S / 2, cy0 = org.y + (rc.b - rc.t) / 2 - S / 2;
        int x = cx0 + (int)(ddx + (ddx < 0 ? -0.5f : 0.5f)), y = cy0 + (int)(ddy + (ddy < 0 ? -0.5f : 0.5f));
        redrawn = 0;
        if (key != lastKey) { SightDraw(px, S, u, r, tilt, (float)cfgSightOpacity / 100.0f); lastKey = key; redrawn = 1; }
        if (redrawn || !shown) {
            LARGE_ a0, a1; QueryPerformanceCounter(&a0);
            POINT_ dst = { x, y }, size = { S, S }, src = { 0, 0 }; u32 bf = 0x01FF0000u;   // AC_SRC_OVER, 255, AC_SRC_ALPHA
            UpdateLayeredWindow(wnd, 0, &dst, &size, dc, &src, 0, &bf, 2);
            QueryPerformanceCounter(&a1); g_perfSight += (u64)(a1.q - a0.q); g_perfSightN++;
            lastX = x; lastY = y;
        } else if (x != lastX || y != lastY) {   // only moved - no new picture
            SetWindowPos(wnd, 0, x, y, 0, 0, 0x1 | 0x4 | 0x10);   // no size, no z-order, not activated
            lastX = x; lastY = y;
        }
        if (!shown) {
            SetWindowPos(wnd, (HANDLE)(i64)-1, 0, 0, 0, 0, 0x1 | 0x2 | 0x10 | 0x40);   // topmost, shown, not activated
            shown = 1;
            if (!said) { said = 1; char b[160]; wsprintfA(b, "crosshair: shown, game area %dx%d at %d,%d, picture %d px", rc.r - rc.l, rc.b - rc.t, org.x, org.y, S); Log(b); }
        }
        // the dim copy in the middle, while the sight itself is somewhere else
        float off = (ddx < 0 ? -ddx : ddx) + (ddy < 0 ? -ddy : ddy);
        int wantG = cfgGhost && wndG && g_ghost && off > 6.0f * u;
        if (wantG) {
            if (redrawn || !shownG) {
                POINT_ dst = { cx0, cy0 }, size = { S, S }, src = { 0, 0 }; u32 bf = 0x01730000u;   // the same picture at 45 %
                UpdateLayeredWindow(wndG, 0, &dst, &size, dc, &src, 0, &bf, 2);
                lastGX = cx0; lastGY = cy0;
            } else if (cx0 != lastGX || cy0 != lastGY) { SetWindowPos(wndG, 0, cx0, cy0, 0, 0, 0x1 | 0x4 | 0x10); lastGX = cx0; lastGY = cy0; }
            if (!shownG) { SetWindowPos(wndG, (HANDLE)(i64)-1, 0, 0, 0, 0, 0x1 | 0x2 | 0x10 | 0x40); shownG = 1; }
        } else if (shownG) { ShowWindow(wndG, 0); shownG = 0; }
        Sleep(15);   // about 60 updates a second at most
    }
    return 0;
}

// ---------------------------------------------------------------- inline hook (first 6 bytes: push rbx; sub rsp,70h)
static u8* AllocNear(u8* target)
{
    u64 t = (u64)target & ~0xFFFFull;
    for (u64 d = 0x10000; d < 0x70000000ull; d += 0x10000) {
        void* p = VirtualAlloc((void*)(t - d), 0x1000, 0x3000, 0x40);
        if (p) return (u8*)p;
        p = VirtualAlloc((void*)(t + d), 0x1000, 0x3000, 0x40);
        if (p) return (u8*)p;
    }
    return 0;
}

static int HookApply(u8* target)
{
    static const u8 expect[6] = { 0x40, 0x53, 0x48, 0x83, 0xEC, 0x70 };
    for (int i = 0; i < 6; i++) if (target[i] != expect[i]) return 0;
    u8* m = AllocNear(target); if (!m) return 0;
    // stub: jmp [rip] -> hk_Apply
    m[0] = 0xFF; m[1] = 0x25; *(u32*)(m + 2) = 0; *(u64*)(m + 6) = (u64)(void*)&hk_Apply;
    // trampoline: original 6 bytes, then jmp [rip] -> target+6
    u8* tr = m + 16;
    memcpy(tr, target, 6);
    tr[6] = 0xFF; tr[7] = 0x25; *(u32*)(tr + 8) = 0; *(u64*)(tr + 12) = (u64)(target + 6);
    o_Apply = (Apply_t)(void*)tr;
    DWORD old;
    if (!VirtualProtect(target, 6, 0x40, &old)) return 0;
    u8 patch[6]; patch[0] = 0xE9; *(i32*)(patch + 1) = (i32)(m - (target + 5)); patch[5] = 0x90;
    memcpy(target, patch, 6);
    VirtualProtect(target, 6, old, &old);
    FlushInstructionCache(GetCurrentProcess(), target, 6);
    return 1;
}

// ---------------------------------------------------------------- player capture
// The movement function runs for Snake every frame as ActMovement(snakeObject, actionWork, flag) and calls the current
// action as action(snakeObject, actionWork). 18 bytes in, after anything another entry hook would have moved, a
// small stub stores rcx (Snake's object).
static int HookMovement(u8* at)
{
    static const u8 expect[11] = { 0x48, 0x33, 0xC4, 0x48, 0x89, 0x84, 0x24, 0xB0, 0x00, 0x00, 0x00 };
    static const u8 pre[] = {
        0x50, 0x51, 0x52, 0x41, 0x50, 0x41, 0x51, 0x41, 0x52, 0x41, 0x53,   // push rax rcx rdx r8 r9 r10 r11
        0x55, 0x48, 0x89, 0xE5,   // push rbp; mov rbp,rsp
        0x48, 0x83, 0xE4, 0xF0, 0x48, 0x83, 0xEC, 0x20,   // and rsp,-16; sub rsp,20h
        0x4C, 0x8D, 0x8D, 0x48, 0x01, 0x00, 0x00,   // lea r9,[rbp+148h] (the function's return address)
        0xFF, 0x15, 0, 0, 0, 0,   // call [rip+slot] (rcx = Snake, rdx = action work, r8d = message)
        0x48, 0x89, 0xEC, 0x5D,   // mov rsp,rbp; pop rbp
        0x41, 0x5B, 0x41, 0x5A, 0x41, 0x59, 0x41, 0x58, 0x5A, 0x59, 0x58   // pop r11 r10 r9 r8 rdx rcx rax
    };
    // The function's prologue is push rbx; push rsi; push rdi; sub rsp,0F0h, so at the hook point the return address is
    // at rsp+108h; the stub has pushed 8 more registers (40h) by the time rbp is set.
    for (int i = 0; i < 11; i++) if (at[i] != expect[i]) return 0;
    u8* m = AllocNear(at); if (!m) return 0;
    u8* slot = m + 0x80;  *(u64*)slot = (u64)(void*)&OnMove;
    u8* slotE = m + 0x90; *(u64*)slotE = (u64)(void*)&OnMoveEnd;
    g_savedRet = (u64*)(m + 0x98); *g_savedRet = 0;
    int n = (int)sizeof pre;
    memcpy(m, pre, (u64)n);
    *(i32*)(m + 32) = (i32)(slot - (m + 36));   // operand of the call
    memcpy(m + n, expect, 11);   // the displaced instructions
    u8* j = m + n + 11;
    j[0] = 0xFF; j[1] = 0x25; *(u32*)(j + 2) = 0; *(u64*)(j + 6) = (u64)(at + 11);
    // second stub, entered when the movement function returns: keep rax, call OnMoveEnd, go to the real caller
    u8* a = m + 0x100;
    a[0] = 0x50;   // push rax
    a[1] = 0x48; a[2] = 0x83; a[3] = 0xEC; a[4] = 0x28;   // sub rsp,28h
    a[5] = 0xFF; a[6] = 0x15; *(i32*)(a + 7) = (i32)(slotE - (a + 11));   // call [OnMoveEnd]
    a[11] = 0x48; a[12] = 0x83; a[13] = 0xC4; a[14] = 0x28;   // add rsp,28h
    a[15] = 0x58;   // pop rax
    a[16] = 0xFF; a[17] = 0x25; *(i32*)(a + 18) = (i32)((u8*)g_savedRet - (a + 22));   // jmp [saved return address]
    g_after = a;
    DWORD old;
    if (!VirtualProtect(at, 11, 0x40, &old)) return 0;
    u8 patch[11]; patch[0] = 0xE9; *(i32*)(patch + 1) = (i32)(m - (at + 5));
    for (int i = 5; i < 11; i++) patch[i] = 0x90;
    memcpy(at, patch, 11);
    VirtualProtect(at, 11, old, &old);
    FlushInstructionCache(GetCurrentProcess(), at, 11);
    return 1;
}

// ---------------------------------------------------------------- shots follow the crosshair
// - each weapon class has a fire routine (exe+359BE0, 35A7xx, 35B78x, 35C6E0, 35E240). In first-person view it creates
//   the bullet with a target point (exe+25F4E0 / 25F630: matrix, target, ...); the bullet then flies from the matrix
//   position straight at that point (exe+25E320: direction = target - position). The point is what the middle of the
//   screen covers. That is why first-person shots land exactly on the centre.
// - in third person it creates the bullet without a target (exe+25F240 / 25F390: matrix, ...), and the bullet flies
//   along the gun's own axis.
// while the shoulder view is on, the plugin catches the third-person call and makes the first-person one instead.
// Normally the bullet is started on the view line itself, level with the muzzle, and sent along that line: it then hits
// exactly what the middle of the screen covers, at any distance, enemies included. When something stands between the
// gun and that line or between the gun and the point aimed at (a tree at his shoulder), the bullet leaves the real
// muzzle toward the point aimed at, and stops where the crosshair says.
typedef void* (*NB_t)(const float* m, u32 area, u32 flags, u32 a4, u32 a5, u32 a6, float a7, u32 a8, u32 a9, u32 a10, u32 a11);
typedef void* (*NBAt_t)(const float* m, const float* tgt, u32 area, u32 flags, u32 a4, u32 a5, u32 a6, float a7, u32 a8, u32 a9, u32 a10, u32 a11);
typedef void* (*NB2_t)(const float* m, u32 area, u32 flags, u32 a4, u32 a5, u32 a6, float a7, u32 a8, u32 a9, u32 a10, u32 a11, u32 a12);
typedef void* (*NBAt2_t)(const float* m, const float* tgt, u32 area, u32 flags, u32 a4, u32 a5, u32 a6, float a7, u32 a8, u32 a9, u32 a10, u32 a11, u32 a12);
static NB_t o_NB; static NBAt_t f_NBAt; static NB2_t o_NB2; static NBAt2_t f_NBAt2;
static u8 *g_nbLo, *g_nbHi;   // Snake's fire routines lie between these

// 0 = leave the shot alone, 1 = along the view line (mm = the matrix moved onto it), 2 = from the muzzle at the point
static int ShotPlan(const float* m, u32 type, void* ret, float* mm, float* tgt)
{
    if (!cfgEnabled || !g_aimLatch || !m) return 0;
    if ((u8*)ret < g_nbLo || (u8*)ret >= g_nbHi) return 0;   // only Snake's own guns, never an enemy's
    u8* pl = g_pl;
    if (!Readable(pl, 0x800) || Bit(ST_FPV)) return 0;
    if (cfgPoseFix) PoseLearn(pl, Wpn(pl), m);   // where the barrel really is (the gun's vertical aim)
    PaLearn(pl, Wpn(pl), m);   // how far the barrel line runs above his shoulder
    { static u32 lastF = 0xFFFFFFFFu; if (lastF != g_frame) { lastF = g_frame; FireShot(); } }   // press -> shot time
    if (!cfgShots) return 0;
    int w = Wpn(pl), st = g_proneLatch ? 2 : g_crouchLatch ? 1 : 0, why = 0;
    float E[3], F[3], R[3], U[3], Q[3], P[3];
    for (int i = 0; i < 3; i++) { E[i] = g_tEye[i]; F[i] = g_tF[i]; R[i] = g_tR[i]; U[i] = g_tU[i]; Q[i] = g_tQ[i]; P[i] = g_tPt[i]; }
    float M[3] = { m[12], m[13], m[14] };
    float ox = M[0] - Q[0], oy = M[1] - Q[1], oz = M[2] - Q[2];
    float mo[3] = { ox * F[0] + oy * F[1] + oz * F[2], ox * R[0] + oy * R[1] + oz * R[2], ox * U[0] + oy * U[1] + oz * U[2] };
    float dm = (M[0] - E[0]) * F[0] + (M[1] - E[1]) * F[1] + (M[2] - E[2]) * F[2];   // how far along the view line the muzzle is
    float lx = M[0] - (E[0] + F[0] * dm), ly = M[1] - (E[1] + F[1] * dm), lz = M[2] - (E[2] + F[2] * dm);
    float lat = __builtin_sqrtf(lx * lx + ly * ly + lz * lz);   // how far beside it
    if ((u32)(g_frame - g_tFrame) > 3) why = 1;   // no fresh aim line (the view is still swinging in)
    else if (ox * ox + oy * oy + oz * oz > 3000.0f * 3000.0f || lat > 2500.0f || dm < 100.0f) why = 3;   // not where his gun can be
    if (!why && w > 0 && w < 64) {
        // averaged over shots once known (single shots scatter by up to 250 units with the aim pitch); logged when it is
        // new or has moved
        float k = g_moSrc[w][st] >= 2 ? 0.3f : 1.0f, mv = 0;
        for (int i = 0; i < 3; i++) { float nv = g_mo[w][st][i] + (mo[i] - g_mo[w][st][i]) * k; float dd = nv - g_moSaved[w][st][i]; mv += dd * dd; g_mo[w][st][i] = nv; }
        int first = g_moSrc[w][st] < 2;
        g_moOk[w][st] = 1; g_moSrc[w][st] = 3;
        if (first || mv > 40.0f * 40.0f) {
            for (int i = 0; i < 3; i++) g_moSaved[w][st][i] = g_mo[w][st][i];
            if (first) { char b[120]; wsprintfA(b, "muzzle position measured: weapon %d stance %d (%d, %d, %d)", w, st, (int)g_mo[w][st][0], (int)g_mo[w][st][1], (int)g_mo[w][st][2]); Log(b); }
        }
    }
    int how = 0;
    if (!why) {
        tgt[3] = m[15];
        if (g_tMuzzle) {
            // from the muzzle at the point aimed at; if that point is not properly ahead of the muzzle, far along the
            // line
            float ah = (P[0] - M[0]) * F[0] + (P[1] - M[1]) * F[1] + (P[2] - M[2]) * F[2];
            for (int i = 0; i < 3; i++) tgt[i] = ah >= 400.0f ? P[i] : E[i] + F[i] * (dm + 150000.0f);
            how = 2;
        } else {
            for (int i = 0; i < 16; i++) mm[i] = m[i];
            for (int i = 0; i < 3; i++) { mm[12 + i] = E[i] + F[i] * dm; tgt[i] = E[i] + F[i] * (dm + 150000.0f); }
            how = 1;
        }
        g_shotsAimed++;
        if (g_otsShotFrame == 0x80000000u) { g_otsShotFrame = g_frame; g_rkAtShot = g_rkN; }
        g_otsShots++; g_shotT = GetTickCount() ? GetTickCount() : 1;
        if (cfgRecoil && cfgRecoilPct && (g_kP || g_kY) && w > 0 && w < 32 && ((u32)cfgSpreadMask >> w & 1)) {   // automatic guns only
            // the gun's current kick (rolled by the game, see hk_Recoil) turns the shot away from the crosshair, as in
            // first person: up/down around the screen's right axis, sideways around its up axis
            const float* O = how == 1 ? mm + 12 : M;
            float D[3] = { tgt[0] - O[0], tgt[1] - O[1], tgt[2] - O[2] };
            float L = __builtin_sqrtf(D[0] * D[0] + D[1] * D[1] + D[2] * D[2]);
            float k = 6.2831853f / 65536.0f * (float)cfgRecoilPct / 100.0f;
            float ap = -(float)g_kP * k, ay = (float)g_kY * k;   // up, left (radians)
            for (int i = 0; i < 3; i++) tgt[i] += (U[i] * ap - R[i] * ay) * L;
            g_rkSpread++;
        }
    }
    u32 n = ++g_shots;
    float upE = 0; int upOk = 0;   // the real barrel against the line from the muzzle to the spot
    if (g_ctrDist >= 0) {
        float bb[3] = { m[8], m[9], m[10] }, dd[3] = { P[0] - m[12], P[1] - m[13], P[2] - m[14] };
        if (Norm3(bb) && Norm3(dd)) {
            upE = (Pitch3(bb) - Pitch3(dd)) * 57.29578f; upOk = 1;
            int k = g_pfUse ? 1 : 0; g_pfShotErr[k] += fabs_(upE); g_pfShotN[k]++;
            if (g_ctrDist < 3000) { int k2 = (u32)(g_frame - g_paUseFrame) <= 2 ? 1 : 0; g_paShotErr[k2] += fabs_(upE); g_paShotN[k2]++; }   // spots within 30 m
        }
    }
    if (cfgLog && cfgLogLvl >= 2 && (n <= 60 || (n & 31) == 0)) {
        char b[900];
        wsprintfA(b, "shot %u: wpn=%d type=%u how=%d why=%d stance=%d muzzle ahead/right/up=%d,%d,%d beside-line=%d along=%d axes.aim=%d,%d,%d blocked=%d sideclear=%d ctr=%d land=%d",
            n, w, type, how, why, st, (int)mo[0], (int)mo[1], (int)mo[2], (int)lat, (int)dm,
            (int)((m[0] * F[0] + m[1] * F[1] + m[2] * F[2]) * 1000.0f), (int)((m[4] * F[0] + m[5] * F[1] + m[6] * F[2]) * 1000.0f),
            (int)((m[8] * F[0] + m[9] * F[1] + m[10] * F[2]) * 1000.0f), (int)g_tMuzzle, (int)g_sideClear, (int)g_ctrDist, (int)g_landDist);
        // does his gun point at the spot the shot lands on? Angle between the gun's own axis (the game's bullet matrix,
        // its third row) and the line from the muzzle to the point under the crosshair (3DS pose: small)
        if (g_ctrDist >= 0) {
            float d0 = P[0] - m[12], d1 = P[1] - m[13], d2 = P[2] - m[14];
            float cx = d1 * m[10] - d2 * m[9], cy = d2 * m[8] - d0 * m[10], cz = d0 * m[9] - d1 * m[8];
            int a10 = (int)(atan2_(__builtin_sqrtf(cx * cx + cy * cy + cz * cz), d0 * m[8] + d1 * m[9] + d2 * m[10]) * 572.9578f);
            wsprintfA(b + slen(b), " gun-to-crosshair=%d.%d deg", a10 / 10, a10 % 10);
            g_gunAngSum += (u32)a10; g_gunAngN++;
            if (upOk) {   // split into up / down and sideways (screen right), + = the barrel points above / right of the spot
                float bb[3] = { m[8], m[9], m[10] }, dd[3] = { P[0] - m[12], P[1] - m[13], P[2] - m[14] }; Norm3(bb); Norm3(dd);
                float br = bb[0] * R[0] + bb[1] * R[1] + bb[2] * R[2], dr = dd[0] * R[0] + dd[1] * R[1] + dd[2] * R[2];
                float sideE = (atan2_(br, sqrt_(1.0f - br * br)) - atan2_(dr, sqrt_(1.0f - dr * dr))) * 57.29578f;
                char t1[16], t2[16], t3[16], t4[16], t5[16];
                int ww = w > 0 && w < PF_NW ? w : 0;
                wsprintfA(b + slen(b), " barrel-vs-spot up=%s side=%s deg; gun pose fix %s (bone %d, correction %s deg, hand check %s, frame check %s deg)",
                    Tenths(t1, upE), Tenths(t2, sideE), g_pfUse ? "ON" : g_pfState == 4 ? "switched off" : ww && g_pfIdx[ww] ? "ready" : "learning",
                    ww && g_pfIdx[ww] ? kPfBone[g_pfIdx[ww] - 1] : -1, Tenths(t3, (float)g_pfC10 / 10.0f), g_pfLastHand >= 0 ? Tenths(t4, g_pfLastHand) : "-", g_pfLastCam >= 0 ? Tenths(t5, g_pfLastCam) : "-");
                // the arms' turn about the shoulder (PoseAim)
                int on = (u32)(g_frame - g_paUseFrame) <= 2;
                PaW* pk = ww ? &g_pa[ww][st == 1 ? 1 : 0] : 0;
                wsprintfA(b + slen(b), "; gun aim %s (barrel line %d above the shoulder, %d above the hand from %d shot(s), turn asked %s / the game's own %s / done %s deg)",
                    on ? "ON" : !cfgPoseAim ? "off (PoseAim=0)" : st == 2 ? "off (lying)" : !PoseAimW(w) ? "off (weapon)" : "off",
                    (int)g_paC, pk ? (int)pk->c : 0, pk ? pk->n : 0, Tenths(t1, (float)g_paQ10 / 10.0f), Tenths(t2, (float)g_paG10 / 10.0f), g_paTurn10 > -9999 ? Tenths(t3, (float)g_paTurn10 / 10.0f) : "-");
            }
        }
        Log(b);
    }
    return how;
}
static NBAt_t o_NBAt; static NBAt2_t o_NBAt2;
static void NoteFpShot(void* ret)
{
    if ((u8*)ret >= g_nbLo && (u8*)ret < g_nbHi && Bit(ST_FPV)) { g_fpShotFrame = g_frame; g_fpShotN++; }
}
static __attribute__((noinline)) void* hk_NBAt(const float* m, const float* t, u32 area, u32 flags, u32 a4, u32 a5, u32 a6, float a7, u32 a8, u32 a9, u32 a10, u32 a11)
{
    NoteFpShot(__builtin_return_address(0));
    return o_NBAt(m, t, area, flags, a4, a5, a6, a7, a8, a9, a10, a11);
}
static __attribute__((noinline)) void* hk_NBAt2(const float* m, const float* t, u32 area, u32 flags, u32 a4, u32 a5, u32 a6, float a7, u32 a8, u32 a9, u32 a10, u32 a11, u32 a12)
{
    NoteFpShot(__builtin_return_address(0));
    return o_NBAt2(m, t, area, flags, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}
static __attribute__((noinline)) void* hk_NB(const float* m, u32 area, u32 flags, u32 a4, u32 a5, u32 a6, float a7, u32 a8, u32 a9, u32 a10, u32 a11)
{
    __attribute__((aligned(16))) float mm[16], tg[4];
    int how = f_NBAt ? ShotPlan(m, a9, __builtin_return_address(0), mm, tg) : 0;
    if (how) return f_NBAt(how == 1 ? mm : m, tg, area, flags, a4, a5, a6, a7, a8, a9, a10, a11);
    return o_NB(m, area, flags, a4, a5, a6, a7, a8, a9, a10, a11);
}
static __attribute__((noinline)) void* hk_NB2(const float* m, u32 area, u32 flags, u32 a4, u32 a5, u32 a6, float a7, u32 a8, u32 a9, u32 a10, u32 a11, u32 a12)
{
    __attribute__((aligned(16))) float mm[16], tg[4];
    int how = f_NBAt2 ? ShotPlan(m, a9, __builtin_return_address(0), mm, tg) : 0;
    if (how) return f_NBAt2(how == 1 ? mm : m, tg, area, flags, a4, a5, a6, a7, a8, a9, a10, a11, a12);
    return o_NB2(m, area, flags, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}
// entry hook for a function that starts with a 5-byte "mov [rsp+10h],reg" (rbx: third byte 5C, rdx: 54); returns the
// way to call the original
static void* Hook5y(u8* target, void* hook, u8 third, u8 fifth)
{
    const u8 expect[5] = { 0x48, 0x89, third, 0x24, fifth };
    for (int i = 0; i < 5; i++) if (target[i] != expect[i]) return 0;
    u8* m = AllocNear(target); if (!m) return 0;
    m[0] = 0xFF; m[1] = 0x25; *(u32*)(m + 2) = 0; *(u64*)(m + 6) = (u64)hook;
    u8* tr = m + 16;
    memcpy(tr, target, 5);
    tr[5] = 0xFF; tr[6] = 0x25; *(u32*)(tr + 7) = 0; *(u64*)(tr + 11) = (u64)(target + 5);
    DWORD old;
    if (!VirtualProtect(target, 5, 0x40, &old)) return 0;
    u8 patch[5]; patch[0] = 0xE9; *(i32*)(patch + 1) = (i32)(m - (target + 5));
    memcpy(target, patch, 5);
    VirtualProtect(target, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), target, 5);
    return tr;
}
static void* Hook5x(u8* target, void* hook, u8 third) { return Hook5y(target, hook, third, 0x10); }
static void* Hook5(u8* target, void* hook) { return Hook5x(target, hook, 0x5C); }

// ---------------------------------------------------------------- crouch walking
// The game itself has no crouch walk: crouched, pushing the stick makes Snake drop to the ground and crawl.
// - while Snake squats (his "squat still" action, exe+3686B0) and the stick is pushed, he is given the "stand move
//   stalk" motion, which the motion loader (exe+751B20) is made to swap for the squat walk (motion 199 of his main
//   motion archive 6891CC) and its walking sounds (exe+755EE0); its speed is set every frame (6 / 3 = walk / stalk;
//   stick under 180 = stalk);
// - the line in the squat action that sends him to the ground when the stick is pushed (exe+36882C) is switched off;
//   instead, as in the plugin, holding the crouch button stands him up or lays him down (exe+36A590 is asked after the
//   squat action, not during it);
// - his camo index while crouch walking goes through CamoIndexPercent / CamoIndexAdd (exe+358C80), and crouchwalk (the
//   low walk at walk speed) loses CrouchWalkCamo percent.
typedef i64 (*CwSquat_t)(u8* work, u8* mw, i64 a3, i64 a4);
typedef i64 (*CwHold_t)(u8* work, u8* mw);
typedef void (*CwSetMot_t)(u8* mc, int layer, int motion, int time, i64 mask);
typedef i64 (*CwSeq_t)(u8* sq, int layer, int num, int flag, int loop);
typedef int* (*CwCamo_t)(int* a1, int a2);
typedef u32 (*CwPlMot_t)(u8* work, int motion);
typedef i64 (*CwStChk_t)(u32 n);
typedef i64 (*CwStSet_t)(int a, i64 b, i64 c, i64 d);
static CwSquat_t o_CwSquat; static CwHold_t o_CwHold; static CwSetMot_t o_CwSetMot; static CwSeq_t o_CwSeq; static CwCamo_t o_CwCamo;
static CwPlMot_t f_CwPlMot; static CwStChk_t f_CwStChk; static CwStSet_t f_CwStSet;
static u8* g_cwSquatAct; static u8* volatile g_cwMw; static u8* volatile g_cwMc; static u8* g_cwCamo; static volatile double* g_cwWait;
static void CwOnMove(void)
{
    u8* mw = g_cwMw;
    if (g_cwInstalled && mw && *(u64*)(mw + 0x58) != (u64)g_cwSquatAct) { g_cwOn = 0; g_cwMoving = 0; }
}
static u32 CwPlayerMotion(u8* work, int motion) { return f_CwPlMot(work, motion > 0 ? motion - 1 : motion); }
static i64 hk_CwHold(u8* work, u8* mw) { if (g_cwIgnoreHold) return 0; return o_CwHold(work, mw); }
static volatile int g_cwArcN = -1;   // motions in Snake's main archive (-1 = not seen yet, 0 = could not tell)
static void hk_CwSetMot(u8* mc, int layer, int motion, int time, i64 mask)
{
    if (mc) {
        u8* mt = *(u8**)(mc + 8);
        if (mt && *(u32*)(mt + 0x54) == 0x6891CC) {
            g_cwMc = mc;   // Snake's main motion control
            if (g_cwArcN < 0) {   // once: is the squat walk (motion 199, from the 3DS) in his motion archive?
                u8* ah = *(u8**)(mt + 8);
                g_cwArcN = (Readable(ah, 16) && *(u32*)ah == 0x7261744Du) ? (int)*(u32*)(ah + 8) : 0;
                char b[220];
                if (!g_cwArcN) wsprintfA(b, "crouch walking: could not read the number of motions in Snake's archive - going ahead");
                else if (g_cwArcN > 199) wsprintfA(b, "crouch walking: Snake's motion archive has %d motions - the squat walk (199) is there", g_cwArcN);
                else wsprintfA(b, "crouch walking: Snake's motion archive has %d motions - the squat walk (199) is MISSING: install the motion archive that has it", g_cwArcN);
                Log(b);
            }
        }
    }
    if (mc && mc == g_cwMc && motion == 4 && g_cwOn && (g_cwArcN > 199 || g_cwArcN == 0)) {   // "stand move stalk" -> the squat walk
        time = (int)*(float*)(mc + 0x128);
        motion = 199;
        g_cwHijack = 1; g_cwSwaps++;
    }
    o_CwSetMot(mc, layer, motion, time, mask);
    g_cwHijack = 0;
}
static i64 hk_CwSeq(u8* sq, int layer, int num, int flag, int loop)
{
    if (g_cwHijack) num = g_cwSlow ? 3 : 4;   // the slow / stalk walking sounds
    // the added motions (slot 199 and up) have no entry in Snake's sound sequence list, so the game played
    // no footsteps for them (every shoulder-view walk, the crouchnormal walk, lying sideways). While the plugin asks
    // for one of them, the sequence of a game walk of the same length is started instead.
    else if (g_seqSlot >= 0 && num == g_seqSlot && g_seqSub >= 0) {
        num = g_seqSub; g_seqSwaps++;
        if (!g_seqTold) { g_seqTold = 1; char b[160]; wsprintfA(b, "footsteps: motion %d plays the sounds of game motion %d (the added motions have none of their own)", g_seqSlot, g_seqSub); Log(b); }
    }
    return o_CwSeq(sq, layer, num, flag, loop);
}
static int* hk_CwCamo(int* a1, int a2)
{
    int* r = o_CwCamo(a1, a2);
    if (g_cwCamo && a2 >= 0 && a2 <= 64 && g_aimLatch && !g_crouchLatch && !g_proneLatch && g_markKind >= 0 && g_markKind <= 2 && (u32)(g_frame - g_markFrame) <= 2) {
        // standing in the shoulder view the game's own camo index is used as it is (the marks above make it the moving
        // one) - only written to the log
        int v = *(int*)(g_cwCamo + (i64)a2 * 128 + 0x24); g_otsCamoN++;
        static int told, lastK = -1; static DWORD lastT; DWORD t = GetTickCount();
        if (g_markKind != lastK && told < 30 && t - lastT > 300 && v < 1000) {
            static const char* nm[3] = { "full speed (run)", "slow walk (walk)", "stalk" };
            char b[160]; wsprintfA(b, "camo index, shoulder view, standing %s: the game's own %s%d.%d%%", nm[g_markKind], v < 0 ? "-" : "", (v < 0 ? -v : v) / 10, (v < 0 ? -v : v) % 10);
            Log(b); told++; lastT = t; lastK = g_markKind;
        }
    }
    if (g_cwCamo && a2 >= 0 && a2 <= 64 && g_proneLatch && Readable(g_pl, 0x800)) {
        u8* p = g_pl; u16 st = *(u16*)(p + 0x7E0);
        int otsC = g_aimLatch && (u32)(g_frame - g_legFrame) <= 2 && g_legDir == 0 && g_legSlot >= 0;
        int tpC = !g_aimLatch && st > 15 && !Bit(6);
        if (otsC || tpC) {
            int slow = otsC ? CrawlWalkNow(p) : (st < 190 || Bit(0x13B)), k = (otsC ? 2 : 0) + slow;
            int v = *(int*)(g_cwCamo + (i64)a2 * 128 + 0x24);
            static int lastK = -1, told; static DWORD lastT; DWORD t = GetTickCount();
            if (k != lastK && told < 40 && t - lastT > 300 && v < 1000) {
                char b[180]; wsprintfA(b, "camo index, %s, crawling forward %s: the game's own %s%d.%d%%", otsC ? "shoulder view" : "normal view", slow ? "slowly (walk)" : "at full speed",
                    v < 0 ? "-" : "", (v < 0 ? -v : v) / 10, (v < 0 ? -v : v) % 10);
                Log(b); told++; lastT = t; lastK = k;
            }
        }
    }
    int ots = OtsCrouchMoving();   // crouched in the shoulder view too
    if (!g_cwCamo || (!ots && (!g_cwOn || !g_cwMoving)) || a2 < 0 || a2 > 64) return r;
    int* ci = (int*)(g_cwCamo + (i64)a2 * 128 + 0x24);
    if (*ci >= 1000) return r;   // stealth camouflage and the like
    int v = *ci, v0 = v;
    v = v < 0 ? v * 100 / cfgCwCamoPct : v * cfgCwCamoPct / 100;
    v += cfgCwCamoAdd * 10;
    // the faster the move, the bigger the loss: crouchnormal (full speed) CamoCrouchNormal, crouchwalk CamoCrouchWalk,
    // crouchstalk CamoCrouchStalk - each on top of the game's own
    int mode = ots ? (g_band >= 0 && g_band <= 2 ? g_band : 1) : g_cwSlow ? 2 : g_lastWalk == 1 ? 1 : 0;
    if (ots) g_otsCamoN++;
    v += (mode == 2 ? cfgCwStalkCamo : mode == 1 ? cfgCwWalkCamo : cfgCwNormCamo) * 10;
    if (v > 950) v = 950; if (v < -1000) v = -1000;
    *ci = v; g_cwCamoN++;
    {   static int lastMode = -1, lastV0 = 99999, told; DWORD t = GetTickCount(); static DWORD lastT;
        if ((mode != lastMode || v0 != lastV0) && told < 60 && t - lastT > 300) {
            static const char* nm[3] = { "crouchnormal", "crouchwalk", "crouchstalk" };
            char b[200]; wsprintfA(b, "camo index, %smoving %s: the game's own %s%d.%d%% -> %s%d.%d%% (changed by %d%%)", ots ? "shoulder view, " : "", nm[mode],
                v0 < 0 ? "-" : "", (v0 < 0 ? -v0 : v0) / 10, (v0 < 0 ? -v0 : v0) % 10, v < 0 ? "-" : "", (v < 0 ? -v : v) / 10, (v < 0 ? -v : v) % 10, (v - v0) / 10);
            Log(b); told++; lastT = t;
            lastMode = mode; lastV0 = v0;
        }
    }
    return r;
}
static i64 hk_CwSquat(u8* work, u8* mw, i64 a3, i64 a4)
{
    g_cwMw = mw;
    g_cwIgnoreHold = 1;
    i64 r = o_CwSquat(work, mw, a3, a4);
    g_cwIgnoreHold = 0;
    if (!f_CwStChk(0xE0)) {   // holding the crouch button: stand up / lie down
        i64 bs = o_CwHold(work, mw);
        if (bs == 1) *(u32*)(mw + 0x68) |= 0x40;
        else if (bs == 2) *(u32*)(mw + 0x68) |= 0x30;
    }
    i16 pf = *(i16*)(work + 0x7E0);
    int was = g_cwMoving, wasSlow = g_cwSlow;
    // "pushed" = more than 15. 15 leaves out the 1s a pad sends at rest
    g_cwMoving = (int)pf > 15;
    g_cwSlow = pf < 180;
    if (g_cwMoving && !f_CwStChk(0xDE)) {   // 0xDE: not first person
        *(u32*)(mw + 0x60) = CwPlayerMotion(work, g_cwOn ? 4 : 6);
        u8* mc = g_cwMc;
        if (mc) {
            u8* mt = *(u8**)(mc + 8);
            int full = !g_cwSlow && cfgCrouchAll && cfgCRun[0] >= 0 && (o_Inp ? g_padRaw : (u16)pf) >= cfgFastAbove && !SlowHeld();
            float sp = (float)(g_cwSlow ? cfgCwStalkRate : full ? 600 : cfgCwWalkRate) / 100.0f;
            if (g_cwWait) sp = (float)((double)sp * (*g_cwWait / (1.0 / 60.0)));
            if (mt) *(float*)(mt + 0x4C) = sp;
            u8* sq = *(u8**)(mc + 0x78);
            if (was && wasSlow != g_cwSlow && sq) o_CwSeq(sq, 0, g_cwSlow ? 3 : 4, 5, 0x1B0);
        }
        f_CwStSet(11, 14, 0x10C, (i64)0xFFFFFFFFu);   // walking sounds in grass
        g_cwOn = 1; g_cwFrames++;
        static int told; if (!told) { told = 1; Log("crouch walking: Snake walks crouched"); }
    }
    if (g_cwOn && (*(u32*)(mw + 0x68) & 0x40)) g_cwOn = 0;

    return r;
}
// hook a function whose first n bytes are whole instructions; relOff >= 0: a rel32 jump target inside them (at byte
// relOff, the instruction ending at relEnd) is moved so it still points where it did
static void* HookCopy(u8* target, void* hook, int n, int relOff, int relEnd)
{
    if (!target || n < 5 || n > 32 || target[0] == 0xE9 || target[0] == 0xFF) return 0;
    u8* m = AllocNear(target); if (!m) return 0;
    m[0] = 0xFF; m[1] = 0x25; *(u32*)(m + 2) = 0; *(u64*)(m + 6) = (u64)hook;
    u8* tr = m + 16;
    memcpy(tr, target, (u64)n);
    if (relOff >= 0) {
        u8* dest = target + relEnd + *(i32*)(target + relOff);
        i64 nr = dest - (tr + relEnd);
        if (nr > 0x7FFFFFFFll || nr < -0x80000000ll) return 0;
        *(i32*)(tr + relOff) = (i32)nr;
    }
    tr[n] = 0xFF; tr[n + 1] = 0x25; *(u32*)(tr + n + 2) = 0; *(u64*)(tr + n + 6) = (u64)(target + n);
    DWORD old;
    if (!VirtualProtect(target, 5, 0x40, &old)) return 0;
    u8 patch[5]; patch[0] = 0xE9; *(i32*)(patch + 1) = (i32)(m - (target + 5));
    memcpy(target, patch, 5);
    VirtualProtect(target, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), target, 5);
    return tr;
}
static int Same(const u8* p, const char* hex)   // the bytes at p are these (no wildcards)
{
    for (int i = 0; hex[0] && hex[1]; i++) {
        int v = 0;
        for (int k = 0; k < 2; k++) { char c = hex[k]; v = v * 16 + (c >= 'a' ? c - 'a' + 10 : c >= 'A' ? c - 'A' + 10 : c - '0'); }
        if (p[i] != (u8)v) return 0;
        hex += 2; while (*hex == ' ') hex++;
    }
    return 1;
}
static void SoundInstall(void)   // only the sound hook
{
    if (o_CwSeq || !cfgSnd) return;
    u8* seq = Scan("4D 63 D8 48 85 C9 74 6B 48 63 C2 48 8D 14 40 48");
    if (seq && Same(seq, "4D 63 D8 48 85 C9")) o_CwSeq = (CwSeq_t)HookCopy(seq, (void*)&hk_CwSeq, 6, -1, 0);
    Log(o_CwSeq ? "footsteps for the added motions: sound hook installed" : "footsteps for the added motions: sound hook NOT installed");
}
static void CrouchWalkInstall(void)
{
    char b[300];
    if (!cfgCw) { Log("crouch walking: off ([CrouchWalk] Enabled=0)"); SoundInstall(); return; }
    if (GetModuleHandleA("MGS3CrouchWalk.asi") || GetModuleHandleA("MGS3CrouchWalk.dll")) {
        Log("crouch walking: another crouch-walk plugin is loaded - this one stays off so the two do not clash (remove the other one to use this one)");
        SoundInstall(); return;
    }
    u8* squat = Scan("4C 8B DC 55 57 41 56 49 8D 6B A1 48 81 EC 00 01");
    u8* setmot = Scan("48 85 C9 0F 84 42 03 00 00 4C 8B DC 55 53 56 41");
    u8* camo = Scan("48 83 EC 30 0F 29 74 24 20 48 8B F9 48 63 F2 E8"); if (camo) camo -= 0x10;
    u8* hold = Scan("44 0F B7 8A 8E 00 00 00 4C 8B C2 66 45 85 C9 78");
    u8* seq = Scan("4D 63 D8 48 85 C9 74 6B 48 63 C2 48 8D 14 40 48");
    u8* prone = Scan("00 00 7E 19 83 4F 68 10");
    u8* wait = Scan("83 3D ?? ?? ?? ?? 00 ?? ?? F2 0F 10 0D");
    u8* cinit = Scan("85 D2 75 33 0F 57 C0 48 63 C2 48 C1 E0 07 48 8D");
    u8* plmot = Scan("B9 0F 01 00 00 E8 ?? 36 FF FF 85 C0 74 2A BA FF"); if (plmot) plmot -= 0x10;
    u8* stchk = Scan("8B D1 B8 01 00 00 00 83 E1 1F D3 E0 8B CA 48 C1");
    u8* stpat = Scan("04 89 0F AB D0 41 89 04"); u8* stset = 0;
    if (stpat) for (int k = 0x30; k <= 0x60 && !stset; k++) if (Same(stpat - k, "89 4C 24 08 48 89 54 24 10 8B D1")) stset = stpat - k;
    int c1 = squat && Same(squat, "4C 8B DC 55 57"), c2 = setmot && Same(setmot, "48 85 C9 0F 84"), c3 = camo && Same(camo, "48 89 5C 24 08"),
        c4 = hold && Same(hold, "44 0F B7 8A 8E 00 00 00"), c5 = seq && Same(seq, "4D 63 D8 48 85 C9"), c6 = prone != 0,
        c7 = plmot && Same(plmot, "48 89 5C 24 08"), c8 = stchk && Same(stchk, "8B D1 B8 01 00 00 00"), c9 = stset != 0;
    if (!(c1 && c2 && c3 && c4 && c5 && c6 && c7 && c8 && c9)) {
        wsprintfA(b, "crouch walking: game code not as expected - squat %d, motion loader %d, camo %d, button hold %d, sounds %d, ground switch %d, set motion %d, status check %d, status set %d (1 = fine) - no crouch walking",
            c1, c2, c3, c4, c5, c6, c7, c8, c9);
        Log(b); SoundInstall(); return;
    }
    g_cwSquatAct = squat;
    f_CwPlMot = (CwPlMot_t)(void*)plmot; f_CwStChk = (CwStChk_t)(void*)stchk; f_CwStSet = (CwStSet_t)(void*)stset;
    f_StSet5 = (StSet5_t)(void*)stset;   // the same function, five marks
    {   // the game's crawl sound routine (exe+36A070), checked byte for byte
        u8* cs = g_base + 0x36A070;
        if (Same(cs, "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 48 83 EC 20 48 8B D9 33 FF 45 8B F1 41 8B E8 48 8B F2 8D 4F 07"))
            f_CrawlSnd = (CrawlSnd_t)(void*)cs;
        Log(f_CrawlSnd ? "crawl sounds: the game's crawl sound routine found (exe+36A070) - crawling in the shoulder view sounds and rustles like the normal crawl"
                       : "crawl sounds: the game's crawl sound routine NOT found - crawling in the shoulder view only gets the grass marks");
    }
    g_cwWait = wait ? (volatile double*)Rip(wait + 9, 4, 8) : 0;
    g_cwCamo = cinit ? Rip(cinit + 0xE, 3, 7) : 0;   // the camo index table (exe+1E16CD0), 128 bytes a slot
    // order: the functions the others call first
    o_CwHold = (CwHold_t)HookCopy(hold, (void*)&hk_CwHold, 8, -1, 0);
    o_CwSeq = (CwSeq_t)HookCopy(seq, (void*)&hk_CwSeq, 6, -1, 0);
    o_CwSetMot = (CwSetMot_t)HookCopy(setmot, (void*)&hk_CwSetMot, 9, 5, 9);
    o_CwCamo = g_cwCamo ? (CwCamo_t)HookCopy(camo, (void*)&hk_CwCamo, 5, -1, 0) : 0;
    if (!o_CwHold || !o_CwSeq || !o_CwSetMot) { Log("crouch walking: could not hook the game - no crouch walking"); return; }
    o_CwSquat = (CwSquat_t)HookCopy(squat, (void*)&hk_CwSquat, 5, -1, 0);
    if (!o_CwSquat) { Log("crouch walking: could not hook the squat action - no crouch walking"); return; }
    {   DWORD old;   // the stick does not send a squatting Snake to the ground
        if (prone[7] == 0x10 && VirtualProtect(prone + 7, 1, 0x40, &old)) { prone[7] = 0x00; VirtualProtect(prone + 7, 1, old, &old); FlushInstructionCache(GetCurrentProcess(), prone + 7, 1); }
    }
    g_cwInstalled = 1;
    wsprintfA(b, "crouch walking installed: squat action exe+%X, motion loader exe+%X, sounds exe+%X, button hold exe+%X, camo exe+%X%s; walk %d, stalk %d (x100), camo %d%% plus %d; camo on top of the game's own: crouchnormal %d%%, crouchwalk %d%%, crouchstalk %d%%",
        (u32)(squat - g_base), (u32)(setmot - g_base), (u32)(seq - g_base), (u32)(hold - g_base), (u32)(camo - g_base), o_CwCamo ? "" : " (camo hook NOT installed)",
        cfgCwWalkRate, cfgCwStalkRate, cfgCwCamoPct, cfgCwCamoAdd, cfgCwNormCamo, cfgCwWalkCamo, cfgCwStalkCamo);
    Log(b);
}

// ---------------------------------------------------------------- setup
static DWORD API InitThread(void* unused)
{
    (void)unused;
    char b[256];
    g_base = (u8*)GetModuleHandleA(0);
    if (!FindText()) { Log("no .text section found"); return 0; }
    static const char PAT_APPLY[] =
        "40 53 48 83 EC 70 F3 0F 10 05 ?? ?? ?? ?? 48 8B D9 0F 29 74 24 60 0F 29 7C 24 50 "
        "44 0F 29 44 24 40 44 0F 29 4C 24 30 F3 44 0F 10 89 A0 00 00 00";
    u8* apply = 0;
    for (int i = 0; i < 1200 && !apply; i++) { apply = Scan(PAT_APPLY); if (!apply) Sleep(100); }
    if (!apply) { Log("third-person camera code not found: different game version, mod inactive"); return 0; }
    wsprintfA(b, "camera apply at exe+%X", (u32)(apply - g_base)); Log(b);

    u8* tb = Scan("8B D1 B8 01 00 00 00 83 E1 1F D3 E0 8B CA 48 C1 E9 05 48 8D 15 ?? ?? ?? ?? 23 04 8A C3");
    g_bits = (volatile u32*)Rip(tb ? tb + 18 : 0, 3, 7);
    u8* mvp = Scan("81 EC F0 00 00 00 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 84 24 B0 00 00 00 48 8B F9");
    p_held = (volatile u32*)Rip(Scan("48 8D 3D ?? ?? ?? ?? BA ?? ?? ?? ?? 41 B8"), 3, 7);
    p_gpad = (volatile u32*)Rip(Scan("8B 87 ?? ?? ?? ?? 85 05 ?? ?? ?? ?? 74"), 8, 12);
    p_cutscene = (volatile i32*)Rip(Scan("8B 05 ?? ?? ?? ?? 45 33 F6 8B 0D"), 2, 6);
    p_scripted = (volatile i32*)Rip(Scan("44 39 35 ?? ?? ?? ?? 89 15"), 3, 7);
    p_tick = (volatile i32*)Rip(Scan("89 05 ?? ?? ?? ?? BA 10 81 7E 02"), 2, 6);
    wsprintfA(b, "status table exe+%X, movement function exe+%X, held exe+%X, pad exe+%X, cutscene exe+%X, scripted exe+%X",
        g_bits ? (u32)((u8*)g_bits - g_base) : 0, mvp ? (u32)(mvp - 5 - g_base) : 0,
        p_held ? (u32)((u8*)p_held - g_base) : 0, p_gpad ? (u32)((u8*)p_gpad - g_base) : 0,
        p_cutscene ? (u32)((u8*)p_cutscene - g_base) : 0, p_scripted ? (u32)((u8*)p_scripted - g_base) : 0);
    Log(b);
    if (!g_bits) { Log("status table not found: mod inactive"); return 0; }

    if (!HookApply(apply)) { Log("could not install the camera hook: mod inactive"); return 0; }
    if (mvp && HookMovement(mvp + 13)) Log("player capture hook installed");
    else Log("player capture hook NOT installed: facing and strafing will do nothing");
    {
        u8* rq = Scan("48 8B C4 53 48 83 EC 70 F3 0F 10 81 D4 00 00 00 48 8B D9 48 89 68 08 48 89 78 18 48 8B FA");
        if (rq && HookReq(rq)) { wsprintfA(b, "animation request hook installed at exe+%X", (u32)(rq - g_base)); Log(b); }
        else Log("animation request hook NOT installed: Snake will glide without leg animation");
    }
    {
        u8* aa = Scan("4C 8B DC 55 56 57 48 81 EC 90 00 00 00 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 44 24 50 F3 0F 2C B1 2C 05 00 00");
        if (aa && HookAimAdj(aa)) { wsprintfA(b, "upper-body aim hook installed at exe+%X", (u32)(aa - g_base)); Log(b); }
        else Log("upper-body aim hook NOT installed: Snake's torso will not follow the aim");
    }
    {   u8* q = Scan("8B 05 ?? ?? ?? ?? 0B 05 ?? ?? ?? ?? 7C 0C 83 3D ?? ?? ?? ?? 00 75 03 33 C0 C3 B8 01 00 00 00 C3");
        f_noCtl = (int (*)(void))(void*)q;
        if (q) { wsprintfA(b, "event scenes: the game's no-control check found at exe+%X", (u32)(q - g_base)); Log(b); }
        else Log("event scenes: the game's no-control check NOT found (statuses 0x139 / 0x9B still checked)");
    }
    {   // where the game copies the pad into Snake
        u8* ip = Scan("48 89 54 24 10 48 89 4C 24 08 53 55 56 57 41 55 41 56 41 57 48 83 EC 40 0F 10 05 ?? ?? ?? ?? 8B 05 ?? ?? ?? ?? 33 DB 39 1D");
        if (ip) g_rawPad = Rip(ip + 0x18, 3, 7);   // the pad as the game read it (exe+1E2C560), for the log
        o_Inp = ip ? (Inp_t)Hook5x(ip, (void*)&hk_Inp, 0x54) : 0;
        if (o_Inp) { wsprintfA(b, "input hook installed at exe+%X: no wall hugging in the shoulder view", (u32)(ip - g_base)); Log(b); }
        else Log("input hook NOT installed: Snake can still stick to walls in the shoulder view");
    }
    {   // the camera style switch (R3) and the game's classic-camera mark
        u8* q = Scan("40 53 48 83 EC 20 48 8B 05 ?? ?? ?? ?? 48 8B D9 F7 40 08 00 80 00 00 75 ?? C7 05 ?? ?? ?? ?? 01 00 00 00 48 8B 89 E0 00 00 00 E8 ?? ?? ?? ?? 85 C0 74 08 48 8B CB E8 ?? ?? ?? ?? 81 25 ?? ?? ?? ?? FF FF FF FE");
        if (q) { g_camStyle = (volatile u32*)Rip(q + 0x19, 2, 10); g_camFlags = (volatile u32*)Rip(q + 0x3B, 2, 10); }
        if (q) { wsprintfA(b, "classic camera: the game's camera style switch found (exe+%X; style exe+%X, classic mark bit 1000000 of exe+%X) - now style %d, mark %d",
            (u32)(q - g_base), (u32)((u8*)g_camStyle - g_base), (u32)((u8*)g_camFlags - g_base), (int)*g_camStyle, (*g_camFlags & 0x1000000u) ? 1 : 0); Log(b); }
        else Log("classic camera: the game's camera style switch NOT found - the camera is checked against its camera number instead");
    }
    CrouchWalkInstall();
    wsprintfA(b, "active: Side=%d Height=%d Back=%d CrouchHeight=%d ProneHeight=%d ProneFloor=%d PitchLimit=%d PitchDown=%d EntryPitch=%d LeftShoulder=%d Strafe=%d MoveMode=%d tick exe+%X",
        cfgSide, cfgHeight, cfgBack, cfgCrouchH, cfgProneH, cfgProneFloor, cfgPitch, cfgPitchDown, cfgEntryPitch, cfgLeft, cfgStrafe, cfgMoveMode, p_tick ? (u32)((u8*)p_tick - g_base) : 0);
    Log(b);
    wsprintfA(b, "active: ProneSidePercent=%d ProneBack=%d ProneSpeed=%d Converge=%d ConvergeDistance=%d Range=%d ShotsToCrosshair=%d Sight Scale=%d Tilt=%d TiltSpeed=%d Ghost=%d FovPercent=%d",
        cfgProneSide, cfgProneBack, cfgProneSpeed, cfgConverge, cfgConvDist, cfgConvMax, cfgShots, cfgSightScale, cfgTilt, cfgTiltSpeed, cfgGhost, cfgFov);
    Log(b);
    wsprintfA(b, "active: PoseTurn=%d PadDeadZone=%d", cfgPoseTurn, cfgPadDZ); Log(b);
    wsprintfA(b, "active: TwoSpeeds=%d FastAbove=%d SlowKey=%d FastSpeed=%d CrouchFastSpeed=%d Collision=%d FollowBody=%d NoWallPress=%d KeepInStep=%d AfterFirstPerson=%d NoShoulderWeapons=%d NoRaiseWeapons=%d LookDownPose=%d",
        cfgTwoSpeed, cfgFastAbove, cfgSlowKey, cfgRunRoot, cfgCRunRoot, cfgCollide, cfgFollow, cfgNoWall, cfgSync, cfgAfterFpv, cfgNoOts, cfgNoRaise, cfgLookDown);
    Log(b);
    wsprintfA(b, "active: CrouchRunEverywhere=%d WalkAbove=%d StalkPercent=%d CrouchRunKeepsWeapon=%d BodyScale=%d BodyEase=%d ProneLookDownBack=%d ThrowableWeapons=%d ThrowableLookDown=%d FirstPersonHandover=%d",
        cfgCrouchAll, cfgWalkAbove, cfgStalkPct, cfgCrouchArms, cfgBodyScale, cfgBodyEase, cfgProneLook, cfgThrowMask, cfgThrowDown, cfgFpvHold);
    Log(b);
    wsprintfA(b, "active: CameraRigid=%d LockToHead=%d Blend=%d SpeedSwitchMs=%d Recoil=%d RecoilPercent=%d RecoilViewPercent=%d RecoilShakePercent=%d PadDirection=%d KeyboardWalkKeys=%d FirstPersonBackMs=%d",
        cfgRigid, cfgHeadLock, cfgBlend, cfgSpeedSwitch, cfgRecoil, cfgRecoilPct, cfgRecoilView, cfgRecoilShake, cfgPadDir, cfgKeyWalk, cfgFpvBackMs);
    Log(b);
    wsprintfA(b, "active: Sight GapPercent=%d RecoilSpreadWeapons=%d CrouchRunPercent=%d PadBlockCameraButton=%d PadStickUpLooksUp=%d PadDpadStalk=%d StalkKey=%d",
        cfgSightGap, cfgSpreadMask, cfgCrouchRunPct, cfgBlockCamBtn, cfgPadLookUp, cfgDpadStalk, cfgStalkKey);
    Log(b);
    wsprintfA(b, "active: FastMemoryCheck=%d WallDistance=%d MinBack=%d CollisionInPercent=%d CollisionOutPercent=%d CollisionHoldFrames=%d FirstPersonDirection=%d Sounds=%d (%d/%d/%d/%d/%d) Crawl=%d",
        cfgFastMem, cfgCamRadius, cfgCamMinBack, cfgCamIn, cfgCamOut, cfgCamHold, cfgFpvYaw, cfgSnd, cfgSndStandFast, cfgSndStandSlow, cfgSndCrouchFast, cfgSndCrouchSlow, cfgSndProne, cfgCrawlSnd);
    Log(b);
    wsprintfA(b, "active: PadFirstPersonKeepsButton=%d PadFirstPersonRestore=%d PadFirstPersonHandBack=%d PadFirstPersonKeepsRB=%d NoShoulderInClassicCamera=%d CrawlWalkPercent=%d", cfgFpvKeepL1, cfgFpvRestore, cfgFpvHandBack, cfgR1Keep, cfgClassicBlock, cfgCrawlWalkPct);
    Log(b);

    {
        u8* cs = Scan("BF 42 00 84 00 BE 1F 00 00 00 0F 44 F9 B9 0F 00 00 00 0F 44 F1");
        if (cs && cs[0x38] == 0x8B && cs[0x39] == 0x0D && cs[0x52] == 0xE8 && cs[0x6C] == 0xE8 && cs[0x81] == 0xE8 && cs[0x86] == 0xE8 && cs[0xCE] == 0xE8 && cs[0xE6] == 0xE8) {
            c_areaArg = (volatile u32*)Rip(cs + 0x38, 2, 6);
            c_area = (ColArea_t)(void*)Rip(cs + 0x52, 1, 5); c_sort = (ColSort_t)(void*)Rip(cs + 0x81, 1, 5);
            c_num = (ColNum_t)(void*)Rip(cs + 0x86, 1, 5); c_flag = (ColFlag_t)(void*)Rip(cs + 0xCE, 1, 5);
            c_point = (ColPoint_t)(void*)Rip(cs + 0xE6, 1, 5);
            c_test = (ColTest_t)(void*)Rip(cs + 0x6C, 1, 5);
            wsprintfA(b, "collision test found at exe+%X", (u32)((u8*)c_test - g_base)); Log(b);
        } else Log("collision test NOT found: the crosshair cannot react to obstacles and shots are left to the game");
    }
    if (c_test) {
        // the two bullet makers are reached through the calls in Snake's fire routines
        u8* fa = Scan("49 8D 97 E0 06 00 00 44 89 74 24 50 44 8B CD 89 74 24 48 44 8B C0 C7 44 24 40 88 13 00 00 F3 0F 11 44 24 38 "
                      "C7 44 24 30 32 00 00 00 C7 44 24 28 00 00 00 00 C7 44 24 20 00 00 00 00 E8");
        if (fa && fa[0x3C] == 0xE8 && fa[0x76] == 0xE8) {
            u8* at = Rip(fa + 0x3C, 1, 5); u8* nb = Rip(fa + 0x76, 1, 5);
            g_nbLo = fa - 0x3000; g_nbHi = fa + 0x6000;
            f_NBAt = (NBAt_t)(void*)at;
            o_NB = (NB_t)Hook5(nb, (void*)&hk_NB);
            if (o_NB) { wsprintfA(b, "bullet hook installed at exe+%X (aimed variant exe+%X): shots follow the crosshair", (u32)(nb - g_base), (u32)(at - g_base)); Log(b); }
            else { f_NBAt = 0; Log("bullet hook NOT installed: shots are left to the game"); }
            if (o_NB) {   // notice first-person shots (to learn the recoil); the plugin's own calls go to the original
                o_NBAt = (NBAt_t)Hook5(at, (void*)&hk_NBAt);
                if (o_NBAt) { f_NBAt = o_NBAt; Log("first-person shot watch installed"); }
                else Log("first-person shot watch NOT installed");
            }
            {   // the game's recoil roll for Snake's guns
                u8* rc = Scan("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 48 83 EC 20 44 0F BF B2 C4 00 00 00 48 8B F1 8B 4A 7C");
                o_Recoil = rc ? (Recoil_t)Hook5y(rc, (void*)&hk_Recoil, 0x5C, 0x08) : 0;
                if (o_Recoil) { wsprintfA(b, "recoil hook installed at exe+%X: the shoulder view gets the recoil the game rolls for each shot", (u32)(rc - g_base)); Log(b); }
                else Log("recoil hook NOT installed: no recoil in the shoulder view");
                u8* bf = Scan("4C 8B DC 55 57 41 57 49 8D 6B A1 48 81 EC B0 00 00 00 41 0F 29 73 B8 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 45 F7 49 8B 00 4D 8B F9 49 89 73 D8");
                f_Bone = (Bone_t)(void*)bf;
                if (bf) { wsprintfA(b, "bone routine found at exe+%X (head height in the log)", (u32)(bf - g_base)); Log(b); }
                // the two routines it calls - a bone's quaternion to a matrix (exe+754240) and Snake's model matrix
                // (exe+B8860)
                if (bf && bf[0x80] == 0xE8 && bf[0x14A] == 0xE8) {
                    f_QMat = (QMat_t)(void*)Rip(bf + 0x80, 1, 5); f_MMat = (MMat_t)(void*)Rip(bf + 0x14A, 1, 5);
                    wsprintfA(b, "gun pose: bone matrix routines found at exe+%X / exe+%X - the barrel can be followed every frame (PoseFix=%d)", (u32)((u8*)f_QMat - g_base), (u32)((u8*)f_MMat - g_base), cfgPoseFix); Log(b);
                } else Log("gun pose: bone matrix routines NOT found - the gun's vertical aim is left to the game");
            }
            u8* fd = Scan("48 8D 93 E0 06 00 00 44 8B 83 20 01 00 00 41 B9 85 03 00 00 C7 44 24 60 03 00 00 00 48 8B CF 89 44 24 58 89 44 24 50 "
                          "89 74 24 48 C7 44 24 40 E8 03 00 00 F3 0F 11 44 24 38 C7 44 24 30 32 00 00 00 89 44 24 28 89 44 24 20 E8");
            if (o_NB && fd && fd[0x49] == 0xE8 && fd[0x9A] == 0xE8 && fd > g_nbLo && fd < g_nbHi) {
                u8* at2 = Rip(fd + 0x49, 1, 5); u8* nb2 = Rip(fd + 0x9A, 1, 5);
                f_NBAt2 = (NBAt2_t)(void*)at2;
                o_NB2 = (NB2_t)Hook5(nb2, (void*)&hk_NB2);
                if (o_NB2) { o_NBAt2 = (NBAt2_t)Hook5(at2, (void*)&hk_NBAt2); if (o_NBAt2) f_NBAt2 = o_NBAt2; }
                if (o_NB2) { wsprintfA(b, "second bullet hook installed at exe+%X (aimed variant exe+%X)", (u32)(nb2 - g_base), (u32)(at2 - g_base)); Log(b); }
                else { f_NBAt2 = 0; Log("second bullet hook NOT installed (one weapon class keeps the game's own shots)"); }
            } else Log("second bullet maker not found (one weapon class keeps the game's own shots)");
        } else Log("bullet maker NOT found: shots are left to the game");
    }
    CreateThread(0, 0, SightThread, 0, 0, 0);
    CreateThread(0, 0, InputThread, 0, 0, 0);
    g_lthread = 1;   // from here on the log is written below
    int tick = 0, f6 = 0, f7 = 0, f8 = 0, f9 = 0;
    for (;;) {
        Sleep(30);
        LogFlush();
        // a key press the plugin made is always let go again, even if the camera code that normally does it has stopped
        // running (a loading screen right after the press left the aim key "held" before)
        if (g_injDown && GetTickCount() - g_injT > 150) { AimKeyEvent(0); g_injDown = 0; }
        if (g_fkDown && GetTickCount() - g_fkT > 80) { KeyEvent(g_fkDown, 0); g_fkDown = 0; }
        if (++tick >= 33) {   // pick up ini edits while the game runs
            // only when the file has changed
            static u64 lastW; FATTR_ fa;
            tick = 0;
            if (GetFileAttributesExA(g_ini, 0, &fa)) { u64 wt = ((u64)fa.w1 << 32) | fa.w0; if (wt != lastW) { if (lastW) Log("settings file changed: read again"); lastW = wt; LoadConfig(); } }
            else LoadConfig();
        }
        if (!cfgHotkeys) { g_strafeKey = g_modeKey = g_legsKey = g_spineKey = -1; continue; }
        int d = (GetAsyncKeyState(0x75) & 0x8000) != 0;   // F6: strafing on / off
        if (d && !f6) { g_strafeKey = !Strafe(); wsprintfA(b, "F6: Strafe=%d", (int)g_strafeKey); Log(b); }
        f6 = d;
        d = (GetAsyncKeyState(0x76) & 0x8000) != 0;   // F7: how Snake is moved (1 or 2)
        if (d && !f7) { g_modeKey = MoveMode() == 2 ? 1 : 2; wsprintfA(b, "F7: MoveMode=%d", (int)g_modeKey); Log(b); }
        f7 = d;
        d = (GetAsyncKeyState(0x77) & 0x8000) != 0;   // F8: leg animation on / off
        if (d && !f8) { g_legsKey = !Legs(); wsprintfA(b, "F8: Legs=%d", (int)g_legsKey); Log(b); }
        f8 = d;
        d = (GetAsyncKeyState(0x78) & 0x8000) != 0;   // F9: upper body follows the aim on / off
        if (d && !f9) { g_spineKey = !Spine(); wsprintfA(b, "F9: Spine=%d", (int)g_spineKey); Log(b); }
        f9 = d;
    }
    return 0;
}

static int HasSub(const char* s, const char* sub)
{
    for (; *s; s++) { int i = 0; while (sub[i] && s[i] == sub[i]) i++; if (!sub[i]) return 1; }
    return 0;
}

BOOL API DllMain(HANDLE mod, DWORD reason, void* reserved)
{
    (void)reserved;
    if (reason == 1) {
        DisableThreadLibraryCalls(mod);
        char exe[260];
        GetModuleFileNameA(0, exe, 260);
        if (!HasSub(exe, "METAL GEAR SOLID3")) return 1;   // ignore the launcher
        GetModuleFileNameA(mod, g_dir, 260);
        int n = slen(g_dir); while (n > 0 && g_dir[n - 1] != '\\') n--; g_dir[n] = 0;
        wsprintfA(g_ini, "%smgs3danim.ini", g_dir);
        wsprintfA(g_logPath, "%smgs3danim.log", g_dir);
        LoadConfig();
        if (cfgLog) g_log = CreateFileA(g_logPath, 0x40000000, 1, 0, 2, 0x80, 0);
        Log(cfgLogLvl >= 2 ? "mgs3danim 1.0 loaded (log: advanced)" : "mgs3danim 1.0 loaded (log: basic - set [Debug] Log=2 in mgs3danim.ini for the full diagnostic log)");
        // what the mod learns while playing (muzzle positions, Snake's head, the gun pose, the barrel lines) is kept in memory
        // only: every game start begins from the built-in values and learns again within the first shots
        MuzzleDefaults();
        for (int w = 0; w < 64; w++) for (int st = 0; st < 3; st++) for (int i = 0; i < 3; i++) g_moSaved[w][st][i] = g_mo[w][st][i];
        CreateThread(0, 0, InitThread, 0, 0, 0);
    }
    return 1;
}
