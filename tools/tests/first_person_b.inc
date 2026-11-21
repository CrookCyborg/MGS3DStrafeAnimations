static void test_part14(void)
{
    printf("\n\n-- part 14 --\n");
    Inp_t oInp = o_Inp; u8* oPl = g_pl;
    // ---- [R]
    printf("\n-- [R] first person ends after LB was let go, RB still held --\n");
    struct { int restore, letGo, rbHeld; const char* nm; } rc[] = {
        { 1, 1, 1, "LB let go, then first person ends with RB held" }, { 1, 1, 0, "LB let go, then RB let go (first person ends normally)" },
        { 1, 0, 1, "first person ends with LB and RB both held" }, { 0, 1, 1, "as the first, PadFirstPersonRestore=0" } };
    for (int c = 0; c < 4; c++) {
        cfgFpvRestore = rc[c].restore; cfgFpvKeepL1 = 1; g_l1Keep = 0; g_letGoF = 0; g_pl = pl; o_Inp = s_inpXX; setbit(ST_FPV, 0); *(u32*)(pl + 0x7E8) = 0;
        u32 r0 = g_fpReN, ok0 = g_fpReOkN;
        padSet(0, 0, 0x300, 0); simBtn = 0x8C00; setbit(ST_FPV, 1);
        for (int i = 0; i < 3; i++) { g_frame++; hk_Inp(pl, work, 0x80, 0); }
        if (rc[c].letGo) { padSet(0, 0, 0x200, 0); simBtn = 0x8800; for (int i = 0; i < 3; i++) { g_frame++; hk_Inp(pl, work, 0x80, 0); } }
        if (!rc[c].rbHeld) { padSet(0, 0, 0, 0); simBtn = 0x8000; }
        setbit(ST_FPV, 0);   // first person ends
        char seen[32]; int n = 0;
        for (int i = 0; i < 6; i++) {
            g_frame++; hk_Inp(pl, work, 0x80, 0);
            u32 h = *(u32*)(pl + 0x7E8), pr = *(u32*)(pl + 0x7EC);
            seen[n++] = (pr & 0x800) ? 'P' : (h & 0x800) ? 'R' : '-';
            if (i == 3 && g_fpReN != r0) setbit(ST_FPV, 1);   // the game starts first person again on the press
        }
        seen[n] = 0;
        printf("%-58s: RB as the game sees it after the end %s; pressed again %u, first person back %u\n", rc[c].nm, seen, g_fpReN - r0, g_fpReOkN - ok0);
        setbit(ST_FPV, 0); padSet(0, 0, 0, 0); simBtn = 0; for (int i = 0; i < 3; i++) { g_frame++; hk_Inp(pl, work, 0x80, 0); }
    }
    printf("   want --PRRR 1 1 / ------ 0 0 / RRRRRR 0 0 / RRRRRR 0 0\n");
    o_Inp = oInp; g_pl = oPl; cfgFpvRestore = 1; *(u32*)(pl + 0x7E8) = 0; *(u32*)(pl + 0x7EC) = 0; *(u32*)(pl + 0x7F0) = 0; g_padConn = 0; simPad.conn = 0;

    // ---- [K]
    printf("\n-- [K] the classic camera by the game's own mark --\n");
    {   static u32 fakeStyle = 0, fakeFlags = 0x1000000; g_camStyle = &fakeStyle; g_camFlags = &fakeFlags;
        cfgClassicBlock = 1; cfgAimMode = 1; simLatch = 0; keyDown[2] = 0; wp[0x20] = 5;
        setbit(ST_WEAPON, 0); setbit(ST_TPS, 1); setbit(ST_FPV, 0); setbit(ST_WALL, 0); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); setbit(3, 0); setbit(6, 0);
        *(u16*)(work + 0x338) = 0x6000;   // (the camera number away from the view: no longer what counts)
        padOff(); clockMs += 2000; for (int i = 0; i < 80; i++) { padOff(); frame(); }
        int r[6];
        for (int i = 0; i < 20; i++) { padSet(0, 0, 0x100, 0); frame(); } r[0] = g_aimLatch;   // 3D camera: LB works
        fakeStyle = 1; fakeFlags = 0; for (int i = 0; i < 5; i++) { padSet(0, 0, 0x100, 0); frame(); } r[1] = g_aimLatch;   // R3 while in the shoulder view
        for (int i = 0; i < 40; i++) { padOff(); frame(); }
        u32 b0 = g_classicBlockN;
        for (int i = 0; i < 20; i++) { padSet(0, 0, 0x100, 0); frame(); } r[2] = g_aimLatch; r[3] = (int)(g_classicBlockN - b0);
        for (int i = 0; i < 40; i++) { padOff(); frame(); }
        fakeStyle = 0; fakeFlags = 0x1000000; for (int i = 0; i < 3; i++) { padOff(); frame(); }
        for (int i = 0; i < 20; i++) { padSet(0, 0, 0x100, 0); frame(); } r[4] = g_aimLatch;
        for (int i = 0; i < 60; i++) { padOff(); frame(); }
        cfgClassicBlock = 0; fakeStyle = 1; fakeFlags = 0; for (int i = 0; i < 3; i++) { padOff(); frame(); }
        for (int i = 0; i < 20; i++) { padSet(0, 0, 0x100, 0); frame(); } r[5] = g_aimLatch;
        for (int i = 0; i < 60; i++) { padOff(); frame(); }
        printf("3D camera: LB -> shoulder view %d; R3 to classic while in it: shoulder view %d; classic: LB -> %d (refused %d); 3D again: LB -> %d; classic with NoShoulderInClassicCamera=0: LB -> %d\n",
            r[0], r[1], r[2], r[3], r[4], r[5]);
        printf("   want 1, 0, 0 (1), 1, 1\n");
        g_camStyle = 0; g_camFlags = 0; cfgClassicBlock = 0; g_classic = 0; *(u16*)(work + 0x338) = 0; cfgAimMode = 0; padOff(); simPad.conn = 0; g_padConn = 0;
    }

    cfgClassicBlock = 1; cfgFpvRestore = 1;
}
