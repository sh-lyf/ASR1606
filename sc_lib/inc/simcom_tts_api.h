#ifndef __SIMCOM_TTS_API_H__
#define __SIMCOM_TTS_API_H__
#ifdef FEATURE_SIMCOM_TTS
#include "simcom_os.h"
#ifdef CUS_GWSD
#include "simcom_gwsd_tts.h"
#else
#include "simcom_tts.h"
#endif

BOOL sAPI_TTSSetParameters(UINT8 volume,UINT8 sysVolume,UINT8 digitMode,UINT8 pitch,UINT8 speed);
#endif
#endif
