#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#define __declspec(x)
#define __stdcall
#define __cdecl
#define __fastcall
#include "../mgs3danim.c"
static const char* g_mtarPath = "006891cc.mtar";   // Snake's motion archive from the game's data (path: first argument)
// ---- stubs for the Windows imports
static DWORD clockMs = 100000;
HANDLE GetModuleHandleA(const char* a) { (void)a; return 0; }
void* VirtualAlloc(void* a, u64 b, DWORD c, DWORD d) { (void)a;(void)b;(void)c;(void)d; return 0; }
BOOL VirtualProtect(void* a, u64 b, DWORD c, DWORD* d) { (void)a;(void)b;(void)c;(void)d; return 0; }
static long nVQ, nIBR, nXI; u64 VirtualQuery(const void* p, MBI* m, u64 n) { nVQ++; (void)n; if ((u64)p < 0x10000) return 0; m->BaseAddress = (void*)p; m->RegionSize = 1ull << 40; m->State = 0x1000; m->Protect = 4; return 1; }
HANDLE CreateThread(void* a, u64 b, DWORD (*c)(void*), void* d, DWORD e, DWORD* f) { (void)a;(void)b;(void)c;(void)d;(void)e;(void)f; return 0; }
void Sleep(DWORD a) { (void)a; }
DWORD GetTickCount(void) { return clockMs; }
static i64 qpc; BOOL QueryPerformanceCounter(LARGE_* a) { qpc += 1000; a->q = qpc; return 1; }
BOOL QueryPerformanceFrequency(LARGE_* a) { a->q = 10000000; return 1; }
BOOL GetFileAttributesExA(const char* a, int b, FATTR_* c) { (void)a;(void)b;(void)c; return 0; }
BOOL IsBadReadPtr(const void* p, u64 n) { (void)n; nIBR++; return (u64)p < 0x10000; }
BOOL ReadProcessMemory(HANDLE h, const void* p, void* b, u64 n, u64* got) { (void)h; nIBR++; if ((u64)p < 0x10000) { if (got) *got = 0; return 0; } memcpy(b, p, n); if (got) *got = n; return 1; }
DWORD GetModuleFileNameA(HANDLE a, char* b, DWORD c) { (void)a;(void)c; b[0] = 0; return 0; }
u32 GetPrivateProfileIntA(const char* a, const char* b, i32 c, const char* d) { (void)a;(void)b;(void)d; return (u32)c; }
static int wpsN; static char wpsVal[32];
BOOL WritePrivateProfileStringA(const char* a, const char* b, const char* c, const char* d) { (void)a;(void)b;(void)d; wpsN++; int i=0; while (c[i] && i < 31) { wpsVal[i] = c[i]; i++; } wpsVal[i] = 0; return 1; }
HANDLE CreateFileA(const char* a, DWORD b, DWORD c, void* d, DWORD e, DWORD f, HANDLE g) { (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g; return 0; }
static int showLog = 1;
BOOL WriteFile(HANDLE h, const void* p, DWORD n, DWORD* w, void* o) { (void)h;(void)o; if (showLog) fwrite(p, 1, n, stdout); if (w) *w = n; return 1; }
BOOL FlushInstructionCache(HANDLE a, const void* b, u64 c) { (void)a;(void)b;(void)c; return 1; }
HANDLE GetCurrentProcess(void) { return 0; }
BOOL DisableThreadLibraryCalls(HANDLE a) { (void)a; return 1; }
int wsprintfA(char* b, const char* f, ...) { va_list v; va_start(v, f); int n = vsprintf(b, f, v); va_end(v); return n; }
static int keyDown[256], simLatch, injClicks, latchable = 1;
i16 GetAsyncKeyState(int a) { return keyDown[a & 255] ? (i16)0x8000 : 0; }
static int kbInj[256];
u32 SendInput(u32 a, INPUT_* b, int c) { (void)a;(void)c; if (b->type == 1 && !(b->u.ki.flags & 2)) kbInj[b->u.ki.vk & 255]++; if (b->type == 0 && (b->u.mi.flags & 0x8)) { injClicks++; keyDown[2] = 1; if (latchable) simLatch = !simLatch; } if (b->type == 0 && (b->u.mi.flags & 0x10)) keyDown[2] = 0; return 1; }
static HANDLE simFw = (HANDLE)0x1234; char simClass[64] = "GameWindowClass";
HANDLE GetForegroundWindow(void) { return simFw; }
DWORD GetWindowThreadProcessId(HANDLE a, DWORD* b) { (void)a; if (b) *b = 1; return 0; }
u32 MapVirtualKeyA(u32 a, u32 b) { (void)a;(void)b; return 0; }
DWORD GetCurrentProcessId(void) { return 1; }
BOOL ReadFile(HANDLE a, void* b, DWORD c, DWORD* d, void* e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
BOOL CloseHandle(HANDLE a) { (void)a; return 1; }
static struct { int conn; i16 lx, ly, ry; u16 btn; } simPad;
static u32 s_xget(u32 i, XSTATE_* st) { nXI++; (void)i; if (!simPad.conn) return 1167; st->lx = simPad.lx; st->ly = simPad.ly; st->ry = simPad.ry; st->rx = 0; st->buttons = simPad.btn; st->lt = st->rt = 0; return 0; }
HANDLE LoadLibraryA(const char* a) { return (a && a[0] == 'x') ? (HANDLE)1 : 0; }
void* GetProcAddress(HANDLE a, const char* b) { (void)a; return (b && b[0] == 'X') ? (void*)&s_xget : 0; }
u16 RegisterClassA(const WNDCLASSA_* a) { (void)a; return 0; }
HANDLE CreateWindowExA(DWORD a, const char* b, const char* c, DWORD d, i32 e, i32 f, i32 g, i32 h, HANDLE i, HANDLE j, HANDLE k, void* l) { (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;(void)i;(void)j;(void)k;(void)l; return 0; }
i64 DefWindowProcA(HANDLE a, u32 b, u64 c, i64 d) { (void)a;(void)b;(void)c;(void)d; return 0; }
BOOL ShowWindow(HANDLE a, int b) { (void)a;(void)b; return 0; }
BOOL SetWindowPos(HANDLE a, HANDLE b, i32 c, i32 d, i32 e, i32 f, u32 g) { (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g; return 0; }
BOOL UpdateLayeredWindow(HANDLE a, HANDLE b, POINT_* c, POINT_* d, HANDLE e, POINT_* f, u32 g, u32* h, u32 i) { (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;(void)i; return 0; }
BOOL GetClientRect(HANDLE a, RECT_* b) { (void)a; b->l = 0; b->t = 0; b->r = 1280; b->b = 720; return 1; }
int GetClassNameA(HANDLE a, char* b, int c) { (void)a;(void)c; memcpy(b, simClass, 64); return 15; }
BOOL ClientToScreen(HANDLE a, POINT_* b) { (void)a;(void)b; return 0; }
BOOL PeekMessageA(MSG_* a, HANDLE b, u32 c, u32 d, u32 e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
i64 DispatchMessageA(const MSG_* a) { (void)a; return 0; }
BOOL IsWindow(HANDLE a) { (void)a; return 0; }
BOOL IsIconic(HANDLE a) { (void)a; return 0; }
HANDLE CreateCompatibleDC(HANDLE a) { (void)a; return 0; }
HANDLE CreateDIBSection(HANDLE a, const BMI_* b, u32 c, void** d, HANDLE e, DWORD f) { (void)a;(void)b;(void)c;(void)d;(void)e;(void)f; return 0; }
HANDLE SelectObject(HANDLE a, HANDLE b) { (void)a;(void)b; return 0; }
BOOL DeleteObject(HANDLE a) { (void)a; return 1; }

// ---- a little world: ground y = 0, a wall at z = 20000, optionally a tree trunk in front of the gun
static int tree, backWall, sideWall, postWall, flick, flickN, wallZ = -600; static float hits[8][3]; static int nh;
static void addhit(const float* a, const float* b, float t) { if (t > 0 && t < 1 && nh < 8) { for (int i = 0; i < 3; i++) hits[nh][i] = a[i] + (b[i] - a[i]) * t; nh++; } }
static u32 s_area(u32 base, const float* a, const float* b) { (void)a;(void)b; return base; }
static int s_test(u32 type, u32 area, u32 flags, const float* a, const float* b, float r)
{
    (void)type;(void)area;(void)flags;(void)r; nh = 0;
    if (a[1] != b[1]) addhit(a, b, (0 - a[1]) / (b[1] - a[1]));
    if (a[2] != b[2]) addhit(a, b, (20000 - a[2]) / (b[2] - a[2]));
    if (backWall && a[2] != b[2]) addhit(a, b, ((float)wallZ - a[2]) / (b[2] - a[2]));
    if (flick && (flickN & 1) && a[2] != b[2]) addhit(a, b, (-900 - a[2]) / (b[2] - a[2]));
    if (sideWall && a[0] != b[0]) addhit(a, b, (-300 - a[0]) / (b[0] - a[0]));
    if (postWall && a[0] != b[0]) { float t = (-560 - a[0]) / (b[0] - a[0]); float z = a[2] + (b[2] - a[2]) * t; if (z < -800) addhit(a, b, t); }
    if (tree && a[2] != b[2]) {   // front face of a box x -300..200, z 1500..1900
        float t = (1500 - a[2]) / (b[2] - a[2]); float x = a[0] + (b[0] - a[0]) * t;
        if (x > -300 && x < 200) addhit(a, b, t);
    }
    return nh > 0;
}
static void s_sort(void) {}
static int s_num(void) { return nh; }
static u32 s_flag(int i) { (void)i; return 0; }
static void s_point(int i, float* o) { o[0] = hits[i][0]; o[1] = hits[i][1]; o[2] = hits[i][2]; o[3] = 1; }
static float camEye[3]; static int camInit; static float kSeen = -1;
static void s_apply(u8* w)
{   // the game's apply: the camera goes work+0xA0 of the way to the desired eye (all of it on a cut)
    float* d = (float*)(w + 0x3D0);
    float k = *(float*)(w + 0xA0); kSeen = k;
    if (*(u32*)(w + 0x320) & 1) { *(u32*)(w + 0x320) &= ~1u; k = 1.0f; }
    if (!camInit) { memcpy(camEye, d, 12); camInit = 1; }
    for (int i = 0; i < 3; i++) camEye[i] += (d[i] - camEye[i]) * k;
}
static float PitOfElev(float e)
{
    if (e <= kElev[0]) return kPit[0];
    for (int i = 1; i < NPIT; i++) if (e <= kElev[i]) return kPit[i - 1] + (kPit[i] - kPit[i - 1]) * (e - kElev[i - 1]) / (kElev[i] - kElev[i - 1]);
    return kPit[NPIT - 1];
}
static u8 work[0x800] __attribute__((aligned(16))), pl[0x1000] __attribute__((aligned(16))), wp[0x80], cam[0x100];
static u32 bits[32];
static float mouse;   // elevation change the "game" applies this frame
static void frame(void)
{
    float ex = *(float*)(work + 0x340) + mouse; if (ex < 0) ex = 0; if (ex > 1) ex = 1;
    *(float*)(work + 0x340) = ex;
    float p = PitOfElev(ex) * 0.0174532925f;
    float* eye = (float*)(work + 0x3D0); float* tgt = (float*)(work + 0x3E0);
    tgt[0] = *(float*)(pl + 0x130); tgt[1] = *(float*)(pl + 0x134) + 695; tgt[2] = *(float*)(pl + 0x138);
    eye[0] = tgt[0]; eye[1] = tgt[1] - sinf(p) * 4000; eye[2] = tgt[2] - cosf(p) * 4000;
    clockMs += 16;
    *(u32*)(pl + 0x7E8) = (*(u32*)(pl + 0x7E8) & ~0x8000u) | (simLatch ? 0x8000u : 0);
    InputPoll();   // the input thread, once a frame here
    hk_Apply(work);
}
static void setbit(int n, int v) { if (v) bits[n >> 5] |= 1u << (n & 31); else bits[n >> 5] &= ~(1u << (n & 31)); }
static float orbitDeg(void) { return PitOfElev(*(float*)(work + 0x340)); }

static u8* arcBuf; static long arcLen;
static void loadArc(void) { FILE* f = fopen(g_mtarPath, "rb"); if (!f) { fprintf(stderr, "cannot open Snake's motion archive %s\n", g_mtarPath); exit(2); } fseek(f, 0, 2); arcLen = ftell(f); fseek(f, 0, 0); arcBuf = (u8*)calloc(1, arcLen + 65536); fread(arcBuf, 1, arcLen, f); fclose(f); }
static u8 mcT[0x100], lyT[0x200];
static void playSlot(int slot, float frameNo)
{
    u64 off = *(u64*)(arcBuf + 0x200 + slot * 16);
    u8* hdr = arcBuf + off;
    *(u8**)(pl + 0x2B8) = mcT; *(u8**)(mcT + 8) = lyT;
    *(u8**)(lyT + 0x10) = hdr; *(u8**)(lyT + 0x18) = hdr + *(u32*)(hdr + 0x10);
    *(float*)(lyT + 0x48) = frameNo * (float)*(u32*)(hdr + 8);
    *(u32*)(lyT + 0x54) = 0x6891CC;
}
static u16 inpMag; static u64 s_inp(u8* p, u8* w, u32 m, u64 a) { (void)w;(void)m;(void)a; *(u16*)(p + 0x7E0) = inpMag; return 0; }
static u8 lastReq[0x60]; static void s_req(u8* p, u8* r) { (void)p; memcpy(lastReq, r, 0x60); *(u32*)(p + 0x74) = *(u32*)(r + 0x0C); *(i16*)(p + 0x78) = *(i16*)(r + 0x10); }

// ======================= 0.27 =======================
static void clickX(void) { keyDown[2] = 1; if (latchable) simLatch = !simLatch; for (int q = 0; q < 4; q++) frame(); keyDown[2] = 0; for (int q = 0; q < 40; q++) frame(); }
static int asmFade(int v)   // literal transcription of exe+37A5B9
{
    int r10 = (i16)v, r8 = r10, r9 = 2;
    for (;;) { int eax = r8; int edx = eax < 0 ? -1 : 0; eax = (eax - edx) >> 1; if (eax == 0) break; r9 -= 1; r8 = eax; if (r9 < 0) break; }
    return (i16)(r10 - r8);
}
static u32 grng = 777;
static u32 gR(void) { grng = grng * 0x5D588B65u + 1u; return grng; }
static int fakeRolls;
static u64 s_recoil(u8* p, u8* w)
{   // the game's roll (exe+379290) for weapon type w+0x7C
    fakeRolls++;
    int type = *(int*)(w + 0x7C), prone = Bit(3), crouch = Bit(ST_CROUCH) && !prone;
    int op = *(i16*)(w + 0xC4), oy = *(i16*)(w + 0xC6);
    u32 range, shake;
    if (type == 9 || type == 10) { range = prone ? 0xF : crouch ? 0x1F : 0xFF; shake = (prone || crouch) ? 0x40 : 0x80; }
    else { if (type == 12 && !prone) { int c = crouch ? -(0x24 + (int)(gR() & 0x3F)) : -(0x5B + (int)(gR() & 0x7F)); *(i16*)(p + 0x7C0) += (i16)c; *(i16*)(p + 0x7A0) = *(i16*)(p + 0x7C0); }
           range = prone ? 0xFF : crouch ? 0x3FF : 0xFFFF; shake = (prone || crouch) ? 0x40 : 0x100; }
    *(i16*)(p + 0x7A0) += (i16)((gR() & (shake - 1)) - (shake >> 1)); *(i16*)(p + 0x7A2) += (i16)((gR() & (shake - 1)) - (shake >> 1));
    int np = (i16)(u16)((gR() & range) - (range >> 1)), ny = (i16)(u16)((gR() & range) - (range >> 1));
    if (np - op > 0x16C) np = op + 0x16C; if (np - op < -0x16C) np = op - 0x16C;
    if (ny - oy > 0x16C) ny = oy + 0x16C; if (ny - oy < -0x16C) ny = oy - 0x16C;
    *(i16*)(w + 0xC4) = (i16)np; *(i16*)(w + 0xC6) = (i16)ny;
    return 0;
}
static void test_part00(void)
{
    // ---- fade identical to the game
    { int bad = 0; for (int v = -32768; v < 32768; v++) if (Fade8(v) != asmFade(v)) bad++; printf("\n-- part 00 --\nfade per frame equal to the game's for every value: %s (%d differ)\n", bad ? "NO" : "yes", bad); }

    // ---- 1. rigid camera
    printf("\n-- 1a. the camera keeps up with Snake (game smoothing 0.25 per frame emulated) --\n");
    cfgAimMode = 0; simLatch = 0; keyDown[2] = 0; setbit(ST_FPV, 0); setbit(ST_TPS, 1); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); setbit(3, 0);
    wp[0x20] = 5; playSlot(0, 0); *(float*)(mcT + 0x74) = 8.0f; *(u32*)(mcT + 0x18) = 0;
    for (int rig = 0; rig < 2; rig++) {
        cfgRigid = rig; setbit(ST_WEAPON, 0); clockMs += 2000; for (int i = 0; i < 40; i++) frame();
        setbit(ST_WEAPON, 1); camInit = 0;
        float lagMax = 0, lagEnd = 0;
        for (int i = 0; i < 90; i++) {
            *(float*)(pl + 0x138) += 30.0f; *(float*)(pl + 0x134) = 1000.0f + 25.0f * sinf(i * 0.7f);   // walking on bumpy ground
            frame();
            float* d = (float*)(work + 0x3D0); float L = sqrtf((d[0]-camEye[0])*(d[0]-camEye[0]) + (d[1]-camEye[1])*(d[1]-camEye[1]) + (d[2]-camEye[2])*(d[2]-camEye[2]));
            if (i > 40 && L > lagMax) lagMax = L; lagEnd = L;
        }
        printf("CameraRigid=%d: walking 30/frame: camera behind where it should be: up to %.1f units (last %.1f); smoothing given to the game %.3f, value after the frame %.3f\n",
            rig, lagMax, lagEnd, kSeen, *(float*)(work + 0xA0));
    }
    *(float*)(pl + 0x138) = 0; *(float*)(pl + 0x134) = 1000; cfgRigid = 1;

    printf("\n-- 1b. standing -> crouching: the game blends the pose over 8 frames --\n");
    for (int mode = 0; mode < 2; mode++) {
        // mode 0: the game's value matches the motion data (normal); mode 1: it does not (plugin blends itself)
        setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); playSlot(0, 0);
        for (int i = 0; i < 60; i++) { *(float*)(mcT + 0x74) = mode ? 0.0f : 8.0f; *(u32*)(mcT + 0x18) = 0; frame(); }
        printf("%s: hip source %d (1 = the game's blend, 2 = the plugin's), body %d\n", mode ? "game value wrong" : "game value right", (int)g_hipSrc, (int)g_body100);
        setbit(ST_STAND, 0); setbit(ST_CROUCH, 1);
        float from = 8.0f; playSlot(2, 0); g_hipHdr = 0; float to = BodyHipY(pl);
        for (int i = 0; i < 12; i++) {
            playSlot(2, 0);
            float w = i < 8 ? (i + 1) / 8.0f : 1.0f;
            *(u32*)(mcT + 0x18) = i < 8 ? 0x4000 : 0; *(float*)(mcT + 0x20) = w;
            *(float*)(mcT + 0x74) = mode ? 0.0f : from + (to - from) * w;
            frame();
            printf("   frame %2d: pose blend %3d%%  game hip %.2f  decoded %.2f  camera body %4d  (rigid smoothing %d/1000)\n", i, (int)(w * 100), from + (to - from) * w, g_hip100 / 100.0f, (int)g_body100, (int)g_rigid1000);
        }
        setbit(ST_STAND, 1); setbit(ST_CROUCH, 0);
        if (mode == 0) {   // leave the shoulder view so the next run starts fresh
            setbit(ST_WEAPON, 0); clockMs += 2000; for (int i = 0; i < 40; i++) frame(); setbit(ST_WEAPON, 1); for (int i = 0; i < 40; i++) frame(); }
    }
    setbit(ST_WEAPON, 0); playSlot(0, 0); for (int i = 0; i < 60; i++) { *(float*)(mcT + 0x74) = 8.0f; *(u32*)(mcT + 0x18) = 0; frame(); }

    // ---- 2. first person, gun lowered and raised there, out of first person, shoulder view
    printf("\n-- 2. shoulder view after first person --\n");
    u8* fpc = *(u8**)(pl + 0x730);
    *(float*)(fpc + 0x20) = 0; *(float*)(fpc + 0x24) = 1850; *(float*)(fpc + 0x28) = 60;
    *(float*)(fpc + 0x30) = 0; *(float*)(fpc + 0x34) = 1850; *(float*)(fpc + 0x38) = 1060;
    cfgAimMode = 1; simLatch = 0; wp[0x20] = 12; for (int i = 0; i < 30; i++) frame();
    for (int run = 0; run < 2; run++) {
        clickX();   // shoulder view on
        setbit(ST_TPS, 0); for (int i = 0; i < 15; i++) frame(); setbit(ST_FPV, 1); for (int i = 0; i < 20; i++) frame();
        if (run == 0) { latchable = 0; simLatch = 0; clickX(); simLatch = 1; clickX(); latchable = 1; }   // gun down, gun up in first person
        setbit(ST_FPV, 0);
        if (run == 0) for (int i = 0; i < 15; i++) frame();   // a quarter second in the normal view before the shoulder view comes
        setbit(ST_TPS, 1);
        int flew = 0; float minD = 1e9f;
        for (int i = 0; i < 30; i++) { frame(); if (g_fly == 2) flew = 1;
            float* d = (float*)(work + 0x3D0); float dx = d[0] - 0, dy = d[1] - 1850, dz = d[2] - 60; float L = sqrtf(dx*dx+dy*dy+dz*dz); if (g_amt > 0.5f && L < minD) minD = L; }
        printf("%s: shoulder %d, moved out from his head: %s, nearest the camera came to his eye: %.0f\n",
            run == 0 ? "gun lowered + raised in first person, then out" : "straight back from first person", (int)g_aimLatch, flew ? "yes" : "no", minD);
        clickX(); for (int i = 0; i < 30; i++) frame();
    }

    // ---- 3. recoil
    printf("\n-- 3. recoil --\n");
    static u8 gw[0x100]; o_Recoil = s_recoil; cfgAimMode = 0; simLatch = 0;
    setbit(ST_WEAPON, 1); *(float*)(work + 0x340) = 0.6f; for (int i = 0; i < 60; i++) frame();
    float m[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }, mm[16], tg[4];
    struct { int type; int stance; const char* nm; } cs[] = { { 13, 0, "M63 standing" }, { 13, 1, "M63 crouched" }, { 13, 2, "M63 lying" }, { 12, 0, "AK-47 standing" }, { 10, 0, "Scorpion standing" } };
    for (int c = 0; c < 5; c++) {
        wp[0x20] = (u8)cs[c].type; *(int*)(gw + 0x7C) = cs[c].type; *(i16*)(gw + 0xC4) = 0; *(i16*)(gw + 0xC6) = 0;
        setbit(ST_STAND, cs[c].stance == 0); setbit(ST_CROUCH, cs[c].stance == 1); setbit(3, cs[c].stance == 2);
        g_crouchLatch = cs[c].stance == 1; g_proneLatch = cs[c].stance == 2;
        for (int i = 0; i < 40; i++) frame();
        float p0 = g_pitchDeg; float maxDev = 0; int pos = 0, neg = 0, up = 0, dn = 0;
        printf("%s:", cs[c].nm);
        for (int sh = 0; sh < 12; sh++) {
            hk_Recoil(pl, gw);
            for (int i = 0; i < 3; i++) m[12 + i] = g_tQ[i] + g_tF[i] * 700 + g_tR[i] * 120 + g_tU[i] * -150;
            int how = ShotPlan(m, 11, (void*)0x1000, mm, tg);
            const float* O = how == 1 ? mm + 12 : m + 12;
            float D[3] = { tg[0] - O[0], tg[1] - O[1], tg[2] - O[2] }; float L = sqrtf(D[0]*D[0]+D[1]*D[1]+D[2]*D[2]);
            float right = (D[0]*g_tR[0]+D[1]*g_tR[1]+D[2]*g_tR[2]) / L * 57.3f, upd = (D[0]*g_tU[0]+D[1]*g_tU[1]+D[2]*g_tU[2]) / L * 57.3f;
            // the aimed direction for comparison (no kick)
            float A[3] = { g_tPt[0] - O[0], g_tPt[1] - O[1], g_tPt[2] - O[2] }; float LA = sqrtf(A[0]*A[0]+A[1]*A[1]+A[2]*A[2]);
            if (how == 1) { A[0] = g_tF[0]; A[1] = g_tF[1]; A[2] = g_tF[2]; LA = 1; }
            right -= (A[0]*g_tR[0]+A[1]*g_tR[1]+A[2]*g_tR[2]) / LA * 57.3f; upd -= (A[0]*g_tU[0]+A[1]*g_tU[1]+A[2]*g_tU[2]) / LA * 57.3f;
            if (sh < 6) printf(" (%+.1f,%+.1f)", right, upd);
            float dv = sqrtf(right*right+upd*upd); if (dv > maxDev) maxDev = dv;
            if (right > 0.05f) pos++; if (right < -0.05f) neg++; if (upd > 0.05f) up++; if (upd < -0.05f) dn++;
            for (int i = 0; i < 5; i++) { frame(); *(i16*)(gw + 0xC4) = (i16)asmFade(*(i16*)(gw + 0xC4)); *(i16*)(gw + 0xC6) = (i16)asmFade(*(i16*)(gw + 0xC6)); }
        }
        printf("\n   shots off the crosshair up to %.1f deg; right %d / left %d, up %d / down %d; view pitch moved %+.2f deg; pl+0x7C0 left at %d\n",
            maxDev, pos, neg, up, dn, g_pitchDeg - p0, (int)*(i16*)(pl + 0x7C0));
    }
    setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); setbit(3, 0); g_crouchLatch = g_proneLatch = 0;
    // a gun the game rolls nothing for (pistol): no recoil
    { wp[0x20] = 5; for (int i = 0; i < 30; i++) frame(); u32 e0 = g_rkEmu, s0 = g_rkSpread;
      for (int sh = 0; sh < 5; sh++) { for (int i = 0; i < 3; i++) m[12 + i] = g_tQ[i] + g_tF[i] * 700; ShotPlan(m, 11, (void*)0x1000, mm, tg); for (int i = 0; i < 5; i++) frame(); }
      printf("Mk22 (the game rolls nothing for it here): emulated %u, shots turned %u\n", g_rkEmu - e0, g_rkSpread - s0); }
    // the game rolls in first person but not for shoulder-view shots: the plugin rolls the same way
    { wp[0x20] = 13; setbit(ST_FPV, 1); *(int*)(gw + 0x7C) = 13; hk_Recoil(pl, gw); setbit(ST_FPV, 0); setbit(ST_TPS, 1);
      for (int i = 0; i < 40; i++) frame(); u32 e0 = g_rkEmu, s0 = g_rkSpread;
      for (int sh = 0; sh < 6; sh++) { for (int i = 0; i < 3; i++) m[12 + i] = g_tQ[i] + g_tF[i] * 700; ShotPlan(m, 11, (void*)0x1000, mm, tg); for (int i = 0; i < 5; i++) frame(); }
      printf("M63, the game rolls only in first person: plugin rolled %u times, shots turned %u, kick now %d,%d\n", g_rkEmu - e0, g_rkSpread - s0, (int)g_kP, (int)g_kY); }
    setbit(ST_WEAPON, 0); for (int i = 0; i < 40; i++) frame();

    // ---- 4. walk switch near the edge
    printf("\n-- 4. walk switch --\n");
    { static i32 tk = 5; p_tick = &tk; g_aimLatch = 1;
      for (int kind = 0; kind < 2; kind++) {
        u32 s0 = g_speedSw; int flips = 0, was = g_fast;
        for (int f = 0; f < 60; f++) {
            g_have = 1; g_inYaw = g_camYaw16;
            g_inMag = kind == 0 ? ((f / 4) % 2 ? 160 : 182) : ((f / 20) % 2 ? 128 : 255);   // a pad near the edge / Ctrl toggled on a keyboard
            clockMs += 16; Push(pl); if (g_fast != was) { flips++; was = g_fast; }
        }
        printf("%s: walk changed %d times in a second (asked for %d)\n", kind == 0 ? "stick wobbling around the edge every 4 frames" : "keyboard walk key toggled every 20 frames", flips, kind == 0 ? 14 : 2);
      }
      g_have = 0; }
}
#define TEST_PART01() test_part00()

// ======================= 0.28 =======================
static float simHead[3];   // where the "game" puts his head bone this frame (world)
static u8 skel[0x50 * 20], quats[16 * 20], modelX[0x100];
static void s_bone(float* out, int bone, u8* pair, u8* ctrl) { (void)bone; (void)pair; (void)ctrl; out[0] = simHead[0]; out[1] = simHead[1]; out[2] = simHead[2]; out[3] = 1; }
static void setupBones(void)
{
    for (int b = 0; b < 20; b++) *(int*)(skel + b * 0x50 + 8) = b - 1;
    *(u8**)(modelX + 0xB8) = skel; *(u8**)(pl + 0x2B0) = modelX;
    *(u8**)(mcT + 0x50) = quats; *(int*)(mcT + 0x14) = 20;
    f_Bone = s_bone;
}
static void test_part01(void)
{
    printf("\n\n-- part 01 --\n");
    setupBones(); cfgAimMode = 0; simLatch = 0; keyDown[2] = 0; o_Recoil = s_recoil;
    setbit(ST_FPV, 0); setbit(ST_TPS, 1); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); setbit(3, 0); setbit(6, 0);
    wp[0x20] = 5; playSlot(0, 0); *(float*)(pl + 0x130) = 0; *(float*)(pl + 0x134) = 1000; *(float*)(pl + 0x138) = 0; *(u16*)(pl + 0x162) = 0; *(u16*)(pl + 0x7E0) = 0; *(u32*)(pl + 0x7E8) = 0;
    // standing still: his head 820 above his position, 40 ahead
    simHead[0] = 0; simHead[1] = 1820; simHead[2] = 40;
    setbit(ST_WEAPON, 1); for (int i = 0; i < 80; i++) frame();
    printf("standing still in the shoulder view: learned %d ahead %d right %d up (%d)\n", (int)g_hdF, (int)g_hdR, (int)g_hdU, (int)g_headLearned);
    float e0[3]; memcpy(e0, (float*)(work + 0x3D0), 12);
    float r0[3] = { e0[0] - simHead[0], e0[1] - simHead[1], e0[2] - simHead[2] };
    for (int lock = 1; lock >= 0; lock--) {
        cfgHeadLock = lock; for (int i = 0; i < 10; i++) frame();
        float worst = 0;
        // crouching down: the head drops 650 over 10 frames and leans 120 forward; the hips drop differently
        setbit(ST_STAND, 0); setbit(ST_CROUCH, 1); setbit(6, 1);
        for (int i = 0; i < 40; i++) {
            float t = i < 10 ? (i + 1) / 10.0f : 1.0f;
            simHead[1] = 1820 - 650 * t; simHead[2] = 40 + 120 * t; simHead[0] = 30.0f * sinf(i * 0.4f);   // a little sideways sway too
            if (i == 10) setbit(6, 0);
            playSlot(i < 4 ? 0 : 2, 0);
            frame();
            float* e = (float*)(work + 0x3D0); float d[3] = { e[0] - simHead[0] - r0[0], e[1] - simHead[1] - r0[1], e[2] - simHead[2] - r0[2] };
            float L = sqrtf(d[0]*d[0]+d[1]*d[1]+d[2]*d[2]); if (L > worst) worst = L;
        }
        printf("LockToHead=%d: crouching down with the head moving 650 down / 120 forward / swaying: camera off its place relative to the head by up to %.1f units\n", lock, worst);
        setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); simHead[0] = 0; simHead[1] = 1820; simHead[2] = 40; playSlot(0, 0); for (int i = 0; i < 40; i++) frame();
    }
    cfgHeadLock = 1;
    // turning: the camera turns, he turns with it, his head is ahead of him - stays on the same screen spot?
    {   float worst = 0;
        for (int i = 0; i < 30; i++) {
            u16 y = (u16)(i * 600); *(u16*)(pl + 0x162) = y;
            simHead[0] = sin16(y) * 40; simHead[2] = sin16((u16)(y + 0x4000)) * 40; simHead[1] = 1820;
            frame();
            // in camera space: project eye - head onto the camera axes
            float* e = (float*)(work + 0x3D0); float* t = (float*)(work + 0x3E0);
            float fx = t[0] - e[0], fz = t[2] - e[2], hl = sqrtf(fx*fx+fz*fz); fx /= hl; fz /= hl;
            float hx = simHead[0] - e[0], hz = simHead[2] - e[2];
            float side = hx * (-fz) + hz * fx;
            static float s0; if (i == 0) s0 = side; float dd = fabsf(side - s0); if (dd > worst) worst = dd;
        }
        printf("(camera heading fixed in this test; body turned 0..100 deg): head's sideways place relative to the camera changes %.1f\n", worst); }
    *(u16*)(pl + 0x162) = 0; simHead[0] = 0; simHead[2] = 40;
    // keyboard walk keys and the pad stick
    {   g_padConn = 0; static i32 tk = 5; p_tick = &tk; g_aimLatch = 1;
        setbit(ST_STAND, 0); setbit(ST_CROUCH, 1); g_crouchLatch = 1;
        int mags[] = { 51, 137, 128, 137, 255, 255 }; int shift[] = { 0, 0, 1, 1, 0, 0 }; const char* nm[] = { "walk toggle, straight (51)", "walk toggle, diagonal (137)", "Shift held, straight (128)", "Shift held, diagonal (137)", "no key, straight (255)", "no key, diagonal (255)" };
        for (int k = 0; k < 6; k++) { keyDown[16] = shift[k]; g_have = 1; g_inMag = mags[k]; g_inYaw = g_camYaw16; clockMs += 16; Push(pl); Push(pl);
            printf("crouched, keyboard %-28s -> band %d (%s)\n", nm[k], (int)g_band, g_band == 0 ? "fast crouch walk" : g_band == 1 ? "low walk, walk speed" : "low walk, stalk speed"); }
        keyDown[16] = 0; g_have = 0; setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); g_crouchLatch = 0; }
    g_padConn = 1; g_lx = 0; g_ly = 30000; g_stickT = clockMs; printf("pad stick forward -> heading %04X (camera %04X); right -> %04X; back -> %04X; left -> %04X\n",
        StickYaw(0x1234), 0x1234, (g_lx = 30000, g_ly = 0, StickYaw(0x1234)), (g_lx = 0, g_ly = -30000, StickYaw(0x1234)), (g_lx = -30000, g_ly = 0, StickYaw(0x1234)));
    g_padConn = 0;
    // firing: no view shake, the AK still climbs
    {   static u8 gw[0x100]; wp[0x20] = 12; *(int*)(gw + 0x7C) = 12; for (int i = 0; i < 30; i++) frame();
        float p0 = g_pitchDeg, maxJ = 0;
        for (int sh = 0; sh < 6; sh++) { hk_Recoil(pl, gw); float j = fabsf(g_recTmpP) + fabsf(g_recTmpY); if (j > maxJ) maxJ = j; for (int i = 0; i < 5; i++) frame(); }
        printf("AK-47, 6 shots: view jolt %.3f deg (0 = no shake), view climbed %.2f deg\n", maxJ, g_pitchDeg - p0); }
    setbit(ST_WEAPON, 0); for (int i = 0; i < 30; i++) frame();
}


static void padSet(int lx, int ly, int btn, int ry) { simPad.conn = 1; simPad.lx = (i16)lx; simPad.ly = (i16)ly; simPad.btn = (u16)btn; simPad.ry = (i16)ry;
    g_padConn = 1; g_lx = (i16)lx; g_ly = (i16)ly; g_padBtn = (u16)btn; g_ry = (i16)ry; g_stickT = clockMs ? clockMs : 1; }
static void padOff(void) { padSet(0, 0, 0, 0); }   // the pad stays connected, nothing touched
#ifndef CLICK
#define CLICK() do { keyDown[2] = 1; if (latchable) simLatch = !simLatch; for (int q = 0; q < 4; q++) frame(); keyDown[2] = 0; for (int q = 0; q < 40; q++) frame(); } while (0)
#define STATE(txt) printf("%-58s shoulder=%d gun raised=%d plugin clicks=%d\n", txt, (int)g_aimLatch, simLatch, injClicks)
#endif
static void test_part02(void)
{
    printf("\n\n-- part 02 --\n");
    simPad.conn = 1; clockMs += 2100; padOff(); for (int i = 0; i < 80; i++) { padOff(); frame(); }
    static i32 tk = 5; p_tick = &tk;
    o_Inp = s_inp; o_Req = s_req; cfgAimMode = 0; simLatch = 0; keyDown[2] = 0; showLog = 1; cfgLog = 1;
    setbit(ST_FPV, 0); setbit(ST_TPS, 1); setbit(3, 0); setbit(6, 0); setbit(ST_WALL, 0); wp[0x20] = 5; playSlot(0, 0);
    // ---- item 6: shoulder view, crouched, pad: the walk follows the stick's own push, not the game's value
    printf("\n-- 6. shoulder view, crouched, pad --\n");
    setbit(ST_WEAPON, 1); setbit(ST_STAND, 0); setbit(ST_CROUCH, 1); for (int i = 0; i < 40; i++) frame();
    g_crouchLatch = 1; g_aimLatch = 1;
    {   struct { int push; const char* nm; } cs[] = { { 9000, "stick a little (27 %)" }, { 16000, "stick half (49 %)" }, { 22000, "stick 67 %" }, { 31000, "stick full" } };
        u16 gameMag[] = { 0, 90, 200, 0, 30, 255, 0, 140, 1, 60 };   // what the game made of it, frame by frame
        for (int c = 0; c < 4; c++) {
            int bands[3] = { 0, 0, 0 };
            for (int f = 0; f < 60; f++) {
                padSet(0, cs[c].push, 0, 0); clockMs += 16;
                g_have = 1; g_inMag = gameMag[f % 10]; g_inYaw = g_camYaw16;
                Push(pl); if (g_band >= 0 && g_band < 3) bands[g_band]++;
            }
            printf("%-22s (stick value %3d): fast crouch walk %2d frames, low walk %2d, stalk %2d  (the game's value jumped 0..255)\n", cs[c].nm, StickMag(), bands[0], bands[1], bands[2]);
        }
        int bands[3] = { 0, 0, 0 };
        for (int f = 0; f < 60; f++) { padSet(0, 0, f % 3 == 0 ? 1 : f % 3 == 1 ? 5 : 8, 0); clockMs += 16; g_have = 1; g_inMag = gameMag[f % 10]; Push(pl); if (g_band >= 0 && g_band < 3) bands[g_band]++; }
        printf("%-22s                  : fast crouch walk %2d frames, low walk %2d, stalk %2d\n", "d-pad (up, up-right..)", bands[0], bands[1], bands[2]);
        padOff();
        int mags[] = { 255, 51, 128 }; int sh[] = { 0, 0, 1 }; const char* nm[] = { "keyboard, no key", "keyboard, Ctrl walk", "keyboard, Shift held" };
        for (int k = 0; k < 3; k++) { keyDown[16] = sh[k]; g_have = 1; g_inMag = (u16)mags[k]; clockMs += 16; Push(pl); Push(pl); printf("%-22s                  : band %d\n", nm[k], (int)g_band); }
        keyDown[16] = 0;
    }
    // ---- item 11: a stick of 1 from the game while firing with the pad
    printf("\n-- 11. pad shot: the game reports a stick of 1 pointing back --\n");
    {   padSet(0, 0, 0x100, 0); g_camValid = 1; g_wallFrames = 0; g_transLatch = 0; g_have = 0; g_inMag = 0;
        *(u16*)(pl + 0x7E0) = 1; *(u16*)(pl + 0x7E2) = (u16)(g_camYaw16 + 0x8000); *(u32*)(pl + 0x7E8) = 0x400;
        float x0 = *(float*)(pl + 0x130), z0 = *(float*)(pl + 0x138);
        OnMove(pl, work, 0x80, 0);
        printf("after the movement code: taken as a push %d, stick left for the game %u, plugin push %.1f,%.1f (Snake at %.0f,%.0f -> %.0f,%.0f), noise frames %u\n",
            (int)g_have, *(u16*)(pl + 0x7E0), g_have ? *(float*)(pl + 0x620) : 0.0f, g_have ? *(float*)(pl + 0x628) : 0.0f, x0, z0, *(float*)(pl + 0x130), *(float*)(pl + 0x138), g_noiseN);
        *(u16*)(pl + 0x7E0) = 40; *(u32*)(pl + 0x7E8) = 0x400; g_have = 0; padSet(0, 9000, 0x100, 0); OnMove(pl, work, 0x80, 0);
        printf("a real light push (stick 27 %%, game value 40): taken as a push %d\n", (int)g_have);
        *(u32*)(pl + 0x7E8) = 0; g_have = 0; padOff();
    }
    g_crouchLatch = 0; setbit(ST_WEAPON, 0); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); clockMs += 2000; for (int i = 0; i < 40; i++) frame();
    // ---- item 1: normal view crouch walking
    printf("\n-- 1. normal view, crouched (crouch walking on) --\n");
    {   g_aimLatch = 0; setbit(ST_STAND, 0); setbit(ST_CROUCH, 1);
        struct { int kind; int mag; int shift; int lx, ly, btn; const char* nm; } cs[] = {
            { 0, 255, 0, 0, 0, 0, "keyboard WASD" }, { 0, 128, 0, 0, 0, 0, "keyboard Ctrl walk (diag 137 / straight 51)" }, { 0, 51, 0, 0, 0, 0, "keyboard Ctrl walk, straight" },
            { 0, 128, 1, 0, 0, 0, "keyboard Shift held" }, { 1, 255, 0, 0, 31000, 0, "pad stick full" }, { 1, 120, 0, 0, 16000, 0, "pad stick half" },
            { 1, 51, 0, 0, 0, 1, "pad d-pad up" }, { 1, 51, 0, 0, 0, 9, "pad d-pad up-right" } };
        for (int k = 0; k < 8; k++) {
            if (cs[k].kind) padSet(cs[k].lx, cs[k].ly, cs[k].btn, 0); else padOff();
            keyDown[16] = cs[k].shift; inpMag = (u16)cs[k].mag;
            if (k == 1) inpMag = 137;
            playSlot(0, 0); *(u32*)(pl + 0x74) = 0x4C4AF2; *(i16*)(pl + 0x78) = 5;
            hk_Inp(pl, work, 0, 0);
            u16 seen = *(u16*)(pl + 0x7E0);
            u8 req[0x60] = {0}; *(u32*)(req + 0x0C) = 0x4C4AF2; *(i16*)(req + 0x10) = 5; *(u64*)(req + 0x18) = 0x7FFFFF; *(u32*)(req + 0x34) = 0x6891CC;
            hk_Req(pl, req);
            int fastSwap = *(u32*)(lastReq + 0x0C) == 0x6891CC;
            if (fastSwap) { playSlot(*(i16*)(lastReq + 0x10) - 1, 3); *(u32*)(pl + 0x74) = 0x6891CC; *(i16*)(pl + 0x78) = *(i16*)(lastReq + 0x10); }
            *(float*)(lyT + 0x4C) = 5.0f;
            hk_Inp(pl, work, 0, 0);
            const char* what = fastSwap ? "crouchnormal (upright fast crouch walk)" : seen >= 180 ? "crouchwalk (low walk, mod's normal speed)" : seen == 51 ? "crouchstalk (low walk, mod's slow speed, the game's own stalk value 51)" : "low walk, slow";
            printf("%-44s: game sees stick %3d -> %s, animation rate %.2f\n", cs[k].nm, seen, what, fastSwap ? *(float*)(lyT + 0x4C) : 0.0f);
        }
        keyDown[16] = 0; padOff(); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0);
        printf("speeds measured in the game: crouchnormal 31 units/frame at rate 5.0 -> %.1f at %.1f; crouchwalk 27-28; running 65\n", 31.0f * 1.3f, 5.0f * 1.3f);
    }
    // ---- items 2 + 4: the game's camera button
    printf("\n-- 2/4. the game's camera button --\n");
    {   cfgAimMode = 1; *(u32*)(work + 0xC4) = 0x4; *(u32*)(work + 0x7C) = 0x4;
        padSet(0, 0, 0, 0); for (int i = 0; i < 3; i++) { padSet(0, 0, 0, 0); frame(); }
        printf("pad connected: the camera's button mask %X (was 4)\n", *(u32*)(work + 0xC4));
        simPad.conn = 0; frame(); frame();
        printf("pad gone: mask %X\n", *(u32*)(work + 0xC4));
        *(u32*)(work + 0xC4) = 0; *(u32*)(work + 0x7C) = 0;
        simPad.conn = 1; clockMs += 1000; for (int i = 0; i < 40; i++) { padOff(); clockMs += 40; frame(); } for (int i = 0; i < 40; i++) { padOff(); frame(); } cfgAimMode = 0;
    }
    // ---- item 8: the pad's vertical look
    printf("\n-- 8. pad: stick up looks up --\n");
    {   setbit(ST_WEAPON, 0); *(float*)(work + 0x340) = 0.6f; for (int i = 0; i < 20; i++) frame();
        for (int g = 0; g < 2; g++) {
            float e0 = *(float*)(work + 0x340);
            mouse = g ? -0.004f : 0.004f;   // the game's camera setting: 0 = stick up raises the camera (looks down), 1 = lowers it
            for (int i = 0; i < 20; i++) { padSet(0, 0, 0, 20000); frame(); }
            mouse = 0; padOff(); frame();
            printf("normal view, stick held up 20 frames, game set to move %s: elevation %.3f -> %.3f (%s)\n", g ? "the right way" : "the wrong way", e0, *(float*)(work + 0x340), *(float*)(work + 0x340) < e0 ? "camera lower = looking up" : "looking down");
        }
        setbit(ST_WEAPON, 1); for (int i = 0; i < 40; i++) frame();
        float p0 = g_pitchDeg; mouse = 0.004f; for (int i = 0; i < 20; i++) { padSet(0, 0, 0, 20000); frame(); } mouse = 0; padOff(); frame();
        printf("shoulder view, stick held up, game set the wrong way: aim pitch %.1f -> %.1f (%s)\n", p0, g_pitchDeg, g_pitchDeg > p0 ? "up" : "down");
        p0 = g_pitchDeg; mouse = 0.004f; for (int i = 0; i < 20; i++) frame(); mouse = 0; frame();
        printf("shoulder view, mouse (no pad stick): pitch %.1f -> %.1f (left to the game)\n", p0, g_pitchDeg);
        setbit(ST_WEAPON, 0); for (int i = 0; i < 40; i++) frame();
    }
    // ---- item 9: crosshair
    printf("\n-- 9. crosshair: rest radius %.1f units (was 59), moving %.1f (was 72); at 1440p %d px from the middle (was %d)\n",
        59.0f * cfgSightGap / 100.0f, 72.0f * cfgSightGap / 100.0f, (int)(59.0f * cfgSightGap / 100.0f * 1440 / 640), (int)(59.0f * 1440 / 640));
    // ---- item 10: recoil
    printf("\n-- 10. recoil --\n");
    {   static u8 gw[0x100]; o_Recoil = s_recoil; setbit(ST_WEAPON, 1); *(float*)(work + 0x340) = 0.6f;
        int ty[] = { 5, 6, 8, 12, 13, 9 }; const char* nm[] = { "Mk22", "M1911A1", "SAA", "AK-47", "M63", "Patriot" };
        float m[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }, mm[16], tg[4];
        for (int k = 0; k < 6; k++) {
            wp[0x20] = (u8)ty[k]; *(int*)(gw + 0x7C) = ty[k]; for (int i = 0; i < 40; i++) frame();
            float p0 = g_pitchDeg; u32 s0 = g_rkSpread;
            for (int sh = 0; sh < 6; sh++) { hk_Recoil(pl, gw); for (int i = 0; i < 3; i++) m[12 + i] = g_tQ[i] + g_tF[i] * 700; ShotPlan(m, 11, (void*)0x1000, mm, tg); for (int i = 0; i < 5; i++) frame(); }
            printf("%-8s 6 shots: shots turned off the crosshair %u, view climbed %.2f deg\n", nm[k], g_rkSpread - s0, g_pitchDeg - p0);
        }
        wp[0x20] = 5;
    }
    // ---- item 11b: his head moving with the firing motion
    printf("\n-- 11b. the firing motion moves his head 30 back: the camera --\n");
    {   setupBones(); simHead[0] = 0; simHead[1] = 1820; simHead[2] = 40; *(float*)(pl + 0x130) = 0; *(float*)(pl + 0x138) = 0; *(u16*)(pl + 0x162) = 0;
        for (int i = 0; i < 90; i++) frame();
        float e0[3]; memcpy(e0, (float*)(work + 0x3D0), 12); float worst = 0;
        g_shotT = clockMs;
        for (int i = 0; i < 30; i++) {
            simHead[2] = 40 - 30.0f * (i < 10 ? (i + 1) / 10.0f : i < 20 ? 1.0f : (30 - i) / 10.0f);
            frame(); float* e = (float*)(work + 0x3D0); float d = fabsf(e[2] - e0[2]); if (d > worst) worst = d;
        }
        printf("camera moved up to %.1f units during the shot (before: it followed the head, 30 units)\n", worst);
        simHead[2] = 40; for (int i = 0; i < 60; i++) frame();
    }
    // ---- item 5: loading a save with the pad, gun raised and an earlier press
    printf("\n-- 5. new scene with a pad connected --\n");
    {   cfgAimMode = 1; wp[0x20] = 5; setbit(ST_WEAPON, 0); simLatch = 0; injClicks = 0;
        for (int i = 0; i < 30; i++) { padSet(0, 0, 0, 0); frame(); }
        setbit(ST_TPS, 0); for (int i = 0; i < 5; i++) { padSet(0, 0, 0, 0); frame(); }
        keyDown[2] = 1; simLatch = 1; for (int i = 0; i < 4; i++) { padSet(0, 0, 0, 0); frame(); } keyDown[2] = 0;   // the aim key while the view could not switch (gun raised)
        for (int i = 0; i < 20; i++) { padSet(0, 0, 0, 0); frame(); }
        clockMs += 6000;   // loading
        setbit(ST_TPS, 1);
        int on = 0; for (int i = 0; i < 250; i++) { padSet(0, 0, 0, 0); frame(); if (g_aimLatch) on++; }
        printf("after loading (gun raised, aim key pressed before): shoulder view on %d of 250 frames; gun raised now %d; plugin key presses %d\n", on, simLatch, injClicks);
        for (int i = 0; i < 20; i++) { padSet(0, 0, 0x100, 0); frame(); }
        printf("LB held: shoulder %d\n", (int)g_aimLatch);
        for (int i = 0; i < 40; i++) { padSet(0, 0, 0, 0); frame(); }
        printf("LB let go: shoulder %d gun raised %d\n", (int)g_aimLatch, simLatch);
        padOff(); cfgAimMode = 0;
    }
    printf("\n-- 5b. keyboard only: the usual switches still work, and a load starts in third person --\n");
    {   simPad.conn = 0; cfgAimMode = 1; wp[0x20] = 5; simLatch = 0; injClicks = 0; latchable = 1;
        clockMs += 3000; for (int i = 0; i < 200; i++) frame();
        STATE("plain third person (no pad):");
        setbit(ST_FPV, 1); setbit(ST_TPS, 0); for (int i = 0; i < 60; i++) frame(); STATE("first person:");
        CLICK(); STATE("   one press there (aiming down the sights):");
        setbit(ST_FPV, 0); setbit(ST_TPS, 1); for (int i = 0; i < 60; i++) frame(); STATE("   left first person:");
        CLICK(); for (int i = 0; i < 30; i++) frame(); STATE("   one press:");
        setbit(ST_WALL, 1); for (int i = 0; i < 30; i++) frame(); STATE("back against a wall:"); CLICK(); STATE("   one press:");
        setbit(ST_WALL, 0); for (int i = 0; i < 30; i++) frame(); STATE("   off the wall:");
        CLICK(); STATE("   one press:");
        simLatch = 1; clockMs += 6000; int on = 0; for (int i = 0; i < 250; i++) { frame(); if (g_aimLatch) on++; }
        STATE("after a load with the gun raised:"); printf("   (shoulder view on %d of 250 frames)\n", on);
        CLICK(); STATE("   one press:"); CLICK(); STATE("   one more:");
        cfgAimMode = 0;
    }
    showLog = 0;
}


static void test_part03(void)
{
    printf("\n\n-- part 03 --\n");
    cfgAimMode = 0; simPad.conn = 0; setbit(ST_FPV, 0); setbit(ST_TPS, 1); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); setbit(3, 0); setbit(ST_WALL, 0);
    tree = backWall = sideWall = postWall = 0; wp[0x20] = 5; *(float*)(pl + 0x130) = 0; *(float*)(pl + 0x134) = 1000; *(float*)(pl + 0x138) = 0; *(u16*)(pl + 0x162) = 0;
    *(float*)(work + 0x340) = 0.6f;
    setbit(ST_WEAPON, 0); clockMs += 2000; for (int i = 0; i < 40; i++) frame();
    nVQ = nIBR = 0; for (int i = 0; i < 100; i++) frame();
    printf("normal view: system memory queries %.1f a frame (before: about 12), fast checks %.1f\n", nVQ / 100.0f, nIBR / 100.0f);
    setbit(ST_WEAPON, 1); for (int i = 0; i < 60; i++) frame();
    nVQ = nIBR = 0; for (int i = 0; i < 100; i++) frame();
    printf("shoulder view: system memory queries %.1f a frame (before: about 28), fast checks %.1f\n", nVQ / 100.0f, nIBR / 100.0f);
    printf("open ground: eye (%.0f, %.0f, %.0f) wall push %d, camera %d from his head\n", g_tEye[0], g_tEye[1], g_tEye[2], (int)g_wallPush, (int)g_camHead);
    postWall = 1; for (int i = 0; i < 20; i++) frame();
    printf("a wall 60 to the camera's right, beside the camera only: eye x %.0f (wall at -560) wall push %d\n", g_tEye[0], (int)g_wallPush);
    postWall = 0; for (int i = 0; i < 40; i++) frame();
    printf("wall gone: eye x %.0f wall push %d\n", g_tEye[0], (int)g_wallPush);
    backWall = 1; for (int i = 0; i < 20; i++) frame();
    printf("wall 600 behind: eye (%.0f, %.0f, %.0f) side %d%% back %d%%, camera %d from his head (Snake keeps his place on screen)\n", g_tEye[0], g_tEye[1], g_tEye[2], (int)g_sideK100, (int)g_backK100, (int)g_camHead);
    backWall = 0; for (int i = 0; i < 80; i++) frame();
    // a wall right behind him (z = -600 is the plane; move Snake back to z = -450)
    *(float*)(pl + 0x138) = -450; simHead[2] = -410; backWall = 1; for (int i = 0; i < 30; i++) frame();
    printf("wall 150 behind him: back %d%% (MinBack %d), eye (%.0f, %.0f, %.0f), camera %d from his head\n", (int)g_backK100, cfgCamMinBack, g_tEye[0], g_tEye[1], g_tEye[2], (int)g_camHead);
    backWall = 0; *(float*)(pl + 0x138) = 0; simHead[2] = 40; for (int i = 0; i < 80; i++) frame();
    setbit(ST_WEAPON, 0); for (int i = 0; i < 40; i++) frame();
    // the log goes through the buffer once the writer runs
    g_lthread = 1; showLog = 1; Log("buffered line 1"); Log("buffered line 2"); printf("(before the writer runs: nothing printed above)\n"); LogFlush(); g_lthread = 0;
}

static i64 holdSeen;

static int stE0, stDE, holdState, plMotLast = -1, seqLast = -1, setMotLast = -1, setMotTime = -1, stSetN;
static u32 s_plmot(u8* w, int m) { (void)w; plMotLast = m; return (u32)(1000 + m); }
static i64 s_stchk(u32 n) { return n == 0xE0 ? stE0 : n == 0xDE ? stDE : 0; }
static i64 s_stset(int a, i64 b, i64 c, i64 d) { (void)a;(void)b;(void)c;(void)d; stSetN++; return 0; }
static i64 s_hold(u8* w, u8* m) { (void)w;(void)m; return holdState; }
static i64 s_seq(u8* q, int l, int n, int f, int lp) { (void)q;(void)l;(void)f;(void)lp; seqLast = n; return 0; }
static void s_setmot(u8* mc, int l, int m, int t, i64 mk) { (void)mc;(void)l;(void)mk; setMotLast = m; setMotTime = t; hk_CwSeq((u8*)1, 0, m, 0, 0); }
static i64 s_squat(u8* w, u8* m, i64 a, i64 b) { (void)w;(void)a;(void)b; holdSeen = o_CwHold ? hk_CwHold(w, m) : 0; return 7; }
static int* s_camo(int* a, int b) { (void)a;(void)b; return 0; }
static void test_part04(void)
{
    printf("\n-- built-in crouch walking --\n");
    static u8 snake[0x1000], mw[0x100], mc[0x200], mt[0x100], arc[0x40], sq[0x10], camo[0x800], act[1];
    f_CwPlMot = s_plmot; f_CwStChk = s_stchk; f_CwStSet = s_stset; o_CwHold = s_hold; o_CwSeq = s_seq; o_CwSetMot = s_setmot; o_CwSquat = s_squat; o_CwCamo = s_camo;
    g_cwSquatAct = act; g_cwCamo = camo; g_cwInstalled = 1; static double wt = 1.0 / 60.0; g_cwWait = &wt;
    *(u64*)(mw + 0x58) = (u64)act; *(u16*)(mw + 0x90) = 0;
    *(u8**)(mc + 8) = mt; *(u32*)(mt + 0x54) = 0x6891CC; *(u8**)(mt + 8) = arc; memcpy(arc, "Mtar", 4); *(u32*)(arc + 8) = 247;
    *(u8**)(mc + 0x78) = sq; *(float*)(mc + 0x128) = 17.0f;
    hk_CwSetMot(mc, 0, 2, 0, 0);   // the game loads some motion on Snake: the control is known
    struct { int stick; const char* nm; } st[] = { { 0, "stick at rest" }, { 180, "walk (180)" }, { 180, "walk, next frame" }, { 51, "stalk (51)" }, { 255, "full (255)" } };
    for (int k = 0; k < 5; k++) {
        *(i16*)(snake + 0x7E0) = (i16)st[k].stick; seqLast = -1; plMotLast = -1;
        hk_CwSquat(snake, mw, 0, 0);
        printf("%-18s: crouch walking %d moving %d slow %d; motion asked %d; speed %.1f; sound sequence switched to %d\n",
            st[k].nm, (int)g_cwOn, (int)g_cwMoving, (int)g_cwSlow, plMotLast, *(float*)(mt + 0x4C), seqLast);
    }
    *(i16*)(snake + 0x7E0) = 180; hk_CwSquat(snake, mw, 0, 0);
    hk_CwSetMot(mc, 0, 4, 3, 0);
    printf("the game loads 'stand move stalk' (4): loaded %d at time %d (%s); its sounds sequence %d\n", setMotLast, setMotTime, setMotLast == 199 ? "the squat walk" : "unchanged", seqLast);
    hk_CwSetMot(mc, 0, 220, 3, 0); printf("the plugin's fast crouch walk (220) is left alone: loaded %d\n", setMotLast);
    // camo
    *(int*)(camo + 3 * 128 + 0x24) = 600; g_lastWalk = 1; hk_CwCamo(0, 3);
    printf("camo while crouchwalking: 60%% -> %d.%d%%\n", *(int*)(camo + 3 * 128 + 0x24) / 10, *(int*)(camo + 3 * 128 + 0x24) % 10);
    *(int*)(camo + 3 * 128 + 0x24) = 600; *(i16*)(snake + 0x7E0) = 51; hk_CwSquat(snake, mw, 0, 0); g_lastWalk = 2; hk_CwCamo(0, 3);
    printf("camo while crouchstalking: 60%% -> %d.%d%%\n", *(int*)(camo + 3 * 128 + 0x24) / 10, *(int*)(camo + 3 * 128 + 0x24) % 10);
    *(int*)(camo + 3 * 128 + 0x24) = 1000; *(i16*)(snake + 0x7E0) = 180; hk_CwSquat(snake, mw, 0, 0); g_lastWalk = 1; hk_CwCamo(0, 3);
    printf("stealth camouflage (100%%) untouched: %d\n", *(int*)(camo + 3 * 128 + 0x24));
    // hold the crouch button: stand up
    holdState = 1; *(u32*)(mw + 0x68) = 0; hk_CwSquat(snake, mw, 0, 0);
    printf("crouch button held (stand): flag %X, button seen by the game's own squat code %d, crouch walking %d\n", *(u32*)(mw + 0x68), (int)holdSeen, (int)g_cwOn);
    holdState = 2; *(u32*)(mw + 0x68) = 0; hk_CwSquat(snake, mw, 0, 0); printf("crouch button held (down): flag %X\n", *(u32*)(mw + 0x68));
    holdState = 0; *(u32*)(mw + 0x68) = 0; hk_CwSquat(snake, mw, 0, 0); hk_CwSquat(snake, mw, 0, 0);
    printf("walking again: %d; other action: ", (int)g_cwOn); *(u64*)(mw + 0x58) = 0x1234; CwOnMove(); printf("crouch walking %d moving %d\n", (int)g_cwOn, (int)g_cwMoving);
    stDE = 1; *(u64*)(mw + 0x58) = (u64)act; hk_CwSquat(snake, mw, 0, 0); printf("first person: crouch walking %d\n", (int)g_cwOn); stDE = 0;
    g_cwInstalled = 0;
}


static void test_part05(void)
{
    printf("\n\n-- part 05 --\n");
    cfgAimMode = 0; simPad.conn = 0; setbit(ST_FPV, 0); setbit(ST_TPS, 1); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); setbit(3, 0); setbit(ST_WALL, 0);
    tree = backWall = sideWall = postWall = flick = 0; wp[0x20] = 5; *(float*)(pl + 0x130) = 0; *(float*)(pl + 0x134) = 1000; *(float*)(pl + 0x138) = 0; *(u16*)(pl + 0x162) = 0;
    *(float*)(work + 0x340) = 0.6f; setbit(ST_WEAPON, 1); clockMs += 2000; for (int i = 0; i < 80; i++) frame();
    printf("open ground: eye (%.0f, %.0f, %.0f) side %d back %d push %d\n", g_tEye[0], g_tEye[1], g_tEye[2], (int)g_sideK100, (int)g_backK100, (int)g_wallPush); for (int i = 0; i < 200; i++) frame(); printf("   200 frames later: eye (%.0f, %.0f, %.0f) side %d back %d push %d\n", g_tEye[0], g_tEye[1], g_tEye[2], (int)g_sideK100, (int)g_backK100, (int)g_wallPush);
    flick = 1; float lo = 1e9, hi = -1e9, tot = 0, prev = g_tEye[2];
    for (int i = 0; i < 120; i++) { flickN++; frame(); float z = g_tEye[2]; if (i > 20) { if (z < lo) lo = z; if (z > hi) hi = z; tot += fabsf(z - prev); } prev = z; }
    printf("test hits every other frame (z = -900): camera z between %.0f and %.0f, moved %.0f units in 100 frames (before: jumped in and out every frame)\n", lo, hi, tot);
    flick = 0; for (int i = 0; i < 200; i++) frame();
    printf("clear again: eye z %.0f\n", g_tEye[2]);
    // a wall appears 600 behind: never behind it, and how fast in
    backWall = 1; wallZ = -600; int beyond = 0; float zz[8];
    for (int i = 0; i < 40; i++) { frame(); if (g_tEye[2] < -600.0f) beyond++; if (i < 8) zz[i] = g_tEye[2]; }
    printf("wall 600 behind appears: eye z by frame %.0f %.0f %.0f %.0f %.0f %.0f; frames behind the wall: %d; settles at %.0f\n", zz[0], zz[1], zz[2], zz[3], zz[4], zz[5], beyond, g_tEye[2]);
    backWall = 0; float zo[6]; for (int i = 0; i < 90; i++) { frame(); if (i % 15 == 0) zo[i / 15] = g_tEye[2]; }
    printf("wall gone: eye z every 15 frames %.0f %.0f %.0f %.0f %.0f %.0f -> %.0f (eases out)\n", zo[0], zo[1], zo[2], zo[3], zo[4], zo[5], g_tEye[2]);
    for (int i = 0; i < 120; i++) frame();
    // first person -> back: heading taken from first person
    *(u16*)(work + 0x338) = 0x8000;
    g_fpOk = 1; g_fpD[0] = 1; g_fpD[1] = 0; g_fpD[2] = 0; g_lastFpvT = clockMs;
    FpvHeading(work, clockMs);
    printf("first person looked along +x (heading 4000): camera orbit now %04X (want C000)\n", *(u16*)(work + 0x338));
    g_lastFpvT = clockMs - 5000; *(u16*)(work + 0x338) = 0x8000; FpvHeading(work, clockMs); printf("5 s later: left alone %04X\n", *(u16*)(work + 0x338));
    // sounds
    printf("sounds: stand fast %d, stand slow %d, crouch low %d, crouch fast %d, lying %d, game motion 12 %d\n", SoundFor(200, 0, 0), SoundFor(205, 0, 0), SoundFor(217, 0, 1), SoundFor(222, 0, 1), SoundFor(239, 1, 0), SoundFor(12, 1, 0));
    g_seqSlot = 201; g_seqSub = 5; seqLast = -1; o_CwSeq = s_seq; hk_CwSeq((u8*)1, 0, 201, 5, 40); printf("sequence for motion 201 asked: %d started\n", seqLast);
    hk_CwSeq((u8*)1, 1, 15, 5, 40); printf("another layer's motion 15: %d started\n", seqLast); g_seqSlot = -1;
    // camo
    static u8 camo2[0x800]; g_cwCamo = camo2; o_CwCamo = s_camo; g_cwOn = 1; g_cwMoving = 1;
    g_cwSlow = 0; g_lastWalk = 0; *(int*)(camo2 + 0x24) = 600; hk_CwCamo(0, 0); int vn = *(int*)(camo2 + 0x24);
    g_cwSlow = 0; g_lastWalk = 1; *(int*)(camo2 + 0x24) = 600; hk_CwCamo(0, 0); int vw = *(int*)(camo2 + 0x24);
    g_cwSlow = 1; g_lastWalk = 2; *(int*)(camo2 + 0x24) = 600; hk_CwCamo(0, 0); int vs = *(int*)(camo2 + 0x24);
    printf("camo from 60%%: crouchnormal %d%%, crouchwalk %d%%, crouchstalk %d%%\n", vn / 10, vw / 10, vs / 10);
    g_cwOn = 0; g_cwMoving = 0; g_cwCamo = 0;
    printf("spread: RecoilPercent default %d\n", cfgRecoilPct);
    setbit(ST_WEAPON, 0); for (int i = 0; i < 40; i++) frame();
}

// ======================= 0.33 =======================
static int simConv;   // 0: the game's camera looks along work+0x338; 1: the other way
static void frameX(void)
{
    u16 o = *(u16*)(work + 0x338); u16 v = (u16)(o + (simConv ? 0x8000 : 0));
    float a = v * 6.2831853f / 65536.0f;
    float* eye = (float*)(work + 0x3D0); float* tgt = (float*)(work + 0x3E0);
    tgt[0] = *(float*)(pl + 0x130); tgt[1] = *(float*)(pl + 0x134) + 695; tgt[2] = *(float*)(pl + 0x138);
    eye[0] = tgt[0] - sinf(a) * 4000; eye[1] = tgt[1] + 1500; eye[2] = tgt[2] - cosf(a) * 4000;
    clockMs += 16;
    *(u32*)(pl + 0x7E8) = (*(u32*)(pl + 0x7E8) & ~0x8000u) | (simLatch ? 0x8000u : 0);
    InputPoll();
    hk_Apply(work);
}
static u16 viewNow(void) { return (u16)(*(u16*)(work + 0x338) + (simConv ? 0x8000 : 0)); }
static u8 fpcam[0x60];
static void fpvEpisode(u16 look)
{
    *(u8**)(pl + 0x730) = fpcam;
    float a = look * 6.2831853f / 65536.0f;
    float* E = (float*)(fpcam + 0x20); float* T = (float*)(fpcam + 0x30);
    E[0] = *(float*)(pl + 0x130); E[1] = *(float*)(pl + 0x134) + 800; E[2] = *(float*)(pl + 0x138);
    T[0] = E[0] + sinf(a) * 1000; T[1] = E[1]; T[2] = E[2] + cosf(a) * 1000;
    *(u16*)(pl + 0x162) = look;
    setbit(ST_FPV, 1); setbit(ST_TPS, 0);
    for (int i = 0; i < 10; i++) frameX();
    setbit(ST_FPV, 0); setbit(ST_TPS, 1);
    frameX();
}
static int st4[4], st5[5], st4N, st5N; static u32 csH; static int csF1, csF2, csN;
static i64 s_st4(int a, i64 b, i64 c, i64 d) { st4[0] = a; st4[1] = (int)b; st4[2] = (int)c; st4[3] = (int)d; st4N++; return 0; }
static i64 s_st5(int a, i64 b, i64 c, i64 d, i64 e) { st5[0] = a; st5[1] = (int)b; st5[2] = (int)c; st5[3] = (int)d; st5[4] = (int)e; st5N++; return 0; }
static void s_cs(u8* p, u8* mw, int f1, int f2) { (void)p; csH = *(u32*)(mw + 0x60); csF1 = f1; csF2 = f2; csN++; *(int*)(lyT + 0x3C) += 5; }
static void test_part06(void)
{
    printf("\n\n-- part 06 --\n");
    cfgAimMode = 0; simPad.conn = 0; setbit(ST_FPV, 0); setbit(ST_TPS, 1); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); setbit(3, 0); setbit(ST_WALL, 0); setbit(ST_WEAPON, 0);
    tree = backWall = sideWall = postWall = flick = 0; wp[0x20] = 5; *(float*)(pl + 0x130) = 0; *(float*)(pl + 0x134) = 1000; *(float*)(pl + 0x138) = 0;
    clockMs += 2000;
    // ---- item 2: back from first person, for both ways the game might number its camera
    for (int conv = 0; conv < 2; conv++) {
        simConv = conv; g_orbV0 = g_orbV8 = 0; g_orbFixed = -1; g_fpvOk = g_fpvBad = 0;
        *(u16*)(work + 0x338) = 0x1000;
        for (int i = 0; i < 400; i++) frameX();
        printf("[2] game camera %s: votes looks-along %u / opposite %u\n", conv ? "looks opposite to its number" : "looks along its number", g_orbV0, g_orbV8);
        u16 look[3] = { 0xA8BB, 0x29F1, 0x7000 };
        for (int k = 0; k < 3; k++) {
            fpvEpisode(look[k]);
            FpvHeading(work, clockMs);
            for (int i = 0; i < 8; i++) frameX();
            int off = (i16)(u16)(viewNow() - look[k]);
            printf("[2]   first person looked %3d deg -> the shoulder view looks %3d deg (%d off); checks OK %u wrong %u\n", Deg16(look[k]), Deg16(viewNow()), off * 360 / 65536, g_fpvOk, g_fpvBad);
        }
    }
    // learned wrong on purpose: the check turns it round
    simConv = 1; g_orbFixed = -1; g_orbV0 = 5000; g_orbV8 = 0; g_fpvOk = g_fpvBad = 0;
    fpvEpisode(0x3000); FpvHeading(work, clockMs); for (int i = 0; i < 12; i++) frameX();
    printf("[2] learned the wrong way round: first person 67 deg -> shoulder %d deg; OK %u wrong %u (wrong once, then OK)\n", Deg16(viewNow()), g_fpvOk, g_fpvBad);
    fpvEpisode(0x6000); FpvHeading(work, clockMs); for (int i = 0; i < 8; i++) frameX();
    printf("[2]   next time: first person 135 deg -> shoulder %d deg; OK %u wrong %u\n", Deg16(viewNow()), g_fpvOk, g_fpvBad);
    simConv = 0; g_orbFixed = -1; g_orbV0 = g_orbV8 = 0;
    // ---- item 1: the crawl sounds and grass marks
    {   static u8 mw[0x100]; *(u32*)(mw + 0x60) = 0x1234; g_moveWork = mw;
        f_CwStSet = s_st4; f_StSet5 = s_st5; f_CrawlSnd = s_cs;
        *(u8**)(pl + 0x2B8) = mcT; *(u8**)(mcT + 8) = lyT; *(int*)(lyT + 0x3C) = 0;
        *(u32*)(pl + 0xC0) = 0x6891CC; *(u32*)(pl + 0xC4) = 0; *(u32*)(pl + 0xC8) = 0x4C4AF2; *(u32*)(pl + 0xCC) = 0;
        g_msg = 0x80; g_proneLatch = 1; g_have = 1; g_inMag = 255;
        struct { int dir, slot; u32 arc; const char* nm; } c[] = { { 0, 12, 0x6891CC, "forward, main crawl" }, { 1, 13, 0x6891CC, "backward" }, { 0, 10, 0x4C4AF2, "forward, weapon crawl" }, { 2, 200, 0x4C4AF2, "left" } };
        for (int k = 0; k < 4; k++) {
            g_legDir = c[k].dir; g_legSlot = c[k].slot; g_legFrame = g_frame; *(u32*)(pl + 0x74) = c[k].arc; *(i16*)(pl + 0x78) = (i16)(c[k].slot + 1);
            st4N = st5N = csN = 0; csH = 0;
            CrawlSounds(pl);
            printf("[1] %-22s: marks %s (%d,%d,%d,%d%s), sound routine %d time(s), motion handle %04X, frames %d/%d, action's own motion put back %s\n", c[k].nm,
                st5N ? "5" : st4N ? "4" : "none", st5N ? st5[0] : st4[0], st5N ? st5[1] : st4[1], st5N ? st5[2] : st4[2], st5N ? st5[3] : st4[3], st5N ? ", -1" : "",
                csN, csH, csF1, csF2, *(u32*)(mw + 0x60) == 0x1234 ? "yes" : "NO");
        }
        *(i16*)(pl + 0x78) = 12; g_legSlot = 12; st4N = csN = 0; CrawlSounds(pl);
        printf("[1] the game's own lying pose playing instead: marks %d, sound routine %d (both 0)\n", st4N, csN);
        g_proneLatch = 0; g_have = 0; g_inMag = 0; f_CwStSet = 0; f_StSet5 = 0; f_CrawlSnd = 0; g_legDir = -1; g_legSlot = -1;
    }
    // ---- items 4 / 8: the backward crawl is never learned as the forward crawl; gun lowered by the game: still
    // replaced
    {   o_Req = s_req; setbit(ST_STAND, 0); setbit(ST_CROUCH, 0); setbit(3, 1); setbit(ST_WEAPON, 0); for (int i = 0; i < 5; i++) frameX();
        g_aimLatch = 0; g_proneLatch = 1; g_transLatch = 0; wp[0x20] = 5; g_crawlArc[5] = 0; g_crawlMot[5] = 0; *(u16*)(pl + 0x7E0) = 255;
        for (int i = 0; i < 25; i++) { u8 req[0x60] = {0}; *(u32*)(req + 0x0C) = 0x6891CC; *(i16*)(req + 0x10) = 14; *(u64*)(req + 0x18) = 0x7FFFFF; hk_Req(pl, req); }
        printf("[8] 25 frames of the game crawling him backward in the normal view: learned forward crawl for weapon 5 = %X/%d (want none)\n", g_crawlArc[5], g_crawlMot[5]);
        for (int i = 0; i < 25; i++) { u8 req[0x60] = {0}; *(u32*)(req + 0x0C) = 0x6891CC; *(i16*)(req + 0x10) = 13; *(u64*)(req + 0x18) = 0x7FFFFF; hk_Req(pl, req); }
        printf("[8] then 25 frames crawling forward: learned %X/%d (want 6891CC/13)\n", g_crawlArc[5], g_crawlMot[5]);
        g_crawlArc[5] = 0x6891CC; g_crawlMot[5] = 14;
        g_aimLatch = 1; g_rawAim = 1; g_camValid = 1; g_moveFrame = g_frame; g_inYaw = g_camYaw16; g_have = 1; g_inMag = 255;
        { u8 req[0x60] = {0}; *(u32*)(req + 0x0C) = 0x4C4AF2; *(i16*)(req + 0x10) = 18; *(u64*)(req + 0x18) = 0x7FFFFF; hk_Req(pl, req); }
        printf("[8] shoulder view, forward, even with the backward crawl learned: plays %X/%d (want 6891CC/13)\n", *(u32*)(lastReq + 0x0C), *(i16*)(lastReq + 0x10));
        g_inYaw = (u16)(g_camYaw16 + 0x8000); g_moveFrame = g_frame;
        { u8 req[0x60] = {0}; *(u32*)(req + 0x0C) = 0x4C4AF2; *(i16*)(req + 0x10) = 18; *(u64*)(req + 0x18) = 0x7FFFFF; hk_Req(pl, req); }
        printf("[8] shoulder view, backward: plays %X/%d (want 6891CC/14)\n", *(u32*)(lastReq + 0x0C), *(i16*)(lastReq + 0x10));
        g_crawlArc[5] = 0x6891CC; g_crawlMot[5] = 13;
        g_rawAim = 0; g_inYaw = (u16)(g_camYaw16 + 0x4000); g_moveFrame = g_frame; wp[0x20] = 12; g_wpnArc = 0x23291;
        { u8 req[0x60] = {0}; *(u32*)(req + 0x0C) = 0x23291; *(i16*)(req + 0x10) = 10; *(u64*)(req + 0x18) = 0x1FFFFFFFFFFFFFFull; hk_Req(pl, req); }
        printf("[4] gun lowered by the game (AK-47 asks for its 23291/10), pushing left: plays %X/%d (before left 23291/10 and slid him; want a side motion)\n", *(u32*)(lastReq + 0x0C), *(i16*)(lastReq + 0x10));
        g_aimLatch = 0; g_proneLatch = 0; g_have = 0; g_inMag = 0; setbit(3, 0); setbit(ST_STAND, 1); wp[0x20] = 5; *(u16*)(pl + 0x7E0) = 0;
    }
    // ---- item 4: the crawl direction check
    {   g_msg = 0x80; g_aimLatch = 1; g_proneLatch = 1; g_transLatch = 0; g_have = 1; g_inMag = 255; g_inYaw = 0;
        *(u32*)(pl + 0x74) = 0x6891CC; *(i16*)(pl + 0x78) = 13; g_legSlot = 12; g_legDir = 0;
        for (int i = 0; i < 50; i++) { g_legFrame = g_frame; *(float*)(pl + 0x138) += 15.0f; ProneCheck(pl); }
        *(i16*)(pl + 0x78) = 14; g_legSlot = 13;
        for (int i = 0; i < 50; i++) { g_legFrame = g_frame; *(float*)(pl + 0x138) -= 9.0f; ProneCheck(pl); }
        *(u32*)(pl + 0x74) = 0x23291; *(i16*)(pl + 0x78) = 10; g_legFrame = g_frame - 10;
        for (int i = 0; i < 14; i++) { *(float*)(pl + 0x130) += 9.0f; ProneCheck(pl); }
        printf("[4] direction checks: OK %u, PROBLEM %u, slides %u (want 1, 1, 1)\n", g_proneOkN, g_proneBadN, g_slideN);
        g_aimLatch = 0; g_proneLatch = 0; g_have = 0; g_inMag = 0; g_legDir = -1; g_legSlot = -1;
    }
    // ---- item 5: windows of the game's process that are not the game
    {   extern char simClass[64]; memcpy(simClass, "ConsoleWindowClass", 19); simFw = (HANDLE)0x9999; InputPoll(); int con = g_fgCache;
        memcpy(simClass, "GameWindowClass", 16); simFw = (HANDLE)0x5678; InputPoll(); int game = g_fgCache;
        printf("[5] console window in front: game in front %d (want 0); the game's window: %d (want 1)\n", con, game);
    }
    // ---- item 6: a new scene that starts in first person with the gun raised (pad connected, no button held)
    {   cfgAimMode = 1; simPad.conn = 1; simPad.btn = 0; simPad.lx = simPad.ly = 0; wp[0x20] = 5; simLatch = 1; injClicks = 0;
        setbit(ST_WEAPON, 1); setbit(ST_FPV, 1); setbit(ST_TPS, 0); clockMs += 3000;
        for (int i = 0; i < 40; i++) frameX();
        printf("[6] loaded into first person, gun raised, pad connected: plugin key presses %d, gun raised now %d (want 1 press, 0)\n", injClicks, simLatch);
        simLatch = 1; injClicks = 0; simPad.btn = 0x200; clockMs += 3000;   // the same while a pad button is held: left alone
        for (int i = 0; i < 40; i++) frameX();
        printf("[6] the same with a pad button held (a player going into first person): presses %d (want 0)\n", injClicks);
        simPad.btn = 0; simPad.conn = 0; setbit(ST_FPV, 0); setbit(ST_TPS, 1); setbit(ST_WEAPON, 0); simLatch = 0; cfgAimMode = 0; for (int i = 0; i < 40; i++) frameX();
    }
    // ---- item 3 / 7
    {   static u8 camo3[0x800]; g_cwCamo = camo3; o_CwCamo = s_camo; g_cwOn = 1; g_cwMoving = 1; int v[3];
        for (int m = 0; m < 3; m++) { g_cwSlow = m == 2; g_lastWalk = m; *(int*)(camo3 + 0x24) = 900; hk_CwCamo(0, 0); v[m] = *(int*)(camo3 + 0x24); }
        printf("[3] camo from the game's 90%%: crouchnormal %d%%, crouchwalk %d%%, crouchstalk %d%% (want 80 / 85 / 90)\n", v[0] / 10, v[1] / 10, v[2] / 10);
        g_cwOn = 0; g_cwMoving = 0; g_cwCamo = 0;
        printf("[7] PadStickUpLooksUp default %d (want 0: the game's own camera setting)\n", cfgPadLookUp);
    }
}

// ======================= 0.34 =======================
static void test_part07(void)
{
    printf("\n\n-- part --\n");
    cfgAimMode = 0; simPad.conn = 0; setbit(ST_FPV, 0); setbit(ST_TPS, 1); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); setbit(3, 0); setbit(ST_WALL, 0); setbit(ST_WEAPON, 0);
    wp[0x20] = 5; clockMs += 2000; simConv = 0;
    // ---- [1] leaving the shoulder view after looking up / down: the view's pitch must move one way only
    {   for (int i = 0; i < 20; i++) frame();
        float mv[3] = { -0.01f, 0.01f, 0.0f }; const char* nm[3] = { "after looking up", "after looking down", "without looking up or down" };
        for (int c = 0; c < 3; c++) {
            setbit(ST_WEAPON, 1); for (int i = 0; i < 30; i++) frame();
            mouse = mv[c]; for (int i = 0; i < 25; i++) frame(); mouse = 0; for (int i = 0; i < 5; i++) frame();
            setbit(ST_WEAPON, 0);
            float pp[60]; int rev = 0, dir = 0, held = 0;
            for (int i = 0; i < 60; i++) {
                frame();
                float* e = (float*)(work + 0x3D0); float* t = (float*)(work + 0x3E0);
                float dx = t[0] - e[0], dy = t[1] - e[1], dz = t[2] - e[2];
                pp[i] = atan2f(dy, sqrtf(dx * dx + dz * dz)) * 57.29578f;
                if (i < 2 && g_amt >= 0.999f) held++;
                if (i > 0) { float d = pp[i] - pp[i - 1]; int sg = d > 0.05f ? 1 : d < -0.05f ? -1 : 0; if (sg && dir && sg != dir) rev++; if (sg) dir = sg; }
            }
            printf("[1] leaving %-26s: view pitch %.1f -> %.1f -> %.1f -> %.1f deg, direction reversals %d (want 0), blend waited %d frame(s)\n",
                nm[c], pp[0], pp[2], pp[6], pp[59], rev, held);
        }
    }
    // ---- [2] the forward crawl for each kind of weapon, and sideways sounds
    {   o_Req = s_req; setbit(ST_STAND, 0); setbit(3, 1); for (int i = 0; i < 5; i++) frameX();
        g_aimLatch = 1; g_rawAim = 1; g_camValid = 1; g_proneLatch = 1; g_transLatch = 0; g_have = 1; g_inMag = 255;
        struct { int w; u32 wa; u32 learnA; int learnM; const char* nm; u32 wantA; int wantM; } c[] = {
            { 12, 0x23291, 0, 0, "AK-47", 0x6891CC, 13 }, { 11, 0x6C745A, 0, 0, "XM16E1", 0x6891CC, 13 },
            { 5, 0x4C4AF2, 0x6891CC, 13, "Mk22, crawl learned", 0x6891CC, 13 }, { 7, 0x482B59, 0, 0, "EZ gun, nothing learned", 0x482B59, 11 },
            { 9, 0xBAD929, 0x6891CC, 14, "Patriot, backward crawl learned by an old build", 0x6891CC, 13 } };
        for (int k = 0; k < 5; k++) {
            wp[0x20] = (u8)c[k].w; g_wpnArc = c[k].wa; g_crawlArc[c[k].w] = c[k].learnA; g_crawlMot[c[k].w] = c[k].learnM;
            g_inYaw = g_camYaw16; g_moveFrame = g_frame;
            u8 req[0x60] = {0}; *(u32*)(req + 0x0C) = c[k].wa; *(i16*)(req + 0x10) = 18; *(u64*)(req + 0x18) = 0x7FFFFF; hk_Req(pl, req);
            printf("[2] %-48s forward plays %X/%d (want %X/%d)\n", c[k].nm, *(u32*)(lastReq + 0x0C), *(i16*)(lastReq + 0x10), c[k].wantA, c[k].wantM);
        }
        g_aimLatch = 0; g_rawAim = 0; g_proneLatch = 0; g_have = 0; g_inMag = 0; setbit(3, 0); setbit(ST_STAND, 1); wp[0x20] = 5;
    }
    // ---- [3] first person without a gun, normal camera turned afterwards, then the aim key
    {   simConv = 0; g_orbFixed = -1; *(u16*)(work + 0x338) = 0x1000; for (int i = 0; i < 30; i++) frameX();
        fpvEpisode(0xA000);   // looked at 225 degrees
        for (int i = 0; i < 30; i++) frameX();   // half a second in the normal view
        *(u16*)(work + 0x338) = 0x3000; frameX();   // the player turns the camera to 67 degrees
        FpvHeading(work, clockMs); frameX();
        printf("[3] camera turned after first person: the shoulder view looks %d deg (want 67, the camera's; now: 225)\n", Deg16(*(u16*)(work + 0x338)));
        fpvEpisode(0xA000); FpvHeading(work, clockMs); for (int i = 0; i < 6; i++) frameX();
        printf("[3] straight back from first person: looks %d deg (want 225, first person's)\n", Deg16(*(u16*)(work + 0x338)));
        fpvEpisode(0x6000); for (int i = 0; i < 40; i++) frameX(); FpvHeading(work, clockMs); for (int i = 0; i < 6; i++) frameX();
        printf("[3] back from first person later, camera not touched: looks %d deg (want 135)\n", Deg16(*(u16*)(work + 0x338)));
    }
    // ---- [4] [5]
    printf("[4] torch (18) gets no shoulder view: %d (want 1); pistol (5): %d (want 0)\n", NoOts(18), NoOts(5));
    printf("[5] cig spray (3) is handled like the throwables: %d (want 1); grenade (19): %d (want 1); Mk22 (5): %d (want 0)\n", Throwable(3), Throwable(19), Throwable(5));
    // ---- [7] loading into first person: the toggle switching itself on
    {   static u32 padw; *(u8**)(pl + 0x7F8) = (u8*)&padw; *(u32*)(pl + 0x7F4) = 0;
        cfgAimMode = 1; simLatch = 0; setbit(ST_WEAPON, 0); padw = 0; cfgFpvKey = 0;
        clockMs += 3000;   // a new scene
        for (int i = 0; i < 20; i++) { *(u32*)(pl + 0x7E8) = padw & ~*(u32*)(pl + 0x7F4); frameX(); }
        padw = 0x800;   // the game's toggle switches on by itself
        int seen = 0;
        for (int i = 0; i < 30; i++) { u32 m = *(u32*)(pl + 0x7F4); m &= padw;  *(u32*)(pl + 0x7F4) = m; *(u32*)(pl + 0x7E8) = padw & ~m; if (*(u32*)(pl + 0x7E8) & 0x800) seen++; frameX(); }
        printf("[7] toggle on by itself after a load, key not known: frames the game saw it %d (want 1), mask holds it %d\n", seen, (*(u32*)(pl + 0x7F4) & 0x800) ? 1 : 0);
        padw = 0; for (int i = 0; i < 5; i++) { u32 m = *(u32*)(pl + 0x7F4) & padw; *(u32*)(pl + 0x7F4) = m; *(u32*)(pl + 0x7E8) = padw & ~m; frameX(); }
        printf("[7]   the player's next press switches the toggle off: mask %d (want 0)\n", (*(u32*)(pl + 0x7F4) & 0x800) ? 1 : 0);
        // the player's own presses: key 'V' (0x56) held at each switch-on -> learned
        clockMs += 8000;
        for (int r = 0; r < 2; r++) {
            keyDown[0x56] = 1; padw = 0x800; for (int i = 0; i < 3; i++) { *(u32*)(pl + 0x7E8) = padw; frameX(); } keyDown[0x56] = 0;
            for (int i = 0; i < 10; i++) frameX(); padw = 0; for (int i = 0; i < 10; i++) { *(u32*)(pl + 0x7E8) = padw; frameX(); }
        }
        printf("[7] the player's own first-person presses (V): learned key %02X (want 56), saved to the ini %d time(s) as \"%s\"\n", cfgFpvKey, wpsN, wpsVal);
        // next load: toggle on by itself -> held off and V pressed once
        clockMs += 3000; for (int i = 0; i < 10; i++) { *(u32*)(pl + 0x7E8) = 0; frameX(); }
        padw = 0x800; int before = kbInj[0x56];
        for (int i = 0; i < 5; i++) { u32 m = *(u32*)(pl + 0x7F4) & padw; *(u32*)(pl + 0x7F4) = m; *(u32*)(pl + 0x7E8) = padw & ~m; frameX(); }
        printf("[7] next load: V pressed by the plugin %d time(s) (want 1), held off %d\n", kbInj[0x56] - before, (*(u32*)(pl + 0x7F4) & 0x800) ? 1 : 0);
        padw = 0; for (int i = 0; i < 5; i++) { u32 m = *(u32*)(pl + 0x7F4) & padw; *(u32*)(pl + 0x7F4) = m; *(u32*)(pl + 0x7E8) = 0; frameX(); }
        // in the middle of play (not a fresh scene), the toggle with nothing held is left alone
        for (int i = 0; i < 400; i++) frameX();   // well into the scene (6 seconds)
        padw = 0x800; seen = 0;
        for (int i = 0; i < 10; i++) { u32 m = *(u32*)(pl + 0x7F4) & padw; *(u32*)(pl + 0x7F4) = m; *(u32*)(pl + 0x7E8) = padw & ~m; if (*(u32*)(pl + 0x7E8) & 0x800) seen++; frameX(); }
        printf("[7] during play (no new scene): first person left alone - frames seen %d (want 10)\n", seen);
        padw = 0; *(u32*)(pl + 0x7F4) = 0; *(u32*)(pl + 0x7E8) = 0; *(u8**)(pl + 0x7F8) = 0; cfgAimMode = 0;
    }
}


// the game's held-over mask (Snake+0x7F4): Snake's buttons = input & ~mask, and a mask bit clears once the input bit is
// off
static u32 inW;
static void frameM(void)
{
    float ex = *(float*)(work + 0x340) + mouse; if (ex < 0) ex = 0; if (ex > 1) ex = 1;
    *(float*)(work + 0x340) = ex;
    float p = PitOfElev(ex) * 0.0174532925f;
    float* eye = (float*)(work + 0x3D0); float* tgt = (float*)(work + 0x3E0);
    tgt[0] = *(float*)(pl + 0x130); tgt[1] = *(float*)(pl + 0x134) + 695; tgt[2] = *(float*)(pl + 0x138);
    eye[0] = tgt[0]; eye[1] = tgt[1] - sinf(p) * 4000; eye[2] = tgt[2] - cosf(p) * 4000;
    clockMs += 16;
    inW = simLatch ? 0x8000u : 0;
    u32 m = *(u32*)(pl + 0x7F4) & inW; *(u32*)(pl + 0x7F4) = m;
    *(u32*)(pl + 0x7E8) = (*(u32*)(pl + 0x7E8) & ~0x8000u) | (inW & ~m);
    InputPoll();
    hk_Apply(work);
}
static float eyeStep(const float* a) { const float* e = (const float*)(work + 0x3D0); return sqrtf((e[0]-a[0])*(e[0]-a[0]) + (e[1]-a[1])*(e[1]-a[1]) + (e[2]-a[2])*(e[2]-a[2])); }

static void test_part08(void)
{
    printf("\n\n-- part 08 --\n");
    cfgAimMode = 1; simPad.conn = 0; g_padConn = 0; setbit(ST_FPV, 0); setbit(ST_TPS, 1); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); setbit(3, 0); setbit(ST_WALL, 0);
    setbit(ST_WEAPON, 0); latchable = 1; simLatch = 0; keyDown[2] = 0; g_noLatch = 0;
    showLog = 1;

    {   wp[0x20] = 11; injClicks = 0; clockMs += 3000;
        for (int i = 0; i < 300; i++) { padSet(0, 0, 0, 0); frame(); }
        int okN = 0;
        for (int r = 0; r < 3; r++) {
            for (int i = 0; i < 20; i++) { padSet(0, 0, 0x100, 0); frame(); }
            okN += simLatch;
            for (int i = 0; i < 30; i++) { padSet(0, 0, 0, 0); frame(); }
        }
        printf("[A] pad, XM16E1: LB 3 times - gun raised %d of 3 (want 3), plugin presses %d\n", okN, injClicks);
        latchable = 0; int c0 = injClicks;   // the game does not take the presses now
        for (int i = 0; i < 150; i++) { padSet(0, 0, 0x100, 0); frame(); }
        printf("[A] presses not taken: plugin presses %d, then it stops; XM16E1 written off as \"aim key does nothing\": %d (before: 1, want 0)\n",
            injClicks - c0, (int)((g_noLatch >> 11) & 1));
        for (int i = 0; i < 30; i++) { padSet(0, 0, 0, 0); frame(); }
        latchable = 1; c0 = injClicks;   // the game takes them again
        for (int i = 0; i < 30; i++) { padSet(0, 0, 0x100, 0); frame(); }
        printf("[A] later, presses taken again: LB - gun raised %d (before: 0, want 1), plugin presses %d\n", simLatch, injClicks - c0);
        for (int i = 0; i < 40; i++) { padSet(0, 0, 0, 0); frame(); }
        printf("[A]   LB let go: shoulder %d gun raised %d (want 0 0)\n", (int)g_aimLatch, simLatch);
        padOff(); g_padConn = 0; simPad.conn = 0; for (int i = 0; i < 10; i++) frame();
    }
    // ---- [B] grenade: the aim key really does nothing - written off exactly as before
    {   wp[0x20] = 19; latchable = 0; simLatch = 0; injClicks = 0; g_noLatch = 0; clockMs += 3000; for (int i = 0; i < 300; i++) frame();
        CLICK(); for (int i = 0; i < 120; i++) frame();
        int c1 = injClicks; int on = g_aimLatch;
        CLICK(); CLICK(); for (int i = 0; i < 120; i++) frame();
        printf("[B] grenade, the aim key changes nothing: shoulder %d, plugin presses %d, written off %d (want 1, as before); next time the view is on: presses %d (want 0)\n",
            on, c1, (int)((g_noLatch >> 19) & 1), injClicks - c1);
        latchable = 1; wp[0x20] = 5; simLatch = 0; CLICK(); for (int i = 0; i < 20; i++) frame();
        if (g_aimLatch) CLICK();
        for (int i = 0; i < 30; i++) frame();
    }
    // ---- [C] the game's input says "raised" but Snake's held-over mask hides it
    {   static u32 padw; *(u8**)(pl + 0x7F8) = (u8*)&inW; (void)padw;
        wp[0x20] = 5; latchable = 1; simLatch = 0; injClicks = 0; *(u32*)(pl + 0x7F4) = 0; clockMs += 3000; for (int i = 0; i < 300; i++) frameM();
        keyDown[2] = 1; simLatch = 1; for (int q = 0; q < 4; q++) frameM(); keyDown[2] = 0; for (int q = 0; q < 30; q++) frameM();
        int ok0 = g_aimLatch && (*(u32*)(pl + 0x7E8) & 0x8000u);
        *(u32*)(pl + 0x7F4) = 0x8000;   // the game fills its mask (status 0x138) with the toggle on
        int c0 = injClicks, frames = -1;
        for (int i = 0; i < 90; i++) { frameM(); if (frames < 0 && (*(u32*)(pl + 0x7E8) & 0x8000u)) frames = i + 1; }
        printf("[C] shoulder view on (%d), then the mask hides the raised gun: Snake sees it raised again after %d frame(s) (before: after its presses), plugin presses %d (before: 2, want 0)\n",
            ok0, frames, injClicks - c0);
        keyDown[2] = 1; simLatch = !simLatch; for (int q = 0; q < 4; q++) frameM(); keyDown[2] = 0; for (int q = 0; q < 40; q++) frameM();
        printf("[C]   aim key: shoulder %d gun raised %d (want 0 0)\n", (int)g_aimLatch, simLatch);
        *(u8**)(pl + 0x7F8) = 0; *(u32*)(pl + 0x7F4) = 0;
    }
    // ---- [D] the game starts the switch to first person and calls it off
    {   wp[0x20] = 11; simLatch = 0; injClicks = 0; clockMs += 3000; for (int i = 0; i < 300; i++) frame();
        CLICK(); for (int i = 0; i < 60; i++) frame();
        float e0[3]; memcpy(e0, (float*)(work + 0x3D0), 12);
        float prev[3]; memcpy(prev, e0, 12); float maxIn = 0, maxBack = 0, far = 0;
        setbit(ST_TPS, 0);   // the game leaves third person
        for (int i = 0; i < 5; i++) { frame(); float s = eyeStep(prev); if (s > maxIn) maxIn = s; float f = eyeStep(e0); if (f > far) far = f; memcpy(prev, (float*)(work + 0x3D0), 12); }
        setbit(ST_TPS, 1);   // ... and calls it off
        int backAt = -1;
        for (int i = 0; i < 40; i++) {
            frame(); float s = eyeStep(prev); if (s > maxBack) maxBack = s; memcpy(prev, (float*)(work + 0x3D0), 12);
            if (backAt < 0 && eyeStep(e0) < 5.0f) backAt = i + 1;
        }
        printf("[D] first person called off after 5 frames: the camera had got %.0f units toward his head (steps up to %.0f a frame); going back: biggest step %.0f units a frame (before: all of it in one frame), back on the shoulder after %d frame(s); shoulder %d\n",
            far, maxIn, maxBack, backAt, (int)g_aimLatch);
        CLICK(); for (int i = 0; i < 40; i++) frame();
        printf("[D]   aim key: shoulder %d gun raised %d (want 0 0)\n", (int)g_aimLatch, simLatch);
        // a real switch to first person still works as before
        CLICK(); for (int i = 0; i < 60; i++) frame();
        setbit(ST_TPS, 0); for (int i = 0; i < 14; i++) frame(); setbit(ST_FPV, 1); for (int i = 0; i < 30; i++) frame();
        int inFp = Bit(ST_FPV) && !g_aimLatch;
        setbit(ST_FPV, 0); setbit(ST_TPS, 1); for (int i = 0; i < 40; i++) frame();
        printf("[D]   a real switch: first person %d, back on the shoulder afterwards %d (want 1 1)\n", inFp, (int)g_aimLatch);
        CLICK(); for (int i = 0; i < 40; i++) frame();
    }
    printf("[counters] called off %u, presses paused %u, mask let through %u, taken back %u\n", g_fpCancelN, g_pressFailN, g_maskFreeN, g_noLatchN);
    showLog = 0; cfgAimMode = 0;
}

#include "tests/pad_stick.inc"
#include "tests/moves.inc"
#include "tests/gun_pose.inc"
#include "tests/gun_aim.inc"
#include "tests/first_person_a.inc"
#include "tests/first_person_b.inc"
#include "tests/first_person_c.inc"
int main(int argc, char** argv)
{
    // usage: sim <Snake's motion archive 006891cc.mtar>
    if (argc > 1) g_mtarPath = argv[1];
    cfgLogLvl = 2;

    cfgClassicBlock = 0;
    g_log = (HANDLE)1; showLog = 0; cfgLog = 0; cfgAimMode = 0; cfgHold = 100;
    g_bits = bits; g_pl = pl; o_Apply = s_apply;
    c_area = s_area; c_test = s_test; c_sort = s_sort; c_num = s_num; c_flag = s_flag; c_point = s_point;
    static u32 areaArg = 1; c_areaArg = &areaArg;
    *(u8**)(pl + 0x6F8) = wp; wp[0x20] = 11; *(u32*)(pl + 0x120) = 1;
    *(float*)(pl + 0x130) = 0; *(float*)(pl + 0x134) = 1000; *(float*)(pl + 0x138) = 0;
    *(float*)(work + 0x98) = 0; *(float*)(work + 0x9C) = 1; *(float*)(work + 0xA0) = 0.25f; *(float*)(work + 0xD8) = 1.9f; *(float*)(work + 0x94) = 0.8f;
    *(u8**)(work + 0xE0) = cam; *(float*)(cam + 0x7C) = 1.9f;
    setbit(ST_TPS, 1); setbit(ST_STAND, 1);
    g_nbLo = (u8*)0; g_nbHi = (u8*)~0ull;
    float starts[] = { 0.6667f, 0.85f, 0.45f, 0.25f };
    for (int s = 0; s < 4; s++) {
        *(float*)(work + 0x340) = starts[s]; mouse = 0;
        setbit(ST_WEAPON, 0); for (int i = 0; i < 40; i++) frame();
        float third = orbitDeg();
        setbit(ST_WEAPON, 1); for (int i = 0; i < 40; i++) frame();
        printf("start elev %.3f: third-person pitch %6.1f -> shoulder pitch %6.1f (aim %d amt %.2f, eye y %.0f, zoom written %.3f)\n", starts[s], third, g_pitchDeg, (int)g_aimLatch, g_amt,
            *(float*)(work + 0x3D0 + 4), *(float*)(work + 0xD8));
        // look up a little with the mouse while aiming, then leave
        mouse = -0.002f; for (int i = 0; i < 25; i++) frame(); mouse = 0; frame();
        float sh = g_pitchDeg;
        setbit(ST_WEAPON, 0); clockMs += 500; for (int i = 0; i < 40; i++) frame();
        printf("   after moving the view: shoulder pitch %6.1f -> back in third person %6.1f (elev %.3f, zoom %.3f)\n", sh, orbitDeg(), *(float*)(work + 0x340), *(float*)(work + 0xD8));
    }
    // ---- shots
    *(float*)(work + 0x340) = 0.55f; setbit(ST_WEAPON, 1); for (int i = 0; i < 60; i++) frame();
    printf("\naim pitch %.1f; crosshair covers a point %d units ahead: (%.0f, %.0f, %.0f); from-gun=%d sight=%.3f,%.3f ghost=%d obs=%d\n", g_pitchDeg, (int)g_ctrDist, g_tPt[0], g_tPt[1], g_tPt[2], (int)g_tMuzzle, g_sx, g_sy, (int)g_ghost, (int)g_obsMm);
    float m[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }, mm[16], tg[4];
    for (int i = 0; i < 3; i++) m[12 + i] = g_tQ[i] + g_tF[i] * 700 + g_tR[i] * 120 + g_tU[i] * -150;
    printf("true muzzle (%.0f, %.0f, %.0f)\n", m[12], m[13], m[14]);
    showLog = 1; cfgLog = 1;
    int how = ShotPlan(m, 11, (void*)0x1000, mm, tg);
    printf("shot 1: how=%d start (%.0f, %.0f, %.0f) target dir (%.4f, %.4f, %.4f) vs aim (%.4f, %.4f, %.4f)\n", how, mm[12], mm[13], mm[14],
        (tg[0] - mm[12]) / 150000, (tg[1] - mm[13]) / 150000, (tg[2] - mm[14]) / 150000, g_tF[0], g_tF[1], g_tF[2]);
    {   // where does that bullet land compared with the point the crosshair covers?
        float t = (g_tPt[2] - mm[14]) / (tg[2] - mm[14]);
        printf("   bullet passes the aimed point at (%.1f, %.1f) vs point (%.1f, %.1f)\n", mm[12] + (tg[0] - mm[12]) * t, mm[13] + (tg[1] - mm[13]) * t, g_tPt[0], g_tPt[1]);
    }
    cfgLog = 0; for (int i = 0; i < 5; i++) frame();
    printf("after the first shot (muzzle known): from-gun=%d side=%d sight=%.3f,%.3f ghost=%d land=%d\n", (int)g_tMuzzle, (int)g_sideClear, g_sx, g_sy, (int)g_ghost, (int)g_landDist);
    tree = 1; for (int i = 0; i < 5; i++) frame();
    printf("tree in front of the gun: from-gun=%d side=%d sight=%.3f,%.3f ghost=%d land=%d obs=%d\n", (int)g_tMuzzle, (int)g_sideClear, g_sx, g_sy, (int)g_ghost, (int)g_landDist, (int)g_obsMm);
    cfgLog = 1; how = ShotPlan(m, 11, (void*)0x1000, mm, tg);
    printf("shot 2: how=%d target (%.0f, %.0f, %.0f)\n", how, tg[0], tg[1], tg[2]);
    how = ShotPlan(m, 12, (void*)0x1000, mm, tg); printf("shot with another weapon type: how=%d\n", how);
    m[12] += 5000; how = ShotPlan(m, 11, (void*)0x1000, mm, tg); printf("shot from 5 m away: how=%d\n", how);
    // ---- movement
    printf("\n");
    static i32 tick = 5; p_tick = &tick; g_have = 1; g_inMag = 255; g_camYaw16 = 0x2000;
    for (int st = 0; st < 3; st++) {
        g_proneLatch = st == 2; g_crouchLatch = st == 1;
        for (int deg = 0; deg < 360; deg += 45) {
            g_inYaw = (u16)(g_camYaw16 + deg * 65536 / 360);
            int d = PickDir(); g_legFrame = g_frame; g_legDir = d;
            Push(pl);
            float vx = *(float*)(pl + 0x620), vz = *(float*)(pl + 0x628);
            // what the walk / crawl motion itself adds
            static const u16 card[4] = { 0, 0x8000, 0x4000, 0xC000 };
            float base = st == 2 ? (d == 0 ? 15.1f : d == 1 ? 8.6f : 4.8f) : st == 1 ? 12.0f : 20.0f;
            u16 y = (u16)(g_camYaw16 + card[d]);
            float tx = vx + sinf(y * 6.2831853f / 65536) * base, tz = vz + cosf(y * 6.2831853f / 65536) * base;
            printf("%s input %3d deg: dir %d, plugin adds %6.1f,%6.1f -> total speed %5.1f\n", st == 2 ? "prone " : st == 1 ? "crouch" : "stand ", deg, d, vx, vz, sqrtf(tx * tx + tz * tz));
        }
    }

    printf("\n");
    g_proneLatch = 0;
    for (int st = 0; st < 2; st++) for (int fast = 0; fast < 2; fast++) {
        g_crouchLatch = st; g_inMag = fast ? 255 : 120; g_have = 1;
        float mx = 0, mn = 1e9f;
        for (int deg = 0; deg < 360; deg += 15) {
            g_inYaw = (u16)(g_camYaw16 + deg * 65536 / 360);
            int d = PickDir(); g_legFrame = g_frame; g_legDir = d; g_fastLeg = fast;
            Push(pl);
            static const u16 card[4] = { 0, 0x8000, 0x4000, 0xC000 };
            float base = fast ? (st ? 28.2f : 38.4f) : (st ? 12.0f : 20.0f);
            u16 y = (u16)(g_camYaw16 + card[d]);
            float tx = *(float*)(pl + 0x620) + sinf(y * 6.2831853f / 65536) * base, tz = *(float*)(pl + 0x628) + cosf(y * 6.2831853f / 65536) * base;
            float sp = sqrtf(tx * tx + tz * tz); if (sp > mx) mx = sp; if (sp < mn) mn = sp;
        }
        printf("%s %s: wanted fast=%d, speed over all directions %.1f .. %.1f\n", st ? "crouch" : "stand ", fast ? "stick full " : "stick light", (int)g_fast, mn, mx);
    }
    keyDown[16] = 1; g_inMag = 255; Push(pl); printf("walk key held, stick full: fast=%d\n", (int)g_fast); keyDown[16] = 0;
    g_have = 0;
    printf("\n");
    tree = 0; setbit(ST_WEAPON, 1); *(float*)(work + 0x340) = 0.5f;
    for (int i = 0; i < 80; i++) frame();
    printf("open ground: eye (%.0f, %.0f, %.0f) side %d%% back %d%%\n", g_tEye[0], g_tEye[1], g_tEye[2], (int)g_sideK100, (int)g_backK100);
    backWall = 1; for (int i = 0; i < 3; i++) frame();
    printf("wall 600 behind: eye (%.0f, %.0f, %.0f) side %d%% back %d%%\n", g_tEye[0], g_tEye[1], g_tEye[2], (int)g_sideK100, (int)g_backK100);
    backWall = 0; for (int i = 0; i < 60; i++) frame();
    printf("wall gone, one second later: eye z %.0f back %d%%\n", g_tEye[2], (int)g_backK100);
    sideWall = 1; for (int i = 0; i < 3; i++) frame();
    printf("wall 300 to the right: eye (%.0f, %.0f, %.0f) side %d%% back %d%%\n", g_tEye[0], g_tEye[1], g_tEye[2], (int)g_sideK100, (int)g_backK100);
    sideWall = 0; for (int i = 0; i < 60; i++) frame();
    printf("\n");
    for (float e = 0.5f; e < 0.96f; e += 0.09f) {
        setbit(ST_WEAPON, 0); clockMs += 500; for (int i = 0; i < 40; i++) frame();
        *(float*)(work + 0x340) = e; for (int i = 0; i < 20; i++) frame();
        setbit(ST_WEAPON, 1); for (int i = 0; i < 90; i++) frame();
        float ax = g_aimPt[0] - *(float*)(pl + 0x130), az = g_aimPt[2] - *(float*)(pl + 0x138);
        printf("pitch %6.1f: body aims at a point %5d away, %4.1f degrees to the right of straight ahead\n", g_pitchDeg, (int)g_convDist, atan2f(ax, az) * 57.29578f);
    }
    printf("\n");
    cfgAimMode = 1; showLog = 1; cfgLog = 0; g_log = (HANDLE)1;
    setbit(ST_WEAPON, 0); *(float*)(work + 0x340) = 0.6f; clockMs += 3000; simLatch = 0;
    for (int i = 0; i < 30; i++) frame();
#define CLICK() do { keyDown[2] = 1; if (latchable) simLatch = !simLatch; for (int q = 0; q < 4; q++) frame(); keyDown[2] = 0; for (int q = 0; q < 40; q++) frame(); } while (0)
#define STATE(txt) printf("%-58s shoulder=%d gun raised=%d plugin clicks=%d\n", txt, (int)g_aimLatch, simLatch, injClicks)
    STATE("A. plain third person:");
    CLICK(); STATE("   one press:");
    CLICK(); STATE("   one more press:");
    simLatch = 1; for (int i = 0; i < 60; i++) frame(); STATE("B. gun found raised in the normal view, one second later:");
    CLICK(); STATE("   one press:");
    setbit(ST_FPV, 1); setbit(ST_TPS, 0); for (int i = 0; i < 90; i++) frame(); STATE("C. first-person view from the shoulder view:");
    simLatch = 0; for (int i = 0; i < 30; i++) frame();   // a shot there drops the game's toggle
    setbit(ST_FPV, 0); setbit(ST_TPS, 1); for (int i = 0; i < 60; i++) frame(); STATE("   fired, then left first person:");
    clockMs += 5000; for (int i = 0; i < 120; i++) frame(); STATE("D. after a loading screen (gun was raised):");
    CLICK(); STATE("   one press:");
    CLICK();
    wp[0x20] = 1; latchable = 0; injClicks = 0; for (int i = 0; i < 30; i++) frame();
    STATE("E. knife:"); CLICK(); for (int i = 0; i < 200; i++) frame(); STATE("   one press, three seconds later:");
    CLICK();
    wp[0x20] = 4; latchable = 1; injClicks = 0; for (int i = 0; i < 30; i++) frame();
    STATE("F. handkerchief:"); CLICK(); STATE("   one press:");
    CLICK(); wp[0x20] = 5; injClicks = 0; for (int i = 0; i < 30; i++) frame();
    static i32 cutFlag = 1; p_cutscene = &cutFlag; clockMs += 4000; for (int i = 0; i < 30; i++) frame();
    STATE("G. new area, the game still reports a cutscene:"); CLICK(); STATE("   one press:");
    cutFlag = 0; for (int i = 0; i < 60; i++) frame(); STATE("   the report ends a moment later:");
    CLICK(); STATE("   one press:");
    setbit(ST_FPV, 1); setbit(ST_TPS, 0); for (int i = 0; i < 60; i++) frame(); STATE("H. first person from plain third person:");
    CLICK(); STATE("   one press there (aiming down the sights):");
    setbit(ST_FPV, 0); setbit(ST_TPS, 1); for (int i = 0; i < 60; i++) frame(); STATE("   left first person:");
    CLICK();
    setbit(ST_WALL, 1); for (int i = 0; i < 30; i++) frame(); STATE("I. back against a wall:"); CLICK(); STATE("   one press:");
    setbit(ST_WALL, 0); for (int i = 0; i < 30; i++) frame(); STATE("   off the wall:");

    // ======================= 0.24 =======================
    cfgAimMode = 0; showLog = 1; g_log = (HANDLE)1; cfgLog = 0;
    loadArc();
    printf("\n-- body height decoder (real main archive) --\n");
    { int sl[] = { 0, 2, 204, 216, 220, 15 }; float fr[] = { 0, 10.5f, 33 };
      for (int i = 0; i < 6; i++) { printf("slot %3d:", sl[i]); for (int j = 0; j < 3; j++) { playSlot(sl[i], fr[j]); g_hipHdr = 0; printf("  f%.1f y=%.3f", fr[j], BodyHipY(pl)); } printf("\n"); } }
    printf("\n-- camera locked to the body --\n");
    setbit(ST_FPV, 0); setbit(ST_TPS, 1); setbit(ST_WEAPON, 1); setbit(6, 0); simLatch = 0; wp[0x20] = 5;
    *(float*)(work + 0x340) = 0.6667f; mouse = 0;
    playSlot(0, 0); for (int i = 0; i < 80; i++) frame();
    printf("standing idle:      hip %.2f body %d lock %d eye y %.0f\n", g_hip100 / 100.0f, (int)g_body100, (int)g_lock, *(float*)(work + 0x3D4));
    setbit(ST_STAND, 0); setbit(ST_CROUCH, 1);
    for (int i = 0; i < 12; i++) { playSlot(2, 0); frame(); if (i % 3 == 0) printf("  crouching, frame %2d: body %d\n", i, (int)g_body100); }
    for (int i = 0; i < 80; i++) frame();
    printf("crouched idle:      hip %.2f body %d eye y %.0f\n", g_hip100 / 100.0f, (int)g_body100, *(float*)(work + 0x3D4));
    for (int i = 0; i < 80; i++) { playSlot(220, (float)(i % 70)); frame(); }
    printf("fast crouch walk:   hip %.2f body %d\n", g_hip100 / 100.0f, (int)g_body100);
    for (int i = 0; i < 80; i++) { playSlot(216, (float)(i % 112)); frame(); }
    printf("low crouch walk:    hip %.2f body %d\n", g_hip100 / 100.0f, (int)g_body100);
    setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); playSlot(0, 0); for (int i = 0; i < 60; i++) frame();

    printf("\n-- crouch walking in the normal view (crouch walking on) --\n");
    { o_Inp = s_inp; o_Req = s_req; g_aimLatch = 0; setbit(ST_WEAPON, 0);
      int mags[] = { 255, 128, 51, 255 }; int shift[] = { 0, 0, 0, 1 };
      for (int k = 0; k < 4; k++) {
        keyDown[16] = shift[k]; inpMag = (u16)mags[k];
        hk_Inp(pl, work, 0, 0);
        u16 seen = *(u16*)(pl + 0x7E0);
        u8 req[0x60] = {0}; *(u32*)(req + 0x0C) = 0x4C4AF2; *(i16*)(req + 0x10) = 5; *(u64*)(req + 0x18) = 0x7FFFFF; *(u32*)(req + 0x34) = 0x6891CC;
        hk_Req(pl, req);
        printf("stick %3d shift %d: mod sees stick %3d (%s) ; motion asked %X/%d, layer1 %X/%d mask %llX ; request given back %X/%d\n", mags[k], shift[k], seen,
            seen >= 180 ? "its normal speed" : "its slow speed", *(u32*)(lastReq + 0x0C), *(i16*)(lastReq + 0x10), *(u32*)(lastReq + 0x34), *(i16*)(lastReq + 0x38),
            (unsigned long long)*(u64*)(lastReq + 0x40), *(u32*)(req + 0x0C), *(i16*)(req + 0x10));
      }
      keyDown[16] = 0; g_log = (HANDLE)1; }
    setbit(ST_STAND, 1); setbit(ST_CROUCH, 0);



    printf("\n-- walking speeds in the shoulder view (keyboard: stick 255 / 128 / 51, Shift held or not) --\n");
    o_Req = s_req; cfgAimMode = 0; setbit(ST_WEAPON, 1);
    for (int stc = 0; stc < 2; stc++) {
      setbit(ST_STAND, !stc); setbit(ST_CROUCH, stc); for (int i = 0; i < 40; i++) frame();
      int mags[] = { 255, 128, 51, 128 }; int shift[] = { 0, 0, 0, 1 }; const char* nm[] = { "full", "128 (walk)", "51 (stalk)", "128 + Shift" };
      for (int k = 0; k < 4; k++) {
        keyDown[16] = shift[k]; float mn = 1e9f, mx = 0; int r100 = 0;
        for (int deg = 0; deg < 360; deg += 15) {
            g_inMag = mags[k]; g_inYaw = (u16)(g_camYaw16 + deg * 65536 / 360); g_have = 1;
            Push(pl);
            int d = PickDir();
            u8 req[0x60] = {0}; *(u32*)(req + 0x0C) = 0x6891CC; *(i16*)(req + 0x10) = stc ? 3 : 1; *(u64*)(req + 0x18) = 0x7FFFFF; g_moveFrame = g_frame;
            hk_Req(pl, req);
            playSlot(*(i16*)(pl + 0x78) - 1, 3); *(u32*)(lyT + 0x54) = 0x6891CC;
            Push(pl);
            float rate = *(float*)(lyT + 0x4C); r100 = (int)(rate * 100);
            float base = stc ? (g_fastLeg ? 28.2f : 12.0f) : (g_fastLeg ? 38.4f : 20.0f);
            static const u16 card[4] = { 0, 0x8000, 0x4000, 0xC000 };
            u16 y = (u16)(g_camYaw16 + card[d]);
            float carry = base * rate / 5.0f;
            float tx = *(float*)(pl + 0x620) + sinf(y * 6.2831853f / 65536) * carry, tz = *(float*)(pl + 0x628) + cosf(y * 6.2831853f / 65536) * carry;
            float sp = sqrtf(tx * tx + tz * tz); if (sp < mn) mn = sp; if (sp > mx) mx = sp;
        }
        printf("%s %-12s band %2d, legs rate %.2f: speed in every direction %.1f .. %.1f\n", stc ? "crouched" : "standing", nm[k], (int)g_band, r100 / 100.0f, mn, mx);
      }
    }
    keyDown[16] = 0; g_have = 0;
    printf("\n-- lying sideways --\n");
    setbit(ST_STAND, 0); setbit(ST_CROUCH, 0); setbit(3, 1); for (int i = 0; i < 30; i++) frame();
    g_inMag = 255; g_inYaw = (u16)(g_camYaw16 + 0x4000); g_have = 1; g_legFrame = g_frame; g_legDir = 2; Push(pl);
    printf("lying, moving left with the side motion playing: plugin push %.1f, %.1f (the motion alone moves him)\n", *(float*)(pl + 0x620), *(float*)(pl + 0x628));
    g_have = 0; setbit(3, 0); setbit(ST_STAND, 1); setbit(ST_WEAPON, 0); playSlot(0, 0); for (int i = 0; i < 40; i++) frame();
    printf("\n-- crosshair obstacle check works before the first shot (built-in muzzle) --\n");
    MuzzleDefaults(); wp[0x20] = 5; tree = 1; setbit(ST_WEAPON, 1); *(float*)(work + 0x340) = 0.6f;
    for (int i = 0; i < 60; i++) frame();
    printf("Mk22, tree in front, no shot yet: muzzle source %d, gun line blocked=%d, crosshair moved %.3f,%.3f\n", g_moSrc[5][0], (int)g_tMuzzle, g_sx, g_sy);
    { int ty[] = { 16, 15, 17, 14, 13, 6, 9 }; const char* nm[] = { "Mosin", "SVD", "RPG-7", "M37", "M63", "M1911A1", "Patriot" };
      for (int k = 0; k < 7; k++) {
        wp[0x20] = (u8)ty[k]; tree = 1; for (int i = 0; i < 20; i++) frame();
        int bl = g_tMuzzle; float sx = g_sx;
        tree = 0; for (int i = 0; i < 20; i++) frame();
        printf("%-8s no shot ever: tree in front -> blocked=%d (crosshair x %.3f); no tree -> blocked=%d\n", nm[k], bl, sx, (int)g_tMuzzle);
      }
      wp[0x20] = 5; setbit(ST_STAND, 0); setbit(3, 1); *(float*)(pl + 0x134) = 500; tree = 1; for (int i = 0; i < 40; i++) frame();
      int bl = g_tMuzzle; tree = 0; for (int i = 0; i < 30; i++) frame();
      printf("Mk22 lying, no shot: tree -> blocked=%d; open ground -> blocked=%d\n", bl, (int)g_tMuzzle);
      setbit(3, 0); setbit(ST_STAND, 1); *(float*)(pl + 0x134) = 1000; }
    tree = 0; setbit(ST_WEAPON, 0); wp[0x20] = 5; for (int i = 0; i < 40; i++) frame();

    printf("\n-- shoulder view <-> first person --\n");
    static u8 fpc[0x100]; *(u8**)(pl + 0x730) = fpc;
    cfgAimMode = 1; simLatch = 0; keyDown[2] = 0; playSlot(0, 0); showLog = 1;
    for (int i = 0; i < 40; i++) frame();
    CLICK();
    printf("shoulder view: eye (%.0f, %.0f, %.0f)\n", *(float*)(work + 0x3D0), *(float*)(work + 0x3D4), *(float*)(work + 0x3D8));
    keyDown[4] = 1; for (int i = 0; i < 3; i++) frame();   // middle button pressed to switch
    setbit(ST_TPS, 0);
    for (int i = 0; i <= 15; i++) { frame(); if (i % 3 == 0) printf("  to first person, %3d ms: eye (%.0f, %.0f, %.0f) cut flag %d zoom mult %d\n", (i + 1) * 16, *(float*)(work + 0x3D0), *(float*)(work + 0x3D4), *(float*)(work + 0x3D8), (int)(*(u32*)(work + 0x320) & 1), (int)g_zoomMul); *(u32*)(work + 0x320) = 0; }
    keyDown[4] = 0;
    // first person: the game's first-person camera at Snake's head, looking 20 degrees up
    *(float*)(fpc + 0x20) = 0; *(float*)(fpc + 0x24) = 1000 + 850; *(float*)(fpc + 0x28) = 60;
    *(float*)(fpc + 0x30) = 0; *(float*)(fpc + 0x34) = 1850 + 364; *(float*)(fpc + 0x38) = 60 + 1000;
    setbit(ST_FPV, 1); for (int i = 0; i < 30; i++) frame();
    setbit(ST_FPV, 0); setbit(ST_TPS, 1);
    for (int i = 0; i <= 14; i++) { frame(); if (i % 3 == 0) printf("  back to the shoulder, %3d ms: eye (%.0f, %.0f, %.0f) pitch %.1f\n", (i + 1) * 16, *(float*)(work + 0x3D0), *(float*)(work + 0x3D4), *(float*)(work + 0x3D8), g_pitchDeg); *(u32*)(work + 0x320) = 0; }
    for (int i = 0; i < 20; i++) frame();
    setbit(ST_TPS, 0);
    for (int i = 0; i <= 15; i++) { frame(); if (i == 15) printf("second switch, flight end: eye (%.0f, %.0f, %.0f) (learned eye %.0f above the body point)\n", *(float*)(work + 0x3D0), *(float*)(work + 0x3D4), *(float*)(work + 0x3D8), g_hoU[0]); }
    setbit(ST_FPV, 1); for (int i = 0; i < 10; i++) frame(); setbit(ST_FPV, 0); setbit(ST_TPS, 1); for (int i = 0; i < 30; i++) frame();
    CLICK();


    printf("\n-- weapon put away in first person --\n");
    wp[0x20] = 12; CLICK();
    setbit(ST_TPS, 0); for (int i = 0; i < 15; i++) frame(); setbit(ST_FPV, 1); for (int i = 0; i < 10; i++) frame();
    printf("in first person: hold %d amt %.2f fly %d\n", (int)g_hold, g_amt, (int)g_fly);
    wp[0x20] = 0; frame(); frame();
    printf("weapon put away: hold %d amt %.2f fly %d\n", (int)g_hold, g_amt, (int)g_fly);
    setbit(ST_FPV, 0); for (int i = 0; i < 5; i++) frame(); setbit(ST_TPS, 1); for (int i = 0; i < 20; i++) frame();
    printf("back to third person: shoulder %d amt %.2f (the game's own camera)\n", (int)g_aimLatch, g_amt);
    printf("\n-- first person ends through a moment without any view flag --\n");
    wp[0x20] = 12; simLatch = 0; for (int i = 0; i < 20; i++) frame(); CLICK();
    setbit(ST_TPS, 0); for (int i = 0; i < 15; i++) frame(); setbit(ST_FPV, 1); for (int i = 0; i < 10; i++) frame();
    setbit(ST_FPV, 0); for (int i = 0; i < 3; i++) frame();
    printf("first person off, third person not yet on: hold %d amt %.2f\n", (int)g_hold, g_amt);
    setbit(ST_TPS, 1); frame();
    printf("third person on: shoulder %d fly %d amt %.2f\n", (int)g_aimLatch, (int)g_fly, g_amt);
    for (int i = 0; i < 20; i++) frame(); CLICK();

    TEST_PART01();
    test_part01();
    test_part02();
    test_part03();
    test_part04();
    test_part05();
    test_part06();
    test_part07();
    printf("\n-- claymore / C3 --\n");
    injClicks = 0; { int ty[] = { 26, 27 }; for (int k = 0; k < 2; k++) { wp[0x20] = (u8)ty[k]; for (int i = 0; i < 5; i++) frame(); showLog = 0; CLICK(); int on = g_aimLatch; if (on) CLICK(); showLog = 1; printf("type %d: shoulder view %s, plugin key presses %d\n", ty[k], on ? "ON" : "off", injClicks); } }
    test_part08();
    test_part09();
    test_part10();
    test_part11();
    test_part12();
    test_part13();
    test_part14();
    test_part15();
    return 0;
}
