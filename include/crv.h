/**
 * @file crv.h
 */
#ifndef CRV_H
#define CRV_H

#include "common.h"
#include <vec.h>
#include <mat.h>
#include <cm.h>

struct CONSTR;

extern VTCRV D_002176D0;
extern VTCRV D_00217708;
extern float D_00249FCC;
extern float D_00249FC8;
extern float D_00249FD0;

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
    VTCRV* pvtable;
    CRVK crvk;
    int fClosed;
    int ccv;
    float *picvu;
    float *picvs;
    VECTOR *picvpos;
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
struct CRVC : CRV
{
    VECTOR* pmappos;           
    VECTOR* pmapos;            
    char chPad24[0x0C];        
    VECTOR4 vecCachePos[0x14]; 
    float dCacheS[0x14];      
    int icvCache;            
};

/**
* @brief Curve with multiple control points.
*/
struct CRVMC : CRV
{
    CRVK crvk;
    int ccv;
    float *picvu;
    float *picvs;
    VECTOR *picvpos;
};

/**
 * @brief Curve Measure Segment.
 */
struct CRVMS {
    CRV* pcrv;     
    VECTOR* pvec1;  
    VECTOR* pvec2;   
};
// ...

float SMeasureApos(int ccv, VECTOR* pvec, float* pS);
 
float GWrapApos(float dG, int cpos, float* pApos, int fClosed);
 
int IposFindAposG(
    float dG,
    int cpos,
    float* apos,
    int fClosed,
    float* out1,
    float* out2);
 
void EvaluateAposG(
    float dG,
    int cpos,
    VECTOR* apos,
    float* ag,
    int fClosed,
    VECTOR* out1,
    VECTOR* out2);
 
void EvaluateCrvcFromS(CRVC* crvc, float dS, VECTOR* pOut1, VECTOR* pOut2);

void FindAposClosestPointAll(
    VECTOR* pvec,
    CONSTR* pconstr,
    int ccv,
    VECTOR* pmpicvpos,
    VECTOR* pvec2,
    VECTOR* pvec3,
    int* piOut,
    float* pdOut);

int FindAposClosestPointSegment(
    VECTOR* pvec,
    CONSTR* pconstr,
    int cpos,
    VECTOR* apos,
    int iPos,
    int fClosed,
    VECTOR* pvec2,
    VECTOR* pvec3,
    int* piPosOut,
    float* pdSOut);
 
void ConvertApos(int ccv, VECTOR* pMpicvpos, MATRIX4* pM1, MATRIX4* pM2);
 
CRV* PcrvNew(CRVK crvk);

void EvaluateCrvcFromS(CRVC* pcrvc, float dS, VECTOR* pvecOut1, VECTOR* pvecOut2);
 
float SFromCrvU(CRV* pcrv, float dU);
 
float UFromCrvS(CRV* pcrv, float dS);
 
int IcvFindCrvU(CRV* pcrv, float dU, float* pg0, float* pg1);
 
int IcvFindCrvS(CRV* pcrv, float dS, float* pg0, float* pg1);

float GMeasureCrvU(CRVMC* pcrvmc, float dU);
 
float UMaxCrv(CRV* pcrv);
 
float SMaxCrv(CRV* pcrv);
 
float SMeasureCrvSegmentU(CRVMS* pcrvms, float dU);
 
float DuGetCrvSearchIncrement(CRV* pcrv);
 
void EvaluateCrvlFromU(CRVL* crvl, float dU, VECTOR* out1, VECTOR* out2);
 
void EvaluateCrvlFromS(CRVL* crvl, float dS, VECTOR* out1, VECTOR* out2);
 
void RenderCrvlSegment(
    CRVL* pcrvl,
    int icv,
    MATRIX4* pmat,
    CM* pcm,
    RGBA rgba,
    int unk);
 
void ConvertCrvl(CRVL* crvl, MATRIX4* pM1, MATRIX4* pM2);
 
float SFromCrvlU(CRVL* crvl, float dU);
 
float UFromCrvlS(CRVL* crvl, float dS);
 
float MeasureCrvl(CRVL* pcrvl);
 
void FindCrvlClosestPointAll(
    CRVL* pcrvl,
    VECTOR* pvec,
    CONSTR* pconstr,
    VECTOR* pvec2,
    VECTOR* pvec3,
    float* pdUOut,
    float* pdSOut);
 
void FindCrvlClosestPointFromU(
    CRVL* pcrvl,
    VECTOR* pvec,
    float dU,
    CONSTR* pconstr,
    VECTOR* pvec2,
    VECTOR* pvec3,
    float* pdUOut,
    float* pdSOut);
 
void InvalidateCrvcCache(CRVC* crvc);
 
void FillCrvcCache(CRVC* pcrvc, int icv);
 
float SFromCrvcU(CRVC* pcrvc, float dU);
 
float UFromCrvcS(CRVC* pcrvc, float dS);

void MeasureCrvc(CRVC* pcrvc);
 
void FindCrvcClosestPointFromS(
    CRVC* pcrvc,
    VECTOR* pvec,
    float dS,
    CONSTR* pconstr,
    VECTOR* pvec2,
    VECTOR* pvec3,
    float* pdUOut,
    float* pdSOut);




#endif // CRV_H
