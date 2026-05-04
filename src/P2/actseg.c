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

INCLUDE_ASM("asm/nonmatchings/P2/actseg", GetActsegTwistGoal__FP6ACTSEGPfT1);

INCLUDE_ASM("asm/nonmatchings/P2/actseg", GetActsegScale__FP6ACTSEGP7MATRIX3);

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
