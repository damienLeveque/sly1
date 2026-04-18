/**
 * @file include/gcc/vu0.h
 *
 * @brief VU0 macro-mode inline assembly helpers.
 */
#ifndef VU0_H
#define VU0_H

#define VU0_LOAD_VECTOR(v) \
    asm volatile("lqc2 $vf1, 0x0(%0)\n" : : "r"(v))

#define VU0_SCRATCH_REG register int tmp asm("v1")

#define VU0_DOT_XYZ(dot, threshold) \
    asm volatile( \
        "vmul.xyz $vf1,$vf1,$vf1\n" \
        "vaddw.x $vf2,$vf0,$vf0w\n" \
        "vadday.x ACC,$vf1,$vf1y\n" \
        "vmaddz.x  $vf1,$vf2,$vf1z\n"\
        ".set noat\n" \
        "lui $1, 0x4080\n" \
        "mtc1 $1, %1\n" \
        ".set at\n" \
        "qmfc2.ni %0, $vf1\n" \
        "mtc1 %0, %2\n" \
        : "=r"(tmp), "=f"(threshold), "=f"(dot) \
        : \
    )

#define VU0_DOT_XYZ_EPS(dot, threshold) \
    asm volatile( \
        "vmul.xyz $vf1,$vf1,$vf1\n" \
        "vaddw.x $vf2,$vf0,$vf0w\n" \
        "vadday.x ACC,$vf1,$vf1y\n" \
        "vmaddz.x  $vf1,$vf2,$vf1z\n"\
        ".set noat\n" \
        "lui $1, 0x39D1\n" \
        "ori $1, $1, 0xB717\n" \
        "mtc1 $1, %1\n" \
        ".set at\n" \
        "qmfc2.ni %0, $vf1\n" \
        "mtc1 %0, %2\n" \
        : "=r"(tmp), "=f"(threshold), "=f"(dot) \
        : \
    )

#endif // VU0_H