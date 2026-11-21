// The aiming motion holds the gun tilted up by gTilt degrees - the kind of offset that leaves the barrel off the spot
// in the game. [H] the camera's head lock while firing and crouching; [S] strafe walk through shots; [C] crosshair
// hiding; [N] camera collision with fence nets.
#define NB 57
static u8 skelX[0x50 * NB] __attribute__((aligned(16))), modelXX[0x100]; static float quatsX[4 * NB] __attribute__((aligned(16)));
static float offX[NB][3]; static int par[NB];
static float gTilt = 10.0f; static int gGunBone = 14, gStale = 0;
static const float gBl[3] = { 0.05f, 0.03f, 0.9986f }, gOl[3] = { 0.0f, 60.0f, 250.0f };   // the gun on its bone: barrel, muzzle
static void qmul(const float* a, const float* b, float* o)   // Hamilton a*b (exe+753F40 computes out = t*q)
{
    float x = a[3]*b[0] + a[0]*b[3] + a[1]*b[2] - a[2]*b[1];
    float y = a[3]*b[1] - a[0]*b[2] + a[1]*b[3] + a[2]*b[0];
    float z = a[3]*b[2] + a[0]*b[1] - a[1]*b[0] + a[2]*b[3];
    float w = a[3]*b[3] - a[0]*b[0] - a[1]*b[1] - a[2]*b[2];
    o[0] = x; o[1] = y; o[2] = z; o[3] = w;
}
static void qaxis(float ax, float ay, float az, float ang, float* q) { float s = sinf(ang * 0.5f); q[0] = ax*s; q[1] = ay*s; q[2] = az*s; q[3] = cosf(ang * 0.5f); }
static void s_qmat(float* m, const float* q)   // exe+754240, transcribed
{
    float x = q[0], y = q[1], z = q[2], w = q[3];
    for (int i = 0; i < 16; i++) m[i] = 0;
    m[0] = 1 - 2*(y*y + z*z); m[1] = 2*(x*y + w*z);     m[2] = 2*(x*z - w*y);
    m[4] = 2*(x*y - w*z);     m[5] = 1 - 2*(x*x + z*z); m[6] = 2*(y*z + w*x);
    m[8] = 2*(x*z + w*y);     m[9] = 2*(y*z - w*x);     m[10] = 1 - 2*(x*x + y*y); m[15] = 1;
}
static void s_mmat(u8* place, float* m)   // exe+B8860: Snake's heading and position (row vectors)
{
    u8* p = place - 0x120; float h = (float)*(u16*)(p + 0x162) * 6.2831853f / 65536.0f, c = cosf(h), s = sinf(h);
    for (int i = 0; i < 16; i++) m[i] = 0;
    m[0] = c; m[2] = -s; m[5] = 1; m[8] = s; m[10] = c; m[15] = 1;
    m[12] = *(float*)(p + 0x130); m[13] = *(float*)(p + 0x134); m[14] = *(float*)(p + 0x138);
}
static void vmul(const float* v, const float* m, float* o) { for (int j = 0; j < 3; j++) o[j] = v[0]*m[j] + v[1]*m[4+j] + v[2]*m[8+j]; }
static void s_boneX(float* out, int bone, u8* pair, u8* ctrl)   // exe+372800, transcribed
{
    (void)pair; float acc[3] = { 0, 0, 0 };
    for (int b = bone; b > 0; b = par[b]) { float m[16], t[3]; s_qmat(m, &quatsX[4 * par[b]]); vmul(offX[b], m, t); for (int i = 0; i < 3; i++) acc[i] += t[i]; }
    float mm[16], w[3]; s_mmat(ctrl, mm); vmul(acc, mm, w);
    out[0] = w[0] + mm[12]; out[1] = w[1] + mm[13]; out[2] = w[2] + mm[14]; out[3] = 1;
}
static int gT;   // frames, for the hands' sway
static void poseBase(void)   // the aiming motion: the arms tilted up by gTilt
{
    for (int b = 0; b < NB; b++) { quatsX[4*b] = quatsX[4*b+1] = quatsX[4*b+2] = 0; quatsX[4*b+3] = 1; }
    float q[4]; qaxis(1, 0, 0, -gTilt * 0.0174532925f, q);   // about the model's x axis; negative = up
    for (int b = 5; b <= 14; b++) {
        // each forearm / hand bone sways a little on its own (a few degrees, each at its own pace), so only the bone
        // that holds the gun predicts the gun
        float e[4], o[4], ax = (float)((b * 7) % 3) - 1.0f, ay = (float)((b * 5) % 3) - 1.0f, az = 0.5f; float n = sqrtf(ax*ax + ay*ay + az*az);
        qaxis(ax / n, ay / n, az / n, b >= 7 ? 0.08f * sinf((float)gT * 0.004f * (float)(b - 5) + (float)b) : 0.0f, e);
        qmul(q, e, o); memcpy(&quatsX[4*b], o, 16);
    }
}
static int ikCalls; static float ikPitch, ikLim;
static void s_ik(u8* p, u8* st)   // exe+3700A0, the parts that matter here
{
    (void)st; ikCalls++;
    float A[3] = { *(float*)(p + 0x520), *(float*)(p + 0x524), *(float*)(p + 0x528) }, B[4];
    s_boneX(B, 14, p + 0x2B0, p + 0x120);   // bone 14 as the pose is now (the motion, before the turn)
    float h = sqrtf((A[0]-B[0])*(A[0]-B[0]) + (A[2]-B[2])*(A[2]-B[2]));
    float pitch = atan2f(-(A[1] - B[1]), h);   // + = down (exe+118370)
    float yaw = atan2f(A[0] - *(float*)(p + 0x130), A[2] - *(float*)(p + 0x138)) - (float)*(u16*)(p + 0x162) * 6.2831853f / 65536.0f;
    while (yaw > 3.14159f) yaw -= 6.2831853f; while (yaw < -3.14159f) yaw += 6.2831853f;
    float dn = (float)*(i16*)(p + 0x538) * 6.2831853f / 65536.0f, up = (float)*(i16*)(p + 0x53C) * 6.2831853f / 65536.0f;
    float yl = (float)*(i16*)(p + 0x53A) * 6.2831853f / 65536.0f;
    ikLim = 0; if (pitch > dn) { pitch = dn; ikLim = 1; } if (pitch < up) { pitch = up; ikLim = -1; }
    if (yaw > yl) yaw = yl; if (yaw < -yl) yaw = -yl;
    ikPitch = pitch;
    float qp[4], qy[4], t[4];
    qaxis(1, 0, 0, pitch, qp); qaxis(0, 1, 0, yaw, qy); qmul(qy, qp, t);
    for (int b = 4; b <= 14; b++) if (b == 4 || b >= 5) { float o[4]; qmul(t, &quatsX[4*b], o); memcpy(&quatsX[4*b], o, 16); }
}
static void gunX(float* bw, float* mw)   // the true barrel and muzzle in the world
{
    float m[16], mm[16], t[3], P[4];
    s_qmat(m, &quatsX[4 * gGunBone]); s_mmat(pl + 0x120, mm);
    float l[3]; vmul(gBl, m, l); vmul(l, mm, bw); float n = sqrtf(bw[0]*bw[0]+bw[1]*bw[1]+bw[2]*bw[2]); for (int i = 0; i < 3; i++) bw[i] /= n;
    vmul(gOl, m, l); vmul(l, mm, t);
    s_boneX(P, gGunBone, pl + 0x2B0, pl + 0x120); for (int i = 0; i < 3; i++) mw[i] = P[i] + t[i];
}
static float upErr(void)   // + = the barrel points above the crosshair spot (degrees)
{
    float b[3], M[3]; gunX(b, M);
    float d[3] = { g_tPt[0]-M[0], g_tPt[1]-M[1], g_tPt[2]-M[2] };
    return (atan2f(b[1], sqrtf(b[0]*b[0]+b[2]*b[2])) - atan2f(d[1], sqrtf(d[0]*d[0]+d[2]*d[2]))) * 57.29578f;
}
static float sideErr(void)   // sideways, + = right of the spot (screen right = -x at heading 0)
{
    float b[3], M[3]; gunX(b, M);
    float d[3] = { g_tPt[0]-M[0], g_tPt[1]-M[1], g_tPt[2]-M[2] };
    return (atan2f(-b[0], b[2]) - atan2f(-d[0], d[2])) * 57.29578f;
}
static void shoot(void)   // a shot: the game's bullet matrix is the gun's
{
    float b[3], M[3]; gunX(b, M);
    float m[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }, mm[16], tg[4];
    float r[3] = { b[2], 0, -b[0] }; float rn = sqrtf(r[0]*r[0]+r[2]*r[2]); r[0] /= rn; r[2] /= rn;
    float u[3] = { b[1]*r[2] - b[2]*r[1], b[2]*r[0] - b[0]*r[2], b[0]*r[1] - b[1]*r[0] };
    for (int i = 0; i < 3; i++) { m[i] = r[i]; m[4+i] = u[i]; m[8+i] = b[i]; m[12+i] = M[i]; }
    ShotPlan(m, (u32)wp[0x20], (void*)0x1000, mm, tg);
}
static float postZ = 1800.0f; static int postX, netX, grassX, wallOn;
static u32 hitF[8];
static int s_testXX(u32 type, u32 area, u32 flags, const float* a, const float* b, float r)
{
    (void)type; (void)area; (void)r; nh = 0;
    float gy = *(float*)(pl + 0x134);   // the ground at his feet
    if (a[1] != b[1]) { addhit(a, b, (gy - a[1]) / (b[1] - a[1])); if (nh) hitF[nh - 1] = 0x42; }
    if (postX && (flags & 0x42) && a[2] != b[2]) { int k = nh; addhit(a, b, (postZ - a[2]) / (b[2] - a[2])); if (nh > k) hitF[nh - 1] = 0x42; }
    // a fence net 300 behind Snake (the camera side): only tests that ask for 0x40000 find it
    if (netX && (flags & 0x40000) && a[2] != b[2]) { int k = nh; addhit(a, b, (-300.0f - a[2]) / (b[2] - a[2])); if (nh > k) hitF[nh - 1] = 0x40000; }
    // tall grass 500 behind him: flagged 0x800000 as well (the game's camera lets it through)
    if (grassX && (flags & 0x840000) && a[2] != b[2]) { int k = nh; addhit(a, b, (-500.0f - a[2]) / (b[2] - a[2])); if (nh > k) hitF[nh - 1] = 0x840000; }
    // a solid wall 600 behind him (bullets and camera both)
    if (wallOn && (flags & 0x42) && a[2] != b[2]) { int k = nh; addhit(a, b, (-600.0f - a[2]) / (b[2] - a[2])); if (nh > k) hitF[nh - 1] = 0x42; }
    return nh > 0;
}
static u32 s_flagX(int i) { return i >= 0 && i < 8 ? hitF[i] : 0; }
static void setupSkel(void)
{
    static const struct { int b, p; float x, y, z; } t[] = {
        { 1, 0, 0, 1000, 0 }, { 2, 1, 0, 400, 0 }, { 3, 2, 0, 250, 0 }, { 4, 3, 0, 100, 50 },
        { 5, 2, 150, 200, 0 }, { 6, 5, 100, 0, 0 }, { 7, 6, 0, -100, 280 }, { 8, 7, -200, 0, 240 }, { 9, 8, 0, 0, 60 },
        { 10, 2, -150, 200, 0 }, { 11, 10, -100, 0, 0 }, { 12, 11, 0, -100, 280 }, { 13, 12, 0, 0, 260 }, { 14, 13, 0, 0, 80 } };
    for (int b = 0; b < NB; b++) { par[b] = b ? 0 : -1; offX[b][0] = offX[b][1] = offX[b][2] = 0; }
    for (unsigned i = 0; i < sizeof t / sizeof t[0]; i++) { par[t[i].b] = t[i].p; offX[t[i].b][0] = t[i].x; offX[t[i].b][1] = t[i].y; offX[t[i].b][2] = t[i].z; }
    for (int b = 0; b < NB; b++) *(int*)(skelX + b * 0x50 + 8) = par[b];
    *(u8**)(modelXX + 0xB8) = skelX; *(u8**)(pl + 0x2B0) = modelXX;
    *(u8**)(mcT + 0x50) = (u8*)quatsX; *(int*)(mcT + 0x14) = NB; *(u8**)(pl + 0x2B8) = mcT;
    f_Bone = s_boneX; f_QMat = (QMat_t)(void*)s_qmat; f_MMat = (MMat_t)(void*)s_mmat; o_AimAdj = s_ik;
    *(i16*)(pl + 0x538) = 0x238E; *(i16*)(pl + 0x53A) = 0x0AAA; *(i16*)(pl + 0x53C) = (i16)0xC71D; *(i16*)(pl + 0x53E) = (i16)0xF556;
    poseBase();
}
static void frameG(void)   // a game frame: motion, the aim routine, the camera
{
    gT++; poseBase();
    if (!gStale) hk_AimAdj(pl, work);
    frame();
    if (gStale) hk_AimAdj(pl, work);   // (failure case: the turn is not there yet when the camera runs)
}
static void test_part11(void)
{
    printf("\n\n-- part 11 --\n");
    showLog = getenv("SHOWLOG") ? 1 : 0; cfgLog = 1;
    // ---------------- [G] the gun's vertical aim
    {   cfgAimMode = 0; cfgConverge = 3; cfgPoseTurn = 45; cfgSpine = 1; g_spineKey = -1; cfgHeadLock = 1;
        c_test = s_testXX; c_flag = s_flagX; postX = 1; netX = grassX = wallOn = 0;
        setbit(ST_FPV, 0); setbit(ST_TPS, 1); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); setbit(3, 0); setbit(6, 0); setbit(ST_WALL, 0);
        wp[0x20] = 5; *(float*)(pl + 0x130) = 0; *(float*)(pl + 0x134) = 1000; *(float*)(pl + 0x138) = 0; *(u16*)(pl + 0x162) = 0; *(u16*)(pl + 0x16A) = 0;
        *(u16*)(pl + 0x7E0) = 0; *(u32*)(pl + 0x7E8) = 0; *(u32*)(pl + 0x9C) = 0x4C4AF2; *(i16*)(pl + 0xA0) = 15;
        setupSkel(); g_shotT = 0; g_fireNow = 0;
        *(float*)(work + 0x340) = 0.6f; mouse = 0;
        setbit(ST_WEAPON, 1); for (int i = 0; i < 90; i++) frameG();
        printf("[G] post 1.8 m ahead, the arms' motion tilted up %.0f deg; before any shot: barrel %+.1f deg above the spot, %+.1f right (aim %d, correction %.1f, state %d)\n",
            gTilt, upErr(), sideErr(), (int)g_aimLatch, g_pfC[0] * 57.29578f, (int)g_pfState);
        float side0 = sideErr();
        // three shots at three heights of the view
        float looks[3] = { -0.004f, 0.006f, -0.003f };
        for (int s = 0; s < 3; s++) {
            mouse = looks[s]; for (int i = 0; i < 8; i++) frameG(); mouse = 0; for (int i = 0; i < 12; i++) frameG();
            shoot(); clockMs += 500; for (int i = 0; i < 2; i++) frameG();
            printf("[G] shot %d (view %+.0f deg): bone in use %d, state %d\n", s + 1, g_pitchDeg, g_pfIdx[5] ? kPfBone[g_pfIdx[5] - 1] : -1, (int)g_pfState);
        }
        int conv = -1; float e0 = upErr();
        for (int i = 0; i < 60; i++) { frameG(); if (conv < 0 && fabsf(upErr()) < 0.5f) conv = i; }
        printf("[G] after learning: barrel %+.1f deg above the spot (was %+.1f), within 0.5 deg after %d frames; sideways %+.1f (before %+.1f); correction %.1f deg, state %d\n",
            upErr(), e0, conv, sideErr(), side0, g_pfC[0] * 57.29578f, (int)g_pfState);
        // look up and down: it follows
        float worst = 0;
        for (int k = 0; k < 4; k++) { mouse = k & 1 ? -0.006f : 0.006f;
            for (int i = 0; i < 45; i++) { if (i == 20) mouse = 0; frameG();
                if (getenv("DBGG")) printf("  dbgG use=%d aimY=%.0f pfY=%.0f A520=%.0f,%.0f,%.0f dist=%d k=%d i=%d view=%.1f c=%.2f ev=%.2f up=%.2f ik=%.1f lim=%d atlim=%d state=%d T=%.0f,%.0f,%.0f ctr=%d\n", (int)g_pfUse, g_aimPt[1], g_pfAim[1], *(float*)(pl+0x520), *(float*)(pl+0x524), *(float*)(pl+0x528), (int)g_convDist, k, i, g_pitchDeg, g_pfC[0]*57.3f, g_pfEv10/10.0f, upErr(), ikPitch*57.3f, (int)ikLim, (int)g_pfAtLim, (int)g_pfState, g_tPt[0], g_tPt[1], g_tPt[2], (int)g_ctrDist); }
            if (fabsf(upErr()) > worst) worst = fabsf(upErr()); }
        printf("[G] looking up and down (4 times, then still for 25 frames): barrel off the spot vertically by at most %.1f deg when settled\n", worst);
        // the post gone (far spot)
        postX = 0; for (int i = 0; i < 60; i++) frameG();
        printf("[G] open view (spot %d units ahead): barrel %+.1f deg above it\n", (int)g_ctrDist, upErr());
        postX = 1; for (int i = 0; i < 60; i++) frameG();
        // firing: the correction is held still
        {   float c0 = g_pfC[0]; int moved = 0;
            for (int s = 0; s < 6; s++) { gTilt = 10.0f + (s & 1 ? 4.0f : -4.0f); shoot(); for (int i = 0; i < 5; i++) { frameG(); if (fabsf(g_pfC[0] - c0) > 0.0001f) moved++; } }
            gTilt = 10.0f;
            printf("[G] six shots with the gun kicking 4 deg up and down: the correction changed in %d of 30 frames (want 0)\n", moved);
            clockMs += 500; for (int i = 0; i < 40; i++) frameG(); }
        // looking down at the ground near him: the arms' limit
        {   postX = 0; mouse = 0.03f; for (int i = 0; i < 30; i++) frameG(); mouse = 0; for (int i = 0; i < 60; i++) frameG();
            float on = upErr(); int lim = (int)ikLim;
            int pd = cfgPoseDown; cfgPoseDown = 0; for (int i = 0; i < 60; i++) frameG();
            printf("[G] looking down %.0f deg at the ground %d units ahead: barrel %+.1f deg above the spot (arms at their limit %d); with the game's own 50 deg limit (PoseDown=0) %+.1f (at limit %d)\n",
                g_pitchDeg, (int)g_ctrDist, on, lim, upErr(), (int)ikLim);
            cfgPoseDown = pd; mouse = -0.03f; for (int i = 0; i < 20; i++) frameG(); mouse = 0; postX = 1; for (int i = 0; i < 60; i++) frameG(); }
        cfgPoseFix = 0; for (int i = 0; i < 60; i++) frameG();
        printf("[G] PoseFix=0: barrel %+.1f deg above the spot (reference), sideways %+.1f\n", upErr(), sideErr());
        cfgPoseFix = 1; for (int i = 0; i < 60; i++) frameG();
        printf("[G] PoseFix=1 again: %+.1f (view %.0f deg, spot %d ahead, state %d, error seen %.1f, correction %.1f, arms' angle %.0f limit %d)\n", upErr(), g_pitchDeg, (int)g_ctrDist, (int)g_pfState, g_pfEv10 / 10.0f, g_pfC[0] * 57.29578f, ikPitch * 57.3f, (int)ikLim);
        // crouched (the same bone, the other correction slot)
        setbit(ST_STAND, 0); setbit(ST_CROUCH, 1); *(float*)(pl + 0x134) = 1000;
        for (int i = 0; i < 90; i++) frameG();
        printf("[G] crouched: barrel %+.1f deg above the spot, correction %.1f\n", upErr(), g_pfC[1] * 57.29578f);
        setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); for (int i = 0; i < 60; i++) frameG();
        // the bone search: a gun held by bone 13 (another weapon)
        gGunBone = 13; wp[0x20] = 6;
        for (int s = 0; s < 3; s++) { mouse = looks[s]; for (int i = 0; i < 8; i++) frameG(); mouse = 0; for (int i = 0; i < 12; i++) frameG(); shoot(); clockMs += 500; frameG(); }
        for (int i = 0; i < 60; i++) frameG();
        printf("[G] weapon 6 held by bone 13: bone found %d, barrel %+.1f deg above the spot\n", g_pfIdx[6] ? kPfBone[g_pfIdx[6] - 1] : -1, upErr());
        // a shotgun: eight pellets in one frame are one sample
        {   wp[0x20] = 14; gGunBone = 14; u32 n0 = g_pfLearnN; float b[3], M[3]; gunX(b, M);
            float m[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }, mm[16], tg[4]; for (int i = 0; i < 3; i++) { m[8+i] = b[i]; m[12+i] = M[i]; }
            for (int k = 0; k < 8; k++) ShotPlan(m, 14, (void*)0x1000, mm, tg);
            printf("[G] shotgun: 8 pellets in one frame counted as %u sample(s) (want 1)\n", g_pfLearnN - n0); frameG(); }
        // failure case: the pose the camera sees is the one before the turn (the plugin cannot see its own effect)
        {   wp[0x20] = 8; gGunBone = 14; gStale = 1;
            for (int s = 0; s < 3; s++) { mouse = looks[s]; for (int i = 0; i < 8; i++) frameG(); mouse = 0; for (int i = 0; i < 12; i++) frameG(); shoot(); clockMs += 500; frameG(); }
            float mx = 0; for (int i = 0; i < 300; i++) { frameG(); if (fabsf(g_pfC[0]) > mx) mx = fabsf(g_pfC[0]); }
            printf("[G] the barrel never follows (failure case): the correction went up to %.1f deg, then: switched off %d, correction now %.1f, barrel %+.1f deg above the spot\n",
                mx * 57.29578f, (int)g_pfDead[8], g_pfC[0] * 57.29578f, upErr());
            gStale = 0; }
        wp[0x20] = 5; gGunBone = 14;
        setbit(ST_WEAPON, 0); for (int i = 0; i < 40; i++) frameG();
        printf("[G] counters: learned %u, frames corrected %u, average error then %.1f deg, shots on %u (%.1f deg) / off %u (%.1f deg)\n",
            g_pfLearnN, g_pfFrames, g_pfFrames ? g_pfEvSum / g_pfFrames : 0.0f, g_pfShotN[1], g_pfShotN[1] ? g_pfShotErr[1] / g_pfShotN[1] : 0.0f, g_pfShotN[0], g_pfShotN[0] ? g_pfShotErr[0] / g_pfShotN[0] : 0.0f);
        postX = 0;
    }
    // ---------------- [H] the camera and his head while firing
    {   setbit(ST_WEAPON, 0); for (int i = 0; i < 40; i++) frameG();
        *(float*)(work + 0x340) = 0.6667f; mouse = 0;   // the view level-ish again (the game's resting angle)
        setbit(ST_WEAPON, 1); for (int i = 0; i < 60; i++) frameG();
        float r0[3]; for (int i = 0; i < 3; i++) r0[i] = ((float*)(work + 0x3D0))[i] - g_head[i];
        for (int pass = 0; pass < 2; pass++) {
            int keep = cfgHeadHoldU; if (pass == 0) cfgHeadHoldU = 1000000;
            float worst = 0, kickWorst = 0;
            // firing every 8 frames while he crouches: his head drops 400 over 12 frames (the stance change)
            for (int i = 0; i < 70; i++) {
                if ((i & 7) == 0) { g_shotT = clockMs ? clockMs : 1; }
                float drop = i < 20 ? 0 : i < 32 ? (i - 20) * 400.0f / 12.0f : 400.0f;
                offX[1][1] = 1000.0f - drop;   // the hips (and all above) go down
                if (i == 20) { setbit(ST_STAND, 0); setbit(ST_CROUCH, 1); }
                frameG();
                if (getenv("DBGH") && (i % 4 == 0)) printf("  dbg i=%d eye=%.0f,%.0f,%.0f head=%.0f,%.0f,%.0f lock=%d use=%d hdU=%.0f amt=%.2f backK=%d sideK=%d hold=%u drag=%u\n", i, ((float*)(work+0x3D0))[0], ((float*)(work+0x3D0))[1], ((float*)(work+0x3D0))[2], g_head[0], g_head[1], g_head[2], (int)g_lock, (int)g_headUse100, g_hdU, g_amt, (int)g_backK100, (int)g_sideK100, g_shotHold, g_headDragN);
                float d[3]; for (int k = 0; k < 3; k++) d[k] = ((float*)(work + 0x3D0))[k] - g_head[k] - r0[k];
                float L = fabsf(d[1]); if (i >= 34 && L > worst) worst = L;   // up / down: the stance change
            }
            // a kick alone: the head 16 forward and 20 down for a few frames
            offX[1][1] = 1000.0f; setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); for (int i = 0; i < 60; i++) frameG(); clockMs += 1000; for (int i = 0; i < 30; i++) frameG();
            for (int i = 0; i < 3; i++) r0[i] = ((float*)(work + 0x3D0))[i] - g_head[i];
            float e0[3]; memcpy(e0, (float*)(work + 0x3D0), 12);
            g_shotT = clockMs;
            for (int i = 0; i < 20; i++) { float k = i < 6 ? (float)i / 6.0f : (i < 14 ? 1.0f - (i - 6) / 8.0f : 0); offX[4][1] = 100 - 20 * k; offX[4][2] = 50 + 16 * k; frameG();
                float* e = (float*)(work + 0x3D0); float m = sqrtf((e[0]-e0[0])*(e[0]-e0[0]) + (e[1]-e0[1])*(e[1]-e0[1]) + (e[2]-e0[2])*(e[2]-e0[2])); if (m > kickWorst) kickWorst = m; }
            offX[4][1] = 100; offX[4][2] = 50;
            printf("[H] %s: firing while crouching (head 400 down): camera off its place at his head by up to %.0f units once he is down; a kick alone (16 forward / 20 down) moved the camera %.1f units\n",
                pass == 0 ? "before (whole head held 0.6 s)" : "now (HeadHoldUnits 24)", worst, kickWorst);
            cfgHeadHoldU = keep; clockMs += 1000; for (int i = 0; i < 60; i++) frameG();
            for (int i = 0; i < 3; i++) r0[i] = ((float*)(work + 0x3D0))[i] - g_head[i];
        }
        printf("[H] counters: followed past the kick %u frames, largest %d units\n", g_headDragN, (int)g_dragMaxAll);
        setbit(ST_WEAPON, 0); for (int i = 0; i < 40; i++) frameG();
    }
    // ---------------- [S] strafing through shots (keyboard): the game reports its stalk value on a shot's frame
    {   static i32 tk = 5; p_tick = &tk; g_padConn = 0; simPad.conn = 0; g_aimLatch = 1; g_crouchLatch = 0; g_proneLatch = 0; g_have = 1; g_inYaw = g_camYaw16;
        for (int pass = 0; pass < 2; pass++) {
            int keep = cfgShotWalkMs; if (pass == 0) cfgShotWalkMs = 0;
            int flips = 0, was = g_fast; g_shotT = 0;
            for (int f = 0; f < 120; f++) {
                int shotNow = (f % 10) == 3;
                if (shotNow) g_shotT = clockMs ? clockMs : 1;
                g_inMag = ((f % 10) == 3 || (f % 10) == 4) ? 51 : 255;   // 255 -> 51 -> 255 at each shot
                clockMs += 16; g_frame++; g_legFrame = g_frame; Push(pl);
                if (g_fast != was) { flips++; was = g_fast; }
            }
            printf("[S] %s: walking at full speed, a shot every 10 frames: the walk switched %d times in 2 s (want 0)\n", pass ? "now" : "before (ShotWalkHoldMs=0)", flips);
            cfgShotWalkMs = keep;
        }
        // the stalk key still switches at once, right after a shot
        g_shotT = clockMs; keyDown[16] = 1; g_inMag = 255; clockMs += 16; g_frame++; Push(pl);
        printf("[S] Shift (stalk key) pressed right after a shot: band %d (want 2: stalk at once)\n", (int)g_band);
        keyDown[16] = 0; clockMs += 1000; g_inMag = 255; Push(pl); Push(pl);
        printf("[S] counters: walk kept through a shot %u frames\n", g_shotWalkN);
        g_have = 0; g_aimLatch = 0;
    }
    // ---------------- [C] crosshair
    {   setbit(ST_WEAPON, 1); wp[0x20] = 5; for (int i = 0; i < 60; i++) frameG();
        printf("[C] standing, Mk22: hidden %d (want 0)\n", (int)g_sightHide);
        setbit(ST_STAND, 0); setbit(3, 1); for (int i = 0; i < 30; i++) frameG();
        printf("[C] lying still: hidden %d (want 0)\n", (int)g_sightHide);
        int hid = 0, n = 0; for (int i = 0; i < 30; i++) { g_moveFrame = g_frame; frameG(); n++; if (g_sightHide == 1) hid++; }
        int after[4]; int idx = 0; for (int i = 0; i < 30; i++) { frameG(); if (i == 5 || i == 15 || i == 19 || i == 25) after[idx++] = (int)g_sightHide; }
        printf("[C] lying, crawling: hidden in %d of %d frames (want all); after he stops: 80 ms %d, 240 ms %d, 300 ms %d, 400 ms %d (want 1 1 1 0)\n", hid, n, after[0], after[1], after[2], after[3]);
        setbit(3, 0); setbit(ST_STAND, 1); for (int i = 0; i < 30; i++) frameG();
        g_moveFrame = g_frame; frameG(); printf("[C] standing, walking: hidden %d (want 0)\n", (int)g_sightHide);
        int ws[4] = { 7, 10, 11, 5 }, res[4];
        for (int k = 0; k < 4; k++) { wp[0x20] = (u8)ws[k]; for (int i = 0; i < 5; i++) frameG(); res[k] = (int)g_sightHide; }
        printf("[C] EZ gun %d, Scorpion %d, XM16E1 %d, Mk22 %d (want 2 2 0 0)\n", res[0], res[1], res[2], res[3]);
        setbit(ST_WEAPON, 0); for (int i = 0; i < 40; i++) frameG();
        printf("[C] shoulder view off: hidden %d (want 0); counters crawling %u / laser %u\n", (int)g_sightHide, g_sightCrawlN, g_sightLaserN);
        wp[0x20] = 5;
    }
    // ---------------- [N] camera collision: a fence net behind him (bullets pass, the game's camera stops)
    {   postX = 0; setbit(ST_WEAPON, 1); for (int i = 0; i < 60; i++) frameG();
        printf("[N] open: camera back %d%%\n", (int)g_backK100);
        const char* nm[4] = { "fence net", "tall grass (flag 800000)", "solid wall", "fence net, CollisionFlags=66" };
        for (int k = 0; k < 4; k++) {
            netX = k == 0 || k == 3; grassX = k == 1; wallOn = k == 2; if (k == 3) cfgCamFlags = 0x42;
            for (int i = 0; i < 60; i++) frameG();
            float pc = 0; int ctr = (int)g_ctrDist;
            {   float* e = (float*)(work + 0x3D0); pc = e[2]; }
            printf("[N] %s: camera back %d%% (eye at z %.0f; the net at -300, grass -500, wall -600), crosshair spot %d ahead (shots pass the net)\n", nm[k], (int)g_backK100, pc, ctr);
            netX = grassX = wallOn = 0; cfgCamFlags = 0; for (int i = 0; i < 90; i++) frameG();
        }
        printf("[N] counters: frames held in by a surface only the camera sees %u, hits let through %u\n", g_camOnlyN, g_camSkipN);
        setbit(ST_WEAPON, 0); for (int i = 0; i < 40; i++) frameG();
    }
    c_test = s_test; c_flag = s_flag; showLog = 0; cfgLog = 0;
}
