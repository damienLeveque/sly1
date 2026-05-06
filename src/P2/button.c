#include <button.h>
#include <chkpnt.h>
#include <find.h>
#include <alo.h>

void PostAshLoad(SW *psw, ASH *pash, ALO *palo)
{
    int coid = pash->coid;
    OID *poid = pash->aoid;
    for(int i = 0; i < coid; i++)
    {
        ASEG *paseg = (ASEG *)PloFindSwObject(psw, 0x104, poid[i], (LO *)palo);
        if (!paseg) continue;
        FAddAshAseg(pash, paseg);
        SnipLo((LO *)paseg);
    }
}

int FFoundAshAseg(ASH* pash, ASEG* paseg)
{
    OID* aoid = pash->aoid;
    
    for (int i = pash->coid - 1; i >= 0; i--)
    {
        if (FMatchesLoName((LO*)paseg, aoid[i]))
        {
            return 1;
        }
    }
    
    return 0;
}

int FAddAshAseg(ASH* pash, ASEG* paseg)
{
    int cpaseg = pash->cpaseg;
    pash->apaseg[cpaseg] = paseg;
    cpaseg++;
    
    pash->cpaseg = cpaseg;
    
    return cpaseg < 16;
}

int FAddAshOid(ASH* pash, OID oid)
{
    int coid = pash->coid;
    pash->aoid[coid] = oid;
    coid++;
    pash->coid = coid;
    return coid < 16;
}

void InitBtn(BTN *pbtn)
{
    STRUCT_OFFSET(pbtn, 0x120, int) = IchkAllocChkmgr(&g_chkmgr); // pbtn->ichkPushed
    STRUCT_OFFSET(pbtn, 0x140, OID) = OID_Nil; // pbtn->oidDialog
}

INCLUDE_ASM("asm/nonmatchings/P2/button", LoadBtn__FP3BTNP3ALO);

INCLUDE_ASM("asm/nonmatchings/P2/button", PostBtnLoad__FP3BTN);

INCLUDE_ASM("asm/nonmatchings/P2/button", RestoreBtnFromCheckpointCallback__FP3BTN5MSGIDPv);

void SetBtnRsmg(BTN* pbtn, int irsmg, OID oid1, OID oid2, OID oid3)
{
    FAddRsmg(pbtn->arsmg, 8, &pbtn->crsmg, irsmg, oid1, oid2, oid3);
}

void SetBtnButtons(BTN* pbtn, BUTTONS buttons)
{
    BUTTONS oldButtons = pbtn->buttons;
    
    if (buttons != oldButtons)
    {
        int fTrigger = (oldButtons == BUTTONS_NoContact) && (buttons == BUTTONS_Contact);
        int fUntrigger = (oldButtons != BUTTONS_Nil) && (buttons == BUTTONS_NoContact);
        
        pbtn->buttons = buttons;
        pbtn->tButtons = g_clock.t;
        
        ResolveAlo(pbtn->paloOwner);
        
        if (fTrigger)
        {
            TriggerBtn(pbtn, 0, 0);
        }
        
        if (fUntrigger)
        {
            UntriggerBtn(pbtn, 0);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/P2/button", FAddRsmg__FP4RSMGiPii3OIDN24);

INCLUDE_ASM("asm/nonmatchings/P2/button", TriggerRsmg__FP2SWiP4RSMGP2LOi);

INCLUDE_ASM("asm/nonmatchings/P2/button", RunBtnAsegs__FP3BTN4IASHii);

void TriggerBtn(BTN *pbtn, int fSeekToEnd, int fChkTrigger)
{
    if (fChkTrigger)
        HandleLoSpliceEvent(pbtn->paloOwner, 0x16, 0, NULL);
    else
        HandleLoSpliceEvent(pbtn->paloOwner, 2, 0, NULL);

    if (!STRUCT_OFFSET(pbtn, 0x134, int) && !fChkTrigger) // pbtn->fSilent
    {
        float sStart = 3000.0f;
        float sFull = 300.0f;
        float uVolAtSource = 1.0f;
        StartSound(SFXID_Click1, NULL, pbtn->paloOwner, NULL, sStart, sFull, uVolAtSource, 0.0f,
                   0.0f, NULL, NULL);
    }

    RunBtnAsegs(pbtn, IASH_On, fSeekToEnd, fChkTrigger);

    if (!STRUCT_OFFSET(pbtn, 0x13C, int)) // pbtn->fManualReset
        SetBtnButtons(pbtn, BUTTONS_Pushed);

    if (STRUCT_OFFSET(pbtn, 0x11C, int))                           // pbtn->fCheckpointed
        SetChkmgrIchk(&g_chkmgr, STRUCT_OFFSET(pbtn, 0x120, int)); // pbtn->ichkPushed

    pbtn->paloOwner->pvtlo->pfnSendLoMessage(pbtn->paloOwner, MSGID_button_trigger, pbtn);
}

void UntriggerBtn(BTN *pbtn, int fSeekToEnd)
{
    HandleLoSpliceEvent(pbtn->paloOwner, 3, 0, NULL); // untrigger event
    RunBtnAsegs(pbtn, IASH_Off, fSeekToEnd, 0);
    pbtn->paloOwner->pvtlo->pfnSendLoMessage(pbtn->paloOwner, MSGID_button_untrigger, pbtn);
}

void InitButton(BUTTON *pbutton)
{
    InitSo(pbutton);
    InitBtn(&pbutton->btn); 
}

void LoadButtonFromBrx(BUTTON *pbutton, CBinaryInputStream *pbis)
{
    LoadSoFromBrx(pbutton, pbis);
    LoadBtn(&pbutton->btn, pbutton); 
}

INCLUDE_ASM("asm/nonmatchings/P2/button", InsertButtonPos__FP6BUTTONP3PNTP6VECTOR);

INCLUDE_ASM("asm/nonmatchings/P2/button", InsertButtonMat__FP6BUTTONP3XFMP7MATRIX3);

INCLUDE_ASM("asm/nonmatchings/P2/button", PostButtonLoad__FP6BUTTON);

void CloneButton(BUTTON* pbuttonNew, BUTTON* pbuttonSrc)
{
    int ichkPushed = pbuttonNew->btn.ichkPushed;
    
    CloneSo(pbuttonNew, pbuttonSrc);
    
    pbuttonNew->btn.ichkPushed = ichkPushed;
    pbuttonNew->btn.paloOwner = pbuttonNew;
}

void SetButtonButtons(BUTTON* pbutton, BUTTONS buttons)
{
    SetBtnButtons(&pbutton->btn, buttons);
}

INCLUDE_ASM("asm/nonmatchings/P2/button", FCheckButtonObject__FP6BUTTONP2SO);

INCLUDE_ASM("asm/nonmatchings/P2/button", IposFindButtonClosest__FP6BUTTONPf);

INCLUDE_ASM("asm/nonmatchings/P2/button", ImatFindButtonClosest__FP6BUTTONPf);

INCLUDE_ASM("asm/nonmatchings/P2/button", PresetButtonAccel__FP6BUTTONf);

INCLUDE_ASM("asm/nonmatchings/P2/button", UpdateButtonInternalXps__FP6BUTTON);

INCLUDE_ASM("asm/nonmatchings/P2/button", UpdateButton__FP6BUTTONf);

int FAbsorbButtonWkr(BUTTON* pbutton, WKR* pwkr)
{
    int fAbsorbed = FAbsorbSoWkr(pbutton, pwkr);

    if (fAbsorbed && 
        (pwkr->grfic & 0x4) == 0 && 
        pbutton->unk_550 == 3 && 
        pbutton->btn.buttons == BUTTONS_NoContact)
    {
        SetButtonButtons(pbutton, BUTTONS_Contact);
    }

    return fAbsorbed;
}

void InitVolbtn(VOLBTN* pvolbtn)
{
    InitSo(pvolbtn);
    
    InitBtn(&pvolbtn->btn);
    
    pvolbtn->grfVolbtn |= 0x80000000000ULL;
    
    SetSoConstraints(pvolbtn, (CT)3, 0, (CT)3, 0);
}

void LoadVolbtnFromBrx(VOLBTN* pvolbtn, CBinaryInputStream* pbis)
{
    LoadSoFromBrx(pvolbtn, pbis);
    LoadBtn(&pvolbtn->btn, pvolbtn);
    
    if (pvolbtn->unk_790 != 0)
    {
        pvolbtn->btn.fManualReset = 1;
    }
}

INCLUDE_ASM("asm/nonmatchings/P2/button", PostVolbtnLoad__FP6VOLBTN);

void CloneVolbtn(VOLBTN* pvolbtnNew, VOLBTN* pvolbtnSrc)
{
    int ichkPushed = pvolbtnNew->btn.ichkPushed;
    
    CloneSo(pvolbtnNew, pvolbtnSrc);
    
    pvolbtnNew->btn.ichkPushed = ichkPushed;
    pvolbtnNew->btn.paloOwner = pvolbtnNew;
}


void SetVolbtnButtons(VOLBTN* pvolbtn, BUTTONS buttons)
{
    SetBtnButtons(&pvolbtn->btn, buttons);
}

INCLUDE_ASM("asm/nonmatchings/P2/button", UpdateVolbtn__FP6VOLBTNf);

INCLUDE_ASM("asm/nonmatchings/P2/button", FGetVolbtnPushObjectsWithinList__FP6VOLBTNPv);

void AddBtnAseg(BTN* pbtn, ALO* palo, OID oid)
{
    FAddAshOid(pbtn->aash, oid);
}

void AddButtonAseg(BUTTON* pbutton, OID oid)
{
    AddBtnAseg(&pbutton->btn, pbutton, oid);
}

void AddVolbtnAseg(VOLBTN* pvolbtn, OID oid)
{
    AddBtnAseg(&pvolbtn->btn, pvolbtn, oid);
}

void AddBtnOffAseg(BTN* pbtn, ALO* palo, OID oid)
{
    FAddAshOid(&pbtn->aash[1], oid);
}

void AddButtonOffAseg(BUTTON* pbutton, OID oid)
{
    AddBtnOffAseg(&pbutton->btn, pbutton, oid);
}

void AddVolbtnOffAseg(VOLBTN* pvolbtn, OID oid)
{
    AddBtnOffAseg(&pvolbtn->btn, pvolbtn, oid);
}

void AddButtonPushObject(BUTTON* pbutton, OID oid)
{
    int c = pbutton->cPushObjects;
    pbutton->aoidPushObjects[c] = oid;
    pbutton->cPushObjects = c + 1;
}

void AddButtonNoPushObject(BUTTON* pbutton, OID oid)
{
    int c = pbutton->cNoPushObjects;
    pbutton->aoidNoPushObjects[c] = oid;
    pbutton->cNoPushObjects = c + 1;
};

void AddButtonPushClass(BUTTON* pbutton, CID cid)
{
    int c = pbutton->cPushClasses;
    pbutton->acidPushClasses[c] = cid;
    pbutton->cPushClasses = c + 1;
}

void AddButtonNoPushClass(BUTTON* pbutton, CID cid)
{    
    int c = pbutton->cNoPushClasses;
    pbutton->acidNoPushClasses[c] = cid;
    pbutton->cNoPushClasses = c + 1;
}

void AddVolbtnPushObject(VOLBTN *pvolbtn, OID oid)
{
    int coidPush = STRUCT_OFFSET(pvolbtn, 0x550, int);        // pvolbtn->coidPush
    STRUCT_OFFSET_INDEX(pvolbtn, 0x554, OID, coidPush) = oid; // pvolbtn->aoidPush[coidPush]
    STRUCT_OFFSET(pvolbtn, 0x550, int) = coidPush + 1;        // pvolbtn->coidPush
}

void SetButtonRsmg(BUTTON *pbutton, int fOnTrigger, OID oidRoot, OID oidSM, OID oidGoal)
{
    SetBtnRsmg(&pbutton->btn, fOnTrigger, oidRoot, oidSM, oidGoal); 
}

void SetVolbtnRsmg(VOLBTN *pvolbtn, int fOnTrigger, OID oidRoot, OID oidSM, OID oidGoal)
{
    SetBtnRsmg(&STRUCT_OFFSET(pvolbtn, 0x5c0, BTN), fOnTrigger, oidRoot, oidSM, oidGoal); // pvolbtn->btn
}
