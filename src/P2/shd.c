#include <shd.h>
#include <gs.h>
#include <shdanim.h>
#include <memory.h>
#include <types.h>
#include <sce/memset.h>
#include <sdk/ee/eestruct.h>

sceGsTex0 Tex0FromTexIframeCtk(TEX *ptex, int iframe, CTK ctk)
{
    SHD  *pshd  = ptex->pshd;
    BMP  *pbmp  = ptex->apbmp[iframe];
    int   cctk  = (pshd->shdk == 0) ? 3 : 1;
    CLUT *pclut = ptex->apclut[iframe * cctk + ctk];

    sceGsTex0 tex0 = pbmp->tex0;
    if (pclut != 0) {
        *(u64 *)&tex0 |= *(u64 *)&pclut->tex2;
    }
    return tex0;
}

void PackTexGifs(TEX *tex, int iframe, CTK ctk, SHDK shdk, GIFS *gifs)
{
    u64 reg0;
    u64 reg1;
    u64 reg2;
    u64 reg3;
    u64 reg4;
    sceGsTex0 tex0s;
    u64 tex0;
    int tmp;

    tmp = shdk ^ 5;

    memset(&reg0, 0, 8);
    memset(&reg1, 0, 8);
    memset(&reg2, 0, 8);
    memset(&reg3, 0, 8);
    memset(&reg4, 0, 8);

    tex0s = Tex0FromTexIframeCtk(tex, iframe, ctk);
    tex0 = *(u64 *)&tex0s;

    reg0 = tex0;
    gifs->PackAD(tmp ? 6 : 7, reg0);

    reg1 |= 1;
    reg1 |= 0x20;
    reg1 &= (u64)-0x1C1;
    reg1 |= 0x40;
    gifs->PackAD(tmp ? 0x14 : 0x15, reg1);

    {
        unsigned short flags = *(unsigned short *)((char *)tex + 2);
        if (flags & 1) {
            reg4 &= (u64)-4;
            reg4 |= 1;
        }
    }

    {
        unsigned short flags = *(unsigned short *)((char *)tex + 2);
        if (flags & 2) {
            reg4 &= (u64)-0xD;
            reg4 |= 4;
        }
    }

    gifs->PackAD(tmp ? 8 : 9, reg4);
}

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

void LoadTexFromBrx(CBinaryInputStream *pbis, TEX *ptex)
{
    int i;

    pbis->Read(6, ptex);

    if (ptex->cibmp != 0) {
        ptex->apbmp = (BMP **)PvAllocSwImpl(ptex->cibmp * 4);

        for (i = 0; i < ptex->cibmp; i++) {
            unsigned short ibmp = pbis->U16Read();
            ptex->apbmp[i] = (BMP *)((char *)D_00274504 + (ibmp << 5));
        }
    }

    if (ptex->ciclut != 0) {
        ptex->apclut = (CLUT **)PvAllocSwImpl(ptex->ciclut * 4);

        for (i = 0; i < ptex->ciclut; i++) {
            unsigned short iclut = pbis->U16Read();

            if (iclut == 0xFFFF) {
                ptex->apclut[i] = 0;
            } else {
                ptex->apclut[i] = (CLUT *)((char *)D_0027450C + iclut * 0x18);
            }
        }
    }
}

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

void UnloadShaders()
{
    D_002744F8 = 0;
    D_002744FC = 0;
    D_00274500 = 0;
    D_00274504 = 0;
    D_00274508 = 0;
    D_0027450C = 0;
    g_pfont = 0;
    D_00262260 = 0;
    D_00262264 = 0;

    memset(D_00262268, 0, 0x14);
    g_grfzonShaders = 0;
    D_0027453C = 0;
    D_00274540 = 0;
    D_0027451C = 0;
    D_00274544 = 0;
    
    memset(D_00274520, 0, 0x18);
    g_aqwGifsBackgroundUpload = 0;
    ResetGsMemory();
}

JUNK_WORD(0xE4A00008);

INCLUDE_ASM("asm/nonmatchings/P2/shd", ConvertRgbToHsv__FP6VECTORT0);

INCLUDE_ASM("asm/nonmatchings/P2/shd", ConvertHsvToRgb__FP6VECTORT0);

INCLUDE_ASM("asm/nonmatchings/P2/shd", ConvertUserHsvToUserRgb__FP6VECTORT0);

INCLUDE_ASM("asm/nonmatchings/P2/shd", ConvertUserRgbToUserHsv__FP6VECTORT0);

JUNK_ADDIU(10);
JUNK_NOP();
JUNK_NOP();
JUNK_ADDIU(50);

SHD *PshdFindShader(OID oid)
{
    int i;
    
    if (oid == -1)
        return 0;
    
    i = 0;

    if (D_002744F8 > 0) {
        SHD *pshd = D_002744FC;    
        for (; i < D_002744F8; i++) {
            if (pshd->oid == oid)
                return pshd;
            pshd++;
        }
    }

    return 0;
}

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



void SetSaiDuDv(SAI *psai, float du, float dv)
{
    if (psai->pshd == 0)
        return;

    if (psai->txt.du == du && psai->txt.dv == dv)
        return;

    psai->txt.du = du;
    psai->txt.dv = dv;

    if (psai->psaiNext != 0)
        return;

    if (psai == D_00274544)
        return;

    if (D_00274544 == 0)
        D_00274544 = psai;

    psai->psaiNext = D_0027451C;
    D_0027451C = psai;
}
void PropagateSais(void)
{
    SAI *psai;

    psai = D_0027451C;
    if (psai != 0) {
        do {
            SAIR *psair = psai->psairFirst;
            if (psair != 0) {
                do {
                    if (psai->grfsai & 1) {
                        SHDP *pshdp = psair->pshdp;
                        psair->psur->pvSrc =
                            (char *)pshdp->aaqwRegs +
                            (psai->iframe * pshdp->cqwRegs * 0x10);
                    }

                    PropagateSur(psair->psur);
                    psair = psair->psairNext;
                } while (psair != 0);
            }

            {
                SAI *next = psai->psaiNext;
                psai->psaiNext = 0;
                psai = next;
            }
        } while (psai != 0);
    }

    D_0027451C = 0;
    D_00274544 = 0;
}

void UpdateShaders(float dt)
{
    int i;

    i = 0;
    if (D_0027453C > 0) {
        do {
            SAA *psaa = D_00274540[i];

            if (FUpdatableSaa(psaa) != 0) {
                if (psaa->pvtsaa->pfnUpdate != 0)
                    psaa->pvtsaa->pfnUpdate(psaa, dt);
            }

            i += 1;
        } while (i < D_0027453C);
    }
}
