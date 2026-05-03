/**
 * @file act.h
 */
#ifndef ACT_H
#define ACT_H

#include "common.h"
#include <vec.h>
#include <mat.h>
#include <dl.h>
#include <types.h>
#include <sm.h>

// Forward.
struct SW;
struct ALO;

typedef int GRFRA;

extern VTACT D_00219560;
extern VECTOR D_00248D30;
extern MATRIX3_ALIGNED D_002483D0;
extern SMP D_00260E60;

/**
 * @brief Action.
 * @todo Implement the struct.
 */
struct ACT
{
    /* 0x00 */ VTACT *pvtact;
    /* 0x04 */ ALO *palo;
    /* 0x08 */ DLE dleAlo;
    /* 0x10 */ char ackPos;     
    /* 0x11 */ char ackRot;     
    /* 0x12 */ char bUnk12;     
    /* 0x13 */ char bPoseMode;     
    /* 0x14 */ int nPriority;
    /* 0x18 */ float tMatch;
};

/**
 * @brief Unknown.
 * @todo Implement the struct.
 */
struct ACTVAL : public ACT
{
    /* 0x1C */ int pad_1c;                  
    /* 0x20 */ VECTOR posGoal;    
    /* 0x2C */ float pad_posGoal_w;          
    /* 0x30 */ VECTOR velGoal;            
    /* 0x40 */ MATRIX3_ALIGNED matOrig;     
    /* 0x70 */ VECTOR rotVelGoal;     
    /* 0x7c */ STRUCT_PADDING(1);         
    /* 0x80 */ float radTwistGoal;          
    /* 0x84 */ float dradTwistGoal;
    /* 0x88 */ STRUCT_PADDING(2);              
    /* 0x90 */ MATRIX3_ALIGNED matGoal;    
    /* 0xC0 */ float sRadius;               
    /* 0xC4 */ union {
        int grfalo;
        float* agPoses;
    };                

};

/**
 * @brief Unknown.
 * @todo Implement the struct.
 */
struct ACTREF : public ACT
{
    /* 0x1C */ VECTOR* pvecPosGoal;
    /* 0x20 */ VECTOR* pvecVelGoal; 
    /* 0x24 */ MATRIX3* pmatGoal;
    /* 0x28 */ VECTOR* pvecVelGoalAng; 
    /* 0x2C */ float *pradTwistGoal;
    /* 0x30 */ float *pdradTwistGoal;
    /* 0x34 */ MATRIX3_ALIGNED *pmatIdentityGoal;
    /* 0x38 */ float *psRadiusGoal;
    /* 0x3C */ union {
        int grfGoal;
        float* agPoses;
    };
};

/**
 * @brief Unknown.
 * @todo Implement the struct.
 */
struct ACTADJ : public ACT
{
    /* 0x1C */ STRUCT_PADDING(9);       // 36 bytes padding to reach 0x40
    /* 0x40 */ MATRIX3_ALIGNED mat1;    // The matrix at 0x40
    /* 0x70 */ STRUCT_PADDING(8);       // 32 bytes padding to reach 0x90
    /* 0x90 */ MATRIX3_ALIGNED mat2;    // The matrix at 0x90
    // ...
};

/**
 * @brief Unknown.
 * @todo Implement the struct.
 */
struct ACTBANK : public ACT
{
    /* 0x1c */ float uBank;
    /* 0x20 */ float dtPredict;
};

ACT *PactNew(SW *psw, ALO *palo, VTACT *pvtact);

ACT *PactNewClone(ACT *pactBase, SW *psw, ALO *palo);

void CloneAct(ACT *pact, ACT *pactBase);

void InitAct(ACT *pact, ALO *palo);

void RetractAct(ACT *pact, GRFRA grfra);

void GetActPositionGoal(ACT *pact, float dtOffset, VECTOR *ppos, VECTOR *pv);

void GetActRotationGoal(ACT *pact, float dtOffset, MATRIX3 *pmat, VECTOR *pw);

void GetActTwistGoal(ACT *pact, float *pradTwist, float *pdradTwist);

void GetActScale(ACT *pact, MATRIX3 *pmat);

float GGetActPoseGoal(ACT *pact, int ipose);

void CalculateActDefaultAck(ACT *pact);

void SnapAct(ACT *pact, int fForce);

void CalculateAloPositionSpring(ALO *palo, float dt, VECTOR *pposGoal, VECTOR *pvGoal, VECTOR *pdv);

void ProjectActPosition(ACT *pact);

void CalculateAloRotationSpring(ALO *palo, float dt, MATRIX3 *pmatGoal, VECTOR *pwGoal, VECTOR *pdw);

void ProjectActRotation(ACT *pact);

void ProjectActPose(ACT *pact, int ipose);

void PredictAloPosition(ALO *palo, float dtOffset, VECTOR *ppos, VECTOR *pv);

void PredictAloRotation(ALO *palo, float dtOffset, MATRIX3 *pmat, VECTOR *pw);

void AdaptAct(ACT *pact);

void InitActval(ACTVAL *pactval, ALO *palo);

void GetActvalPositionGoal(ACTVAL *pactval, float dtOffset, VECTOR *ppos, VECTOR *pv);

void GetActvalRotationGoal(ACTVAL *pactval, float dtOffset, MATRIX3 *pmat, VECTOR *pw);

void GetActvalTwistGoal(ACTVAL *pactval, float *pradTwist, float *pdradTwist);

void GetActvalScale(ACTVAL *pactval, MATRIX3 *pmat);

float GGetActvalPoseGoal(ACTVAL *pactval, int ipose);

void InitActref(ACTREF *pactref, ALO *palo);

void GetActrefPositionGoal(ACTREF *pactref, float dtOffset, VECTOR *ppos, VECTOR *pv);

void GetActrefRotationGoal(ACTREF *pactref, float dtOffset, MATRIX3 *pmat, VECTOR *pw);

void GetActrefTwistGoal(ACTREF *pactref, float *pradTwist, float *pdradTwist);

void GetActrefScale(ACTREF *pactref, MATRIX3 *pmat);

float GGetActrefPoseGoal(ACTREF *pactref, int ipose);

void InitActadj(ACTADJ *pactadj, ALO *palo);

void GetActadjPositionGoal(ACTADJ *pactadj, float dtOffset, VECTOR *ppos, VECTOR *pv);

void GetActadjRotationGoal(ACTADJ *pactadj, float dtOffset, MATRIX3 *pmat, VECTOR *pw);

void GetActadjTwistGoal(ACTADJ *pactadj, float *pradTwist, float *pdradTwist);

void GetActadjScale(ACTADJ *pactadj, MATRIX3 *pmat);

float GGetActadjPoseGoal(ACTADJ *pactadj, int ipose);

void InitActbank(ACTBANK *pactbank, ALO *palo);

void GetActbankRotationGoal(ACTBANK *pactbank, float dtOffset, MATRIX3 *pmat, VECTOR *pw);

#endif // ACT_H
