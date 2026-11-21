// The gun is held level in the aiming motion, its muzzle 250 ahead of and 60 above the hand bone.
static float gBlX[3] = { 0, 0, 1 }, gOlX[3] = { 0, 60, 250 };
static void gunXX(float* bw, float* mw)   // the true barrel and muzzle (bone gGunBone, gBlX / gOlX)
{
    float m[16], mm[16], t[3], P[4], l[3];
    s_qmat(m, &quatsX[4 * gGunBone]); s_mmat(pl + 0x120, mm);
    vmul(gBlX, m, l); vmul(l, mm, bw); float n = sqrtf(bw[0]*bw[0]+bw[1]*bw[1]+bw[2]*bw[2]); for (int i = 0; i < 3; i++) bw[i] /= n;
    vmul(gOlX, m, l); vmul(l, mm, t);
    s_boneX(P, gGunBone, pl + 0x2B0, pl + 0x120); for (int i = 0; i < 3; i++) mw[i] = P[i] + t[i];
}
static float barrelPitch(void) { float b[3], M[3]; gunXX(b, M); return atan2f(b[1], sqrtf(b[0]*b[0]+b[2]*b[2])) * 57.29578f; }
static float linePitch(void) { float b[3], M[3]; gunXX(b, M); float d[3] = { g_tPt[0]-M[0], g_tPt[1]-M[1], g_tPt[2]-M[2] }; return atan2f(d[1], sqrtf(d[0]*d[0]+d[2]*d[2])) * 57.29578f; }
static float upErrX(void) { return barrelPitch() - linePitch(); }
static float shotM[16];
static void gunMat(float* m)
{
    float b[3], M[3]; gunXX(b, M);
    for (int i = 0; i < 16; i++) m[i] = (i % 5) == 0 ? 1.0f : 0.0f;
    float r[3] = { b[2], 0, -b[0] }; float rn = sqrtf(r[0]*r[0]+r[2]*r[2]); r[0] /= rn; r[2] /= rn;
    float u[3] = { b[1]*r[2] - b[2]*r[1], b[2]*r[0] - b[0]*r[2], b[0]*r[1] - b[1]*r[0] };
    for (int i = 0; i < 3; i++) { m[i] = r[i]; m[4+i] = u[i]; m[8+i] = b[i]; m[12+i] = M[i]; }
}
static void shootX(void) { float m[16], mm[16], tg[4]; gunMat(m); ShotPlan(m, (u32)wp[0x20], (void*)0x1000, mm, tg); }
static void shoot41late(void) { float mm[16], tg[4]; ShotPlan(shotM, (u32)wp[0x20], (void*)0x1000, mm, tg); }   // the matrix of the frame before
static float gSway = 0.0f;
static void poseBaseX(void)
{
    for (int b = 0; b < NB; b++) { quatsX[4*b] = quatsX[4*b+1] = quatsX[4*b+2] = 0; quatsX[4*b+3] = 1; }
    float q[4]; qaxis(1, 0, 0, -(gTilt + gSway * sinf((float)gT * 0.05f)) * 0.0174532925f, q);
    for (int b = 5; b <= 14; b++) memcpy(&quatsX[4*b], q, 16);
}
static void frameA(void) { gT++; poseBaseX(); hk_AimAdj(pl, work); frame(); }
// the view brought to a pitch (degrees, + = up) with the mouse, then held still
static void viewX(float deg, int n)
{
    for (int i = 0; i < 160; i++) { float e = deg - g_pitchDeg; if (fabsf(e) < 0.2f) break; float m = -e * 0.0006f; if (m > 0.01f) m = 0.01f; if (m < -0.01f) m = -0.01f; mouse = m; frameA(); }
    mouse = 0; for (int i = 0; i < n; i++) frameA();
}
// the view at five heights on the post; returns the gain (barrel turn / line turn) between the lowest and highest,
// prints a row
static float sweep(const char* what)
{
    static const float el[5] = { -20.0f, -10.0f, 0.0f, 10.0f, 20.0f };
    float bp[5], lp[5], er[5], vw[5]; int lim = 0;
    for (int k = 0; k < 5; k++) { viewX(el[k], 30); bp[k] = barrelPitch(); lp[k] = linePitch(); er[k] = bp[k] - lp[k]; vw[k] = g_pitchDeg; if (ikLim) lim++; }
    float gain = (bp[4] - bp[0]) / (lp[4] - lp[0]);
    printf("[A] %-44s view %+4.0f/%+4.0f/%+4.0f/%+4.0f/%+4.0f deg: barrel off the spot %+5.1f %+5.1f %+5.1f %+5.1f %+5.1f deg; gain %.2f (arms at a limit %d)\n",
        what, vw[0], vw[1], vw[2], vw[3], vw[4], er[0], er[1], er[2], er[3], er[4], gain, lim);
    return gain;
}
static void test_part12(void)
{
    printf("\n\n-- part 12 --\n");
    showLog = getenv("SHOWLOG") ? 1 : 0; cfgLog = 1;
    // ---------------- [A] the gun's vertical aim
    {   cfgAimMode = 0; cfgConverge = 3; cfgPoseTurn = 45; cfgSpine = 1; g_spineKey = -1; cfgHeadLock = 1; cfgPoseFix = 0;
        c_test = s_testXX; c_flag = s_flagX; postX = 1; netX = grassX = wallOn = 0;
        setbit(ST_FPV, 0); setbit(ST_TPS, 1); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); setbit(3, 0); setbit(6, 0); setbit(ST_WALL, 0);
        wp[0x20] = 5; *(float*)(pl + 0x130) = 0; *(float*)(pl + 0x134) = 1000; *(float*)(pl + 0x138) = 0; *(u16*)(pl + 0x162) = 0; *(u16*)(pl + 0x16A) = 0;
        *(u16*)(pl + 0x7E0) = 0; *(u32*)(pl + 0x7E8) = 0; *(u32*)(pl + 0x9C) = 0x4C4AF2; *(i16*)(pl + 0xA0) = 15;
        gTilt = 0.0f; gGunBone = 14; gStale = 0; setupSkel(); g_shotT = 0; g_fireNow = 0;
        memset(g_pa, 0, sizeof g_pa);
        *(float*)(work + 0x340) = 0.6f; mouse = 0;
        setbit(ST_WEAPON, 1); for (int i = 0; i < 90; i++) frameA();
        int keep = cfgPoseAim;
        cfgPoseAim = 0; float gX = sweep("before (PoseAim=0: the point = the spot)");
        cfgPoseAim = keep; float gA = sweep("now before any shot (hand + PoseAimBarrel)");
        // three shots at three heights
        static const float sh[3] = { -12.0f, 15.0f, 3.0f };
        for (int s = 0; s < 3; s++) { viewX(sh[s], 20); shootX(); clockMs += 500; frameA(); }
        printf("[A] after 3 shots: barrel %.0f above the hand line (%d shots), slant %.1f deg (%d), state %d\n", g_pa[5][0].c, g_pa[5][0].n, g_pa[5][0].b * 57.29578f, g_pa[5][0].bn, (int)g_paState);
        float gB = sweep("now after 3 shots");
        printf("[A] gain PoseAim=0 %.2f -> PoseAim=1 %.2f (before shots) / %.2f (after) (want 1.00)\n", gX, gA, gB);
        // what the arm check and the counters say
        printf("[A] check: arm turned as asked within %.2f deg (largest %.2f), line off the spot %.2f deg; frames %u, steady %u, limit %u\n", g_paTurnErr, g_paTurnMax, g_paLineErr, g_paFrames, g_paSteadyN, g_paLimN);
        postX = 0; viewX(0.0f, 40); float farOn = upErrX(); cfgPoseAim = 0; viewX(0.0f, 40); float farOff = upErrX(); cfgPoseAim = keep; postX = 1;
        printf("[A] open view (spot %d ahead): barrel %+.2f deg off it now, %+.2f before\n", (int)g_ctrDist, farOn, farOff);
        // a gun slanted 8 degrees up in its aiming motion (another weapon): slant and line learned
        wp[0x20] = 11; gTilt = 8.0f; viewX(0.0f, 30);
        float s0 = sweep("rifle slanted 8 up, before any shot");
        for (int s = 0; s < 4; s++) { viewX(sh[s % 3], 20); shootX(); clockMs += 500; frameA(); }
        float s1 = sweep("rifle slanted 8 up, after 4 shots");
        printf("[A] slanted rifle: barrel %.0f above the hand line, slant %.1f deg (want 8.0); gain %.2f -> %.2f\n", g_pa[11][0].c, g_pa[11][0].b * 57.29578f, s0, s1);
        gTilt = 0.0f;
        // the bullet matrix from the frame before (the game's weapon may run before the arms' turn)
        wp[0x20] = 6; viewX(0.0f, 30);
        for (int s = 0; s < 3; s++) { viewX(sh[s], 20); gunMat(shotM); frameA(); shoot41late(); clockMs += 500; frameA(); }
        sweep("pistol 6, bullets a frame late, 3 shots");
        printf("[A] bullets a frame late: barrel %.0f above the hand line (the frame's own: %.0f for weapon 5), slant %.1f deg\n", g_pa[6][0].c, g_pa[5][0].c, g_pa[6][0].b * 57.29578f);
        // crouched: its own line
        wp[0x20] = 5; setbit(ST_STAND, 0); setbit(ST_CROUCH, 1); offX[1][1] = 700.0f; viewX(0.0f, 30);
        sweep("crouched, before a crouched shot");
        for (int s = 0; s < 3; s++) { viewX(sh[s], 20); shootX(); clockMs += 500; frameA(); }
        sweep("crouched, after 3 crouched shots");
        printf("[A] crouched: barrel %.0f above the hand line (%d shots), standing %.0f (%d)\n", g_pa[5][1].c, g_pa[5][1].n, g_pa[5][0].c, g_pa[5][0].n);
        offX[1][1] = 1000.0f; setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); viewX(0.0f, 30);
        setbit(ST_STAND, 0); setbit(3, 1); viewX(0.0f, 40);
        float lyOn = upErrX(); int st1 = (int)g_paState; cfgPoseAim = 0; viewX(0.0f, 40); float lyOff = upErrX(); cfgPoseAim = keep;
        printf("[A] lying: state %d (want 0), barrel %+.2f deg off now, %+.2f before (want the same)\n", st1, lyOn, lyOff);
        setbit(3, 0); setbit(ST_STAND, 1); viewX(0.0f, 40);
        // a throwable and a weapon outside PoseAimWeapons: left alone
        wp[0x20] = 20; viewX(0.0f, 10); int stT = (int)g_paState; wp[0x20] = 5; viewX(0.0f, 10);
        printf("[A] grenade (type 20) in hand: state %d (want 0)\n", stT);
        // the shot lines say ON
        viewX(5.0f, 20); showLog = 1; shootX(); showLog = getenv("SHOWLOG") ? 1 : 0; frameA();
        setbit(ST_WEAPON, 0); for (int i = 0; i < 40; i++) frameA();
        printf("[A] counters: frames %u, learned %u, near shots on %u (%.1f deg) / off %u (%.1f deg), no bones %u\n",
            g_paFrames, g_paLearnN, g_paShotN[1], g_paShotN[1] ? g_paShotErr[1] / g_paShotN[1] : 0.0f, g_paShotN[0], g_paShotN[0] ? g_paShotErr[0] / g_paShotN[0] : 0.0f, g_paNoBone);
        postX = 0;
    }
    // ---------------- [W] strafe: short walk changes from the game at clicks of the fire button
    {   static i32 tk = 5; p_tick = &tk; g_aimLatch = 1; g_crouchLatch = 0; g_proneLatch = 0; g_inYaw = g_camYaw16; g_shotT = 0; g_fireNow = 0; g_fireEvT = 0;
        // pad: stick pushed all the way, the game's stalk input for one frame at each click (every 10 frames), no
        // bullet
        for (int pass = 0; pass < 2; pass++) {
            int keep = cfgWalkDebounce; if (pass == 0) cfgWalkDebounce = 0;
            int flips = 0, bands = 0; g_fast = 1; g_band = 0; int was = g_fast, wb = g_band;
            for (int f = 0; f < 120; f++) {
                padSet(0, 32000, 0, 0); clockMs += 16; g_frame++;
                if ((f % 10) == 3) g_gStalkT = g_frame;
                g_have = 1; g_inMag = 255; g_legFrame = g_frame; Push(pl); Push(pl);
                if (g_fast != was) { flips++; was = g_fast; } if (g_band != wb) { bands++; wb = g_band; }
            }
            printf("[W] %s: pad, the game's stalk input for a frame at each click (12 clicks, empty gun): walk switched %d times, band %d times (want 0)\n", pass ? "now" : "before (WalkDebounceFrames=0)", flips, bands);
            cfgWalkDebounce = keep;
        }
        // a real stalk (the game's stalk input held 40 frames): taken after the debounce
        {   int at = -1; for (int f = 0; f < 40; f++) { padSet(0, 32000, 0, 0); clockMs += 16; g_frame++; g_gStalkT = g_frame; g_have = 1; g_inMag = 255; Push(pl); if (at < 0 && g_band == 2) at = f; }
            printf("[W] the game's stalk input held: band 2 after %d frames (want %d)\n", at, cfgWalkDebounce);
            for (int f = 0; f < 10; f++) { padSet(0, 32000, 0, 0); clockMs += 16; g_frame++; g_have = 1; g_inMag = 255; Push(pl); }
            printf("[W] released: band %d (want 0)\n", (int)g_band); }
        // keyboard: a click of the left mouse button (3 frames held) with an empty gun - the game reports 51 for 2
        // frames
        g_padConn = 0; simPad.conn = 0; g_fgCache = 1;
        for (int pass = 0; pass < 2; pass++) {
            int keep = cfgWalkDebounce; if (pass == 0) cfgWalkDebounce = 0;
            int flips = 0; g_fast = 1; int was = g_fast; g_fireEvT = 0;
            for (int f = 0; f < 120; f++) {
                clockMs += 16; g_frame++;
                keyDown[1] = (f % 10) >= 3 && (f % 10) < 6;
                g_have = 1; g_inMag = ((f % 10) == 3 || (f % 10) == 4) ? 51 : 255; g_legFrame = g_frame; Push(pl);
                if (g_fast != was) { flips++; was = g_fast; }
            }
            keyDown[1] = 0;
            printf("[W] %s: keyboard, left mouse clicked 12 times with no bullet (the game reports 51 for 2 frames): walk switched %d times (want 0)\n", pass ? "now" : "before (WalkDebounceFrames=0)", flips);
            cfgWalkDebounce = keep;
        }
        // the walk toggle on the keyboard still switches at once (no click)
        {   clockMs += 1000; g_frame++; g_have = 1; g_inMag = 128; Push(pl); int b1 = g_band; clockMs += 16; g_frame++; g_inMag = 255; Push(pl);
            printf("[W] keyboard walk toggle (128) with no click: band %d at once (want 1), back to full: band %d (want 0)\n", b1, (int)g_band); }
        printf("[W] counters: too short to take %u frames, kept through a shot %u frames\n", g_wdbN, g_shotWalkN);
        g_have = 0; g_aimLatch = 0;
    }
    // ---------------- [F] fire presses: time to the shot
    {   postX = 1; c_test = s_testXX; c_flag = s_flagX; wp[0x20] = 5; setbit(ST_WEAPON, 1); for (int i = 0; i < 60; i++) frameA();
        u32 n0 = g_fpN; int tr0 = g_ftN;
        if (getenv("DBGF")) printf("  dbgF aim=%d fg=%d\n", (int)g_aimLatch, (int)g_fgCache);
        // standing: press, the bullet 1 frame later
        for (int k = 0; k < 3; k++) { keyDown[1] = 1; frameA(); shootX(); frameA(); keyDown[1] = 0; for (int i = 0; i < 20; i++) frameA(); }
        // strafing: pushed every frame, the bullet 4 frames later
        for (int k = 0; k < 3; k++) {
            for (int i = 0; i < 5; i++) { g_have = 1; g_moveFrame = g_frame; frameA(); }
            keyDown[1] = 1; for (int i = 0; i < 4; i++) { g_have = 1; g_moveFrame = g_frame; frameA(); }
            shootX(); keyDown[1] = 0; for (int i = 0; i < 20; i++) { g_have = 1; g_moveFrame = g_frame; frameA(); }
        }
        // a press with no bullet (empty)
        keyDown[1] = 1; frameA(); keyDown[1] = 0; for (int i = 0; i < 80; i++) frameA();
        printf("[F] presses %u: standing %u (%u ms / %u frames on average), strafing %u (%u ms / %u frames), no shot %u; frame traces %d\n",
            g_fpN - n0, g_fpLatN[0], g_fpLatN[0] ? g_fpLat[0] / g_fpLatN[0] : 0, g_fpLatN[0] ? g_fpLatFr[0] / g_fpLatN[0] : 0,
            g_fpLatN[1], g_fpLatN[1] ? g_fpLat[1] / g_fpLatN[1] : 0, g_fpLatN[1] ? g_fpLatFr[1] / g_fpLatN[1] : 0, g_fpNoShot[0] + g_fpNoShot[1], g_ftN - tr0);
        setbit(ST_WEAPON, 0); for (int i = 0; i < 40; i++) frameA();
        postX = 0;
    }
    c_test = s_test; c_flag = s_flag; showLog = 0; cfgLog = 0;
}
