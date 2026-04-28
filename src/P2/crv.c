#include <crv.h>
#include <memory.h>
#include <mark.h>
#include <bez.h>
#include <util.h>

INCLUDE_ASM("asm/nonmatchings/P2/crv", SMeasureApos__FiP6VECTORPf);

float GWrapApos(float g, int cpos, float *apos, int fClosed)
{
    float gMin;
    float gMax;
    float dg;

    if (fClosed) {
        gMin = apos[0];
        gMax = apos[cpos - 1];
        dg = gMax - gMin;

        if (g < gMin) {
            g += dg;
            while (g < gMin) {
                g += dg;
            }
        }

        if (gMax < g) {
            do {
                g -= dg;
            } while (g > gMax); 
        }
    }

    return g;
}

int IposFindAposG(float g, int cpos, float* apos, int fClosed, float* out1, float* out2) 
{
    float wrapped = GWrapApos(g, cpos, apos, fClosed);
    int i;

    for (i = 1; i < cpos; i++) {
        if (wrapped < apos[i]) {
            break;
        }
    }

    if (i >= cpos) {
        i--;
    }
    
    i--;

    if (out1 != 0) {
        *out1 = wrapped - apos[i];
    }

    if (out2 != 0) {
        *out2 = apos[i + 1] - apos[i];
    }

    return i;
}

INCLUDE_ASM("asm/nonmatchings/P2/crv", EvaluateAposG__FfiP6VECTORPfiT2T2);

INCLUDE_ASM("asm/nonmatchings/P2/crv", FindAposClosestPointAll__FP6VECTORP6CONSTRiT0N20PiPf);

INCLUDE_ASM("asm/nonmatchings/P2/crv", FindAposClosestPointSegment__FP6VECTORP6CONSTRiT0iiT0T0PiPf);

void ConvertApos(int ccv, VECTOR* mpicvpos, MATRIX4* m1, MATRIX4* m2) 
{
    MATRIX4 local_matrix;
    
    /**
    * We need a 16 byte aligned temporary variable to store the result of the matrix-vector multiplication, 
    * since the function expects it. We can then copy the result back to the original vector after the multiplication.
    */
    typedef struct {
        float x, y, z, w;
    } __attribute__((aligned(16))) vec16_t;
    
    vec16_t local_vector;

    CalculateDmat4(m2, m1, &local_matrix);
    
    int i;
    for (i = 0; i < ccv; i++) {
        MultiplyMatrix4Vector(&local_matrix, mpicvpos, 1.0f, (VECTOR*)&local_vector);
        *(vec16_t*)mpicvpos = local_vector;
        mpicvpos = (VECTOR*)((char*)mpicvpos + 16);
    }
}

CRV *PcrvNew(CRVK crvk)
{
    CRV *pcrv;

    switch (crvk) {
        case 0: 
            pcrv = (CRV *)PvAllocSwClearImpl(0x1C);
            pcrv->pvtable = &D_002176D0;
            break;
            
        case 1: 
            pcrv = (CRV *)PvAllocSwClearImpl(0x1D0);
            pcrv->pvtable = &D_00217708;
            break;
            
        default:
            pcrv = 0;
            break;
    }

    if (pcrv != 0) {
        pcrv->crvk = crvk;
    }

    return pcrv;
}

float SFromCrvU(CRV *crv, float u)
{
    return 0.0f;
}

float UFromCrvS(CRV *crv, float s)
{
    return 0.0f;
}

int IcvFindCrvU(CRV *crv, float u, float *pg0, float *pg1)
{
    return IposFindAposG(u, crv->ccv, crv->picvu, crv->fClosed, pg0, pg1);
}

int IcvFindCrvS(CRV *crv, float s, float *pg0, float *pg1)
{
    return IposFindAposG(s, crv->ccv, crv->picvs, crv->fClosed, pg0, pg1);
}

INCLUDE_ASM("asm/nonmatchings/P2/crv", GMeasureCrvU__FP5CRVMCf);

float UMaxCrv(CRV *crv)
{
    return crv->picvu[crv->ccv - 1];
}

float SMaxCrv(CRV *crv)
{
    return crv->picvs[crv->ccv - 1];
}

JUNK_ADDIU(A0);

float SMeasureCrvSegmentU(CRVMS* pcrvms, float dU)
{
    VECTOR vecLocal;
    float dLocal1;
    float dLocal2;

    if (pcrvms->pcrv->pvtable->func04 != 0)
    {
        pcrvms->pcrv->pvtable->func04(pcrvms->pcrv, dU, &vecLocal, 0);
    }

    FindClosestPointOnLineSegment(&vecLocal, pcrvms->pvec1, pcrvms->pvec2, &dLocal1, &dLocal2);

    return dLocal2;
}

INCLUDE_ASM("asm/nonmatchings/P2/crv", FindCrvClosestPointOnLineSegmentFromU__FP3CRVP6VECTORT1fT1T1PfT6);

JUNK_ADDIU(A0);

float DuGetCrvSearchIncrement(CRV* pcrv)
{
    return (pcrv->picvu[1] - pcrv->picvu[0]) * 0.1f;
}

void LoadCrvlFromBrx(CRVL* pcrvl, CBinaryInputStream* pbis)
{
    int i = 0;

    pcrvl->fClosed = pbis->U8Read();
    pcrvl->ccv = pbis->U8Read();

    pcrvl->picvu = (float*)PvAllocSwImpl(pcrvl->ccv * 4);
    pcrvl->picvs = (float*)PvAllocSwImpl(pcrvl->ccv * 4);
    pcrvl->picvpos = (VECTOR*)PvAllocSwImpl(pcrvl->ccv * 16);

    if (pcrvl->ccv > 0)
    {
        for (; i < pcrvl->ccv; i++)
        {
            pcrvl->picvu[i] = pbis->F32Read();
            
            pbis->ReadVector((VECTOR*)&((VECTOR4*)pcrvl->picvpos)[i]);
        }
    }

    if (pcrvl->pvtable->func24 != 0)
    {
        pcrvl->pvtable->func24((CRV*)pcrvl);
    }
}

void EvaluateCrvlFromU(CRVL *crvl, float u, VECTOR *out1, VECTOR *out2)
{
    EvaluateAposG(u, crvl->ccv, crvl->picvpos, crvl->picvu, crvl->fClosed, out1, out2);
}

void EvaluateCrvlFromS(CRVL *crvl, float s, VECTOR *out1, VECTOR *out2)
{
    EvaluateAposG(s, crvl->ccv, crvl->picvpos, crvl->picvs, crvl->fClosed, out1, out2);
}

void RenderCrvlSegment(CRVL* pcrvl, int icv, MATRIX4* pmat, CM* pcm, RGBA rgba, int unk)
{
    int iNext;
    VECTOR vecStart;    
    VECTOR vecEnd;      

    if (icv == pcrvl->ccv - 1) {
        if (pcrvl->fClosed == 0) {
            return;
        }
        iNext = 0;
    } else {
        iNext = icv + 1;
    }

    MultiplyMatrix4Vector(pmat, (VECTOR*)&((VECTOR4*)pcrvl->picvpos)[icv], 1.0f, &vecStart);
    MultiplyMatrix4Vector(pmat, (VECTOR*)&((VECTOR4*)pcrvl->picvpos)[iNext], 1.0f, &vecEnd);

    DrawLineWorld(&vecStart, &vecEnd, rgba, pcm, unk);
}

void ConvertCrvl(CRVL* crvl, MATRIX4* m1, MATRIX4* m2) 
{
    ConvertApos(crvl->ccv, crvl->picvpos, m1, m2);
}

float SFromCrvlU(CRVL* crvl, float u) 
{
    float du;
    float length;
    
    int icv = IcvFindCrvU((CRV*)crvl, u, &du, &length);
    
    float t = du / length;
    
    return (1.0f - t) * crvl->picvs[icv] + t * crvl->picvs[icv + 1];
}

float UFromCrvlS(CRVL *crvl, float s)
{
    int icv;
    float g0;
    float g1;
    float r;

    icv = IcvFindCrvS((CRV *)crvl, s, &g0, &g1);

    r = g0 / g1;

    return ((1.0f - r) * crvl->picvu[icv]) + (r * crvl->picvu[icv + 1]);
}

float MeasureCrvl(CRVL* pcrvl)
{
    return SMeasureApos(pcrvl->ccv, pcrvl->picvpos, pcrvl->picvs);
}

void FindCrvlClosestPointAll(CRVL* pcrvl, VECTOR* pvec, CONSTR* pconstr, VECTOR* pvec2, VECTOR* pvec3, float* pdUOut, float* pdSOut)
{
    int iLocalPos;
    float dLocalT;

    FindAposClosestPointAll(
        pvec,
        pconstr,
        pcrvl->ccv,
        pcrvl->picvpos,
        pvec2,
        pvec3,
        &iLocalPos,
        &dLocalT
    );

    if (pdSOut != 0)
    {
        *pdSOut = (1.0f - dLocalT) * pcrvl->picvs[iLocalPos] + 
                  dLocalT * pcrvl->picvs[iLocalPos + 1];
    }

    if (pdUOut != 0)
    {
        *pdUOut = (1.0f - dLocalT) * pcrvl->picvu[iLocalPos] + 
                  dLocalT * pcrvl->picvu[iLocalPos + 1];
    }
}

void FindCrvlClosestPointFromU(CRVL* pcrvl, VECTOR* pvec, float dU, CONSTR* pconstr, VECTOR* pvec2, VECTOR* pvec3,  float* pdUOut, float* pdSOut)
{
    int iLocalPos;
    float dLocalT;

    int ipos = IposFindAposG(dU, pcrvl->ccv, pcrvl->picvu, pcrvl->fClosed, 0, 0);

    FindAposClosestPointSegment(
        pvec,
        pconstr,
        pcrvl->ccv,
        pcrvl->picvpos,
        ipos,
        pcrvl->fClosed,
        pvec2, 
        pvec3, 
        &iLocalPos,
        &dLocalT
    );

    if (pdSOut != 0)
    {
        *pdSOut = (1.0f - dLocalT) * pcrvl->picvs[iLocalPos] + 
                  dLocalT * pcrvl->picvs[iLocalPos + 1];
    }

    if (pdUOut != 0)
    {
        *pdUOut = (1.0f - dLocalT) * pcrvl->picvu[iLocalPos] + 
                  dLocalT * pcrvl->picvu[iLocalPos + 1];
    }
}

void FindCrvlClosestPointFromS(
    CRVL* pcrvl,
    VECTOR* pvec,
    float dS,
    CONSTR* pconstr,
    VECTOR* pvec2,
    VECTOR* pvec3,
    float* pdUOut,
    float* pdSOut)
{
    int iLocalPos;
    float dLocalT;

    int ipos = IposFindAposG(dS, pcrvl->ccv, pcrvl->picvs, pcrvl->fClosed, 0, 0);

    FindAposClosestPointSegment(
        pvec,
        pconstr,
        pcrvl->ccv,
        pcrvl->picvpos,
        ipos,
        pcrvl->fClosed,
        pvec2,
        pvec3,
        &iLocalPos,
        &dLocalT
    );

    if (pdSOut != 0)
    {
        *pdSOut = (1.0f - dLocalT) * pcrvl->picvs[iLocalPos] + 
                  dLocalT * pcrvl->picvs[iLocalPos + 1];
    }

    if (pdUOut != 0)
    {
        *pdUOut = (1.0f - dLocalT) * pcrvl->picvu[iLocalPos] + 
                  dLocalT * pcrvl->picvu[iLocalPos + 1];
    }
}

void LoadCrvcFromBrx(CRVC* pcrvc, CBinaryInputStream* pbis)
{
    int i;

    pcrvc->fClosed = pbis->U8Read();
    pcrvc->ccv = pbis->U8Read();

    pcrvc->picvu = (float*)PvAllocSwImpl(pcrvc->ccv * 4);
    pcrvc->picvs = (float*)PvAllocSwImpl(pcrvc->ccv * 4);
    pcrvc->picvpos = (VECTOR*)PvAllocSwImpl(pcrvc->ccv * 16);

    pcrvc->pmappos = (VECTOR*)PvAllocSwImpl(pcrvc->ccv * 16);
    pcrvc->pmapos = (VECTOR*)PvAllocSwImpl(pcrvc->ccv * 16);

    for (i = 0; i < pcrvc->ccv; i++)
    {
        pcrvc->picvu[i] = pbis->F32Read();
        pbis->ReadVector((VECTOR*)&((VECTOR4*)pcrvc->picvpos)[i]);
        pbis->ReadVector((VECTOR*)&((VECTOR4*)pcrvc->pmappos)[i]);
        pbis->ReadVector((VECTOR*)&((VECTOR4*)pcrvc->pmapos)[i]);
    }

    if (pcrvc->pvtable->func24 != 0)
    {
        pcrvc->pvtable->func24((CRV*)pcrvc);
    }

    InvalidateCrvcCache(pcrvc);
}

void InvalidateCrvcCache(CRVC *crvc)
{
    crvc->icvCache = -1;
}

void FillCrvcCache(CRVC* pcrvc, int icv)
{
    if (icv == pcrvc->icvCache)
    {
        return;
    }

    float dU = pcrvc->picvu[icv + 1] - pcrvc->picvu[icv];

    TesselateBezier(
        dU,
        0.0f,
        dU,
        (VECTOR*)&((VECTOR4*)pcrvc->picvpos)[icv],
        (VECTOR*)&((VECTOR4*)pcrvc->pmapos)[icv],      
        (VECTOR*)&((VECTOR4*)pcrvc->picvpos)[icv + 1],
        (VECTOR*)&((VECTOR4*)pcrvc->pmappos)[icv + 1],  
        20,
        (VECTOR*)pcrvc->vecCachePos
    );

    SMeasureApos(
        20, 
        (VECTOR*)pcrvc->vecCachePos, 
        pcrvc->dCacheS
    );

    pcrvc->icvCache = icv;
}

INCLUDE_ASM("asm/nonmatchings/P2/crv", EvaluateCrvcFromU__FP4CRVCfP6VECTORT2);

void EvaluateCrvcFromS(CRVC* pcrvc, float dS, VECTOR* pvecOut1, VECTOR* pvecOut2)
{
    float dU = pcrvc->pvtable->UFromS((CRV*)pcrvc, dS);

    if (pcrvc->pvtable->func04 != 0) 
    {
        pcrvc->pvtable->func04((CRV*)pcrvc, dU, pvecOut1, pvecOut2);
    }
}

INCLUDE_ASM("asm/nonmatchings/P2/crv", RenderCrvcSegment__FP4CRVCiP7MATRIX4P2CMG4RGBAi);

INCLUDE_ASM("asm/nonmatchings/P2/crv", ConvertCrvc__FP4CRVCP7MATRIX4T1);

float SFromCrvcU(CRVC* pcrvc, float dU)
{
    float dLocalU;
    float dLength;

    int icv = IcvFindCrvU((CRV*)pcrvc, dU, &dLocalU, &dLength);

    VECTOR4* pvecPos = (VECTOR4*)pcrvc->picvpos;
    VECTOR4* pvecOut = (VECTOR4*)pcrvc->pmapos;
    VECTOR4* pvecIn  = (VECTOR4*)pcrvc->pmappos;

    float dSLocal = SBezierPosLength(
        dLength,
        dLocalU,
        (VECTOR*)&pvecPos[icv],
        (VECTOR*)&pvecOut[icv],
        (VECTOR*)&pvecPos[icv + 1], 
        (VECTOR*)&pvecIn[icv + 1]   
    );

    return dSLocal + pcrvc->picvs[icv];
}

float UFromCrvcS(CRVC* pcrvc, float dS)
{
    float dSLocal;
    float dOffset;
    float dLength;

    int icv = IcvFindCrvS((CRV*)pcrvc, dS, &dSLocal, 0);

    FillCrvcCache(pcrvc, icv);

    int ipos = IposFindAposG(dSLocal, 20, pcrvc->dCacheS, 0, &dOffset, &dLength);

    float dT = dOffset / dLength;
    float dLerpFactor = ((float)ipos + dT) * (1.0f / 19.0f);

    return (1.0f - dLerpFactor) * pcrvc->picvu[icv] + dLerpFactor * pcrvc->picvu[icv + 1];
}

void MeasureCrvc(CRVC* pcrvc)
{
    int i = 0;

    pcrvc->picvs[0] = 0.0f;

    int cSegments = pcrvc->ccv - 1;

    if (cSegments <= 0)
    {
        return;
    }

    for (; i < cSegments; i++)
    {
        float dU = pcrvc->picvu[i + 1] - pcrvc->picvu[i];

        float dLength = SBezierPosLength(
            dU,
            dU,
            (VECTOR*)&((VECTOR4*)pcrvc->picvpos)[i],
            (VECTOR*)&((VECTOR4*)pcrvc->pmapos)[i],
            (VECTOR*)&((VECTOR4*)pcrvc->picvpos)[i + 1],
            (VECTOR*)&((VECTOR4*)pcrvc->pmappos)[i + 1]
        );

        pcrvc->picvs[i + 1] = pcrvc->picvs[i] + dLength;
    }
}

INCLUDE_ASM("asm/nonmatchings/P2/crv", FindCrvcClosestPointAll__FP4CRVCP6VECTORP6CONSTRT1T1PfT5);

void FindCrvcClosestPointFromU(
    CRVC* pcrvc,
    VECTOR* pvec,
    float dU,
    CONSTR* pconstr,
    VECTOR* pvec2,
    VECTOR* pvec3,
    float* pdUOut,
    float* pdSOut)
{
    CRVMS crvms;
    float dLocalU;

    crvms.pcrv = (CRV*)pcrvc;
    crvms.pvec1 = pvec;
    crvms.pvec2 = (VECTOR*)pconstr;

    MinimizeRange(
        GMeasureCrvU,
        (CRVMC*)&crvms,
        dU,
        (pcrvc->picvu[1] - pcrvc->picvu[0]) * 0.1f,           
        pcrvc->fClosed ? D_00249FCC : pcrvc->picvu[0], 
        pcrvc->fClosed ? D_00249FD0 : pcrvc->picvu[pcrvc->ccv - 1], 
        &dLocalU,
        0
    );

    if (pvec2 != 0 || pvec3 != 0)
    {
        if (pcrvc->pvtable->func04 != 0)
        {
            pcrvc->pvtable->func04((CRV*)pcrvc, dLocalU, pvec2, pvec3);
        }
    }

    if (pdUOut != 0)
    {
        *pdUOut = dLocalU;
    }

    if (pdSOut != 0)
    {
        *pdSOut = pcrvc->pvtable->func14((CRV*)pcrvc, dLocalU);
    }
}

void FindCrvcClosestPointFromS(
    CRVC* pcrvc,
    VECTOR* pvec,
    float dS,
    CONSTR* pconstr,
    VECTOR* pvec2,
    VECTOR* pvec3,
    float* pdUOut,
    float* pdSOut)
{
    float dU = pcrvc->pvtable->UFromS((CRV*)pcrvc, dS);

    if (pcrvc->pvtable->FindClosestPointFromU != 0)
    {
        pcrvc->pvtable->FindClosestPointFromU(
            (CRV*)pcrvc,
            pvec,
            dU,          
            pconstr,
            pvec2,
            pvec3,
            pdUOut,
            pdSOut
        );
    }
}
