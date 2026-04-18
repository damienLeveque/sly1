#include <shd.h>
#include <gs.h>

extern GRFZON g_grfzonShaders;
extern byte *g_pbBulkData;

INCLUDE_ASM("asm/nonmatchings/P2/shd", Tex0FromTexIframeCtk__FP3TEXi3CTK);

INCLUDE_ASM("asm/nonmatchings/P2/shd", PackTexGifs__FP3TEXi3CTK4SHDKP4GIFS);

void LoadClutFromBrx(CBinaryInputStream *pbis, CLUT *pclut)
{
    pbis->Read(0xc, pclut);
    pclut->prgba = (RGBA *)(g_pbBulkData + (uint)pclut->prgba);
    pclut->cqwColors = (uint)pclut->crgba * 4 + 0xf >> 4;
}

INCLUDE_ASM("asm/nonmatchings/P2/shd", LoadColorTablesFromBrx__FP18CBinaryInputStream);

void LoadBmpFromBrx(CBinaryInputStream *pbis, BMP *pbmp)
{
    pbis->Read(0x14, pbmp);
    pbmp->pbPixels = (byte *)(g_pbBulkData + (int)pbmp->pbPixels);
    pbmp->cqwPixels = pbmp->cbPixels + 0xf >> 4;
}

INCLUDE_ASM("asm/nonmatchings/P2/shd", LoadBitmapsFromBrx__FP18CBinaryInputStream);

INCLUDE_ASM("asm/nonmatchings/P2/shd", LoadFontsFromBrx__FP18CBinaryInputStream);

INCLUDE_ASM("asm/nonmatchings/P2/shd", LoadTexFromBrx__FP18CBinaryInputStreamP3TEX);

INCLUDE_ASM("asm/nonmatchings/P2/shd", LoadShadersFromBrx__FP18CBinaryInputStream);

void UploadPermShaders()
{
    GSB gsbPerm;
    InitGsb(&gsbPerm, 0x1e00, 0x4000);
    UploadBitmaps(0x10000000, &gsbPerm);
    g_grfzonShaders = 0;
}

INCLUDE_ASM("asm/nonmatchings/P2/shd", PropagateShaders__Fi);

INCLUDE_ASM("asm/nonmatchings/P2/shd", FillShaders__Fi);

INCLUDE_ASM("asm/nonmatchings/P2/shd", UnloadShaders__Fv);

JUNK_WORD(0xE4A00008);

INCLUDE_ASM("asm/nonmatchings/P2/shd", ConvertRgbToHsv__FP6VECTORT0);

INCLUDE_ASM("asm/nonmatchings/P2/shd", ConvertHsvToRgb__FP6VECTORT0);

INCLUDE_ASM("asm/nonmatchings/P2/shd", ConvertUserHsvToUserRgb__FP6VECTORT0);

INCLUDE_ASM("asm/nonmatchings/P2/shd", ConvertUserRgbToUserHsv__FP6VECTORT0);

JUNK_ADDIU(10);
JUNK_NOP();
JUNK_NOP();
JUNK_ADDIU(50);

INCLUDE_ASM("asm/nonmatchings/P2/shd", PshdFindShader__F3OID);

void SetSaiIframe(SAI *psai, int iframe)
{
    int iframeMax;

    if (psai->pshd == 0)
        return;

    iframeMax = psai->pshd->cframe - 1;

    if (iframe < 0)
        iframe = 0;
    else if (iframeMax < iframe)
        iframe = iframeMax;

    if (psai->iframe == iframe)
        return;

    psai->iframe = iframe;

    if (psai->psaiNext != 0)
        return;

    if (psai == D_00274544)
        return;

    if (D_00274544 == 0)
        D_00274544 = psai;

    psai->psaiNext = D_0027451C;
    D_0027451C = psai;
}

INCLUDE_ASM("asm/nonmatchings/P2/shd", SetSaiDuDv__FP3SAIff);

INCLUDE_ASM("asm/nonmatchings/P2/shd", PropagateSais__Fv);

INCLUDE_ASM("asm/nonmatchings/P2/shd", UpdateShaders__Ff);
