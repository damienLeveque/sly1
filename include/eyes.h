/**
 * @file eyes.h
 */
#ifndef EYES_H
#define EYES_H

#include "common.h"
#include <shd.h>

/**
 * @brief Eyes state.
 */
enum EYESS
{
    EYESS_Nil = -1,
    EYESS_Open = 0,
    EYESS_Closing = 1,
    EYESS_Closed = 2,
    EYESS_Opening = 3,
    EYESS_Max = 4
};

/**
 * @brief Eyes.
 */
struct EYES : public SAA
{
    float dtLoopMin;  // 0x2C - used as a divisor for animation speed
    float dtLoopMax;  // 0x30 - used for random range
    float dtPauseMin; // 0x34 - used for random range
    float dtPauseMax; // 0x38 - used as probability threshold
    OID oid;          // 0x3C
    SAI saiBlink;     // 0x40
    int cmaxFrame;    // 0x5C
    EYESS eyess;      // 0x60
    float tState;     // 0x64 - Timestamp of state change
    float dtBlink;    // 0x68 - Random delay timer
    float viframe;    // 0x6C - Animation velocity/step
    float iframe;     // 0x70 - Current animation frame
    float uOpen;      // 0x74 - 0.0 to 1.0 "Openness"
};   

void InitEyes(EYES *peyes, SAAF *psaaf);

void PostEyesLoad(EYES *peyes);

void SetEyesEyess(EYES *peyes, EYESS eyess);

void UpdateEyes(EYES *peyes, float dt);

void SetEyesClosed(EYES *peyes, float uClosed);

SAI *PsaiFromEyesShd(EYES *peyes, SHD *pshd);

#endif // EYES_H
