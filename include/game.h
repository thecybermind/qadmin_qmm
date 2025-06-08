/*
QADMIN_QMM - Server Administration Plugin
Copyright 2004-2025
https://github.com/thecybermind/qadmin_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < cybermind@gmail.com >

*/

#ifndef __QADMIN_QMM_GAME_H__
#define __QADMIN_QMM_GAME_H__

#if defined(GAME_Q3A)
    #include <q3a/game/g_local.h>
#elif defined(GAME_RTCWMP)
    #include <rtcwmp/game/g_local.h>
#elif defined(GAME_RTCWSP)
    #include <rtcwsp/game/g_local.h>
#elif defined(GAME_JK2MP)
    #include <jk2mp/game/g_local.h>
#elif defined(GAME_JAMP)
    #include <jamp/game/g_local.h>
#elif defined(GAME_WET)
    #include <wet/game/g_local.h>
#elif defined(GAME_STVOYHM)
    #include <stvoyhm/game/g_local.h>
#elif defined(GAME_STEF2)
    #include <stef2/game/g_local.h>
    #include <stef2/game/bg_public.h>
    #include <game_stef2.h>
    #define GAME_NO_DROP_CLIENT
#elif defined(GAME_MOHAA)
    #include <mohaa/fgame/g_local.h>
    #include <mohaa/fgame/bg_public.h>
    #include <game_mohaa.h>
    #define GAME_NO_DROP_CLIENT
    #define GAME_NO_FS_GETFILELIST
    #define GAME_CLIENT_USERINFO_CHANGED GAME_CLIENT_USERINFOCHANGED
#elif defined(GAME_MOHSH)
    #include <mohsh/fgame/g_local.h>
    #include <mohsh/fgame/bg_public.h>
    #include <game_mohsh.h>
    #define GAME_NO_DROP_CLIENT
    #define GAME_NO_FS_GETFILELIST
    #define GAME_CLIENT_USERINFO_CHANGED GAME_CLIENT_USERINFOCHANGED
#elif defined(GAME_MOHBT)
    #include <mohaa/fgame/g_local.h>
    #include <mohaa/fgame/bg_public.h>
    #include <game_mohbt.h>
    #define GAME_NO_DROP_CLIENT
    #define GAME_NO_FS_GETFILELIST
    #define GAME_CLIENT_USERINFO_CHANGED GAME_CLIENT_USERINFOCHANGED
#elif defined(GAME_QUAKE2)
    #include <quake2/game/g_local.h>
    #include <game_quake2.h>
    #define GAME_NO_NAME_COLOR
    #define GAME_NO_SEND_SERVER_COMMAND
    #define GAME_NO_DROP_CLIENT
    #define GAME_NO_FS_GETFILELIST
    #define GAME_NO_GET_USERINFO
    #define GAME_NO_LEVELTIME
    typedef edict_t gentity_t;
#elif defined(GAME_Q2R)
    #include <q2r/rerelease/g_local.h>
    #include <game_q2r.h>
    #define GAME_NO_NAME_COLOR
    #define GAME_NO_SEND_SERVER_COMMAND
    #define GAME_NO_DROP_CLIENT
    #define GAME_NO_FS_GETFILELIST
    #define GAME_NO_GET_USERINFO
    #define GAME_NO_LEVELTIME
    #define G_CPRINTF G_CLIENT_PRINT
    typedef edict_t gentity_t;
#elif defined(GAME_SOF2MP)
    #include <sof2mp/game/g_local.h>
#endif

#endif // __QADMIN_QMM_GAME_H__
