/*
QADMIN_QMM - Server Administration Plugin
Copyright 2004-2025
https://github.com/thecybermind/qadmin_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < cybermind@gmail.com >

*/

#ifndef __QADMIN_QMM_CMD_H__
#define __QADMIN_QMM_CMD_H__

#include <vector>
#include <string>

#include "main.h"

typedef int (*pfnAdminCmd)(int clientnum, int access, std::vector<std::string> args, bool say);		// signature of a command handler

// command handler info
typedef struct admincmd_s {
	const char* cmd;
	pfnAdminCmd func;
	int reqaccess;
	int minargs;
	const char* usage;
	const char* help;
} admincmd_t;

extern std::vector<admincmd_t> g_admincmds;
extern std::vector<admincmd_t> g_saycmds;

void reload();
int handlecommand(int clientnum, std::vector<std::string> args);
int admin_adduser(addusertype_t type, std::vector<std::string> args);

#endif // __QADMIN_QMM_UTIL_H__
