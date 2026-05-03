#include <act.h>
#include <sw.h>
#include <so.h>
#include <slotheap.h>
#include <types.h>
#include <memory.h>
#include <clock.h>

ACT* PactNew(SW* psw, ALO* palo, VTACT* pvtact)
{
    ACT* pact = (ACT*)PvAllocSlotheapClearImpl(&psw->slotheapAct); 
    pact->pvtact = pvtact;
    pact->pvtact->pfnInit(pact, palo);
    return pact;
}

ACT* PactNewClone(ACT* pactSrc, SW* psw, ALO* palo)
{
    ACT* pactNew = PactNew(psw, palo, &D_00219560);

    pactNew->pvtact->pfnClone(pactNew, pactSrc);

    return pactNew;
}

void CloneAct(ACT* pactNew, ACT* pactSrc)
{
    DLE dlSaved = pactNew->dleAlo;
    
    ALO* paloSaved = pactNew->palo;

    CopyAb(pactNew, pactSrc, g_psw->slotheapAct.cb);

    pactNew->dleAlo = dlSaved;
    pactNew->palo = paloSaved;
}

void InitAct(ACT* pact, ALO* palo)
{
    char c = -1;
    pact->bUnk12 = c;       
    pact->palo = palo;
    pact->bPoseMode = -1;       
    pact->ackRot = c;        
    pact->ackPos = c;
}

void RetractAct(ACT* pact, int fForce)
{
    ALO* palo = pact->palo;

    RemoveDlEntry(&palo->dlAct, pact);

    ACT* pactPos = palo->pactPos; 
    ACT* pactRot = palo->pactRot; 

    palo->pvtalo->pfnOnActRetract(palo);

    if (pact == pactPos && palo->pactPos == 0 && (fForce & 1))
    {
        if (palo->pvtalo->grfcid & 2)
        {
            VECTOR vecResult;
            SO* pso = (SO*)palo; 
        
            ApplySoConstraintLocal(
                pso, 
                &pso->constrPos, 
                &pso->posOrig,   
                &vecResult, 
                0
            );

            palo->pvtalo->pfnSetVelocity(palo, &vecResult);
        }
        else
        {
            SetAloVelocityVec(palo, &D_00248D30);
        }
    }

    if (pact == pactRot && palo->pactRot == 0 && (fForce & 2))
    {
        if (palo->pvtalo->grfcid & 2)
        {
            VECTOR vecResult;
            SO* pso = (SO*)palo;

            ApplySoConstraintLocal(
                pso, 
                &pso->constrRot, 
                &pso->eulOrig,  
                &vecResult, 
                0
            );

            palo->pvtalo->pfnSetAngularVelocity(palo, &vecResult);
        }
        else
        {
            SetAloAngularVelocityVec(palo, &D_00248D30);
        }
    }

    FreeSlotheapPv(&palo->psw->slotheapAct, pact);
}

void GetActPositionGoal(ACT* pact, float dt, VECTOR* pvecPos, VECTOR* pvecVel)
{
    *(qword*)pvecPos = *(qword*)&pact->palo->posGoal;
    *(qword*)pvecVel = *(qword*)&D_00248D30;

    if (pact->palo->pvtalo->pfnUpdatePositionGoal != 0)
    {
        pact->palo->pvtalo->pfnUpdatePositionGoal(pact->palo, pvecPos);
    }
}

void GetActRotationGoal(ACT* pact, float dt, MATRIX3* pmatRot, VECTOR* pvecVel)
{
    qword* pDstMat = (qword*)pmatRot;
    qword* pDstVec = (qword*)pvecVel;
    
    qword* pSrcVec = (qword*)&D_00248D30;
    qword* paloArray = (qword*)pact->palo;

    pDstMat[0] = paloArray[26]; 
    pDstMat[1] = paloArray[27]; 
    pDstMat[2] = paloArray[28]; 

    pDstVec[0] = pSrcVec[0];

    pact->palo->pvtalo->pfnUpdateRotationGoal(pact->palo, pmatRot, pvecVel);
}

void GetActTwistGoal(ACT* pact, float* pradTwist, float* pdradTwist)
{
    *pradTwist = pact->palo->palox->radTwist;
    *pdradTwist = 0.0f; 
}

void GetActScale(ACT* pact, MATRIX3* pmat)
{
    ((MATRIX3_ALIGNED*)pmat)->row[0] = ((MATRIX3_ALIGNED*)&D_002483D0)->row[0];
    ((MATRIX3_ALIGNED*)pmat)->row[1] = ((MATRIX3_ALIGNED*)&D_002483D0)->row[1];
    ((MATRIX3_ALIGNED*)pmat)->row[2] = ((MATRIX3_ALIGNED*)&D_002483D0)->row[2];
}

float GGetActPoseGoal(ACT* pact, int iposec)
{
    return ((float*)pact->palo->grfalo)[iposec];
}

void CalculateActDefaultAck(ACT* pact)
{
    ALO* palo = pact->palo;
    ALO* paloTemp = (((int*)palo->pvtalo)[2] & 2) ? palo : 0;

    signed char ackPos = ((signed char*)palo)[0x2C9];

    if (ackPos != -1)
    {
        pact->ackPos = ackPos;
    }

    else if (paloTemp == 0 || *(int*)((char*)paloTemp + 0x450) == 3)
    {
        pact->ackPos = 2;
    }
    else
    {
        pact->ackPos = 3;
    }

    signed char ackRot = ((signed char*)palo)[0x2CA];

    if (ackRot != -1)
    {
        pact->ackRot = ackRot;
        return;
    }

    if (paloTemp != 0)
    {
        if (*(int*)((char*)paloTemp + 0x470) != 3)
        {
            pact->ackRot = 3;
            return;
        }
    }

    pact->ackRot = 2;
}

void SnapAct(ACT* pact, int fForce)
{
    ALO* palo = pact->palo;

    if (pact == palo->pactPos)
    {
        if (fForce || pact->ackPos == 2)
        {
            VECTOR vecPos;
            VECTOR vecVel;

            pact->pvtact->pfnGetPositionGoal(pact, 0.0f, &vecPos, &vecVel);

            palo->pvtalo->pfnSetPosition(palo, &vecPos);
            palo->pvtalo->pfnSetVelocity(palo, &vecVel);
        }
    }

    if (pact == palo->pactRot)
    {
        if (fForce || pact->ackRot == 2)
        {
            VECTOR vecAngVel;
            MATRIX3 matRot;

            pact->pvtact->pfnGetRotationGoal(pact, 0.0f, &matRot, &vecAngVel);

            palo->pvtalo->pfnSetRotation(palo, &matRot);
            palo->pvtalo->pfnSetAngularVelocity(palo, &vecAngVel);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/P2/act", CalculateAloPositionSpring__FP3ALOfP6VECTORN22);

INCLUDE_ASM("asm/nonmatchings/P2/act", ProjectActPosition__FP3ACT);

INCLUDE_ASM("asm/nonmatchings/P2/act", CalculateAloRotationSpring__FP3ALOfP7MATRIX3P6VECTORT3);

INCLUDE_ASM("asm/nonmatchings/P2/act", ProjectActRotation__FP3ACT);

// INCLUDE_ASM("asm/nonmatchings/P2/act", ProjectActPose__FP3ACTi);

void ProjectActPose(ACT* pact, int iPose)
{
    float fTarget = pact->pvtact->pfnGetPoseTarget(pact, iPose);

    switch (pact->bPoseMode)
    {
        case 2:
            pact->palo->pagPose[iPose] = fTarget;
            break;
            
        case 3:
        {
            float val = pact->palo->pagPose[iPose];
            float dt = pact->palo->fRealClock ? g_clock.dtReal : g_clock.dt;
            
            pact->palo->pagPose[iPose] = GSmooth(val, fTarget, dt, &D_00260E60, 0);
            break;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/P2/act", PredictAloPosition__FP3ALOfP6VECTORT2);

INCLUDE_ASM("asm/nonmatchings/P2/act", PredictAloRotation__FP3ALOfP7MATRIX3P6VECTOR);

void AdaptAct(ACT* pact)
{
    char* p = (char*)pact;
    if (p[0x10] == 7) p[0x10] = 3;
    if (p[0x11] == 7) p[0x11] = 3;
}
void InitActval(ACTVAL* pactval, ALO* palo)
{
    InitAct((ACT*)pactval, palo);

    *(qword*)&pactval->posGoal = *(qword*)&palo->posGoal;

    ((qword*)&pactval->matOrig)[0] = ((qword*)&palo->matOrig)[0];
    ((qword*)&pactval->matOrig)[1] = ((qword*)&palo->matOrig)[1];
    ((qword*)&pactval->matOrig)[2] = ((qword*)&palo->matOrig)[2];

    if (palo->palox != 0 && (palo->palox->grfalox & 0x20) != 0)
    {
        pactval->radTwistGoal = palo->palox->radTwist;
    }

    ((qword*)&pactval->matGoal)[0] = ((qword*)&D_002483D0)[0];
    ((qword*)&pactval->matGoal)[1] = ((qword*)&D_002483D0)[1];
    ((qword*)&pactval->matGoal)[2] = ((qword*)&D_002483D0)[2];

    *(int*)&pactval->sRadius = *(int*)&palo->sRadius;
    pactval->grfalo = palo->grfalo; 
}

void GetActvalPositionGoal(ACTVAL* pactval, float dt, VECTOR* pvecPos, VECTOR* pvecVel)
{
    // The use of volatile is required to match, as the compiler must not optimize the loads into registers, as they are used in a function call below that may modify them.
    *(volatile qword*)pvecPos = *(volatile qword*)&pactval->posGoal; 
    *(volatile qword*)pvecVel = *(volatile qword*)&pactval->velGoal;

    if (pactval->palo->pvtalo->pfnUpdatePositionGoal != 0)
    {
        pactval->palo->pvtalo->pfnUpdatePositionGoal(pactval->palo, pvecPos);
    }
}


void GetActvalRotationGoal(ACTVAL* pactval, float dt, MATRIX3* pmat, VECTOR* pvec)
{
    qword* pDstMat = (qword*)pmat;
    qword* pDstVec = (qword*)pvec;

    qword* pSrc = (qword*)pactval; 

    pDstMat[0] = pSrc[4]; 
    pDstMat[1] = pSrc[5]; 
    pDstMat[2] = pSrc[6]; 

    pDstVec[0] = pSrc[7]; 

    pactval->palo->pvtalo->pfnUpdateRotationGoal(pactval->palo, pmat, pvec);
}

void GetActvalTwistGoal(ACTVAL *pactval, float *pradTwist, float *pdradTwist)
{
    *pradTwist = pactval->radTwistGoal;
    *pdradTwist = pactval->dradTwistGoal;
}

void GetActvalScale(ACTVAL* pactval, MATRIX3* pmat)
{
    qword* pDst = (qword*)pmat;
    qword* pSrc = (qword*)pactval; 

    pDst[0] = pSrc[9];
    pDst[1] = pSrc[10];
    pDst[2] = pSrc[11];
}

float GGetActvalPoseGoal(ACTVAL *pactval, int ipose)
{
    return pactval->agPoses[ipose];
}

void InitActref(ACTREF* pactref, ALO* palo)
{
    ALOX* palox; 

    InitAct((ACT*)pactref, palo);

    pactref->pvecPosGoal = &palo->posGoal;
    pactref->pmatGoal = &palo->matOrig;
    pactref->psRadiusGoal = &palo->sRadius;
    
    pactref->pvecVelGoal = &D_00248D30;
    pactref->pvecVelGoalAng = &D_00248D30;
    
    pactref->grfGoal = palo->grfalo;

    palox = palo->palox;
    if (palox != 0)
    {
        if (palox->grfalox & 0x20)
        {
            pactref->pdradTwistGoal = (float*)&D_00248D30;
            pactref->pradTwistGoal = &palox->radTwist;
        }
    }

    pactref->pmatIdentityGoal = &D_002483D0;
}

void GetActrefPositionGoal(ACTREF* pactref, float dt, VECTOR* pvecPos, VECTOR* pvecVel)
{
    *(qword*)pvecPos = *(qword*)pactref->pvecPosGoal;
    *(qword*)pvecVel = *(qword*)pactref->pvecVelGoal;

    if (pactref->palo->pvtalo->pfnUpdatePositionGoal != 0)
    {
        pactref->palo->pvtalo->pfnUpdatePositionGoal(pactref->palo, pvecPos);
    }
}

void GetActrefRotationGoal(ACTREF* pactref, float dt, MATRIX3* pmatRot, VECTOR* pvecVel)
{
    qword* pDstMat = (qword*)pmatRot;
    qword* pDstVec = (qword*)pvecVel;
    
    qword* pSrcMat = (qword*)pactref->pmatGoal;
    qword* pSrcVec; 

    pDstMat[0] = pSrcMat[0];
    pDstMat[1] = pSrcMat[1];
    pDstMat[2] = pSrcMat[2];

    pSrcVec = (qword*)pactref->pvecVelGoalAng;
    
    pDstVec[0] = pSrcVec[0];

    pactref->palo->pvtalo->pfnUpdateRotationGoal(pactref->palo, pmatRot, pvecVel);
}

void GetActrefTwistGoal(ACTREF *pactref, float *pradTwist, float *pdradTwist)
{
    *pradTwist = *pactref->pradTwistGoal;
    *pdradTwist = *pactref->pdradTwistGoal;
}

void GetActrefScale(ACTREF* pactref, MATRIX3* pmat)
{
    qword* pSrc = (qword*)pactref->pmatIdentityGoal;
    qword* pDst = (qword*)pmat;

    pDst[0] = pSrc[0];
    pDst[1] = pSrc[1];
    pDst[2] = pSrc[2];
}

float GGetActrefPoseGoal(ACTREF* pactref, int ipose)
{
    return pactref->agPoses[ipose];
}

void InitActadj(ACTADJ* pactadj, ALO* palo)
{
    InitAct((ACT*)pactadj, palo);

    qword* pMat1 = (qword*)((char*)pactadj + 0x40);
    qword* pMat2 = (qword*)((char*)pactadj + 0x90);
    qword* pSrc  = (qword*)&D_002483D0;

    *(qword*)((char*)pactadj + 0x40) = pSrc[0]; 
    pMat1[1] = pSrc[1];
    pMat1[2] = pSrc[2];

    *(qword*)((char*)pactadj + 0x90) = pSrc[0];
    pMat2[1] = pSrc[1];
    pMat2[2] = pSrc[2];
}

INCLUDE_ASM("asm/nonmatchings/P2/act", GetActadjPositionGoal__FP6ACTADJfP6VECTORT2);

INCLUDE_ASM("asm/nonmatchings/P2/act", GetActadjRotationGoal__FP6ACTADJfP7MATRIX3P6VECTOR);

void GetActadjTwistGoal(ACTADJ* pactadj, float* pradTwist, float* pdradTwist)
{
    ACT* pactCur = (ACT*)pactadj;

    while ((pactCur = (ACT*)pactCur->dleAlo.next) != 0)
    {
        if (pactCur->ackRot == 0)
        {
            pactCur = 0;
            break;
        }
        if (pactCur->ackRot != -1)
        {
            break;
        }
    }

    if (pactCur != 0)
    {
        pactCur->pvtact->pfnGetTwistGoal(pactCur, pradTwist, pdradTwist);
    }
    else
    {
        GetActTwistGoal(pactadj, pradTwist, pdradTwist);
    }
}

INCLUDE_ASM("asm/nonmatchings/P2/act", GetActadjScale__FP6ACTADJP7MATRIX3);

float GGetActadjPoseGoal(ACTADJ *pactadj, int ipose)
{
    return 0.0f;
}

void InitActbank(ACTBANK *pactbank, ALO *palo)
{
    InitAct(pactbank, palo);
    pactbank->uBank = 1.0f;
    pactbank->dtPredict = 0.25f;
}

INCLUDE_ASM("asm/nonmatchings/P2/act", GetActbankRotationGoal__FP7ACTBANKfP7MATRIX3P6VECTOR);
