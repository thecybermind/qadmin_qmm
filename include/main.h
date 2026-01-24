/*
QADMIN_QMM - Server Administration Plugin
Copyright 2004-2026
https://github.com/thecybermind/qadmin_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < k.m.masterson@gmail.com >

*/

#ifndef QADMIN_QMM_MAIN_H
#define QADMIN_QMM_MAIN_H

#include <vector>
#include <map>
#include <string>

#include "game.h"

#ifdef MAX_STRING_LENGTH
#undef MAX_STRING_LENGTH
#endif
#define MAX_STRING_LENGTH 1024
#define MAX_DATA_LENGTH 200
#define MAX_COMMAND_LENGTH 64
#define MAX_NUMBER_LENGTH 11

#define SERVER_CONSOLE -2

typedef enum addusertype_e {
	au_ip = 1,
	au_name = 2,
	au_id = 3
} addusertype_t;

#ifdef WIN32
 #define strcasecmp stricmp
#endif

#define LEVEL_0		0
#define LEVEL_1		(1<<0)
#define LEVEL_2		(1<<1)
#define LEVEL_4		(1<<2)
#define LEVEL_8		(1<<3)
#define LEVEL_16	(1<<4)
#define LEVEL_32	(1<<5)
#define LEVEL_64	(1<<6)
#define LEVEL_128	(1<<7)
#define LEVEL_256	(1<<8)
#define LEVEL_512	(1<<9)
#define LEVEL_1024	(1<<10)
#define LEVEL_2048	(1<<11)
#define LEVEL_4096	(1<<12)
#define LEVEL_8192	(1<<13)
#define LEVEL_16384	(1<<14)
#define LEVEL_32768	(1<<15)
#define LEVEL_65536	(1<<16)
#define LEVEL_131072	(1<<17)

#define ACCESS_IMMUNITY	LEVEL_1024

typedef struct playerinfo_s {
	std::string guid;
	std::string ip;
	std::string name;
	std::string stripname;
	int access = 0;
	bool authed = false;
	bool gagged = false;
} playerinfo_t;

typedef struct userinfo_s {
	std::string user;
	std::string pass;
	int access;
	addusertype_t type;
} userinfo_t;

extern std::map<intptr_t, playerinfo_t> g_playerinfo;
extern std::vector<userinfo_t> g_userinfo;

extern time_t g_mapstart;
extern time_t g_leveltime;

extern std::vector<std::string> g_gaggedCmds;

#endif // QADMIN_QMM_MAIN_H
