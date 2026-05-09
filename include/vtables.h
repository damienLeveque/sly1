/**
 * @file vtables.h
 *
 * @brief Manually crafted vtables for various structs.
 */
#ifndef VTABLES_H
#define VTABLES_H

#include <cid.h>
#include <mq.h>

typedef int GRFCID;

/**
 * @brief Generic VT struct. Used to determine which VTables an entity inherits at runtime.
 */
struct VT
{
    VT *pvtSuper;
    CID cid;
    GRFCID grfcid;
    int cb;
};


/*****************************************************************
 * BASIC-DERIVED VTABLES
 *****************************************************************/

struct CBinaryInputStream;
struct LO;


/**
 * @brief VT for basic objects.
 */
struct VTBASIC
{
    VT *pvtSuper;
    CID cid;
};

/**
 * @brief VT for LO objects.
 */
struct VTLO : VT
{
    void (*pfnInitLo)(LO *);
    void (*pfnSetLoDefaults)(LO *);
    void (*pfnAddLo)(LO *);
    void (*pfnRemoveLo)(LO *);
    void (*pfnAddLoHierarchy)(LO *);
    void (*pfnRemoveLoHierarchy)(LO *);
    void (*pfnOnLoAdd)(LO *);
    void (*pfnOnLoRemove)(LO *);
    void (*pfnCloneLoHierarchy)(LO *, LO *);
    void (*pfnCloneLo)(LO *, LO *);
    void (*pfnLoadLoFromBrx)(LO *, CBinaryInputStream *);
    void (*pfnAddLoRecursive)(/* @todo: null in release & proto? */);
    void (*pfnRemoveLoRecursive)(/* @todo: null in release & proto? */);
    void (*pfnHandleLoMessage)(LO *, MSGID, void *);
    void (*pfnSendLoMessage)(LO *, MSGID, void *);
    void (*pfnBindLo)(LO *);
    void (*pfnPostLoLoad)(LO *);

    /**
     * @todo From prototype, not confirmed if in release
     */
    void (*pfnUpdateLo)();
    void (*pfnUpdateLoXfWorld)();
    void (*pfnUpdateLoXfWorldHierarchy)();
    void (*pfnFreezeLo)();
    void (*pfnSetLoParent)(LO*, LO*);
    void (*pfnApplyLoProxy)();
    void (*pfnSubscribeLoObject)(LO*, LO*);
    void (*pfnUnsubscribeLoObject)(LO*, LO*);
    void (*pfnSubscribeLoStruct)();
    void (*pfnUnsubscribeLoStruct)();
    void (*pfnGetLoParams)();
    void (*pfnUpdateLoLiveEdit)();
};

/**
 * @brief VT for ALO objects.
 */

struct ALO;
struct VECTOR;
struct MATRIX3;
struct VTALO : VTLO
{
    /* 0x84 */ void (*pfnSetPosition)(ALO* palo, VECTOR* pvecPos);
    /* 0x88 */ void (*pfnSetRotation)(ALO* palo, MATRIX3* pmatRot);
    /* 0x8C */ void* unk8C; // Probably a function pointer, but unknown prototype.
    /* 0x90 */ void (*pfnSetVelocity)(ALO* palo, VECTOR* pvecVel);
    /* 0x94 */ void (*pfnSetAngularVelocity)(ALO* palo, VECTOR* pvecAngVel);
    /* 0x98 */ int padding_vtalo[6]; 
    /* 0xB0 */ void (*pfnUpdatePositionGoal)(ALO* palo, VECTOR* pvecPos);
    /* 0xB4 */ void (*pfnUpdateRotationGoal)(ALO* palo, MATRIX3* pmatRot, VECTOR* pvecVel);
    /* 0xB8 */ int padding_vtalo2[1];
    /* 0xBC */ void (*pfnOnActRetract)(ALO* palo);
};

/*****************************************************************
 * BLOT-DERIVED VTABLES
 *****************************************************************/

struct BLOT;

/**
 * @brief VT for generic blots.
 */
struct VTBLOT
{
    void (*pfnInitBlot)(BLOT *);
    void (*pfnPostBlotLoad)(BLOT *);
    void (*pfnUpdateBlot)(BLOT *);
    void (*pfnOnBlotActive)(BLOT *);
    void (*pfnUpdateBlotActive)(BLOT *);
    void (*pfnOnBlotReset)(BLOT *);
    void (*pfnOnBlotPush)(BLOT *);
    void (*pfnOnBlotPop)(BLOT *);
    void (*pfnSetBlotAchzDraw)(BLOT *, const char *);
    void (*pfnDrawBlot)(BLOT *);
    void (*pfnRenderBlot)(BLOT *);
    void (*pfnDtAppearBlot)(BLOT *);
    void (*pfnDtVisibleBlot)(BLOT *);
    void (*pfnDtDisappearBlot)(BLOT *);
    void (*pfnShowBlot)(BLOT *);
    void (*pfnHideBlot)(BLOT *);
    void (*pfnSetBlotBlots)(BLOT *);
    void (*pfnSetBlotClock)(BLOT *);
    int  (*pfnFIncludeBlotForPeg)(BLOT *);
};

/**
 * @brief VT for the note blot.
 */
struct VTNOTE
{
    void (*pfnInitBlot)(BLOT *);
    void (*pfnPostNoteLoad)(BLOT *);
    void (*pfnUpdateBlot)(BLOT *);
    void (*pfnOnBlotActive)(BLOT *);
    void (*pfnUpdateBlotActive)(BLOT *);
    void (*pfnOnBlotReset)(BLOT *);
    void (*pfnOnBlotPush)(BLOT *);
    void (*pfnOnBlotPop)(BLOT *);
    void (*pfnSetNoteAchzDraw)(BLOT *, const char *);
    void (*pfnDrawNote)(BLOT *);
    void (*pfnRenderBlot)(BLOT *);
    void (*pfnDtAppearBlot)(BLOT *);
    void (*pfnDtVisibleBlot)(BLOT *);
    void (*pfnDtDisappearBlot)(BLOT *);
    void (*pfnShowBlot)(BLOT *);
    void (*pfnHideBlot)(BLOT *);
    void (*pfnSetBlotBlots)(BLOT *);
    void (*pfnSetBlotClock)(BLOT *);
    int  (*pfnFIncludeBlotForPeg)(BLOT *);
};

/**
 * @brief VT for the binoc blot.
 */
struct VTBINOC
{
    void (*pfnInitBinoc)(BLOT *);
    void (*pfnPostBinocLoad)(BLOT *);
    void (*pfnUpdateBlot)(BLOT *);
    void (*pfnOnBinocActive)(BLOT *);
    void (*pfnUpdateBinocActive)(BLOT *);
    void (*pfnOnBinocReset)(BLOT *);
    void (*pfnOnBinocPush)(BLOT *);
    void (*pfnOnBinocPop)(BLOT *);
    void (*pfnSetBinocAchzDraw)(BLOT *, const char *);
    void (*pfnDrawBinoc)(BLOT *);
    void (*pfnRenderBlot)(BLOT *);
    void (*pfnDtAppearBinoc)(BLOT *);
    void (*pfnDtVisibleBlot)(BLOT *);
    void (*pfnDtDisappearBinoc)(BLOT *);
    void (*pfnShowBlot)(BLOT *);
    void (*pfnHideBlot)(BLOT *);
    void (*pfnSetBinocBlots)(BLOT *);
    void (*pfnSetBlotClock)(BLOT *);
    int  (*pfnFIncludeBlotForPeg)(BLOT *);
};

/**
 * @brief VT for the timer blot.
 */
struct VTTIMER
{
    void (*pfnInitBlot)(BLOT *);
    void (*pfnPostTimerLoad)(BLOT *);
    void (*pfnUpdateTimer)(BLOT *);
    void (*pfnOnBlotActive)(BLOT *);
    void (*pfnUpdateBlotActive)(BLOT *);
    void (*pfnOnBlotReset)(BLOT *);
    void (*pfnOnBlotPush)(BLOT *);
    void (*pfnOnBlotPop)(BLOT *);
    void (*pfnSetBlotAchzDraw)(BLOT *, const char *);
    void (*pfnDrawTimer)(BLOT *);
    void (*pfnRenderBlot)(BLOT *);
    void (*pfnDtAppearBlot)(BLOT *);
    void (*pfnDtVisibleBlot)(BLOT *);
    void (*pfnDtDisappearBlot)(BLOT *);
    void (*pfnShowBlot)(BLOT *);
    void (*pfnHideBlot)(BLOT *);
    void (*pfnSetBlotBlots)(BLOT *);
    void (*pfnSetBlotClock)(BLOT *);
    int  (*pfnFIncludeBlotForPeg)(BLOT *);
};

/**
 * @brief VT for the totals blot.
 */
struct VTTOTALS
{
    void (*pfnInitBlot)(BLOT *);
    void (*pfnPostBlotLoad)(BLOT *);
    void (*pfnUpdateBlot)(BLOT *);
    void (*pfnOnBlotActive)(BLOT *);
    void (*pfnUpdateBlotActive)(BLOT *);
    void (*pfnOnBlotReset)(BLOT *);
    void (*pfnOnBlotPush)(BLOT *);
    void (*pfnOnBlotPop)(BLOT *);
    void (*pfnSetBlotAchzDraw)(BLOT *, const char *);
    void (*pfnDrawBlot)(BLOT *);
    void (*pfnRenderBlot)(BLOT *);
    void (*pfnDtAppearBlot)(BLOT *);
    void (*pfnDtVisibleBlot)(BLOT *);
    void (*pfnDtDisappearBlot)(BLOT *);
    void (*pfnShowBlot)(BLOT *);
    void (*pfnHideBlot)(BLOT *);
    void (*pfnSetBlotBlots)(BLOT *);
    void (*pfnSetBlotClock)(BLOT *);
    int (*pfnFIncludeBlotForPeg)(BLOT *);
};

struct GAME;

/**
 * @brief VT for the game struct.
 */
struct VTGAME
{
    // ...
};

struct GOMER;

/**
 * @brief VT for the gomer struct.
 */
struct VTGOMER
{
    // ...
};

/**
 * @brief VT for a waypoint related struct.
 */
struct VTWPSG
{
    // ...
};

struct SAA;
struct SAAF;
struct SAI;
struct ALO;
struct RPL;
struct SHD;
struct SGVR;
struct GLOBSET;
struct GLOB;
struct SUBGLOB;

/**
 * @brief VT for SAA struct.
 */
struct VTSAA
{
    void (*pfnInit)(SAA*, SAAF*);     
    void (*pfnPostLoad)(SAA*);         
    void (*pfnUpdate)(SAA*, float);   
    float (*pfnUComplete)(SAA*);            
    void (*pfnNotifyRender)(SAA*, ALO*, RPL*);                
    SAI* (*pfnPsaiFromSaaShd)(SAA*, SHD*); 
    void (*pfnSetSgvr)(SAA*, SGVR*, GLOBSET*, GLOB*, SUBGLOB*);              
    void* pfnUnk1C;                                 
};

struct CRV;
struct VECTOR;
struct CONSTR;
struct VTCRV {
    void* unk00;                                   
    void (*func04)(CRV*, float, VECTOR*, VECTOR*);  
    void* unk08;
    void* unk0C;
    void* unk10;
    float (*func14)(CRV*, float);
    float (*UFromS)(CRV* pcrv, float dS);
    void* func1C;
    void* func20;
    void (*func24)(CRV* pcrv);
    void* func28;

    void (*FindClosestPointFromU)(
        CRV* pcrv, 
        VECTOR* pvec, 
        float dU, 
        CONSTR* pconstr, 
        VECTOR* pvec2, 
        VECTOR* pvec3, 
        float* pdUOut, 
        float* pdSOut);
};

/**
 * @brief VT for an act related struct.
 */
 struct ACT;
struct VTACT
{
    /* 0x00 */ void (*pfnInit)(ACT* pact, ALO* palo);                       
    /* 0x04 */ void (*pfnClone)(ACT* pactNew, ACT* pactSrc);                 
    /* 0x08 */ void (*pfnRetract)(ACT* pact, int);                           
    /* 0x0C */ void* unk0C;                                                  
    /* 0x10 */ void (*pfnGetPositionGoal)(ACT*, float, VECTOR*, VECTOR*);  
    /* 0x14 */ void (*pfnGetRotationGoal)(ACT*, float, MATRIX3*, VECTOR*);  
    /* 0x18 */ void (*pfnGetTwistGoal)(ACT*, float*, float*);
    /* 0x1C */ void* pad_1C;
    /* 0x20 */ float (*pfnGetPoseTarget)(ACT* pact, int iPose);
};

#endif // VTABLES_H
