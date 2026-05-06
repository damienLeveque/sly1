#include <actseg.h>
#include <aseg.h>
#include <asega.h>

void RetractActseg(ACTSEG *pactseg, GRFRA grfra)
{
    ASEGA *pasega = pactseg->pasega;
    RemoveDlEntry(&pasega->dlActseg, pactseg);

    if (pasega->pactsegError == pactseg)
    {
        pasega->pactsegError = NULL;
    }

    RetractAct(pactseg, grfra);
}

void CloneActseg(ACTSEG *pactseg, ACTSEG *pactsegBase)
{
    ASEGA *pasega = pactseg->pasega;
    DLE dleAsega = pactseg->dleAsega;

    CloneAct(pactseg, pactsegBase);

    pactseg->dleAsega = dleAsega;
    pactseg->pasega = pasega;
}

INCLUDE_ASM("asm/nonmatchings/P2/actseg", GetActsegPositionGoal__FP6ACTSEGfP6VECTORT2);

INCLUDE_ASM("asm/nonmatchings/P2/actseg", GetActsegRotationGoal__FP6ACTSEGfP7MATRIX3P6VECTOR);

void GetActsegTwistGoal(ACTSEG* pactseg, float* pf1, float* pf2)
{
    ASEGA* pasega = pactseg->pasega;
    ASEG_FRAME* pframe = pasega->paseg->aasegframe + pactseg->iAsegd;
    ACT* pactTwist = pframe->pactTwist;
    ALO* palo = pactseg->palo;

    if (pactTwist != 0)
    {
        typedef void (*PfnTwistEval)(ACT*, ALO*, int, float*, float*, float, float);
        
        ((PfnTwistEval)pactTwist->pvtact->pfnInit)(
            pactTwist,
            palo,
            0,
            pf1,
            pf2,
            pasega->tStart,
            pasega->tEnd
        );
    }
    else
    {
        GetActTwistGoal((ACT*)pactseg, pf1, pf2);
    }
}

void GetActsegScale(ACTSEG* pactseg, MATRIX3* pmat)
{
    ASEGA* pasega = pactseg->pasega;
    ASEG_FRAME* pframe = pasega->paseg->aasegframe + pactseg->iAsegd;
    ACT* pactScale = pframe->pactScale;
    
    ALO* palo = pactseg->palo;
    
    if (pactScale != 0)
    {
        typedef void (*PfnScaleEval)(ACT*, ALO*, void*, MATRIX3*, float);
        
        ((PfnScaleEval)pactScale->pvtact->pfnInit)(
            pactScale,
            palo,
            0,
            pmat,
            pasega->tStart
        );
    }
    else
    {
        GetActScale(pactseg, pmat);
    }
}

float GGetActsegPoseGoal(ACTSEG* pactseg, int iPose)
{
    float result;
    
    ASEGA* pasega = pactseg->pasega;
    
    ASEG_FRAME* pasegframe = &pasega->paseg->aasegframe[pactseg->iAsegd];
    
    ACT* pactTarget = pasegframe->apactPoses[iPose];

    typedef void (*PfnEval)(ACT*, ALO*, void*, float*, int, float, float);
    ((PfnEval)pactTarget->pvtact->pfnInit)(pactTarget, pactseg->palo, 0, &result, 0, pasega->tStart, pasega->tEnd);

    return result;
}
