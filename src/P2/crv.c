#include <crv.h>
#include <memory.h>
#include <mark.h>

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

int IposFindAposG(float g, int cpos, float* apos, int fClosed, float* out1, float* out2) {
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

void ConvertApos(int ccv, VECTOR* mpicvpos, MATRIX4* m1, MATRIX4* m2) {
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
    CRV *crv;

    switch (crvk) {
        case 0: 
            crv = (CRV *)PvAllocSwClearImpl(0x1C);
            crv->vtable = &D_002176D0;
            break;
            
        case 1: 
            crv = (CRV *)PvAllocSwClearImpl(0x1D0);
            crv->vtable = &D_00217708;
            break;
            
        default:
            crv = 0;
            break;
    }

    if (crv != 0) {
        crv->crvk = crvk;
    }

    return crv;
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
    return IposFindAposG(u, crv->ccv, crv->mpicvu, crv->fClosed, pg0, pg1);
}

int IcvFindCrvS(CRV *crv, float s, float *pg0, float *pg1)
{
    return IposFindAposG(s, crv->ccv, crv->mpicvs, crv->fClosed, pg0, pg1);
}

INCLUDE_ASM("asm/nonmatchings/P2/crv", GMeasureCrvU__FP5CRVMCf);

float UMaxCrv(CRV *crv)
{
    return crv->mpicvu[crv->ccv - 1];
}

float SMaxCrv(CRV *crv)
{
    return crv->mpicvs[crv->ccv - 1];
}

JUNK_ADDIU(A0);

float SMeasureCrvSegmentU(CRVMS* crvms, float u) {
    VECTOR local_vector;
    float local_float1;
    float local_float2;

    if (crvms->crv->vtable->func04 != 0) {
        crvms->crv->vtable->func04(crvms->crv, u, &local_vector, 0);
    }

    FindClosestPointOnLineSegment(&local_vector, crvms->vec1, crvms->vec2, &local_float1, &local_float2);

    return local_float2;
}

INCLUDE_ASM("asm/nonmatchings/P2/crv", FindCrvClosestPointOnLineSegmentFromU__FP3CRVP6VECTORT1fT1T1PfT6);

JUNK_ADDIU(A0);

INCLUDE_ASM("asm/nonmatchings/P2/crv", DuGetCrvSearchIncrement__FP3CRV);

INCLUDE_ASM("asm/nonmatchings/P2/crv", LoadCrvlFromBrx__FP4CRVLP18CBinaryInputStream);

void EvaluateCrvlFromU(CRVL *crvl, float u, VECTOR *out1, VECTOR *out2)
{
    EvaluateAposG(u, crvl->ccv, crvl->mpicvpos, crvl->mpicvu, crvl->fClosed, out1, out2);
}

void EvaluateCrvlFromS(CRVL *crvl, float s, VECTOR *out1, VECTOR *out2)
{
    EvaluateAposG(s, crvl->ccv, crvl->mpicvpos, crvl->mpicvs, crvl->fClosed, out1, out2);
}

INCLUDE_ASM("asm/nonmatchings/P2/crv", RenderCrvlSegment__FP4CRVLiP7MATRIX4P2CMG4RGBAi);

void ConvertCrvl(CRVL* crvl, MATRIX4* m1, MATRIX4* m2) {
    ConvertApos(crvl->ccv, crvl->mpicvpos, m1, m2);
}

float SFromCrvlU(CRVL* crvl, float u) {
    float du;
    float length;
    
    int icv = IcvFindCrvU((CRV*)crvl, u, &du, &length);
    
    float t = du / length;
    
    return (1.0f - t) * crvl->mpicvs[icv] + t * crvl->mpicvs[icv + 1];
}

float UFromCrvlS(CRVL *crvl, float s)
{
    int icv;
    float g0;
    float g1;
    float r;

    icv = IcvFindCrvS((CRV *)crvl, s, &g0, &g1);

    r = g0 / g1;

    return ((1.0f - r) * crvl->mpicvu[icv]) + (r * crvl->mpicvu[icv + 1]);
}

INCLUDE_ASM("asm/nonmatchings/P2/crv", MeasureCrvl__FP4CRVL);

INCLUDE_ASM("asm/nonmatchings/P2/crv", FindCrvlClosestPointAll__FP4CRVLP6VECTORP6CONSTRT1T1PfT5);

INCLUDE_ASM("asm/nonmatchings/P2/crv", FindCrvlClosestPointFromU__FP4CRVLP6VECTORfP6CONSTRT1T1PfT6);

INCLUDE_ASM("asm/nonmatchings/P2/crv", FindCrvlClosestPointFromS__FP4CRVLP6VECTORfP6CONSTRT1T1PfT6);

INCLUDE_ASM("asm/nonmatchings/P2/crv", LoadCrvcFromBrx__FP4CRVCP18CBinaryInputStream);

void InvalidateCrvcCache(CRVC *crvc)
{
    crvc->icvCache = -1;
}

INCLUDE_ASM("asm/nonmatchings/P2/crv", FillCrvcCache__FP4CRVCi);

INCLUDE_ASM("asm/nonmatchings/P2/crv", EvaluateCrvcFromU__FP4CRVCfP6VECTORT2);

INCLUDE_ASM("asm/nonmatchings/P2/crv", EvaluateCrvcFromS__FP4CRVCfP6VECTORT2);

INCLUDE_ASM("asm/nonmatchings/P2/crv", RenderCrvcSegment__FP4CRVCiP7MATRIX4P2CMG4RGBAi);

INCLUDE_ASM("asm/nonmatchings/P2/crv", ConvertCrvc__FP4CRVCP7MATRIX4T1);

INCLUDE_ASM("asm/nonmatchings/P2/crv", SFromCrvcU__FP4CRVCf);

INCLUDE_ASM("asm/nonmatchings/P2/crv", UFromCrvcS__FP4CRVCf);

INCLUDE_ASM("asm/nonmatchings/P2/crv", MeasureCrvc__FP4CRVC);

INCLUDE_ASM("asm/nonmatchings/P2/crv", FindCrvcClosestPointAll__FP4CRVCP6VECTORP6CONSTRT1T1PfT5);

INCLUDE_ASM("asm/nonmatchings/P2/crv", FindCrvcClosestPointFromU__FP4CRVCP6VECTORfP6CONSTRT1T1PfT6);

INCLUDE_ASM("asm/nonmatchings/P2/crv", FindCrvcClosestPointFromS__FP4CRVCP6VECTORfP6CONSTRT1T1PfT6);
