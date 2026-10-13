/*
QADMIN_QMM - Server Administration Plugin
Copyright 2004-2026
https://github.com/thecybermind/qadmin_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < k.m.masterson@gmail.com >

*/

#ifndef QADMIN_QMM_CMD_H
#define QADMIN_QMM_CMD_H

#include <vector>
#include <string>

#include <qmmapi.h>

#include "main.h"

typedef int (*pfnAdminCmd)(result_vars* results, intptr_t clientnum, int access, std::vector<std::string> args, bool say);		// signature of a command handler

// command handler info
typedef struct {
	const char* cmd;
	pfnAdminCmd func;
	int reqaccess;
	int minargs;
	const char* usage;
	const char* help;
} cmd_info;

extern std::vector<cmd_info> g_admincmds;
extern std::vector<cmd_info> g_saycmds;

void reload();
int handlecommand(result_vars* results, intptr_t clientnum, std::vector<std::string> args);
int admin_adduser(result_vars* results, addusertype type, std::vector<std::string> args);

#endif // QADMIN_QMM_UTIL_H
