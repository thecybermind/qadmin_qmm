/*
QADMIN_QMM - Server Administration Plugin
Copyright 2004-2025
https://github.com/thecybermind/qadmin_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < cybermind@gmail.com >

*/

#define _CRT_SECURE_NO_WARNINGS 1

#include <qmmapi.h>

#include "version.h"
#include "game.h"

#include <string.h>
#include <time.h>
#include "main.h"
#include "vote.h"
#include "util.h"

pluginres_t* g_result = NULL;
plugininfo_t g_plugininfo = {
	QMM_PIFV_MAJOR,									// plugin interface version major
	QMM_PIFV_MINOR,									// plugin interface version minor
	"QAdmin",										// name of plugin
	QADMIN_QMM_VERSION,								// version of plugin
	"Administration Plugin",						// description of plugin
	QADMIN_QMM_BUILDER,								// author of plugin
	"https://github.com/thecybermind/qadmin_qmm/",	// website of plugin
};
eng_syscall_t g_syscall = NULL;
mod_vmMain_t g_vmMain = NULL;
pluginfuncs_t* g_pluginfuncs = NULL;
pluginvars_t* g_pluginvars = NULL;

#ifdef GAME_NO_SEND_SERVER_COMMAND
gentity_t* g_gents = NULL;
int g_gentsize = sizeof(gentity_t);
#endif

playerinfo_t g_playerinfo[MAX_CLIENTS];		// store qadmin-specific user info

userinfo_t g_userinfo[MAX_USER_ENTRIES];	// store user/pass data
intptr_t g_maxuserinfo = -1;

int g_defaultAccess = 1;

time_t g_mapstart;
int g_levelTime;

char** g_gaggedCmds = NULL;

// first function called in plugin, give QMM the plugin info
C_DLLEXPORT void QMM_Query(plugininfo_t** pinfo) {
	QMM_GIVE_PINFO();
}

// second function called, save pointers
C_DLLEXPORT int QMM_Attach(eng_syscall_t engfunc, mod_vmMain_t modfunc, pluginres_t* presult, pluginfuncs_t* pluginfuncs, pluginvars_t* pluginvars) {
	QMM_SAVE_VARS();

	QMM_WRITEQMMLOG("QAdmin v" QADMIN_QMM_VERSION " by " QADMIN_QMM_BUILDER " is loaded\n", QMMLOG_INFO, "QADMIN");

	// make version cvar
	g_syscall(G_CVAR_REGISTER, NULL, "admin_version", QADMIN_QMM_VERSION, CVAR_SERVERINFO | CVAR_ROM | CVAR_ARCHIVE);
	g_syscall(G_CVAR_SET, "admin_version", QADMIN_QMM_VERSION);

	// other cvars
	g_syscall(G_CVAR_REGISTER, NULL, "admin_default_access", "1", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER, NULL, "admin_vote_kick_time", "60", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER, NULL, "admin_vote_map_time", "60", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER, NULL, "admin_config_file", "qmmaddons/qadmin/config/qadmin.cfg", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER, NULL, "admin_gagged_cmds", "say_team,tell,vsay,vsay_team,vtell,vosay,vosay_team,votell,vtaunt", CVAR_ARCHIVE);

	memset(g_userinfo, 0, sizeof(g_userinfo));
	memset(g_playerinfo, 0, sizeof(g_playerinfo));

	return 1;
}

// last function called, clean up stuff allocated in QMM_Attach
C_DLLEXPORT void QMM_Detach() {
	tok_free(g_gaggedCmds);
}

// called before mod's vmMain (engine->mod)
C_DLLEXPORT intptr_t QMM_vmMain(intptr_t cmd, intptr_t* args) {
	intptr_t arg0 = args[0];

	// get client info on connection (moved to Post)
	// clear client info on disconnection
	if (cmd == GAME_CLIENT_DISCONNECT) {
		memset(&g_playerinfo[arg0], 0, sizeof(g_playerinfo[arg0]));
		
	// handle client commands
	} else if (cmd == GAME_CLIENT_COMMAND) {
		return handlecommand((int)arg0, 0);

	// allow admin commands from console with "admin_cmd" or "a_c" commands
	} else if (cmd == GAME_CONSOLE_COMMAND) {
		char command[MAX_COMMAND_LENGTH];
		QMM_ARGV(0, command, sizeof(command));
		if (!_stricmp(command, "admin_cmd") || !_stricmp(command, "a_c"))
			return handlecommand(SERVER_CONSOLE, 1);
		else if (!_stricmp(command, "admin_adduser_ip"))
			return admin_adduser(au_ip);
		else if (!_stricmp(command, "admin_adduser_name"))
			return admin_adduser(au_name);
		else if (!_stricmp(command, "admin_adduser_id"))
			return admin_adduser(au_id);

	// handle the game initialization (independent of mod being loaded)
	} else if (cmd == GAME_INIT) {
		g_levelTime = (int)arg0;

	// handle the game shutdown
	} else if (cmd == GAME_SHUTDOWN) {
		// do shutdown stuff
	
	} else if (cmd == GAME_RUN_FRAME) {
		g_levelTime = (int)arg0;

		if (g_vote.inuse && arg0 >= g_vote.finishtime)
			vote_finish();
	}

	QMM_RET_IGNORED(1);
}

// called after mod's vmMain (engine->mod)
C_DLLEXPORT intptr_t QMM_vmMain_Post(intptr_t cmd, intptr_t* args) {
	intptr_t arg0 = args[0];

	// save client data on connection
	// (this is here so that the game has a chance to do various info checking before
	// we get the values)
	if (cmd == GAME_CLIENT_CONNECT || cmd == GAME_CLIENT_USERINFO_CHANGED) {
		char userinfo[MAX_INFO_STRING];
		g_syscall(G_GET_USERINFO, arg0, userinfo, sizeof(userinfo));

		if (!InfoString_Validate(userinfo))
			QMM_RET_IGNORED(1);

		if (cmd == GAME_CLIENT_CONNECT) {
			memset(&g_playerinfo[arg0], 0, sizeof(g_playerinfo[arg0]));
			g_playerinfo[arg0].connected = 1;
			g_playerinfo[arg0].access = g_defaultAccess;
		}

		strncpy(g_playerinfo[arg0].ip, QMM_INFOVALUEFORKEY(userinfo, "ip"), sizeof(g_playerinfo[arg0].ip));
		// if a situation arises where the ip is exactly 15 bytes long, the 16th byte
		// in the buffer will be ':', so this will terminate the string anyway
		char* temp = strstr(g_playerinfo[arg0].ip, ":");
		if (temp) *temp = '\0';
		strncpy(g_playerinfo[arg0].guid, QMM_INFOVALUEFORKEY(userinfo, "cl_guid"), sizeof(g_playerinfo[arg0].guid));
		strncpy(g_playerinfo[arg0].name, QMM_INFOVALUEFORKEY(userinfo, "name"), sizeof(g_playerinfo[arg0].name));
		strncpy(g_playerinfo[arg0].stripname, StripCodes(g_playerinfo[arg0].name), sizeof(g_playerinfo[arg0].stripname));

	// handle the game initialization (dependent on mod being loaded)
	} else if (cmd == GAME_INIT) {
#ifdef GAME_NO_SEND_SERVER_COMMAND
		g_gents = *(gentity_t**)g_vmMain(GAMEVP_EDICTS);
		g_gentsize = *(int*)g_vmMain(GAMEV_EDICT_SIZE);
#endif

		g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("exec %s.cfg\n", QMM_GETSTRCVAR("mapname")));
		// g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, "exec banned_guids.cfg\n");
		
		time(&g_mapstart);

		reload();
	}

	QMM_RET_IGNORED(1);
}

// called before engine's syscall (mod->engine)
C_DLLEXPORT intptr_t QMM_syscall(intptr_t cmd, intptr_t* args) {
	QMM_RET_IGNORED(1);
}

// called after engine's syscall (mod->engine)
C_DLLEXPORT intptr_t QMM_syscall_Post(intptr_t cmd, intptr_t* args) {
	QMM_RET_IGNORED(1);
}
