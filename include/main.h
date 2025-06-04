/*
QADMIN_QMM - Server Administration Plugin
Copyright 2004-2025
https://github.com/thecybermind/qadmin_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < cybermind@gmail.com >

*/

#ifndef __QADMIN_QMM_MAIN_H__
#define __QADMIN_QMM_MAIN_H__

#define MAX_STRING_LENGTH 1024
#define MAX_DATA_LENGTH 200
#define MAX_COMMAND_LENGTH 64
#define MAX_NUMBER_LENGTH 11

#define MAX_USER_ENTRIES 64

// MAX_NETNAME = (35 + 1)
#define MAX_USER_LENGTH MAX_NETNAME
#define MAX_GUID_LENGTH 32 + 1
#define MAX_PASS_LENGTH 32 + 1
#define MAX_IP_LENGTH 15 + 1

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

typedef int (*pfnAdminCmd)(int,int,int);		// signature of a command handler

// command handler info
typedef struct admincmd_s {
	const char* cmd;
	pfnAdminCmd func;
	int reqaccess;
	int minargs;
	const char* usage;
	const char* help;
} admincmd_t;

extern admincmd_t g_admincmds[];
extern admincmd_t g_saycmds[];

typedef struct playerinfo_s {
	char guid[MAX_GUID_LENGTH];
	char ip[MAX_IP_LENGTH];
	char name[MAX_NETNAME];
	char stripname[MAX_NETNAME];
	int access;
	bool authed;
	bool gagged;
	bool connected;
} playerinfo_t;
extern playerinfo_t g_playerinfo[];

typedef struct userinfo_s {
	char user[MAX_USER_LENGTH];
	char pass[MAX_PASS_LENGTH];
	int access;
	addusertype_t type;
} userinfo_t;
#define usertype(x) (x==au_ip?"IP":(x==au_name?"name":"ID"))

extern userinfo_t g_userinfo[];
extern intptr_t g_maxuserinfo;

extern intptr_t g_defaultAccess;

extern time_t g_mapstart;
extern intptr_t g_levelTime;

extern char** g_gaggedCmds;

void reload();
int handlecommand(int,int);
int admin_adduser(addusertype_t type);

#endif // __QADMIN_QMM_MAIN_H__
