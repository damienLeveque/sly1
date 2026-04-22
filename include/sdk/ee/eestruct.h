/**
 * @file include/sdk/ee/eestruct.h
 *
 * @brief Graphics synthesizer.
 */
#ifndef SCE_EESTRUCT_H
#define SCE_EESTRUCT_H

typedef unsigned long u64;

extern "C"
{
        /**
    * @brief GS TEX0 register (0x06 / 0x07).
    *
    * Sets up the current texture buffer for drawing.
    */
    struct sceGsTex0
    {
        /* 0  */ u64 tbp0   : 14;  /* texture buffer base pointer (word/64)      */
        /* 14 */ u64 tbw    :  6;  /* texture buffer width (texels/64)           */
        /* 20 */ u64 psm    :  6;  /* pixel storage mode                         */
        /* 26 */ u64 tw     :  4;  /* log2(texture width)                        */
        /* 30 */ u64 th     :  4;  /* log2(texture height)                       */
        /* 34 */ u64 tcc    :  1;  /* texture color component (0=rgb, 1=rgba)    */
        /* 35 */ u64 tfx    :  2;  /* texture function (modulate/decal/...)      */
        /* 37 */ u64 cbp    : 14;  /* clut buffer base pointer (word/64)         */
        /* 51 */ u64 cpsm   :  4;  /* clut pixel storage mode                    */
        /* 55 */ u64 csm    :  1;  /* clut storage mode (0=CSM1, 1=CSM2)         */
        /* 56 */ u64 csa    :  5;  /* clut entry offset                          */
        /* 61 */ u64 cld    :  3;  /* clut buffer load control                   */
    };

    /**
    * @brief GS TEX2 register (0x16 / 0x17).
    *
    * Sets only the CLUT-related fields of the texture state, leaving
    * texture buffer parameters untouched. Uses the same bit positions
    * as TEX0 for the fields it does set.
    */
    struct sceGsTex2
    {
        /* 0  */ u64 pad0   : 20; /* unused */
        /* 20 */ u64 psm    :  6; /* pixel storage mode                         */
        /* 26 */ u64 pad1   : 11; /* unused */
        /* 37 */ u64 cbp    : 14; /* clut buffer base pointer (word/64)         */
        /* 51 */ u64 cpsm   :  4; /* clut pixel storage mode                    */
        /* 55 */ u64 csm    :  1; /* clut storage mode (0=CSM1, 1=CSM2)         */
        /* 56 */ u64 csa    :  5; /* clut entry offset                          */
        /* 61 */ u64 cld    :  3; /* clut buffer load control                   */
    };

    /**
     * @todo Figure out is this in the right place.
     */
    void sceGsResetPath();
}

#endif // SCE_EESTRUCT_H
