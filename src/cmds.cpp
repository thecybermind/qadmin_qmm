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

#include <time.h>
#include "main.h"
#include "cmds.h"
#include "vote.h"
#include "util.h"


void reload() {
	// erase all user entries
	g_userinfo.clear();

	// re-exec the config file
	g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("exec %s\n", QMM_GETSTRCVAR("admin_config_file")));

	// refresh gagged command list
	g_gaggedCmds = parse_str(QMM_GETSTRCVAR("admin_gagged_cmds"), ',');

	QMM_WRITEQMMLOG("Configs/cvars (re)loaded\n", QMMLOG_INFO, "QADMIN");
}


#define usertype(x) 
// server command to add a new user
int admin_adduser(addusertype_t type, std::vector<std::string> args) {
	if (args.size() < 4) {
		QMM_WRITEQMMLOG(QMM_VARARGS("Not enough parameters for %s <name|ip|id> <pass> <access>\n", args[0].c_str()), QMMLOG_INFO, "QADMIN");
		QMM_RET_SUPERCEDE(1);
	}
	std::string user = args[1];
	std::string pass = args[2];
	int access = atoi(args[3].c_str());

	const char* strtype = (type == au_ip ? "IP" : (type == au_name ? "name" : "ID"));

	int i = 0;
	for (auto& info : g_userinfo) {
		if (info.type == type && str_striequal(user, info.user)) {
			QMM_WRITEQMMLOG(QMM_VARARGS("User %s entry already exists for \"%s\"\n", strtype, user.c_str()), QMMLOG_INFO, "QADMIN");
			QMM_RET_SUPERCEDE(1);
		}
	}

	userinfo_t newuser = { user, pass, access, type };
	g_userinfo.push_back(newuser);

	QMM_WRITEQMMLOG(QMM_VARARGS("New user %s entry added for \"%s\" (access=%d)\n", strtype, user.c_str(), access), QMMLOG_INFO, "QADMIN");
	QMM_RET_SUPERCEDE(1);
}


// main command handler
// clientnum = client that did the command
// args = all args from engine (or say cmd)
int handlecommand(intptr_t clientnum, std::vector<std::string> args) {
	std::string cmd = args[0];

	for (auto& admincmd : g_admincmds) {
		if (str_striequal(admincmd.cmd, cmd)) {
			// if the client has access, run handler func (get return value, func will set result flag)
			if (player_has_access(clientnum, admincmd.reqaccess)) {
				// only run handler func if we provided enough args
				// otherwise, show the help entry
				if (g_syscall(G_ARGC) < (admincmd.minargs + 1)) {
					player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Missing parameters, usage:\n[QADMIN] %s\n", admincmd.usage));
					QMM_RET_SUPERCEDE(1);
				}
				else
					return (admincmd.func)(clientnum, admincmd.reqaccess, args, false);		// false = console command (not say)
			}

			// if client doesn't have access, give warning message
#ifdef GAME_NO_SEND_SERVER_COMMAND
			g_syscall(G_CPRINTF, clientnum, PRINT_HIGH, QMM_VARARGS("[QADMIN] You do not have access to that command: '%s'\n", cmd.c_str()));
#else
			g_syscall(G_SEND_SERVER_COMMAND, clientnum, QMM_VARARGS("print \"[QADMIN] You do not have access to that command: '%s'\n\"\n", cmd.c_str()));
#endif
			QMM_RET_SUPERCEDE(1);
		}
	}

	// check for gagged commands (but only for gagged users)
	if (g_playerinfo[clientnum].gagged) {
		for (auto gagcmd : g_gaggedCmds) {
			if (str_striequal(cmd, gagcmd)) {
				player_clientprint(clientnum, "[QADMIN] Sorry, you have been gagged.\n");
				QMM_RET_SUPERCEDE(1);
			}
		}
	}

	if (clientnum == SERVER_CONSOLE)
		QMM_RET_SUPERCEDE(1);

	QMM_RET_IGNORED(0);
}


// clientnum = client that did the command
// access = access required to run this command
// args = all command args (include command in [0])
int admin_help(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	int start = 1;
	if (args.size() > 1)
		start = atoi(args[1].c_str());

	if (start <= 0 || start > (g_admincmds.size()) + g_saycmds.size())
		start = 1;
	
	player_clientprint(clientnum, QMM_VARARGS("[QADMIN] admin_help listing for %d-%d\n", start, start+9));

	// counter for valid commands
	int count_cmd = 1;
	// display counter
	int count_show = start;

	for (auto& cmd : g_admincmds) {
		// make sure the command help exists and the user has enough access
		if (player_has_access(clientnum, cmd.reqaccess) && cmd.usage && cmd.usage[0] && cmd.help && cmd.help[0]) {
			// if the command is within our display range
			if (count_cmd >= start && count_cmd <= (start + 9)) {
				player_clientprint(clientnum, QMM_VARARGS("[QADMIN] %d. %s - %s\n", count_show, cmd.usage, cmd.help));
				++count_show;
			}
			++count_cmd;
		}
	}

	QMM_RET_SUPERCEDE(1);
}


int admin_login(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	if (clientnum == SERVER_CONSOLE) {
		player_clientprint(clientnum, "[QADMIN] Trying to login from the server console, eh?\n");
		QMM_RET_SUPERCEDE(1);
	}

	if (g_playerinfo[clientnum].authed) {
		player_clientprint(clientnum, "[QADMIN] Trying to login multiple times, eh?\n");
		QMM_RET_SUPERCEDE(1);
	}

	std::string password = args[1];

	for (auto& info : g_userinfo) {
		std::string match = g_playerinfo[clientnum].name;
		if (info.type == au_ip)
			match = g_playerinfo[clientnum].ip;
		else if (info.type == au_id)
			match = g_playerinfo[clientnum].guid;

		if (str_striequal(info.user, match) && str_striequal(info.pass, password)) {
			g_playerinfo[clientnum].access = info.access;
			g_playerinfo[clientnum].authed = 1;
			player_clientprint(clientnum, QMM_VARARGS("[QADMIN] You have successfully authenticated. You now have %d access.\n", g_playerinfo[clientnum].access));
			QMM_RET_SUPERCEDE(1);
		}
	}

	QMM_RET_SUPERCEDE(1);
}


int admin_ban(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	std::string bancmd = args[0];
	std::string user = args[1];

	std::string message = str_join(args, 2);
	if (message.empty())
		message = "Banned by Admin";

	std::vector<intptr_t> targets = players_with_name(user);
	if (targets.size() == 0) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Match not found for '%s'\n", user.c_str()));
		QMM_RET_SUPERCEDE(1);
	}
	else if (targets.size() > 1) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Ambiguous match for '%s'\n", user.c_str()));
		QMM_RET_SUPERCEDE(1);
	}

	intptr_t targetclient = targets[0];

	// check if the desired user has immunity
	if (player_has_access(targetclient, ACCESS_IMMUNITY)) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Cannot ban %s, user has immunity.\n", g_playerinfo[targetclient].name.c_str()));
		QMM_RET_SUPERCEDE(1);
	}

	// flag. true if at least 1 matching ip user has immunity
	bool immunity = false;
		
	// find users who have the given IP without immunity
	std::vector<intptr_t> findusers = players_with_ip(g_playerinfo[targetclient].ip);
	auto it = findusers.begin();
	while (it != findusers.end()) {
		if (player_has_access(*it, ACCESS_IMMUNITY)) {
			it = findusers.erase(it);
			immunity = true;
		}
		else
			++it;
	}

	// if no users with immunity have the IP, ban the IP and kick the user
	if (!immunity) {
		g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("addip \"%s\" \"%s\"\n", g_playerinfo[targetclient].ip.c_str(), message.c_str()));
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Banned %s by IP (%s): '%s'\n", g_playerinfo[targetclient].name.c_str(), g_playerinfo[targetclient].ip.c_str(), message.c_str()));
		player_kick(targetclient, message);
	}
	// else at least 1 user with immunity has the given IP
	else {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Cannot ban %s by IP, another user with that IP (%s) has immunity.\n", g_playerinfo[targetclient].name.c_str(), g_playerinfo[targetclient].ip.c_str()));
	}

	// kick the users on the IP without immunity
	for (auto& finduser : findusers)
		player_kick(finduser, message);

	QMM_RET_SUPERCEDE(1);
}


int admin_banip(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	std::string bancmd = args[0];
	std::string user = args[1];

	std::string message = str_join(args, 2);
	if (message.empty())
		message = "Banned by Admin";

	// flag. true if at least 1 matching ip user has immunity
	bool immunity = false;

	// find users who have the given IP without immunity
	std::vector<intptr_t> findusers = players_with_ip(user);
	auto it = findusers.begin();
	while (it != findusers.end()) {
		if (player_has_access(*it, ACCESS_IMMUNITY)) {
			it = findusers.erase(it);
			immunity = true;
		}
		else
			++it;
	}

	// if no users with immunity have the IP, ban the IP
	if (!immunity) {
		g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("addip \"%s\" \"%s\"\n", user.c_str(), message.c_str()));
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Banned IP %s: '%s'\n", user.c_str(), message.c_str()));
	}		
	// else at least 1 user with immunity has the given IP
	else {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Cannot ban IP %s, a user with that IP has immunity. Kicking non-immune users.\n", user.c_str()));
	}

	// kick the users on the IP without immunity
	for (auto& finduser : findusers)
		player_kick(finduser, message);

	QMM_RET_SUPERCEDE(1);
}


int admin_unban(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	std::string ip = str_sanitize(args[1]);

	g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("removeip \"%s\"\n", ip.c_str()));
	player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Unbanned IP %s\n", ip.c_str()));

	QMM_RET_SUPERCEDE(1);
}


int admin_cfg(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	std::string file = str_sanitize(args[1]);

	g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("exec \"%s\"\n", file.c_str()));

	QMM_RET_SUPERCEDE(1);
}


int admin_rcon(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	std::string str = str_join(args, 1);
	g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("%s\n", str.c_str()));

	QMM_RET_SUPERCEDE(1);
}


int admin_hostname(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	g_syscall(G_CVAR_SET, "sv_hostname", args[1].c_str());

	QMM_RET_SUPERCEDE(1);
}


int admin_friendlyfire(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	g_syscall(G_CVAR_SET, "g_friendlyfire", args[1].c_str());

	QMM_RET_SUPERCEDE(1);
}


int admin_gravity(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	g_syscall(G_CVAR_SET, "g_gravity", args[1].c_str());

	QMM_RET_SUPERCEDE(1);
}


int admin_gametype(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	g_syscall(G_CVAR_SET, "g_gametype", args[1].c_str());

	QMM_RET_SUPERCEDE(1);
}


int admin_map(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	std::string map = str_sanitize(args[1]);
	g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("map \"%s\"\n", args[1].c_str()));

	QMM_RET_SUPERCEDE(1);
}


int admin_fraglimit(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	g_syscall(G_CVAR_SET, "fraglimit", args[1].c_str());

	QMM_RET_SUPERCEDE(1);
}


int admin_timelimit(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	g_syscall(G_CVAR_SET, "timelimit", args[1].c_str());

	QMM_RET_SUPERCEDE(1);
}


int admin_pass(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	if (str_striequal(args[0], "admin_pass")) {
		g_syscall(G_CVAR_SET, "g_password", args[1].c_str());
		g_syscall(G_CVAR_SET, "g_needpass", "1");
	} else if (str_striequal(args[0], "admin_nopass")) {
		g_syscall(G_CVAR_SET, "g_password", "");
		g_syscall(G_CVAR_SET, "g_needpass", "0");
	}

	QMM_RET_SUPERCEDE(1);
}


int admin_chat(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	std::string message = str_sanitize(str_join(args, 1));
	for (auto& playerinfo : g_playerinfo) {
		if (player_has_access(playerinfo.first, access))
			player_clientprint(playerinfo.first, QMM_VARARGS("To Admins From %s: %s", clientnum == SERVER_CONSOLE ? "Console" : playerinfo.second.name.c_str(), message.c_str()), true);
	}

	QMM_RET_SUPERCEDE(1);
}


int admin_csay(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	std::string message = str_sanitize(str_join(args, 1));
#ifdef GAME_NO_SEND_SERVER_COMMAND
	g_syscall(G_CPRINTF, clientnum, PRINT_HIGH, message.c_str());
#else
	g_syscall(G_SEND_SERVER_COMMAND, -1, QMM_VARARGS("cp \"%s\n\"", message.c_str()));
#endif

	QMM_RET_SUPERCEDE(1);
}


int admin_say(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	std::string message = str_sanitize(str_join(args, 1));
	player_clientprint(-1, message.c_str(), true);

	QMM_RET_SUPERCEDE(1);
}


int admin_psay(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	std::string user = args[1];

	std::vector<intptr_t> targets = players_with_name(user);
	if (targets.size() == 0) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Match not found for '%s'\n", user.c_str()));
		QMM_RET_SUPERCEDE(1);
	}
	else if (targets.size() > 1) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Ambiguous match for '%s'\n", user.c_str()));
		QMM_RET_SUPERCEDE(1);
	}

	intptr_t targetclient = targets[0];

	std::string message = str_sanitize(str_join(args, 2));	
	std::string toname = g_playerinfo[targetclient].name;

	player_clientprint(clientnum, QMM_VARARGS("Private Message To %s: %s", toname.c_str(), message.c_str()), true);
	player_clientprint(targetclient, QMM_VARARGS("Private Message From %s: %s", clientnum == SERVER_CONSOLE ? "Console" : toname.c_str(), message.c_str()), true);

	QMM_RET_SUPERCEDE(1);
}


int admin_listmaps(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
#ifndef GAME_NO_FS_GETFILELIST
	char dirlist[MAX_STRING_LENGTH];
	char* dirptr = dirlist;

	int numfiles = g_syscall(G_FS_GETFILELIST, "maps", ".bsp", dirlist, sizeof(dirlist));

	player_clientprint(clientnum, "[QADMIN] Listing maps...\n");
	for (int i = 1; i < numfiles; ++i) {
		dirptr += strlen(dirptr);
		*dirptr = '\n';
	}
	player_clientprint(clientnum, dirlist);
	player_clientprint(clientnum, "\n[QADMIN] End of maps list\n");
#endif
	QMM_RET_SUPERCEDE(1);
}


int admin_kick(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	std::string kickcmd = args[0];
	std::string user = args[1];

	std::vector<intptr_t> targets = players_with_name(user);
	if (targets.size() == 0) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Match not found for '%s'\n", user.c_str()));
		QMM_RET_SUPERCEDE(1);
	}
	else if (targets.size() > 1) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Ambiguous match for '%s'\n", user.c_str()));
		QMM_RET_SUPERCEDE(1);
	}

	intptr_t targetclient = targets[0];

	if (player_has_access(targetclient, ACCESS_IMMUNITY)) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Cannot kick %s, user has immunity\n", g_playerinfo[targetclient].name.c_str()));
		QMM_RET_SUPERCEDE(1);
	}
	
	std::string message = str_sanitize(str_join(args, 2));
	if (message.empty())
		message = "Kicked by Admin";
	
	player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Kicked %s: '%s'\n", g_playerinfo[targetclient].name.c_str(), message.c_str()));
	player_kick(targetclient, message);

	QMM_RET_SUPERCEDE(1);
}


int admin_reload(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	reload();

	QMM_RET_SUPERCEDE(1);
}


int admin_userlist(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	std::string match = "";

	int banaccess = player_has_access(clientnum, LEVEL_256);

	// if a parameter was given, only display users matching it
	if (args.size() > 1) {
		match = args[1];
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Listing users matching '%s'...\n", match.c_str()));
	}
	else {
		player_clientprint(clientnum, "[QADMIN] Listing users...\n");
	}

	if (banaccess)
		player_clientprint(clientnum, "[QADMIN] Slot Access   Authed IP              Name\n");
	else
		player_clientprint(clientnum, "[QADMIN] Slot Access   Authed Name\n");

	for (auto player : players_with_name(match)) {
		playerinfo_t& info = g_playerinfo[player];
		if (banaccess)
			player_clientprint(clientnum, QMM_VARARGS("[QADMIN] %3d: %-8d %-6s %-15s %s\n", player, info.access, info.authed ? "yes" : "no", info.ip.c_str(), info.name.c_str()));
		else
			player_clientprint(clientnum, QMM_VARARGS("[QADMIN] %3d: %-8d %-6s %s\n", player, info.access, info.authed ? "yes" : "no", info.name.c_str()));
	}

	QMM_RET_SUPERCEDE(1);
}


int admin_gag(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	std::string gagcmd = args[0];
	std::string user = args[1];

	std::vector<intptr_t> targets = players_with_name(user);
	if (targets.size() == 0) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Match not found for '%s'\n", user.c_str()));
		QMM_RET_SUPERCEDE(1);
	}
	else if (targets.size() > 1) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Ambiguous match for '%s'\n", user.c_str()));
		QMM_RET_SUPERCEDE(1);
	}

	intptr_t targetclient = targets[0];

	if (player_has_access(targetclient, ACCESS_IMMUNITY)) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Cannot gag %s, user has immunity\n", g_playerinfo[targetclient].name.c_str()));
	}
	else if (g_playerinfo[targetclient].gagged) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] %s is already gagged\n", g_playerinfo[targetclient].name.c_str()));
	}
	else {
		g_playerinfo[targetclient].gagged = true;
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] %s has been gagged\n", g_playerinfo[targetclient].name.c_str()));
	}

	QMM_RET_SUPERCEDE(1);
}


int admin_ungag(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	std::string ungagcmd = args[0];
	std::string user = args[1];

	std::vector<intptr_t> targets = players_with_name(user);
	if (targets.size() == 0) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Match not found for '%s'\n", user.c_str()));
		QMM_RET_SUPERCEDE(1);
	}
	else if (targets.size() > 1) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Ambiguous match for '%s'\n", user.c_str()));
		QMM_RET_SUPERCEDE(1);
	}

	intptr_t targetclient = targets[0];

	if (g_playerinfo[targetclient].gagged) {
		g_playerinfo[targetclient].gagged = false;
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] %s has been ungagged\n", g_playerinfo[targetclient].name.c_str()));
	} else {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] %s is not gagged\n", g_playerinfo[targetclient].name.c_str()));
	}

	QMM_RET_SUPERCEDE(1);
}


int admin_currentmap(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	player_clientprint(say ? -1 : clientnum, QMM_VARARGS("[QADMIN] The current map is: %s\n", QMM_GETSTRCVAR("mapname")));
	QMM_RETURN(say ? QMM_IGNORED : QMM_SUPERCEDE, 1);
}


int admin_timeleft(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	intptr_t timelimit = g_syscall(G_CVAR_VARIABLE_INTEGER_VALUE, "timelimit");
	if (!timelimit) {
		player_clientprint(say ? -1 : clientnum, "[QADMIN] There is no time limit.\n");
		QMM_RETURN(say ? QMM_IGNORED : QMM_SUPERCEDE, 1);
	}
	time_t timeleft = (timelimit * 60) - (time(NULL) - g_mapstart);

	if (timeleft <= 0)
		player_clientprint(say ? -1 : clientnum, "[QADMIN] Time limit has been reached\n");
	else
		player_clientprint(say ? -1 : clientnum, QMM_VARARGS("[QADMIN] Time remaining: %lu minute(s) %lu second(s)\n", timeleft / 60, timeleft % 60));

	QMM_RETURN(say ? QMM_IGNORED : QMM_SUPERCEDE, 1);
}


void handle_vote_map(int winner, int winvotes, int totalvotes, void* param) {
	std::string map = *(std::string*)param;
	if (winner == 1) {
		player_clientprint(-1, QMM_VARARGS("[QADMIN] Vote to change map to %s was successful\n", map.c_str()));
		g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("map \"%s\"\n", map.c_str()));
	} else {
		player_clientprint(-1, QMM_VARARGS("[QADMIN] Vote to change map to %s has failed\n", map.c_str()));
	}
}


int admin_vote_map(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	// this is static so that it still exists when passed to handle_vote_map as param
	static std::string map;

	int time = (int)QMM_GETINTCVAR("admin_vote_map_time");

	map = args[1];

	if (!is_valid_map(map)) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Unknown map '%s'\n", map.c_str()));
		QMM_RET_SUPERCEDE(1);
	}

	player_clientprint(-1, QMM_VARARGS("[QADMIN] A %d second vote has been started to changed map to %s\n", time, map.c_str()));
	player_clientprint(-1, "[QADMIN] Type 'castvote 1' for YES, or 'castvote 2' for NO\n");
	
	vote_start(clientnum, handle_vote_map, time, 2, (void*)&map);

	QMM_RET_SUPERCEDE(1);
}


void handle_vote_kick(int winner, int winvotes, int totalvotes, void* param) {
	int clientnum = (int)(intptr_t)param;
	
	// user may have authed and gotten immunity during the vote,
	// but don't mention it, just make the vote fail
	if (player_has_access(clientnum, ACCESS_IMMUNITY))
		winner = 0;

	if (winner == 1 && winvotes) {
		player_clientprint(-1, QMM_VARARGS("[QADMIN] Vote to kick %s was successful\n", g_playerinfo[clientnum].name));
		player_kick(clientnum, "Kicked due to vote.");
	} else {
		player_clientprint(-1, QMM_VARARGS("[QADMIN] Vote to kick %s has failed\n", g_playerinfo[clientnum].name));
	}
}


int admin_vote_kick(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	int votetime = (int)QMM_GETINTCVAR("admin_vote_kick_time");
	std::string user = args[1];

	std::vector<intptr_t> targets = players_with_name(user);
	if (targets.size() == 0) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Match not found for '%s'\n", user.c_str()));
		QMM_RET_SUPERCEDE(1);
	}
	else if (targets.size() > 1) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Ambiguous match for '%s'\n", user.c_str()));
		QMM_RET_SUPERCEDE(1);
	}

	intptr_t targetclient = targets[0];
	
	// cannot votekick a user with immunity
	// the user's immunity is also checked in the vote
	// handler in case he auths before the vote ends
	if (player_has_access(targetclient, ACCESS_IMMUNITY)) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Cannot kick %s, user has immunity\n", g_playerinfo[targetclient].name.c_str()));
		QMM_RET_SUPERCEDE(1);
	}

	player_clientprint(-1, QMM_VARARGS("[QADMIN] A %d second vote has been started to kick %s\n", votetime, g_playerinfo[targetclient].name.c_str()));
	player_clientprint(-1, "[QADMIN] Type 'castvote 1' for YES, or 'castvote 2' for NO\n");
	vote_start(clientnum, handle_vote_kick, votetime, 2, (void*)(intptr_t)targetclient);

	QMM_RET_SUPERCEDE(1);
}


int admin_vote_abort(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	player_clientprint(-1, "[QADMIN] The current vote has been canceled\n");
	g_vote.inuse = false;

	QMM_RET_SUPERCEDE(1);
}


// say handler (need to handle subcommands)
int say(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	if (g_playerinfo[clientnum].gagged) {
		player_clientprint(clientnum, "[QADMIN] Sorry, you have been gagged.\n");
		QMM_RET_SUPERCEDE(1);
	}

	// if no args for "say", ignore it
	if (args.size() == 1)
		QMM_RET_SUPERCEDE(1);

	// get new list of args without the first ("say")
	args = std::vector<std::string>(args.begin() + 1, args.end());

	std::string command = args[0];

	// loop through all registered "say" commands
	for (auto& saycmd : g_saycmds) {
		// if we found the command
		if (str_striequal(saycmd.cmd, command)) {
			// if the client has access, run handler func (get return value, func will set result flag)
			if (player_has_access(clientnum, saycmd.reqaccess)) {
				// only run handler func if we provided enough args
				if (args.size() < (saycmd.minargs + 1))
					QMM_RET_IGNORED(0);
				else
					return (saycmd.func)(clientnum, saycmd.reqaccess, args, true);	// true = say command
			}

			// if client doesn't have access, give warning message
			player_clientprint(clientnum, QMM_VARARGS("[QADMIN] You do not have access to that command: '%s'\n", saycmd.cmd));

			QMM_RET_SUPERCEDE(1);
		}
	}

	QMM_RET_IGNORED(0);
}


int castvote(intptr_t clientnum, int access, std::vector<std::string> args, bool say) {
	if (clientnum == SERVER_CONSOLE) {
		player_clientprint(clientnum, "[QADMIN] Trying to vote from the server console, eh?\n");
		QMM_RET_SUPERCEDE(1);
	}

	std::string vote = args[1];
	vote_add(clientnum, atoi(vote.c_str()));

	QMM_RET_SUPERCEDE(1);		
}


// register command handlers
// moved into alphabetical order to make admin_help a bit easier
std::vector<admincmd_t> g_admincmds = {
	{ "admin_ban",			admin_ban,			LEVEL_256,	1, "admin_ban <name> [message]", "Bans the specified user by IP" },
	{ "admin_banip",		admin_banip,		LEVEL_256,	1, "admin_banip <ip> [message]", "Bans the specified IP" },
	{ "admin_cfg",			admin_cfg,			LEVEL_512,	1, "admin_cfg <file.cfg>", "Executes the given .cfg file on the server" },
	{ "admin_chat",			admin_chat,			LEVEL_64,	1, "admin_chat <text>", "Sends the message to all admins with admin_chat access" },
	{ "admin_csay",			admin_csay,			LEVEL_64,	1, "admin_csay <text>", "Displays message to all players in center of screen" },
	{ "admin_currentmap",	admin_currentmap,	LEVEL_0,	0, "admin_currentmap", "Displays current map" },
	{ "admin_fraglimit",	admin_fraglimit,	LEVEL_2,	1, "admin_fraglimit <value>", "Sets the server's fraglimit" },
	{ "admin_friendlyfire",	admin_friendlyfire,	LEVEL_32,	1, "admin_friendlyfire <value>", "Sets the server's friendlyfire" },
	{ "admin_gag",			admin_gag,			LEVEL_2048,	1, "admin_gag <name>", "Gags the specified player from speaking" },
	{ "admin_gametype",		admin_gametype,		LEVEL_32,	1, "admin_gametype <value>", "Sets the server's gametype" },
	{ "admin_gravity",		admin_gravity,		LEVEL_32,	1, "admin_gravity <value>", "Sets the server's gravity" },
	{ "admin_help",			admin_help,			LEVEL_0,	0, "admin_help [start]", "Displays commands you have access to" },
	{ "admin_hostname",		admin_hostname,		LEVEL_512,	1, "admin_hostname <new name>", "Sets the server's hostname" },
	{ "admin_kick",			admin_kick,			LEVEL_128,	1, "admin_kick <name> [message]", "Kicks name from the server" },
#ifndef GAME_NO_FS_GETFILELIST
	{ "admin_listmaps",		admin_listmaps,		LEVEL_0,	0, "admin_listmaps", "Lists all maps on the server" },
#endif
	{ "admin_login",		admin_login,		LEVEL_0,	1, "admin_login <pass>", "Logs you in to get access" },
	{ "admin_map",			admin_map,			LEVEL_8,	1, "admin_map <map>", "Changes to the given map" },
	{ "admin_pass",			admin_pass,			LEVEL_16,	1, "admin_pass <password>", "Changes the server password" },
	{ "admin_psay",			admin_psay,			LEVEL_64,	2, "admin_psay <name> <text>", "Sends the message to specified player" },
	{ "admin_nopass",		admin_pass,			LEVEL_16,	0, "admin_nopass", "Clears the server password" },
	{ "admin_rcon",			admin_rcon,			LEVEL_65536,1, "admin_rcon <command>", "Executes the command on the server" },
	{ "admin_reload",		admin_reload,		LEVEL_4,	0, "admin_reload", "Reloads various QAdmin configs and cvars" },
	{ "admin_say",			admin_say,			LEVEL_64,	1, "admin_say <text>", "Sends the message to all players" },
	{ "admin_timeleft",		admin_timeleft,		LEVEL_0,	0, "admin_timeleft", "Displays the time left on this map" },
	{ "admin_timelimit",	admin_timelimit,	LEVEL_2,	1, "admin_timelimit <value>", "Sets the server's timelimit" },
	{ "admin_unban",		admin_unban,		LEVEL_256,	1, "admin_unban <ip>", "Unbans the specified IP" },
	{ "admin_ungag",		admin_ungag,		LEVEL_2048,	1, "admin_ungag <name>", "Ungags the specified player" },
	{ "admin_userlist",		admin_userlist,		LEVEL_0,	0, "admin_userlist [name]", "Lists all users on the server that match 'name'" },
	{ "admin_vote_abort",	admin_vote_abort,	LEVEL_2,	1, "admin_vote_abort", "Aborts the current map or kick vote" },
	{ "admin_vote_cancel",	admin_vote_abort,	LEVEL_2,	1, nullptr, nullptr },
	{ "admin_vote_kick",	admin_vote_kick,	LEVEL_1,	1, "admin_vote_kick <user>", "Initiates a vote to kick the user" },
	{ "admin_vote_map",		admin_vote_map,		LEVEL_1,	1, "admin_vote_map <map>", "Initiates a vote to change to the map" },
	{ "castvote",			castvote,			LEVEL_1,	1, "castvote <option>", "Places a vote for the given option" },

	{ "say",				say,				LEVEL_0,	0, nullptr, nullptr },
};

std::vector<admincmd_t> g_saycmds = {
	{ "admin_login",	admin_login,			LEVEL_0,	1, nullptr, nullptr },
	{ "castvote",		castvote,				LEVEL_1,	1, nullptr, nullptr },
	{ "currentmap",		admin_currentmap,		LEVEL_0,	0, nullptr, nullptr },
	{ "timeleft",		admin_timeleft,			LEVEL_0,	0, nullptr, nullptr },
};
