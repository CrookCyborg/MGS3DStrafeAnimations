static void fpvRun(int lbFirst, int handBack, int* holdOut, int* aimOut, int* amtOut)
{
    cfgFpvHandBack = handBack; cfgAimMode = 1; simLatch = 0; keyDown[2] = 0; wp[0x20] = 5;
    setbit(ST_WEAPON, 0); setbit(ST_TPS, 1); setbit(ST_FPV, 0); setbit(ST_WALL, 0); setbit(ST_STAND, 1); setbit(ST_CROUCH, 0); setbit(3, 0); setbit(6, 0);
    padOff(); clockMs += 2000; for (int i = 0; i < 80; i++) { padOff(); frame(); }
    for (int i = 0; i < 20; i++) { padSet(0, 0, 0x100, 0); frame(); }   // LB: shoulder view on (gun raised by the plugin)
    simLatch = 1;
    for (int i = 0; i < 5; i++) { padSet(0, 0, 0x300, 0); frame(); }   // RB too: the game goes to first person
    setbit(ST_TPS, 0); for (int i = 0; i < 15; i++) { padSet(0, 0, 0x300, 0); frame(); }
    setbit(ST_FPV, 1); for (int i = 0; i < 20; i++) { padSet(0, 0, 0x300, 0); frame(); }
    if (lbFirst) for (int i = 0; i < 20; i++) { padSet(0, 0, 0x200, 0); frame(); }   // LB let go in first person
    int btn = lbFirst ? 0 : 0x100;   // then RB let go (LB still held, or not)
    setbit(ST_FPV, 0); setbit(ST_TPS, 1);   // the game straight back in third person, gun still raised
    for (int i = 0; i < 40; i++) { padSet(0, 0, btn, 0); frame(); }
    *holdOut = g_hold; *aimOut = g_aimLatch; *amtOut = (int)(g_amt * 100.0f + 0.5f);
    for (int i = 0; i < 20; i++) { padOff(); frame(); } simLatch = 0; for (int i = 0; i < 60; i++) { padOff(); frame(); }
}
static void test_part15(void)
{
    printf("\n\n-- part 15 --\n");
    // ---- [H]
    printf("\n-- [H] LB let go in first person, then RB, gun still raised --\n");
    {   static u8 fpcX[0x100]; u8* ofc = *(u8**)(pl + 0x730); if (!ofc) *(u8**)(pl + 0x730) = fpcX;
        int h, a, m;
        fpvRun(1, 1, &h, &a, &m); printf("LB first, then RB:                          camera held at his head %d, shoulder view %d, camera on the shoulder %d%%   (want 0, 0, 0)\n", h, a, m);
        fpvRun(1, 0, &h, &a, &m); printf("LB first, then RB, PadFirstPersonHandBack=0:camera held at his head %d, shoulder view %d, camera on the shoulder %d%%   (1: the camera stuck at his head)\n", h, a, m);
        fpvRun(0, 1, &h, &a, &m); printf("RB first, LB still held:              camera held at his head %d, shoulder view %d, camera on the shoulder %d%%   (want 0, 1, 100: back to the shoulder)\n", h, a, m);
        cfgFpvHandBack = 1; cfgAimMode = 0; padOff(); simPad.conn = 0; g_padConn = 0;
    }
    // ---- [S]
    printf("\n-- [S] the pad read losing RB for a moment in first person --\n");
    {   Inp_t oInp = o_Inp; u8* oPl = g_pl;
        for (int k = 1; k >= 0; k--) {
            cfgR1Keep = k; g_pl = pl; o_Inp = s_inpXX; g_l1Keep = 0; setbit(ST_FPV, 1); *(u32*)(pl + 0x7E8) = 0;
            char seen[32]; int n = 0;
            padSet(0, 0, 0x200, 0); simBtn = 0x8800; for (int i = 0; i < 3; i++) { g_frame++; hk_Inp(pl, work, 0x80, 0); seen[n++] = (*(u32*)(pl + 0x7E8) & 0x800) ? 'R' : '-'; }
            simBtn = 0x8000; for (int i = 0; i < 2; i++) { g_frame++; hk_Inp(pl, work, 0x80, 0); seen[n++] = (*(u32*)(pl + 0x7E8) & 0x800) ? 'R' : '-'; }   // the read drops RB
            simBtn = 0x8800; for (int i = 0; i < 2; i++) { g_frame++; hk_Inp(pl, work, 0x80, 0); seen[n++] = (*(u32*)(pl + 0x7E8) & 0x800) ? 'R' : '-'; }
            seen[n++] = '|';
            padSet(0, 0, 0, 0); simBtn = 0x8000; for (int i = 0; i < 2; i++) { g_frame++; hk_Inp(pl, work, 0x80, 0); seen[n++] = (*(u32*)(pl + 0x7E8) & 0x800) ? 'R' : '-'; }   // RB really let go
            seen[n] = 0;
            printf("PadFirstPersonKeepsRB=%d: R1 as the game sees it [RB held x3, read drops it x2, back x2 | RB let go x2]: %s   (want %s)\n", k, seen, k ? "RRRRRRR|--" : "RRR--RR|--");
            setbit(ST_FPV, 0); simBtn = 0; g_frame++; hk_Inp(pl, work, 0x80, 0);
        }
        cfgR1Keep = 1; o_Inp = oInp; g_pl = oPl; *(u32*)(pl + 0x7E8) = 0; *(u32*)(pl + 0x7EC) = 0; *(u32*)(pl + 0x7F0) = 0; padOff(); simPad.conn = 0; g_padConn = 0;
    }
    // ---- [V]
    printf("\n-- [V] crouch walking speeds --\n");
    {   static u8 snake[0x1000], mw[0x100], mc[0x200], mt[0x100], arc[0x40], sq[0x10], act[1]; u8* o5 = g_cwSquatAct; u8* omc = g_cwMc;
        f_CwPlMot = s_plmot; f_CwStChk = s_stchk; f_CwStSet = s_stset; o_CwHold = s_hold; o_CwSeq = s_seq; o_CwSetMot = s_setmot; o_CwSquat = s_squat;
        g_cwSquatAct = act; g_cwInstalled = 1; static double wt = 1.0 / 60.0; g_cwWait = &wt;
        *(u64*)(mw + 0x58) = (u64)act; *(u8**)(mc + 8) = mt; *(u32*)(mt + 0x54) = 0x6891CC; *(u8**)(mt + 8) = arc; memcpy(arc, "Mtar", 4); *(u32*)(arc + 8) = 247; *(u8**)(mc + 0x78) = sq;
        hk_CwSetMot(mc, 0, 2, 0, 0);
        Inp_t oI = o_Inp; o_Inp = s_inpXX;   // (the pad copy hooked, as in the game)
        int st[3] = { 180, 51, 255 }, raw[3] = { 128, 51, 255 }; float sp[3];   // the game's stick / the pad as read (g_padRaw)
        for (int k = 0; k < 3; k++) { *(i16*)(snake + 0x7E0) = (i16)st[k]; g_padRaw = (u32)raw[k]; hk_CwSquat(snake, mw, 0, 0); hk_CwSquat(snake, mw, 0, 0); sp[k] = *(float*)(mt + 0x4C); }
        g_crouchLatch = 1; g_fastLeg = 0; g_band = 1; float f1 = LegFactor(); g_band = 2; float f2 = LegFactor(); g_fastLeg = 1; float f0 = LegFactor();
        printf("normal view: crouch walk (walk toggle) plays at %.1f (was 6.0), crouch stalk %.1f (want 3.0), full stick %.1f (want 6.0, unchanged)\n", sp[0], sp[1], sp[2]);
        printf("shoulder view: crouch walk %d%% (was 120), crouch stalk %d%% (want 60), full speed %d%% (want 100)\n", (int)(f1 * 100 + 0.5f), (int)(f2 * 100 + 0.5f), (int)(f0 * 100 + 0.5f));
        o_Inp = oI; g_crouchLatch = 0; g_fastLeg = 0; g_band = -1; g_cwInstalled = 0; g_cwSquatAct = o5; g_cwMc = omc; g_cwOn = 0; g_cwMoving = 0;
    }
}
