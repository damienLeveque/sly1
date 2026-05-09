#include <emitter.h>
#include <sce/memset.h>
#include <memory.h>



INCLUDE_ASM("asm/nonmatchings/P2/emitter", InitEmitb__FP5EMITB);

void InitEmitter(EMITTER* pemitter)
{
    InitAlo(pemitter);

    pemitter->cEmitRemain = -1;   
    
    pemitter->gEmissionLife = 10.0f;   
    pemitter->gEmissionRate = 10.0f;   
    pemitter->tUnpause = -1.0f;        

    pemitter->aoidEmit[0] = OID_Nil;   
    pemitter->aoidEmit[1] = OID_Nil;   
    pemitter->aoidEmit[3] = OID_Nil;   

    pemitter->aoidEmit[2] = OID_Nil;   
    pemitter->aoidTarget[0] = OID_Nil; 
    pemitter->nMode = -1;   
    pemitter->aoidTarget[1] = OID_Nil; 

    InitDl(&pemitter->dlParticles, 0x320);
}

INCLUDE_ASM("asm/nonmatchings/P2/emitter", LoadEmitmeshFromBrx__FP8EMITMESHP18CBinaryInputStream);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", LoadEmitblipColorsFromBrx__FP8EMITBLIPiP2LOP18CBinaryInputStream);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", LoadEmitterFromBrx__FP7EMITTERP18CBinaryInputStream);

void CloneEmitter(EMITTER *pemitter, EMITTER *pemitterBase)
{
    CloneAlo(pemitter, pemitterBase);
    STRUCT_OFFSET(pemitter, 0x2d0, EMITB *)->cRef++;
}

void BindEmitterCallback(EMITTER* pemitter, MSGID msgid, void* pv)
{
    pemitter->pvtlo->pfnSetLoParent(pemitter, pemitter->paseg->ploParent);
}

INCLUDE_ASM("asm/nonmatchings/P2/emitter", BindEmitter__FP7EMITTER);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", PostEmitterLoad__FP7EMITTER);

void HandleEmitterMessage(EMITTER* pemitter, MSGID msgid, void* pv)
{
    if (msgid == 2)
    {
        LO* plo = (LO*)pv;

        if (plo == pemitter->ploUnk344)
        {
            plo->pvtlo->pfnUnsubscribeLoObject(plo, pemitter);
            pemitter->ploUnk344 = 0;
        }
        else if (plo == pemitter->ploUnk348)
        {
            plo->pvtlo->pfnUnsubscribeLoObject(plo, pemitter);
            pemitter->ploUnk348 = 0;
        }
    }
}

EMITB* PemitbCopyOnWrite(EMITB* pemitb)
{
    if (pemitb->cRef < 2)
    {
        return pemitb;
    }

    EMITB* pemitbNew = (EMITB*)PvAllocSwCopyImpl(0x200, pemitb);

    if (pemitb->emito.emitk == 3)
    {
        if (pemitb->emito.pElements != 0)
        {
            pemitbNew->emito.pElements = PvAllocSwCopyImpl(
                pemitb->emito.cElements * 0x28, 
                pemitb->emito.pElements
            );
        }
    }

    pemitbNew->cRef = 1;
    
    pemitb->cRef--;

    return pemitbNew;
}


EMITB *PemitbEnsureEmitter(EMITTER *pemitter, ENSK ensk)
{
    if (ensk == ENSK_Set)
    {
        STRUCT_OFFSET(pemitter, 0x34c, int) = 1; // pemitter->fValuesChanged
        STRUCT_OFFSET(pemitter, 0x2d0, EMITB *) = PemitbCopyOnWrite(STRUCT_OFFSET(pemitter, 0x2d0, EMITB *));
    }

    return STRUCT_OFFSET(pemitter, 0x2d0, EMITB *); // pemitter->pemitb
}

EMITTER *PemitterEnsureEmitter(EMITTER *pemitter, ENSK ensk)
{
    if (ensk == ENSK_Set)
    {
        STRUCT_OFFSET(pemitter, 0x34c, int) = 1; // pemitter->fValuesChanged
    }

    return pemitter;
}

void AddEmitterSkeleton(EMITTER *pemitter, OID oid, OID oidOther, float sRadius, float gDensity, float sRadiusOther, float gDensityOther)
{
    EMITB *pemitb = PemitbEnsureEmitter(pemitter, ENSK_Set);
    AddEmitoSkeleton(&pemitb->emito, oid, oidOther, sRadius, gDensity, sRadiusOther, gDensityOther, pemitter);
}

INCLUDE_ASM("asm/nonmatchings/P2/emitter", ModifyEmitterParticles__FP7EMITTER);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", UpdateEmitter__FP7EMITTERf);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", FUN_00155f28);

void PauseEmitter(EMITTER *pemitter, float dtPause)
{
    STRUCT_OFFSET(pemitter, 0x340, float) = g_clock.t + dtPause; // pemitter->tUnpause
}

void PauseEmitterIndefinite(EMITTER *pemitter)
{
    STRUCT_OFFSET(pemitter, 0x340, float) = DAT_0024a124; // pemitter->tUnpause
}

void UnpauseEmitter(EMITTER *pemitter)
{
    STRUCT_OFFSET(pemitter, 0x340, float) = -1.0f; // pemitter->tUnpause
}

int FPausedEmitter(EMITTER* pemitter)
{
    if (g_clock.t < pemitter->tUnpause)
    {
        return 1;
    }

    if (pemitter->nMode < 2)
    {
        if (pemitter->nMode >= 0)
        {
            if (pemitter->cEmitRemain == 0)
            {
                return 1;
            }
        }
    }

    return 0;
}

void SetEmitterEnabled(EMITTER *pemitter, int fEnabled)
{
    if (fEnabled)
    {
        UnpauseEmitter(pemitter);
        return;
    }

    PauseEmitterIndefinite(pemitter);
}

void GetEmitterEnabled(EMITTER *pemitter, int *pfEnabled)
{
    *pfEnabled = !FPausedEmitter(pemitter);
}

void GetEmitterPaused(EMITTER *pemitter, int *pfPaused)
{
    *pfPaused = FPausedEmitter(pemitter);
}

INCLUDE_ASM("asm/nonmatchings/P2/emitter", OnEmitterValuesChanged__FP7EMITTER);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", SetEmitterParticleCount__FP7EMITTERi);

void SetEmitterAutoPause(EMITTER *pemitter, int fAutoPause)
{
    STRUCT_OFFSET(pemitter, 0x308, int) = fAutoPause; // pemitter->fAutoPause
    STRUCT_OFFSET(pemitter, 0x34c, int) = 1; // pemitter->fValuesChanged
}

INCLUDE_ASM("asm/nonmatchings/P2/emitter", SetEmitbRipt__FP5EMITB4RIPT);

void SetEmitterRipt(EMITTER *pemitter, RIPT ript)
{
    EMITB *pemitb = PemitbEnsureEmitter(pemitter, ENSK_Set);
    SetEmitbRipt(pemitb, ript);
}

void SetExploRipt(EXPLO *pexplo, RIPT ript)
{
    EMITB *pemitb = PemitbEnsureExplo(pexplo, ENSK_Set);
    SetEmitbRipt(pemitb, ript);
}

INCLUDE_ASM("asm/nonmatchings/P2/emitter", WakeSoWater__FP2SOP5WATERfP6VECTORT3ff);

void StandardSmokeCloud(VECTOR* pvecPos, float gPower)
{
    EXPL* pexpl = g_psw->pexplStandardSmoke;
    
    if (pexpl != 0)
    {
        EXPLSO explso;

        explso.sRadius = 0.0f;        
        explso.vecForce = *(qword*)pvecPos;
        explso.grfExplode = 12; 
        explso.gPower = gPower;

        typedef void (*PfnGetLoParams)(EXPL*, EXPLSO*);
        ((PfnGetLoParams)pexpl->pvtlo->pfnGetLoParams)(pexpl, &explso);
    }
}

INCLUDE_ASM("asm/nonmatchings/P2/emitter", ChooseEmitoPos__FP5EMITOiiP6VECTORT3);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", ConvertEmitoPosVec__FP5EMITOP6VECTORT1);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", CalculateEmitvx__FiP2LMiP6EMITVX);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", ChooseEmitVelocity__FP6EMITVXffP2LMP6VECTORiT4);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", ChooseEmitvVelocityAge__FP5EMITVP6EMITVXP5EMITOiP6VECTORN24PfT7);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", EmitRips__FP5EMITBP5EMITGiP6VECTORT3PfT5);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", EmitBlips__FP5EMITBP5EMITGiP6VECTORT3PfT5T3T3);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", OriginateParticles__FiP5EMITBP7EMITGEN);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", OriginateSplineSinkParticles__FiP5EMITBR7EMITGENT2);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", EmitParticles__FiP5EMITBP5EMITG);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", EmitRipsSphere__FP6VECTORT0iP7EMITRIPP5EMITVPP4RIPGP2LO);

JUNK_WORD(0x27bd0290); // junk_00157FF0

INCLUDE_ASM("asm/nonmatchings/P2/emitter", StockSplashBig__FP6VECTORfP2SO);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", StockSplashSmall__FP6VECTORfP2SO);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", AddEmitoSkeleton__FP5EMITO3OIDT1ffffP2LO);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", BindEmitb__FP5EMITBP2LO);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", SetEmitdvEmitb__FP6EMITDVP5EMITB);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", CalculateEmitdvMatrix__FP6EMITDVfP7MATRIX4);

void PostExplLoad(EXPL* pexpl)
{
    PostLoLoad(pexpl);
    pexpl->pvtlo->pfnRemoveLo(pexpl);
}

INCLUDE_ASM("asm/nonmatchings/P2/emitter", CalculateExplTransform__FP4EXPLP6VECTORP7MATRIX3);

void ExplodeExpl(EXPL* pexpl)
{
    EXPLSO explso;
    
    memset(&explso, 0, sizeof(EXPLSO));
    
    typedef void (*PfnGetLoParams)(EXPL*, EXPLSO*);
    ((PfnGetLoParams)pexpl->pvtlo->pfnGetLoParams)(pexpl, &explso);
}

void ExplodeExplParams(EXPL* pexpl, unsigned int grfExplode, ALO* paloSource, VECTOR* pposImpact, VECTOR* pvecForce, float sRadius, float gPower)
{
    EXPLSO explso;

    explso.grfExplode = grfExplode;
    explso.paloSource = paloSource;
    
    explso.posImpact = *(qword*)pposImpact;
    explso.vecForce = *(qword*)pvecForce;

    explso.sRadius = sRadius;
    explso.gPower = gPower;

    typedef void (*PfnGetLoParams)(EXPL*, EXPLSO*);
    ((PfnGetLoParams)pexpl->pvtlo->pfnGetLoParams)(pexpl, &explso);
}

void ExplodeExplExplso(EXPL *pexpl, EXPLSO *pexplso)
{
    return;
}

INCLUDE_ASM("asm/nonmatchings/P2/emitter", LoadExplgFromBrx__FP5EXPLGP18CBinaryInputStream);

void CloneExplg(EXPLG* pexplgNew, EXPLG* pexplgSrc)
{
    CloneLo(pexplgNew, pexplgSrc);

    for (int i = 0; i < pexplgNew->cExpl; i++)
    {
        EXPL* pexplClone = (EXPL*)PloCloneLo(
            pexplgNew->apexpl[i], 
            pexplgNew->psw, 
            pexplgNew->paloParent
        );

        pexplClone->pvtlo->pfnRemoveLo(pexplClone);

        pexplgNew->apexpl[i] = pexplClone;
        
        pexplClone->pexplg = pexplgNew;
    }
}

void BindExplg(EXPLG* pexplg)
{
    for (int i = 0; i < pexplg->cExpl; i++)
    {
        EXPL* pexpl = pexplg->apexpl[i];
        
        if (pexpl->pvtlo->pfnBindLo != 0)
        {
            pexpl->pvtlo->pfnBindLo(pexpl);
        }
    }
}

void ExplodeExplgExplso(EXPLG* pexplg, EXPLSO* pexplso)
{
    for (int i = 0; i < pexplg->cExpl; i++)
    {
        EXPL* pexpl = pexplg->apexpl[i];
        
        typedef void (*PfnGetLoParams)(EXPL*, EXPLSO*);
        ((PfnGetLoParams)pexpl->pvtlo->pfnGetLoParams)(pexpl, pexplso);
    }
}

void InitExplo(EXPLO* pexplo)
{
    InitXfm(pexplo);
    pexplo->oid94 = OID_Nil;
    pexplo->oid98 = OID_Nil;
}

INCLUDE_ASM("asm/nonmatchings/P2/emitter", LoadExploFromBrx__FP5EXPLOP18CBinaryInputStream);

void CloneExplo(EXPLO* pexploNew, EXPLO* pexploSrc)
{
    CloneLo(pexploNew, pexploSrc);
    
    pexploNew->pemitb->cRef++;
}

INCLUDE_ASM("asm/nonmatchings/P2/emitter", BindExplo__FP5EXPLO);

void ExplodeExploExplso(EXPLO *pexplo, EXPLSO *pexplso)
{
    return;
}

void AddExploSkeleton(EXPLO* pexplo, OID oid1, OID oid2, float f1, float f2, float f3, float f4)
{
    EMITB* pemitb = PemitbEnsureExplo(pexplo, ENSK_Set);

    AddEmitoSkeleton(&pemitb->emito, oid1, oid2, f1, f2, f3, f4, pexplo);
}

EMITB* PemitbEnsureExplo(EXPLO* pexplo, ENSK ensk)
{
    if (ensk == ENSK_Set)
    {
        pexplo->pemitb = PemitbCopyOnWrite(pexplo->pemitb);
    }
    
    return pexplo->pemitb;
}

void InitExpls(EXPLS* pexpls)
{
    InitExplo(pexpls);
    
    pexpls->oidAC = OID_Nil;
    pexpls->oidB0 = OID_Nil;
    pexpls->oidB4 = OID_Nil;
}

INCLUDE_ASM("asm/nonmatchings/P2/emitter", BindExpls__FP5EXPLS);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", HandleExplsMessage__FP5EXPLS5MSGIDPv);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", ExplodeExplsExplso__FP5EXPLSP6EXPLSO);

SFX* PsfxEnsureExpls(EXPLS* pexpls, ENSK ensk)
{
    if (pexpls->psfx == 0)
    {
        NewSfx(&pexpls->psfx);
    }
    
    return pexpls->psfx;
}

INCLUDE_ASM("asm/nonmatchings/P2/emitter", FireExplsExplso__FP5EXPLSP6EXPLSO);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", InferExpl__FPP4EXPLP3ALO);

INCLUDE_ASM("asm/nonmatchings/P2/emitter", FireSwTimedExplodeStyles__FP2SW);

// junk_00159668
JUNK_ADDIU(10);
JUNK_WORD(0x7c450000);
JUNK_WORD(0x48220800);
