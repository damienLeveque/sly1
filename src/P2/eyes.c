#include <eyes.h>
#include <shd.h>
#include <shdanim.h>
#include <vtables.h>
#include <oid.h>


void InitEyes(EYES *peyes, SAAF *psaaf) {
    InitSaa(peyes, psaaf);

    peyes->saiBlink.grfsai = peyes->sai.grfsai;

    peyes->oid = (OID)(short)psaaf->dframe; 

    peyes->dtLoopMin  = psaaf->dtLoopMin;
    peyes->dtLoopMax  = psaaf->dtLoopMax;
    peyes->dtPauseMin = psaaf->dtPauseMin;
    peyes->dtPauseMax = psaaf->dtPauseMax;
}

void PostEyesLoad(EYES *peyes)
{
    PostSaaLoad(peyes);

    peyes->saiBlink.pshd = PshdFindShader__F3OID(peyes->oid);

    if (peyes->saiBlink.pshd != NULL)
    {
        if (peyes->saiBlink.pshd->psaa == NULL)
        {
            if (peyes->sai.pshd->psaa == (SAA*)peyes)
                peyes->saiBlink.pshd->psaa = (SAA*)peyes;
        }
    }

    int cframeSai = 0;
    SHD *pshdSai = peyes->sai.pshd;
    if (pshdSai != NULL)
        cframeSai = pshdSai->cframe;

    int cframeBlink = 0;
    SHD *pshdBlink = peyes->saiBlink.pshd;
    if (pshdBlink != NULL)
        cframeBlink = pshdBlink->cframe;

    peyes->eyess = EYESS_Nil;
    
    peyes->cmaxFrame = (cframeBlink < cframeSai) ? cframeSai : cframeBlink;

    SetEyesEyess(peyes, EYESS_Open);
}

void SetEyesEyess(EYES *peyes, EYESS eyess)
{
    if (peyes->eyess == eyess)
        return;

    switch (eyess)
    {
    case EYESS_Open:
        peyes->iframe = (float)(peyes->cmaxFrame - 1) * peyes->uOpen;
        peyes->viframe = ((float)(peyes->cmaxFrame * 2) * (1.0f - peyes->uOpen)) / peyes->dtLoopMin;

        if (GRandInRange(0.0f, 1.0f) < peyes->dtPauseMax)
            peyes->dtBlink = 0.0f;
        else
            peyes->dtBlink = GRandInRange(peyes->dtLoopMax, peyes->dtPauseMin);
        break;

    case EYESS_Closed:
        if ((int)peyes->iframe != peyes->cmaxFrame - 1)
            peyes->iframe = (float)(peyes->cmaxFrame - 1);
        break;

    case EYESS_Closing:
    case EYESS_Opening:
        break;
    }

    peyes->eyess = eyess;
    peyes->tState = g_clock.t;
}

void UpdateEyes(EYES *peyes, float dt)
{
    int cmaxFrame = peyes->cmaxFrame;
    int eyess;

    if (cmaxFrame < 2)
        return;

    eyess = peyes->eyess;

    switch (eyess)
    {
    case EYESS_Open:
        if (peyes->dtBlink <= g_clock.t - peyes->tState)
            eyess = EYESS_Closing;
        break;

    case EYESS_Closing:
        peyes->iframe = peyes->iframe + peyes->viframe * dt;
        if ((float)cmaxFrame <= peyes->iframe)
        {
            peyes->iframe = peyes->iframe - peyes->viframe * dt;
            eyess = EYESS_Opening;
            if ((int)peyes->iframe == cmaxFrame - 1)
                eyess = EYESS_Closed;
        }
        break;

    case EYESS_Closed:
        if (peyes->uOpen < 1.0f)
            eyess = EYESS_Opening;
        break;

    case EYESS_Opening:
        peyes->iframe = peyes->iframe - peyes->viframe * dt;
        if (peyes->iframe <= (float)cmaxFrame * peyes->uOpen)
            eyess = EYESS_Open;
        break;
    }

    SetEyesEyess(peyes, (EYESS)eyess);
    SetSaiIframe(&peyes->sai, (int)peyes->iframe);
    SetSaiIframe(&peyes->saiBlink, (int)peyes->iframe);
}

void SetEyesClosed(EYES *peyes, float uOpen)
{
    peyes->uOpen = uOpen;

    if (1.0f <= uOpen)
    {
        SetEyesEyess(peyes, EYESS_Closed);
    }
    else
    {
        peyes->eyess = EYESS_Nil;
        SetEyesEyess(peyes, EYESS_Open);
    }

    SetSaiIframe(&peyes->sai, (int)peyes->iframe);
    SetSaiIframe(&peyes->saiBlink, (int)peyes->iframe);
}

SAI *PsaiFromEyesShd(EYES *peyes, SHD *pshd)
{
    SAI *psai = PsaiFromSaaShd((SAA *)peyes, pshd);

    if (psai == 0)
    {
        psai = &peyes->saiBlink;
        if (pshd->oid != peyes->oid)
            psai = 0;
    }

    return psai;
}