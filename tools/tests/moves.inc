static u64 s_inpX(u8* p, u8* w, u32 m, u64 a) { (void)w;(void)m;(void)a; *(u16*)(p + 0x7E0) = 255; *(u32*)(p + 0x7E8) = 0x10000; return 0; }
static int camoBuf[64 * 32]; static int* s_camoX(int* a, int b) { (void)a;(void)b; return 0; }
static void moveOnce(int mag, u32 dir)
{
    *(u16*)(pl + 0x7E0) = (u16)mag; *(u32*)(pl + 0x7E8) = dir; *(u16*)(pl + 0x7E2) = 0;
    g_have = 0; g_inMag = 0; g_hidOn = 0; g_frame++;
    OnMove(pl, work, 0x80, 0);
}
static void test_part10(void)
{
    printf("\n\n-- part 10 --\n");
    // ---- [L] the pad's shoulder-view button right after a load / continue
    {   cfgAimMode = 1; wp[0x20] = 5; setbit(ST_WEAPON, 0); setbit(ST_FPV, 0); setbit(ST_WALL, 0); setbit(3, 0); setbit(ST_CROUCH, 0); setbit(ST_STAND, 1);
        simLatch = 0; injClicks = 0; latchable = 1;
        for (int i = 0; i < 30; i++) { padSet(0, 0, 0, 0); frame(); }
        int startAt[2] = { 60, 360 };   // 1 s and 6 s into the scene
        for (int c = 0; c < 2; c++) {
            simLatch = 0; injClicks = 0;
            clockMs += 6000; setbit(ST_TPS, 0);   // loading: the game in neither first nor third person
            for (int i = 0; i < startAt[c]; i++) { padSet(0, 0, 0, 0); frame(); }
            int on = -1;
            for (int i = 0; i < 90; i++) {
                padSet(0, 0, 0x100, 0); frame();
                if (simLatch) setbit(ST_TPS, 1);   // the game: gun raised -> third person
                if (g_aimLatch && on < 0) on = i;
            }
            printf("[L] LB held %s into the scene, game in neither view: shoulder view %s (plugin aim-key presses %d, gun raised %d)\n",
                c ? "6 s" : "1 s", on >= 0 ? "on" : "NEVER on", injClicks, simLatch);
            if (on >= 0) printf("[L]   on after %d frames\n", on);
            for (int i = 0; i < 40; i++) { padSet(0, 0, 0, 0); frame(); }
            printf("[L]   LB let go: shoulder %d, gun raised %d\n", (int)g_aimLatch, simLatch);
        }
        // the normal case: third person already on - one press, straight on, no extra presses by the new code
        clockMs += 6000; setbit(ST_TPS, 1); simLatch = 0; injClicks = 0;
        for (int i = 0; i < 400; i++) { padSet(0, 0, 0, 0); frame(); }
        int on = -1; for (int i = 0; i < 30; i++) { padSet(0, 0, 0x100, 0); frame(); if (g_aimLatch && on < 0) on = i; }
        printf("[L] normal case (third person on): shoulder view after %d frames, plugin presses %d\n", on, injClicks);
        for (int i = 0; i < 40; i++) { padSet(0, 0, 0, 0); frame(); }
        // hanging / ladder (no stance): no press
        clockMs += 6000; setbit(ST_TPS, 0); setbit(ST_STAND, 0); simLatch = 0; injClicks = 0;
        for (int i = 0; i < 400; i++) { padSet(0, 0, 0, 0); frame(); }
        for (int i = 0; i < 30; i++) { padSet(0, 0, 0x100, 0); frame(); }
        printf("[L] no stance (ladder / hanging), LB held: plugin presses %d (want 0), shoulder %d\n", injClicks, (int)g_aimLatch);
        for (int i = 0; i < 40; i++) { padSet(0, 0, 0, 0); frame(); }
        setbit(ST_STAND, 1); setbit(ST_TPS, 1);
        // keyboard only, same load: the aim key works as before (no pad)
        simPad.conn = 0; g_padConn = 0; clockMs += 6000; setbit(ST_TPS, 0); simLatch = 0; injClicks = 0;
        for (int i = 0; i < 400; i++) frame();
        keyDown[2] = 1; simLatch = !simLatch; for (int q = 0; q < 4; q++) { frame(); if (simLatch) setbit(ST_TPS, 1); } keyDown[2] = 0;
        on = -1; for (int i = 0; i < 60; i++) { frame(); if (g_aimLatch && on < 0) on = i; }
        printf("[L] keyboard, aim key after a load (neither view): shoulder view after %d frames (-1 = never), plugin presses %d\n", on, injClicks);
        CLICK(); for (int i = 0; i < 30; i++) frame();
        cfgAimMode = 0; padOff();
    }
    // ---- [E] event scene (status 0x139): shoulder view off, no crosshair; the pad button does nothing there
    {   cfgAimMode = 1; wp[0x20] = 5; setbit(ST_TPS, 1); setbit(ST_STAND, 1); simLatch = 0; injClicks = 0;
        clockMs += 6000; for (int i = 0; i < 300; i++) { padSet(0, 0, 0, 0); frame(); }
        for (int i = 0; i < 30; i++) { padSet(0, 0, 0x100, 0); frame(); }
        printf("[E] LB held in play: shoulder %d\n", (int)g_aimLatch);
        setbit(0x139, 1); for (int i = 0; i < 30; i++) { padSet(0, 0, 0x100, 0); frame(); }
        printf("[E] event starts (0x139) with LB held: shoulder %d, crosshair %d, event kind %d\n", (int)g_aimLatch, (int)(g_amt > 0.01f), g_eventWhy);
        for (int i = 0; i < 20; i++) { padSet(0, 0, 0, 0); frame(); }
        for (int i = 0; i < 30; i++) { padSet(0, 0, 0x100, 0); frame(); }
        printf("[E] LB pressed again during the event: shoulder %d, plugin presses %d\n", (int)g_aimLatch, injClicks);
        setbit(0x139, 0); for (int i = 0; i < 30; i++) { padSet(0, 0, 0, 0); frame(); }
        for (int i = 0; i < 30; i++) { padSet(0, 0, 0x100, 0); frame(); }
        printf("[E] event over, LB pressed: shoulder %d\n", (int)g_aimLatch);
        for (int i = 0; i < 40; i++) { padSet(0, 0, 0, 0); frame(); }
        cfgAimMode = 0; padOff(); simPad.conn = 0; g_padConn = 0;
    }
    // ---- [M] the game's walk / run / stalk marks while walking in the shoulder view; [C] the camo index
    {   f_CwStSet = s_st4; g_aimLatch = 1; g_camValid = 1; g_wallFrames = 0; g_transLatch = 0; g_proneLatch = 0; g_crouchLatch = 0;
        setbit(ST_FPV, 0); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); g_camYaw16 = 0; g_legFrame = g_frame - 100; g_pl = pl;
        simPad.conn = 0; g_padConn = 0; g_stickT = 0;
        o_CwCamo = s_camoX; g_cwCamo = (u8*)camoBuf;
        struct { int mag, shift, crouch; const char* nm; } c[] = { { 255, 0, 0, "standing, keys (full)" }, { 128, 0, 0, "standing, walk toggle" }, { 255, 1, 0, "standing, Shift (stalk)" },
            { 255, 0, 1, "crouched, keys (full)" }, { 128, 0, 1, "crouched, walk toggle" }, { 255, 1, 1, "crouched, Shift (stalk)" } };
        for (int k = 0; k < 6; k++) {
            g_crouchLatch = c[k].crouch; setbit(ST_CROUCH, c[k].crouch); setbit(ST_STAND, !c[k].crouch); keyDown[16] = c[k].shift;
            for (int i = 0; i < 3; i++) { st4N = 0; st4[0] = st4[1] = st4[2] = 0; moveOnce(c[k].mag, 0x10000); }
            camoBuf[0x24 / 4] = 300; hk_CwCamo(0, 0); int cv = camoBuf[0x24 / 4];
            printf("[M] %-26s band %2d, speed x%.2f: marks %s (%X, %X, %X); camo index 30.0%% -> %d.%d%%\n", c[k].nm, (int)g_band, LegFactor(),
                st4N ? "set" : "NONE", st4[0], st4[1], st4[2], cv / 10, cv % 10);
        }
        keyDown[16] = 0;
        // the game's own stalk input (status 0x13B where the game reads the pad), standing
        g_crouchLatch = 0; setbit(ST_CROUCH, 0); setbit(ST_STAND, 1);
        o_Inp = s_inpX; setbit(0x13B, 1); hk_Inp(pl, work, 0x80, 0); setbit(0x13B, 0);
        st4N = 0; moveOnce(255, 0x10000);
        printf("[M] standing, the game's own stalk input (0x13B): band %d, marks %X (want D: stalk), the game's stalk input counted %u\n", (int)g_band, st4[0], g_gStalkN);
        for (int i = 0; i < 4; i++) g_frame++;
        st4N = 0; moveOnce(255, 0x10000);
        printf("[M]   a few frames later, keys only: band %d, marks %X (want C: run)\n", (int)g_band, st4[0]);
        // not moving: no marks, camo left alone
        st4N = 0; g_frame += 5; *(u16*)(pl + 0x7E0) = 0; *(u32*)(pl + 0x7E8) = 0; g_have = 0; g_frame++; OnMove(pl, work, 0x80, 0);
        camoBuf[0x24 / 4] = 300; hk_CwCamo(0, 0);
        printf("[M] standing still: marks %s, camo %d (want 300)\n", st4N ? "SET" : "none", camoBuf[0x24 / 4]);
        // lying: the crawl's own marks, not these
        g_proneLatch = 1; st4N = 0; moveOnce(255, 0x10000); printf("[M] lying: walk marks %s (the crawl sets its own)\n", g_markKind >= 0 && g_markFrame == g_frame ? "SET" : "none");
        g_proneLatch = 0; g_aimLatch = 0; f_CwStSet = 0; o_CwCamo = 0; g_cwCamo = 0; o_Inp = 0; g_have = 0;
        printf("[M] counters: run %u walk %u stalk %u crouched %u; camo %u\n", g_markN[0], g_markN[1], g_markN[2], g_markN[3], g_otsCamoN);
    }
    // ---- [K] the game's aim on another key (Q), W held while walking: the plugin learns Q, not W
    {   simPad.conn = 0; g_padConn = 0; cfgAimMode = 1; wp[0x20] = 5; setbit(ST_TPS, 1); setbit(ST_STAND, 1); setbit(ST_FPV, 0); setbit(3, 0);
        latchable = 0; simLatch = 0; injClicks = 0; clockMs += 6000; for (int i = 0; i < 300; i++) frame();
        g_alSeen = 0; g_alRises = 0; for (int i = 0; i < 256; i++) g_alScore[i] = 0;   // a new session: the right mouse button never raised the gun
        keyDown[0x57] = 1;   // W, held the whole time
        for (int r = 0; r < 4 && cfgAimKey == 2; r++) {
            for (int i = 0; i < 20; i++) frame();
            keyDown[0x51] = 1; frame(); simLatch = 1; for (int i = 0; i < 10; i++) frame();   // Q pressed: the game raises the gun
            keyDown[0x51] = 0; for (int i = 0; i < 10; i++) frame(); simLatch = 0; for (int i = 0; i < 20; i++) frame();
            printf("[K] raise %d with Q (W held): [Aim] Key now %02X\n", r + 1, cfgAimKey);
        }
        keyDown[0x57] = 0; latchable = 1;
        // now Q switches the shoulder view
        keyDown[0x51] = 1; simLatch = !simLatch; for (int i = 0; i < 4; i++) frame(); keyDown[0x51] = 0; for (int i = 0; i < 30; i++) frame();
        printf("[K] Q pressed after learning: shoulder %d, gun raised %d\n", (int)g_aimLatch, simLatch);
        keyDown[0x51] = 1; simLatch = !simLatch; for (int i = 0; i < 4; i++) frame(); keyDown[0x51] = 0; for (int i = 0; i < 30; i++) frame();
        printf("[K] Q again: shoulder %d, gun raised %d; learned %u time(s)\n", (int)g_aimLatch, simLatch, g_aimKeyLearnN);
        cfgAimKey = 2; cfgAimMode = 0;
    }
    // ---- [F] first person by itself while lying: what changed goes to the log
    {   showLog = 1; setbit(ST_STAND, 0); setbit(ST_CROUCH, 0); setbit(3, 1); setbit(ST_FPV, 0); setbit(ST_TPS, 1); setbit(ST_WEAPON, 0);
        for (int i = 0; i < 40; i++) frame();
        setbit(0x6A, 1); for (int i = 0; i < 3; i++) frame(); setbit(ST_TPS, 0); for (int i = 0; i < 10; i++) frame(); setbit(ST_FPV, 1); frame();
        setbit(ST_FPV, 0); setbit(0x6A, 0); setbit(ST_TPS, 1); setbit(3, 0); setbit(ST_STAND, 1); for (int i = 0; i < 20; i++) frame();
        showLog = 0;
        printf("[F] logged %u first-person and %u third-person-end cases\n", g_autoFpN, g_autoTpN);
    }
}
