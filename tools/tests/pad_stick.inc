static i16 seenL1, seenL2; static int stubCalls;
static void aimStub(u8* p, u8* s) { (void)s; seenL1 = *(i16*)(p + 0x53A); seenL2 = *(i16*)(p + 0x53E); stubCalls++; }
static int boxOn8; static float b8x0, b8x1, b8z0, b8z1;
static int s_testX(u32 type, u32 area, u32 flags, const float* a, const float* b, float r)
{
    int h = s_test(type, area, flags, a, b, r);
    if (boxOn8 && a[2] != b[2]) {
        float t = (b8z0 - a[2]) / (b[2] - a[2]); float x = a[0] + (b[0] - a[0]) * t, y = a[1] + (b[1] - a[1]) * t;
        if (t > 0 && t < 1 && x > b8x0 && x < b8x1 && y > 0 && y < 4000) { addhit(a, b, t); h = 1; }
    }
    return h;
}
// the way the plugin pushes Snake (MoveMode 1: Snake+0x620 / +0x628), as a heading in degrees; -999 = no push
static float pushDeg(void)
{
    float vx = *(float*)(pl + 0x620), vz = *(float*)(pl + 0x628);
    if (vx * vx + vz * vz < 0.01f) return -999.0f;
    return atan2f(vx, vz) * 57.29578f;
}
static void test_part09(void)
{
    printf("\n\n-- part 09 --\n");
    // ---- [S] the pad stick in the shoulder view, pushed lightly
    {   g_aimLatch = 1; g_camValid = 1; g_wallFrames = 0; g_transLatch = 0; g_proneLatch = 0; g_crouchLatch = 0;
        setbit(ST_FPV, 0); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0);
        g_camYaw16 = 0; g_legFrame = g_frame - 100;   // no walk motion: the push alone shows the way
        struct { int lx, ly; int gameMag; u16 gameYaw; u32 bits; const char* nm; } c[] = {
            { -13000, 0, 0, 0xC000, 0x40000, "left 40%: game value 0, its heading stale (right)" },
            { -11265, 6500, 0, 0x0000, 0x50000, "left-forward 30 deg off left, 40%: game value 0, stale heading (forward)" },
            { -9800, 0, 0, 0, 0, "left 30%: inside the game's dead zone (no value, no bits)" },
            { -6500, 0, 0, 0, 0, "left 20%: under PadDeadZone 24" },
            { -23000, 0, 150, 0x4000, 0x40000, "left 70%: the game has a value and the right heading" },
            { 0, 30000, 255, 0x0000, 0x10000, "forward, full" } };
        for (int k = 0; k < 6; k++) {
            padSet(c[k].lx, c[k].ly, 0, 0); clockMs += 16;
            *(u16*)(pl + 0x7E0) = (u16)c[k].gameMag; *(u16*)(pl + 0x7E2) = c[k].gameYaw; *(u32*)(pl + 0x7E8) = c[k].bits;
            *(float*)(pl + 0x620) = 0; *(float*)(pl + 0x628) = 0;
            g_have = 0; g_inMag = 0; g_hidOn = 0; g_frame++;
            OnMove(pl, work, 0x80, 0);
            float want = atan2f(-(float)c[k].lx, (float)c[k].ly) * 57.29578f;
            float got = pushDeg();
            if (got == -999.0f) printf("[S] %-72s: does not walk\n", c[k].nm);
            else { float e = got - want; while (e > 180) e -= 360; while (e < -180) e += 360;
                   printf("[S] %-72s: walks %4.0f deg, stick %4.0f deg -> %4.0f deg off\n", c[k].nm, got, want, fabsf(e)); }
        }
        *(u32*)(pl + 0x7E8) = 0; *(u16*)(pl + 0x7E0) = 0; g_have = 0; padOff(); g_aimLatch = 0;
        printf("[S] counters: stale heading replaced %u, walked inside the game's dead zone %u\n", g_stickFixN, g_dzWalkN);
    }
    // ---- [P] the 3DS pose: what Snake's upper body aims at
    {   cfgAimMode = 0; simPad.conn = 0; g_padConn = 0; setbit(ST_FPV, 0); setbit(ST_TPS, 1); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); setbit(3, 0); setbit(ST_WALL, 0);
        tree = 0; backWall = 0; sideWall = 0; postWall = 0; flick = 0; c_test = s_testX; boxOn8 = 0;
        cfgConverge = 3;
        wp[0x20] = 5; *(float*)(pl + 0x130) = 0; *(float*)(pl + 0x134) = 1000; *(float*)(pl + 0x138) = 0; *(float*)(work + 0x340) = 0.6f;
        setbit(ST_WEAPON, 1); for (int i = 0; i < 80; i++) frame();
        int far0 = g_convDist;
        float dm = 2006.0f, cx = g_tEye[0] + g_tF[0] * (dm + 200.0f);
        b8x0 = cx - 150; b8x1 = cx + 150; b8z0 = g_tEye[2] + g_tF[2] * (dm + 200.0f); boxOn8 = 1;
        int fr[6]; int sample[6] = { 1, 3, 6, 10, 20, 40 }, si = 0;
        for (int i = 1; i <= 40; i++) { frame(); if (si < 6 && i == sample[si]) fr[si++] = g_convDist; }
        float vx = g_aimPt[0] - *(float*)(pl + 0x130), vz = g_aimPt[2] - *(float*)(pl + 0x138);
        float turn = atan2f(vx, vz) * 57.29578f - (float)*(u16*)(pl + 0x162) * 360.0f / 65536.0f;
        while (turn > 180) turn -= 360; while (turn < -180) turn += 360;
        printf("[P] Converge=%d - open view: upper body aims %d units ahead; post in front of the camera: %d after 1 frame, %d / %d / %d / %d / %d after 3 / 6 / 10 / 20 / 40 (the post is %d away); turn toward it %.0f deg\n",
            cfgConverge, far0, fr[0], fr[1], fr[2], fr[3], fr[4], fr[5], (int)g_ctrDist + 2006 - 1300, fabsf(turn));
        // the game's aim routine with its own limits (15 degrees each way)
        o_AimAdj = aimStub; *(i16*)(pl + 0x53A) = 0x0AAA; *(i16*)(pl + 0x53E) = (i16)0xF556; stubCalls = 0; g_pl = pl;
        hk_AimAdj(pl, work);
        printf("[P] the game's aim routine was given sideways limits %d / %d degrees (game's own 15 / -15); afterwards Snake's limits are %d / %d (want 15 / -15)\n",
            (seenL1 * 360 + (seenL1 < 0 ? -32768 : 32768)) / 65536, (seenL2 * 360 + (seenL2 < 0 ? -32768 : 32768)) / 65536, (*(i16*)(pl + 0x53A) * 360 + 32768) / 65536, (*(i16*)(pl + 0x53E) * 360 - 32768) / 65536);
        boxOn8 = 0; for (int i = 0; i < 40; i++) frame();
        printf("[P] post gone: aims %d units ahead again\n", g_convDist);
        // a shot at the post: still from the middle of the screen
        boxOn8 = 1; for (int i = 0; i < 40; i++) frame();
        float m[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }, mm[16], tg[4];
        for (int i = 0; i < 3; i++) { m[8 + i] = g_tF[i]; m[12 + i] = g_tQ[i] + g_tF[i] * 700.0f; }
        int how = ShotPlan(m, 11, (void*)0x1000, mm, tg);
        printf("[P] a shot at the post: how=%d (1 = from the middle of the screen, as before)\n", how);
        boxOn8 = 0; c_test = s_test; setbit(ST_WEAPON, 0); for (int i = 0; i < 40; i++) frame();
        printf("[P] counters: twisted past the game's 15 degrees %u frames, needed up to %d degrees\n", g_poseWideN, g_poseYawMax);
    }
}
