#include <crv.h>

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

INCLUDE_ASM("asm/nonmatchings/P2/crv", ConvertApos__FiP6VECTORP7MATRIX4T2);

INCLUDE_ASM("asm/nonmatchings/P2/crv", PcrvNew__F4CRVK);

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

INCLUDE_ASM("asm/nonmatchings/P2/crv", SMeasureCrvSegmentU__FP5CRVMSf);

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

INCLUDE_ASM("asm/nonmatchings/P2/crv", ConvertCrvl__FP4CRVLP7MATRIX4T1);

INCLUDE_ASM("asm/nonmatchings/P2/crv", SFromCrvlU__FP4CRVLf);

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
