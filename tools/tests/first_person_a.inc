static u32 simBtn; static int inpN;
static u64 s_inpXX(u8* p, u8* w, u32 m, u64 a)
{   // the game's pad copy: held, and "pressed this frame" from what it stored last time
    (void)w;(void)m;(void)a; inpN++;
    u32 prev = *(u32*)(p + 0x7E8);
    *(u32*)(p + 0x7E8) = simBtn; *(u32*)(p + 0x7EC) = simBtn & ~prev; *(u32*)(p + 0x7F0) = simBtn & ~prev; *(u16*)(p + 0x7E0) = 0;
    return 0;
}
static i64 s_stchkX(u32 n) { return Bit((int)n); }
static int st4N, st3N; static i64 st4a[5];
static i64 s_stset4(int a, i64 b, i64 c, i64 d) { st3N++; st4a[0] = a; st4a[1] = b; st4a[2] = c; st4a[3] = d; st4a[4] = -2; return 0; }
static i64 s_stset5(int a, i64 b, i64 c, i64 d, i64 e) { st4N++; st4a[0] = a; st4a[1] = b; st4a[2] = c; st4a[3] = d; st4a[4] = e; return 0; }
static void test_part13(void)
{
    printf("\n\n-- part 13 --\n");
    Inp_t oInp = o_Inp; u8* oPl = g_pl;
    // ---- [L]
    printf("\n-- [L] first person from the shoulder view (pad), LB let go before RB --\n");
    for (int keep = 1; keep >= 0; keep--) {
        cfgFpvKeepL1 = keep; g_l1Keep = 0; g_pl = pl; o_Inp = s_inpXX; setbit(ST_FPV, 0);
        *(u32*)(pl + 0x7E8) = 0;
        char seen[64]; int n = 0;
        // LB held, shoulder view; then RB too and first person comes on
        padSet(0, 0, 0x100, 0); simBtn = 0x8400; hk_Inp(pl, work, 0x80, 0); seen[n++] = (*(u32*)(pl + 0x7E8) & 0x400) ? 'L' : '-';
        padSet(0, 0, 0x300, 0); simBtn = 0x8C00; setbit(ST_FPV, 1);
        for (int i = 0; i < 3; i++) { hk_Inp(pl, work, 0x80, 0); seen[n++] = (*(u32*)(pl + 0x7E8) & 0x400) ? 'L' : '-'; }
        seen[n++] = '|';
        // LB let go, RB still held, first person stays on
        padSet(0, 0, 0x200, 0); simBtn = 0x8800;
        for (int i = 0; i < 5; i++) { hk_Inp(pl, work, 0x80, 0); seen[n++] = (*(u32*)(pl + 0x7E8) & 0x400) ? 'L' : '-'; }
        // LB pressed again in first person: no new press for the game while it is kept
        padSet(0, 0, 0x300, 0); simBtn = 0x8C00; hk_Inp(pl, work, 0x80, 0); int pressed = (*(u32*)(pl + 0x7F0) & 0x400) != 0; seen[n++] = (*(u32*)(pl + 0x7E8) & 0x400) ? 'L' : '-';
        padSet(0, 0, 0x200, 0); simBtn = 0x8800; hk_Inp(pl, work, 0x80, 0); seen[n++] = (*(u32*)(pl + 0x7E8) & 0x400) ? 'L' : '-';
        seen[n++] = '|';
        // RB let go: first person ends
        padSet(0, 0, 0, 0); simBtn = 0x8000; setbit(ST_FPV, 0);
        for (int i = 0; i < 3; i++) { hk_Inp(pl, work, 0x80, 0); seen[n++] = (*(u32*)(pl + 0x7E8) & 0x400) ? 'L' : '-'; }
        seen[n] = 0;
        printf("PadFirstPersonKeepsButton=%d: L1 as the game sees it  [LB, LB+RB x3 | LB up x5, LB again, up | RB up x3]: %s; LB pressed again counted as a new press: %d\n",
            keep, seen, pressed);
        printf("   want %s\n", keep ? "LLLL|LLLLLLL|---  (kept held in first person, handed back when it ends), new press 0" : "LLLL|-----L-|---  (before: L1 up in first person), new press 1");
    }
    {   // the order that always worked: RB let go first (first person ends with LB still held), then LB
        cfgFpvKeepL1 = 1; g_l1Keep = 0; setbit(ST_FPV, 0); *(u32*)(pl + 0x7E8) = 0;
        char seen[32]; int n = 0;
        padSet(0, 0, 0x300, 0); simBtn = 0x8C00; setbit(ST_FPV, 1); for (int i = 0; i < 3; i++) { hk_Inp(pl, work, 0x80, 0); seen[n++] = (*(u32*)(pl + 0x7E8) & 0x400) ? 'L' : '-'; }
        padSet(0, 0, 0x100, 0); simBtn = 0x8400; setbit(ST_FPV, 0); for (int i = 0; i < 3; i++) { hk_Inp(pl, work, 0x80, 0); seen[n++] = (*(u32*)(pl + 0x7E8) & 0x400) ? 'L' : '-'; }
        padSet(0, 0, 0, 0); simBtn = 0x8000; for (int i = 0; i < 2; i++) { hk_Inp(pl, work, 0x80, 0); seen[n++] = (*(u32*)(pl + 0x7E8) & 0x400) ? 'L' : '-'; }
        seen[n] = 0;
        printf("RB let go first, then LB: %s (want LLL LLL -- : unchanged)\n", seen);
        // with no pad (keyboard), nothing is kept
        g_l1Keep = 0; g_padConn = 0; simPad.conn = 0; setbit(ST_FPV, 1); simBtn = 0x8C00; hk_Inp(pl, work, 0x80, 0); simBtn = 0x8800; hk_Inp(pl, work, 0x80, 0);
        printf("no pad: L1 kept %d (want 0)\n", (*(u32*)(pl + 0x7E8) & 0x400) ? 1 : 0);
        setbit(ST_FPV, 0); simBtn = 0; hk_Inp(pl, work, 0x80, 0); g_l1Keep = 0;
    }
    o_Inp = oInp; g_pl = oPl; *(u32*)(pl + 0x7E8) = 0; *(u32*)(pl + 0x7EC) = 0; *(u32*)(pl + 0x7F0) = 0;

    // ---- [C]
    printf("\n-- [C] the classic (overhead) camera --\n");
    {   cfgClassicBlock = 1; cfgAimMode = 1; simLatch = 0; keyDown[2] = 0; wp[0x20] = 5; injClicks = 0;
        setbit(ST_WEAPON, 0); setbit(ST_TPS, 1); setbit(ST_FPV, 0); setbit(ST_WALL, 0); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); setbit(3, 0); setbit(6, 0);
        *(u16*)(work + 0x338) = 0; padOff(); clockMs += 2000; for (int i = 0; i < 80; i++) { padOff(); frame(); }
        struct { u16 num; int block; const char* nm; } cs[] = {
            { 0x6000, 1, "camera looking 135 degrees away from its camera number (the classic camera)" },
            { 0x0000, 1, "the normal camera again" },
            { 0x8000, 1, "camera number reversed (180 degrees, as the game has it at times)" },
            { 0x2000, 1, "45 degrees away, but only for 20 frames" },
            { 0x6000, 0, "the classic camera, NoShoulderInClassicCamera=0" } };
        for (int c = 0; c < 5; c++) {
            cfgClassicBlock = cs[c].block; u32 b0 = g_classicBlockN;
            *(u16*)(work + 0x338) = cs[c].num;
            int frames = c == 3 ? 20 : 60;
            for (int i = 0; i < frames; i++) { padOff(); frame(); }
            if (c == 3) *(u16*)(work + 0x338) = 0;
            int cl = g_classic;
            for (int i = 0; i < 20; i++) { padSet(0, 0, 0x100, 0); frame(); }
            int on = g_aimLatch;
            for (int i = 0; i < 60; i++) { padOff(); frame(); }
            printf("%-80s classic %d, LB -> shoulder view %d (refused %u)\n", cs[c].nm, cl, on, g_classicBlockN - b0);
        }
        printf("   want 1/0 (refused 1), 0/1, 0/1, 0/1, 0/1\n");
        // the aim key (keyboard, no pad): refused, and the game's own raise of the gun is not lowered by the plugin
        simPad.conn = 0; g_padConn = 0; cfgClassicBlock = 1; *(u16*)(work + 0x338) = 0x6000; for (int i = 0; i < 60; i++) frame();
        int cl = g_classic; injClicks = 0;
        keyDown[2] = 1; simLatch = 1; for (int q = 0; q < 4; q++) frame(); keyDown[2] = 0; for (int q = 0; q < 40; q++) frame();
        printf("keyboard, classic camera %d: aim key -> shoulder view %d, gun still raised by the game %d, plugin presses %d (want 1: 0, 1, 0)\n", cl, (int)g_aimLatch, simLatch, injClicks);
        keyDown[2] = 1; simLatch = 0; for (int q = 0; q < 4; q++) frame(); keyDown[2] = 0; for (int q = 0; q < 40; q++) frame();
        *(u16*)(work + 0x338) = 0; for (int i = 0; i < 60; i++) frame();
        printf("normal camera again: classic %d\n", (int)g_classic);
        cfgClassicBlock = 1; cfgAimMode = 0; padOff(); simPad.conn = 0; g_padConn = 0;
    }


    // ---- [P]
    printf("\n-- [P] lying in the shoulder view, crawling forward with the walk --\n");
    {   static i32 tk = 5; p_tick = &tk; CwStSet_t o1 = f_CwStSet; StSet5_t o2 = f_StSet5; CrawlSnd_t o3 = f_CrawlSnd;
        f_CwStSet = s_stset4; f_StSet5 = s_stset5; f_CrawlSnd = 0;
        playSlot(12, 0); *(float*)(lyT + 0x4C) = 5.0f; *(u32*)(lyT + 0x54) = 0x6891CC;
        *(u32*)(pl + 0x74) = 0x6891CC; *(i16*)(pl + 0x78) = 13;
        g_aimLatch = 1; g_proneLatch = 1; g_crouchLatch = 0; g_transLatch = 0; g_legDir = 0; g_legSlot = 12; g_msg = 0x80; g_have = 1; g_inMag = 128;
        struct { int band, pct; const char* nm; } cs[] = { { 0, 60, "full speed" }, { 1, 60, "walk" }, { 2, 60, "stalk" }, { 1, 100, "walk, CrawlWalkPercent=100" } };
        for (int c = 0; c < 4; c++) {
            g_band = cs[c].band; cfgCrawlWalkPct = cs[c].pct; g_legFrame = g_frame;
            LegRate(pl); float r = *(float*)(lyT + 0x4C);
            st4N = st3N = 0; CrawlSounds(pl);
            printf("forward crawl, %-28s: played at %.2f (5.00 = normal), marks %s (%d, %d, %d, %d)\n", cs[c].nm, r,
                st4N ? "with status 4 (slow crawl)" : st3N ? "without status 4" : "none", (int)st4a[0], (int)st4a[1], (int)st4a[2], (int)st4a[3]);
        }
        printf("   want 5.00 no 4, 3.00 with 4, 3.00 with 4, 5.00 no 4\n");
        cfgCrawlWalkPct = 60; g_band = 1; g_legDir = 1; g_legFrame = g_frame; LegRate(pl);
        printf("crawling backward with the walk: played at %.2f (want 5.00: forward only)\n", *(float*)(lyT + 0x4C));
        g_legDir = 0; LegRate(pl); g_aimLatch = 0; g_legFrame = g_frame - 10; LegRate(pl);
        printf("shoulder view off afterwards: rate put back to %.2f (want 5.00)\n", *(float*)(lyT + 0x4C));
        f_CwStSet = o1; f_StSet5 = o2; f_CrawlSnd = o3;
        g_proneLatch = 0; g_have = 0; g_inMag = 0; g_band = -1; g_legDir = -1; g_legSlot = -1; playSlot(0, 0);
    }
    cfgFpvKeepL1 = 1; cfgClassicBlock = 1; cfgCrawlWalkPct = 60;
}
