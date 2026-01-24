/*
QADMIN_QMM - Server Administration Plugin
Copyright 2004-2026
https://github.com/thecybermind/qadmin_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < k.m.masterson@gmail.com >

*/

#ifndef QADMIN_QMM_GAME_H
#define QADMIN_QMM_GAME_H

#if defined(GAME_COD11MP)
    #include <cod11mp/bgame/bg_local.h>
    #include <cod11mp/bgame/bg_public.h>
    #include <cod11mp/game/g_public.h>
    #define GAME_NO_NAME_COLOR
#elif defined(GAME_CODMP)
    #include <codmp/bgame/bg_local.h>
    #include <codmp/bgame/bg_public.h>
    #include <codmp/game/g_public.h>
    #define GAME_NO_NAME_COLOR
#elif defined(GAME_CODUOMP)
    #include <coduomp/bgame/bg_local.h>
    #include <coduomp/bgame/bg_public.h>
    #include <coduomp/game/g_public.h>
    #define GAME_NO_NAME_COLOR
#elif defined(GAME_JAMP)
    #include <jamp/game/g_local.h>
#elif defined(GAME_JASP)
    #include <jasp/game/q_shared.h>
    #include <jasp/game/g_local.h>
    #include <jasp/game/bg_public.h>
    #include <game_jasp.h>
#elif defined(GAME_JK2MP)
    #include <jk2mp/game/g_local.h>
#elif defined(GAME_JK2SP)
    #include <jk2sp/game/q_shared.h>
    #include <jk2sp/game/g_local.h>
    #include <jk2sp/game/bg_public.h>
    #include <game_jk2sp.h>
#elif defined(GAME_MOHAA)
    #include <mohaa/fgame/g_local.h>
    #include <mohaa/fgame/bg_public.h>
    #include <game_mohaa.h>
    #define GAME_NO_FS_GETFILELIST
#elif defined(GAME_MOHBT)
    #include <mohaa/fgame/g_local.h>
    #include <mohaa/fgame/bg_public.h>
    #include <game_mohbt.h>
    #define GAME_NO_FS_GETFILELIST
#elif defined(GAME_MOHSH)
    #include <mohsh/fgame/g_local.h>
    #include <mohsh/fgame/bg_public.h>
    #include <game_mohsh.h>
    #define GAME_NO_FS_GETFILELIST
#elif defined(GAME_QUAKE2)
    #include <quake2/game/g_local.h>
    #include <game_quake2.h>
    #define GAME_NO_NAME_COLOR
    #define GAME_CLIENT_ENT_PTRS
    #define GAME_NO_SEND_SERVER_COMMAND
    #define GAME_NO_FS_GETFILELIST
#elif defined(GAME_Q2R)
    #include <q2r/rerelease/g_local.h>
    #include <game_q2r.h>
    #define GAME_NO_NAME_COLOR
    #define GAME_CLIENT_ENT_PTRS
    #define GAME_NO_SEND_SERVER_COMMAND
    #define GAME_NO_FS_GETFILELIST
#elif defined(GAME_Q3A)
    #include <q3a/game/g_local.h>
#elif defined(GAME_RTCWMP)
    #include <rtcwmp/game/g_local.h>
#elif defined(GAME_RTCWSP)
    #include <rtcwsp/game/g_local.h>
#elif defined(GAME_SIN)
    #include <sin/game/g_local.h>
    #include <game_sin.h>
    #define GAME_NO_NAME_COLOR
    #define GAME_CLIENT_ENT_PTRS
    #define GAME_NO_SEND_SERVER_COMMAND
    #define GAME_NO_FS_GETFILELIST
#elif defined(GAME_SOF2MP)
    #include <sof2mp/game/g_local.h>
#elif defined(GAME_STEF2)
    #include <stef2/game/g_local.h>
    #include <stef2/game/bg_public.h>
    #include <game_stef2.h>
#elif defined(GAME_STVOYHM)
    #include <stvoyhm/game/g_local.h>
#elif defined(GAME_STVOYSP)
    #include <stvoysp/game/q_shared.h>
    #include <stvoysp/game/g_local.h>
    #include <stvoysp/game/bg_public.h>
    #include <game_stvoysp.h>
#elif defined(GAME_WET)
    #include <wet/game/g_local.h>
#endif

#endif // QADMIN_QMM_GAME_H
