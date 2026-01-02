/*
QADMIN_QMM - Server Administration Plugin
Copyright 2004-2025
https://github.com/thecybermind/qadmin_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < k.m.masterson@gmail.com >

*/

#define _CRT_SECURE_NO_WARNINGS 1

#include <qmmapi.h>

#include "version.h"
#include "game.h"

#include <string>
#include <vector>
#include <string.h>
#include <time.h>
#include "main.h"
#include "cmds.h"
#include "vote.h"
#include "util.h"

pluginres_t* g_result = nullptr;
plugininfo_t g_plugininfo = {
	QMM_PIFV_MAJOR,									// plugin interface version major
	QMM_PIFV_MINOR,									// plugin interface version minor
	"QAdmin",										// name of plugin
	QADMIN_QMM_VERSION,								// version of plugin
	"Server Administration Plugin",					// description of plugin
	QADMIN_QMM_BUILDER,								// author of plugin
	"https://github.com/thecybermind/qadmin_qmm/",	// website of plugin
	"QADMIN",										// logtag of plugin
};
eng_syscall_t g_syscall = nullptr;
mod_vmMain_t g_vmMain = nullptr;
pluginfuncs_t* g_pluginfuncs = nullptr;
pluginvars_t* g_pluginvars = nullptr;

std::map<intptr_t, playerinfo_t> g_playerinfo;
std::vector<userinfo_t> g_userinfo;

time_t g_mapstart;
time_t g_leveltime;

std::vector<std::string> g_gaggedCmds;


// first function called in plugin, give QMM the plugin info
C_DLLEXPORT void QMM_Query(plugininfo_t** pinfo) {
	QMM_GIVE_PINFO();
}


// second function called, save pointers
C_DLLEXPORT int QMM_Attach(eng_syscall_t engfunc, mod_vmMain_t modfunc, pluginres_t* presult, pluginfuncs_t* pluginfuncs, pluginvars_t* pluginvars) {
	QMM_SAVE_VARS();

	QMM_WRITEQMMLOG(PLID, "QAdmin v" QADMIN_QMM_VERSION " by " QADMIN_QMM_BUILDER " is loaded\n", QMMLOG_INFO);

	// make version cvar
	g_syscall(G_CVAR_REGISTER, nullptr, "admin_version", QADMIN_QMM_VERSION, CVAR_SERVERINFO | CVAR_ROM | CVAR_ARCHIVE);
	g_syscall(G_CVAR_SET, "admin_version", QADMIN_QMM_VERSION);

	// other cvars
	g_syscall(G_CVAR_REGISTER, nullptr, "admin_default_access", "1", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER, nullptr, "admin_vote_kick_time", "60", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER, nullptr, "admin_vote_map_time", "60", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER, nullptr, "admin_config_file", "qmmaddons/qadmin/config/qadmin.cfg", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER, nullptr, "admin_gagged_cmds", "say_team,tell,vsay,vsay_team,vtell,vosay,vosay_team,votell,vtaunt", CVAR_ARCHIVE);

	return 1;
}


// last function called, clean up stuff allocated in QMM_Attach
C_DLLEXPORT void QMM_Detach() {
}


// called before mod's vmMain (engine->mod)
C_DLLEXPORT intptr_t QMM_vmMain(intptr_t cmd, intptr_t* args) {
	// clear client info on disconnection
	if (cmd == GAME_CLIENT_DISCONNECT) {
		if (g_playerinfo.count(args[0]))
			g_playerinfo.erase(args[0]);
	}
	// handle client commands
	else if (cmd == GAME_CLIENT_COMMAND) {
		return handlecommand(args[0], parse_args(0));
	}
	// allow admin commands from console with "admin_cmd" or "a_c" commands
	else if (cmd == GAME_CONSOLE_COMMAND) {
		char command[MAX_COMMAND_LENGTH];
		QMM_ARGV(PLID, 0, command, sizeof(command));
		if (str_striequal(command, "admin_cmd") || str_striequal(command, "a_c"))
			return handlecommand(SERVER_CONSOLE, parse_args(1));
		else if (str_striequal(command, "admin_adduser_ip"))
			return admin_adduser(au_ip, parse_args(0));
		else if (str_striequal(command, "admin_adduser_name"))
			return admin_adduser(au_name, parse_args(0));
		else if (str_striequal(command, "admin_adduser_id"))
			return admin_adduser(au_id, parse_args(0));
	}
	else if (cmd == GAME_INIT) {
		time(&g_mapstart);
		time(&g_leveltime);
	}
	else if (cmd == GAME_RUN_FRAME) {
		time(&g_leveltime);

		if (g_vote.inuse && g_leveltime >= g_vote.finishtime)
			vote_finish();
	}

	QMM_RET_IGNORED(0);
}


// called after mod's vmMain (engine->mod)
C_DLLEXPORT intptr_t QMM_vmMain_Post(intptr_t cmd, intptr_t* args) {
	// save client data on connection
	// (this is here in _Post so that the game has a chance to do various info checking before we get the values)
	if (cmd == GAME_CLIENT_CONNECT || cmd == GAME_CLIENT_USERINFO_CHANGED) {
		intptr_t clientnum = args[0];

		char userinfo[MAX_INFO_STRING];
		g_syscall(G_GET_USERINFO, clientnum, userinfo, sizeof(userinfo));

		// if playerinfo is missing, make a new one
		if (!g_playerinfo.count(clientnum)) {
			g_playerinfo[clientnum] = {};
			g_playerinfo[clientnum].access = 0;
		}
		
		// update ip/guid/name
		playerinfo_t& info = g_playerinfo[clientnum]; 

		std::string ip = QMM_INFOVALUEFORKEY(PLID, userinfo, "ip");
		info.ip = ip.substr(0, ip.find(':'));
		info.guid = QMM_INFOVALUEFORKEY(PLID, userinfo, "cl_guid");
		info.name = QMM_INFOVALUEFORKEY(PLID, userinfo, "name");
		info.stripname = strip_codes(info.name);
	}
	// handle the game initialization (dependent on mod being loaded)
	else if (cmd == GAME_INIT) {
		g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS(PLID, "exec %s.cfg\n", QMM_GETSTRCVAR(PLID, "mapname")));
		// g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, "exec banned_guids.cfg\n");
		
		reload();
	}

	QMM_RET_IGNORED(0);
}


// called before engine's syscall (mod->engine)
C_DLLEXPORT intptr_t QMM_syscall(intptr_t cmd, intptr_t* args) {
	QMM_RET_IGNORED(0);
}


// called after engine's syscall (mod->engine)
C_DLLEXPORT intptr_t QMM_syscall_Post(intptr_t cmd, intptr_t* args) {
	QMM_RET_IGNORED(0);
}
