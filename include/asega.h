/**
 * @file asega.h
 */
#ifndef ASEGA_H
#define ASEGA_H

#include "common.h"
#include <basic.h>
#include <aseg.h>
#include <dl.h>

// Forward.
struct ACTSEG;

/**
 * @class ANIMATION_SEGMENT_APPLICATION
 * @brief Unknown.
 */
struct ASEGA : public BASIC
{
    /* 0x08 */ ASEG* paseg;
    /* 0x0c */ int pad_0c[2];
    /* 0x14 */ float tStart; 
    /* 0x18 */ float tEnd;   
    /* 0x1C */ int pad_1c[2];
    /* 0x24 */ char fHandsOff;
    /* 0x25 */ uchar unk1;
    /* 0x26 */ uchar unk2;
    /* 0x27 */ uchar unk3;
    /* 0x28 */ ACTSEG *pactsegError;
    /* 0x2c */ int padding_5[6];
    /* 0x44 */ DL dlActseg;
    // ...
};

// ...

void RetractAsega(ASEGA *pasega);

#endif // ASEGA_H
