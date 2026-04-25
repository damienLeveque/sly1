/**
 * @file crv.h
 */
#ifndef CRV_H
#define CRV_H

#include "common.h"
#include <vec.h>
#include <mat.h>

extern VTCRV D_002176D0;
extern VTCRV D_00217708;

/**
 * @brief Curve interpolation kind.
 */
enum CRVK
{
    CRVK_Nil = -1,
    CRVK_Linear = 0,
    CRVK_Cubic = 1,
    CRVK_Max = 2
};

/**
 * @brief Curve struct.
 */
struct CRV
{
    VTCRV* vtable;
    CRVK crvk;
    int fClosed;
    int ccv;
    float *mpicvu;
    float *mpicvs;
    VECTOR *mpicvpos;
};

/**
* @brief Curve with linear interpolation.
*/
struct CRVL : CRV
{
    int cpos;
    float *mapos;
};

/**
 * @brief Curve with cubic interpolation.
*/
struct CRVC : CRV {
    VECTOR *mappos;          // 0x1C
    float *mapos;            // 0x20
    char _pad24[0x0C];       // 0x24-0x2F
    VECTOR4 cachePos[0x14];   // 0x30, size 0x140 
    float cacheS[0x14];      // 0x170, size 0x50
    int icvCache;            // 0x1C0
};

/**
* @brief Curve with multiple control points.
*/
struct CRVMC : CRV
{
    CRVK crvk;
    int ccv;
    float *mpicvu;
    float *mpicvs;
    VECTOR *mpicvpos;
};

/**
 * @brief Curve Measure Segment.
 */
struct CRVMS {
    CRV* crv;       // Offset 0x00
    VECTOR* vec1;   // Offset 0x04
    VECTOR* vec2;   // Offset 0x08
};
// ...

float GWrapApos(float g, int cpos, float *apos, int fClosed);

int IposFindAposG(float g, int cpos, float* apos, int fClosed, float* out1, float* out2);

void EvaluateAposG(float g, int cpos, VECTOR *apos, float *ag, int fClosed, VECTOR *out1, VECTOR *out2);

void EvaluateCrvcFromS(CRVC* crvc, float s, VECTOR* out1, VECTOR* out2);

void ConvertCrvl(CRVL* crvl, MATRIX4* m1, MATRIX4* m2);

void ConvertApos(int ccv, VECTOR* mpicvpos, MATRIX4* m1, MATRIX4* m2);

CRV *PcrvNew(CRVK crvk);

float SFromCrvU(CRV *crv, float u);

float UFromCrvS(CRV *crv, float s);

int IcvFindCrvU(CRV *crv, float u, float *pg0, float *pg1);

int IcvFindCrvS(CRV *crv, float s, float *pg0, float *pg1);

float UMaxCrv(CRV *crv);

float SMaxCrv(CRV *crv);

float SMeasureCrvSegmentU(CRVMS* crvms, float u);

void EvaluateCrvlFromU(CRVL *crvl, float u, VECTOR *out1, VECTOR *out2);

void EvaluateCrvlFromS(CRVL *crvl, float s, VECTOR *out1, VECTOR *out2);

void ConvertCrvl(CRVL* crvl, MATRIX4* m1, MATRIX4* m2);

float SFromCrvlU(CRVL* crvl, float u);

float UFromCrvlS(CRVL *crvl, float s);

void InvalidateCrvcCache(CRVC *crvc);



#endif // CRV_H
