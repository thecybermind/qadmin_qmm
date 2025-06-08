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
int handlecommand(int clientnum, std::vector<std::string> args) {
	std::string cmd = args[0];

	for (auto& admincmd : g_admincmds) {
		if (str_striequal(admincmd.cmd, cmd)) {
			// if the client has access, run handler func (get return value, func will set result flag)
			if (player_has_access(clientnum, admincmd.reqaccess)) {
				// only run handler func if we provided enough args
				// otherwise, show the help entry
				if (g_syscall(G_ARGC) < (admincmd.minargs + 1)) {
					ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Missing parameters, usage:\n[QADMIN] %s\n", admincmd.usage));
					QMM_RET_SUPERCEDE(1);
				}
				else
					return (admincmd.func)(clientnum, admincmd.reqaccess, args, false);		// false = console command (not say)
			}

			// if client doesn't have access, give warning message
#ifdef GAME_NO_SEND_SERVER_COMMAND
			g_syscall(G_CPRINTF, ENT_FROM_NUM(clientnum), QMM_VARARGS("print \"[QADMIN] You do not have access to that command: '%s'\n\"", cmd.c_str()));
#else
			g_syscall(G_SEND_SERVER_COMMAND, clientnum, QMM_VARARGS("print \"[QADMIN] You do not have access to that command: '%s'\n\"", cmd.c_str()));
#endif
			QMM_RET_SUPERCEDE(1);
		}
	}

	// check for gagged commands (but only for gagged users)
	if (g_playerinfo[clientnum].gagged) {
		for (auto gagcmd : g_gaggedCmds) {
			if (str_striequal(cmd, gagcmd)) {
				ClientPrint(clientnum, "[QADMIN] Sorry, you have been gagged.\n");
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
int admin_help(int clientnum, int access, std::vector<std::string> args, bool say) {
	int start = 1;
	if (args.size() > 1)
		start = atoi(args[1].c_str());

	if (start <= 0 || start > (g_admincmds.size()) + g_saycmds.size())
		start = 1;
	
	ClientPrint(clientnum, QMM_VARARGS("[QADMIN] admin_help listing for %d-%d\n", start, start+9));

	// counter for valid commands
	int count_cmd = 1;
	// display counter
	int count_show = start;

	for (auto& cmd : g_admincmds) {
		// make sure the command help exists and the user has enough access
		if (player_has_access(clientnum, cmd.reqaccess) && cmd.usage && cmd.usage[0] && cmd.help && cmd.help[0]) {
			// if the command is within our display range
			if (count_cmd >= start && count_cmd <= (start + 9)) {
				ClientPrint(clientnum, QMM_VARARGS("[QADMIN] %d. %s - %s\n", count_show, cmd.usage, cmd.help));
				++count_show;
			}
			++count_cmd;
		}
	}

	QMM_RET_SUPERCEDE(1);
}


int admin_login(int clientnum, int access, std::vector<std::string> args, bool say) {
	if (clientnum == SERVER_CONSOLE) {
		ClientPrint(clientnum, "[QADMIN] Trying to login from the server console, eh?\n");
		QMM_RET_SUPERCEDE(1);
	}

	if (g_playerinfo[clientnum].authed) {
		ClientPrint(clientnum, "[QADMIN] Trying to login multiple times, eh?\n");
		QMM_RET_SUPERCEDE(1);
	}

	std::string password = args[1];

	for (auto info : g_userinfo) {
		std::string match = g_playerinfo[clientnum].name;
		if (info.type == au_ip)
			match = g_playerinfo[clientnum].ip;
		else if (info.type == au_id)
			match = g_playerinfo[clientnum].guid;

		if (str_striequal(info.user, match) && str_striequal(info.pass, password)) {
			g_playerinfo[clientnum].access = info.access;
			g_playerinfo[clientnum].authed = 1;
			ClientPrint(clientnum, QMM_VARARGS("[QADMIN] You have successfully authenticated. You now have %d access.\n", g_playerinfo[clientnum].access));
			QMM_RET_SUPERCEDE(1);
		}
	}

	QMM_RET_SUPERCEDE(1);
}


/*int admin_ban(int clientnum, int datanum, int access) {
	char bancmd[MAX_COMMAND_LENGTH], user[MAX_USER_LENGTH];
	QMM_ARGV(datanum - 1, bancmd, sizeof(bancmd));
	QMM_ARGV(datanum, user, sizeof(user));

	char* message = concatargs(datanum+1);

	// if we are banning a player name, grab his ID or IP and then store in the arg strings
	// so the admin_banid/admin_banip checks will do the actual work
	if (!strcmp(bancmd, "admin_ban")) {
		int slotid = namematch(user);
		if (slotid < 0) {
			ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Ambiguous match or match not found for '%s'\n", user));
			QMM_RET_SUPERCEDE(1);
		}

		char guidban[MAX_NUMBER_LENGTH];

		if (g_syscall(G_ARGC) > 2)
			QMM_ARGV(datanum + 1, guidban, sizeof(guidban));

		// 'guid' was given
		if (g_syscall(G_ARGC) > 2 && !strcmp(guidban, "guid")) {
			strncpy(user, g_playerinfo[slotid].guid, sizeof(user));
			strncpy(bancmd, "admin_banid", sizeof(bancmd));
			message = concatargs(datanum+2);
		} else {
			strncpy(user, g_playerinfo[slotid].name, sizeof(user));
			strncpy(bancmd, "admin_banip", sizeof(bancmd));
		}
		g_syscall(G_DROP_CLIENT, slotid, message);
	}

	if (!strcmp(bancmd, "admin_banid")) {
		g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("addid %s \"%s\"\n", user, message));
		ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Banned ID %s\n", user));
	} else if (!strcmp(bancmd, "admin_banip")) {
		g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("addip %s \"%s\"\n", user, message));
		ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Banned IP %s\n", user));
	}
	

	QMM_RET_SUPERCEDE(1);
}

// TODO: addid and removeid commands to emulate addip/removeip for GUIDs
int admin_unban(int clientnum, int datanum, int access) {
	char unbancmd[MAX_COMMAND_LENGTH], user[MAX_USER_LENGTH];
	QMM_ARGV(datanum - 1, unbancmd, sizeof(unbancmd));
	QMM_ARGV(datanum, user, sizeof(user));

	// if the user used admin_unban, autodetect the usertype by checking for "."
	if (!strcmp(unbancmd, "admin_unban")) {
		if (strstr(user, "."))
			strncpy(unbancmd, "admin_unbanip", sizeof(unbancmd));
		else
			strncpy(unbancmd, "admin_unbanid", sizeof(unbancmd));
	}

	if (!strcmp(unbancmd, "admin_unbanid")) {
		g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("removeid %s\n", user));
		ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Unbanned ID %s\n", user));
	} else if (!strcmp(unbancmd, "admin_unbanip")) {
		g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("removeip %s\n", user));
		ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Unbanned IP %s\n", user));
	}

	QMM_RET_SUPERCEDE(1);
}
*/


int admin_ban(int clientnum, int access, std::vector<std::string> args, bool say) {
	std::string bancmd = args[0];
	std::string user = args[1];

	std::string message = str_join(args, 2);
	if (message.empty())
		message = "Banned by Admin";

	if (str_striequal(bancmd, "admin_ban") || str_striequal(bancmd, "admin_banslot")) {
		int slotid;
		if (str_striequal(bancmd, "admin_ban")) {
			slotid = namematch(user);
			if (slotid < 0) {
				ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Ambiguous match or match not found for '%s'\n", user.c_str()));
				QMM_RET_SUPERCEDE(1);
			}
		} else {
			slotid = atoi(user.c_str());
			if (slotid < 0 || slotid >= MAX_CLIENTS || !g_playerinfo[slotid].connected) {
				ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Invalid slot id '%d'\n", slotid));
				QMM_RET_SUPERCEDE(1);
			}
		}
		
		// check if the desired user has immunity
		if (player_has_access(slotid, ACCESS_IMMUNITY)) {
			ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Cannot ban %s, user has immunity.\n", g_playerinfo[slotid].name.c_str()));
			QMM_RET_SUPERCEDE(1);
		}

		// check if other users have the same IP as the desired user
		bool immunity = false;
		
		// find users who have the given IP
		int finduser = player_with_ip(g_playerinfo[slotid].ip);

		// loop until no more users have the IP
		while (finduser != -1) {
			// if this user has immunity, set the flag
			if (player_has_access(finduser, ACCESS_IMMUNITY)) {
				immunity = true;
				break;
			}

			// get next user with IP
			finduser = player_with_ip(g_playerinfo[slotid].ip, finduser);
		}

		// if no users with immunity have the IP, ban the IP and kick the user
		if (!immunity) {
			g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("addip \"%s\" \"%s\"\n", g_playerinfo[slotid].ip.c_str(), message.c_str()));
			ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Banned %s by IP (%s): '%s'\n", g_playerinfo[slotid].name.c_str(), g_playerinfo[slotid].ip.c_str(), message.c_str()));
			KickClient(slotid, message);
		}

		// else at least 1 user with immunity has the given IP
		else {
			ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Cannot ban %s by IP, another user with that IP (%s) has immunity.\n", g_playerinfo[slotid].name.c_str(), g_playerinfo[slotid].ip.c_str()));
		}

		// but still kick the users on the IP without immunity
		finduser = player_with_ip(g_playerinfo[slotid].ip);

		// loop until no more users have the IP
		while (finduser != -1) {
			// if this user does not have immunity, kick him
			if (!player_has_access(finduser, ACCESS_IMMUNITY))
				KickClient(finduser, message);

			// get next user with IP
			finduser = player_with_ip(g_playerinfo[slotid].ip, finduser);
		}

	}
	// user arg is an IP
	else if (str_striequal(bancmd, "admin_banip")) {
		bool immunity = false;
		
		// find if any users who have the given IP are immune
		int finduser = player_with_ip(user);
		while (finduser != -1) {
			// if this user has immunity, set the flag
			if (player_has_access(finduser, ACCESS_IMMUNITY)) {
				immunity = true;
				break;
			}
			finduser = player_with_ip(user, finduser);
		}
		
		// if no users with immunity have the IP, ban the IP
		if (!immunity) {
			g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("addip \"%s\" \"%s\"\n", user.c_str(), message.c_str()));
			ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Banned IP %s: '%s'\n", user.c_str(), message.c_str()));
		}
		
		// else at least 1 user with immunity has the given IP
		else {
			ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Cannot ban IP %s, a user with that IP has immunity. Kicking non-immune users.\n", user.c_str()));
		}

		// but still kick the users without immunity
		finduser = player_with_ip(user);
		while (finduser != -1) {
			// if this user does not have immunity, kick him
			if (!player_has_access(finduser, ACCESS_IMMUNITY))
				KickClient(finduser, message);
			finduser = player_with_ip(user, finduser);
		}
	
	}	

	QMM_RET_SUPERCEDE(1);
}


int admin_unban(int clientnum, int access, std::vector<std::string> args, bool say) {
	std::string ip = sanitize(args[1]);

	g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("removeip \"%s\"\n", ip.c_str()));
	ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Unbanned IP %s\n", ip.c_str()));

	QMM_RET_SUPERCEDE(1);
}


int admin_cfg(int clientnum, int access, std::vector<std::string> args, bool say) {
	std::string file = sanitize(args[1]);

	g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("exec \"%s\"\n", file.c_str()));

	QMM_RET_SUPERCEDE(1);
}


int admin_rcon(int clientnum, int access, std::vector<std::string> args, bool say) {
	std::string str = str_join(args, 1);
	g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("%s\n", str.c_str()));

	QMM_RET_SUPERCEDE(1);
}


int admin_hostname(int clientnum, int access, std::vector<std::string> args, bool say) {
	g_syscall(G_CVAR_SET, "sv_hostname", args[1].c_str());

	QMM_RET_SUPERCEDE(1);
}


int admin_friendlyfire(int clientnum, int access, std::vector<std::string> args, bool say) {
	g_syscall(G_CVAR_SET, "g_friendlyfire", args[1].c_str());

	QMM_RET_SUPERCEDE(1);
}


int admin_gravity(int clientnum, int access, std::vector<std::string> args, bool say) {
	g_syscall(G_CVAR_SET, "g_gravity", args[1].c_str());

	QMM_RET_SUPERCEDE(1);
}


int admin_gametype(int clientnum, int access, std::vector<std::string> args, bool say) {
	g_syscall(G_CVAR_SET, "g_gametype", args[1].c_str());

	QMM_RET_SUPERCEDE(1);
}


int admin_map(int clientnum, int access, std::vector<std::string> args, bool say) {
	std::string map = sanitize(args[1]);
	g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("map \"%s\"\n", args[1].c_str()));

	QMM_RET_SUPERCEDE(1);
}


int admin_fraglimit(int clientnum, int access, std::vector<std::string> args, bool say) {
	g_syscall(G_CVAR_SET, "fraglimit", args[1].c_str());

	QMM_RET_SUPERCEDE(1);
}


int admin_timelimit(int clientnum, int access, std::vector<std::string> args, bool say) {
	g_syscall(G_CVAR_SET, "timelimit", args[1].c_str());

	QMM_RET_SUPERCEDE(1);
}


int admin_pass(int clientnum, int access, std::vector<std::string> args, bool say) {
	if (str_striequal(args[0], "admin_pass")) {
		g_syscall(G_CVAR_SET, "g_password", args[1].c_str());
		g_syscall(G_CVAR_SET, "g_needpass", "1");
	} else if (str_striequal(args[0], "admin_nopass")) {
		g_syscall(G_CVAR_SET, "g_password", "");
		g_syscall(G_CVAR_SET, "g_needpass", "0");
	}

	QMM_RET_SUPERCEDE(1);
}


int admin_chat(int clientnum, int access, std::vector<std::string> args, bool say) {
	std::string message = sanitize(str_join(args, 1));
	for (int i = 0; i < MAX_CLIENTS; ++i) {
		if (player_has_access(i, access))
			ClientPrint(i, QMM_VARARGS("To Admins From %s: %s", clientnum == SERVER_CONSOLE ? "Console" : g_playerinfo[clientnum].name.c_str(), message.c_str()), true);
	}

	QMM_RET_SUPERCEDE(1);
}


int admin_csay(int clientnum, int access, std::vector<std::string> args, bool say) {
	std::string message = sanitize(str_join(args, 1));
#ifdef GAME_NO_SEND_SERVER_COMMAND
	g_syscall(G_CPRINTF, ENT_FROM_NUM(clientnum), PRINT_HIGH, "%s", message.c_str());
#else
	g_syscall(G_SEND_SERVER_COMMAND, -1, QMM_VARARGS("cp \"%s\n\"", message.c_str()));
#endif

	QMM_RET_SUPERCEDE(1);
}


int admin_say(int clientnum, int access, std::vector<std::string> args, bool say) {
	std::string message = sanitize(str_join(args, 1));
	ClientPrint(-1, message.c_str(), true);

	QMM_RET_SUPERCEDE(1);
}


int admin_psay(int clientnum, int access, std::vector<std::string> args, bool say) {
	std::string user = args[1];

	int slotid = namematch(user);

	if (slotid < 0) {
		ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Ambiguous match or match not found for '%s'\n", user.c_str()));
		QMM_RET_SUPERCEDE(1);
	}

	std::string message = sanitize(str_join(args, 2));	
	std::string toname = g_playerinfo[slotid].name;

	ClientPrint(clientnum, QMM_VARARGS("Private Message To %s: %s", toname.c_str(), message.c_str()), true);
	ClientPrint(slotid, QMM_VARARGS("Private Message From %s: %s", clientnum == SERVER_CONSOLE ? "Console" : toname.c_str(), message.c_str()), true);

	QMM_RET_SUPERCEDE(1);
}


int admin_listmaps(int clientnum, int access, std::vector<std::string> args, bool say) {
#ifndef GAME_NO_FS_GETFILELIST
	char dirlist[MAX_STRING_LENGTH];
	char* dirptr = dirlist;

	int numfiles = g_syscall(G_FS_GETFILELIST, "maps", ".bsp", dirlist, sizeof(dirlist));

	ClientPrint(clientnum, "[QADMIN] Listing maps...\n");
	for (int i = 1; i < numfiles; ++i) {
		dirptr += strlen(dirptr);
		*dirptr = '\n';
	}
	ClientPrint(clientnum, dirlist);
	ClientPrint(clientnum, "\n[QADMIN] End of maps list\n");
#endif
	QMM_RET_SUPERCEDE(1);
}


int admin_kick(int clientnum, int access, std::vector<std::string> args, bool say) {
	std::string kickcmd = args[0];
	std::string user = args[1];

	int slotid = 0;

	if (str_striequal(kickcmd, "admin_kick")) {
		slotid = namematch(user);
		if (slotid < 0) {
			ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Ambiguous match or match not found for '%s'\n", user.c_str()));
			QMM_RET_SUPERCEDE(1);
		}
	} else if (str_striequal(kickcmd, "admin_kickslot")) {
		slotid = atoi(user.c_str());
		if (slotid < 0 || slotid >= MAX_CLIENTS || !g_playerinfo[slotid].connected) {
			ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Invalid slot id '%d'\n", slotid));
			QMM_RET_SUPERCEDE(1);
		}
	}

	if (player_has_access(slotid, ACCESS_IMMUNITY)) {
		ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Cannot kick %s, user has immunity\n", g_playerinfo[slotid].name.c_str()));
		QMM_RET_SUPERCEDE(1);
	}
	
	std::string message = sanitize(str_join(args, 2));
	if (message.empty())
		message = "Kicked by Admin";
	
	ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Kicked %s: '%s'\n", g_playerinfo[slotid].name.c_str(), message.c_str()));
	KickClient(slotid, message);

	QMM_RET_SUPERCEDE(1);
}


int admin_reload(int clientnum, int access, std::vector<std::string> args, bool say) {
	reload();

	QMM_RET_SUPERCEDE(1);
}


int admin_userlist(int clientnum, int access, std::vector<std::string> args, bool say) {
	std::string match;
	if (args.size() > 1)
		match = args[1];

	int banaccess = player_has_access(clientnum, LEVEL_256);

	// if a parameter was given, only display users matching it
	if (args.size() > 1) {
		match = args[1];
		ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Listing users matching '%s'...\n", match.c_str()));
	}
	else {
		ClientPrint(clientnum, "[QADMIN] Listing users...\n");
	}

	if (banaccess)
		ClientPrint(clientnum, "[QADMIN] Slot Access   Authed IP              Name\n");
	else
		ClientPrint(clientnum, "[QADMIN] Slot Access   Authed Name\n");

	if (!match.empty()) {
		// loop through every user that matches the given string
		// (the 1 passed to namematch means to not check ambiguity)
		int i = namematch(match, true);
		while (i != -1) {
			playerinfo_t& info = g_playerinfo[i];
			if (banaccess)
				ClientPrint(clientnum, QMM_VARARGS("[QADMIN] %3d: %-8d %-6s %-15s %s\n", i, info.access, info.authed ? "yes" : "no", info.ip.c_str(), info.name.c_str()));
			else
				ClientPrint(clientnum, QMM_VARARGS("[QADMIN] %3d: %-8d %-6s %s\n", i, info.access, info.authed ? "yes" : "no", info.name.c_str()));
			i = namematch(match, true, i);
		}
	} else {
		for (size_t i = 0; i < g_playerinfo.size(); i++) {
			playerinfo_t& info = g_playerinfo[i];
			if (!info.connected)
				continue;

			if (banaccess)
				ClientPrint(clientnum, QMM_VARARGS("[QADMIN] %3d: %-8d %-6s %-15s %s", i, info.access, info.authed ? "yes" : "no", info.ip, info.name));
			else
				ClientPrint(clientnum, QMM_VARARGS("[QADMIN] %3d: %-8d %-6s %s\n", i, info.access, info.authed ? "yes" : "no", info.name));
		}
	}

	QMM_RET_SUPERCEDE(1);
}


int admin_gag(int clientnum, int access, std::vector<std::string> args, bool say) {
	int slotid = 0;

	std::string gagcmd = args[0];
	std::string user = args[1];

	if (str_striequal(gagcmd, "admin_gag")) {
		slotid = namematch(user);
		if (slotid < 0) {
			ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Ambiguous match or match not found for '%s'\n", user.c_str()));
			QMM_RET_SUPERCEDE(1);
		}
	} else if (str_striequal(gagcmd, "admin_gagslot")) {
		slotid = atoi(user.c_str());
		if (slotid < 0 || slotid >= MAX_CLIENTS || !g_playerinfo[slotid].connected) {
			ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Invalid slot id '%d'\n", slotid));
			QMM_RET_SUPERCEDE(1);
		}
	}

	if (player_has_access(slotid, ACCESS_IMMUNITY)) {
		ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Cannot gag %s, user has immunity\n", g_playerinfo[slotid].name.c_str()));
	}
	else if (g_playerinfo[slotid].gagged) {
		ClientPrint(clientnum, QMM_VARARGS("[QADMIN] %s is already gagged\n", g_playerinfo[slotid].name.c_str()));
	}
	else {
		g_playerinfo[slotid].gagged = true;
		ClientPrint(clientnum, QMM_VARARGS("[QADMIN] %s has been gagged\n", g_playerinfo[slotid].name.c_str()));
	}

	QMM_RET_SUPERCEDE(1);
}


int admin_ungag(int clientnum, int access, std::vector<std::string> args, bool say) {
	int slotid = 0;

	std::string ungagcmd = args[0];
	std::string user = args[1];

	if (str_striequal(ungagcmd, "admin_ungag")) {
		slotid = namematch(user);
		if (slotid < 0) {
			ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Ambiguous match or match not found for '%s'\n", user.c_str()));
			QMM_RET_SUPERCEDE(1);
		}
	} else if (str_striequal(ungagcmd, "admin_ungagslot")) {
		slotid = atoi(user.c_str());
		if (slotid < 0 || slotid >= MAX_CLIENTS || !g_playerinfo[slotid].connected) {
			ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Invalid slot id '%d'\n", slotid));
			QMM_RET_SUPERCEDE(1);
		}
	}

	if (g_playerinfo[slotid].gagged) {
		g_playerinfo[slotid].gagged = false;
		ClientPrint(clientnum, QMM_VARARGS("[QADMIN] %s has been ungagged\n", g_playerinfo[slotid].name.c_str()));
	} else {
		ClientPrint(clientnum, QMM_VARARGS("[QADMIN] %s is not gagged\n", g_playerinfo[slotid].name.c_str()));
	}

	QMM_RET_SUPERCEDE(1);
}


int admin_currentmap(int clientnum, int access, std::vector<std::string> args, bool say) {
	ClientPrint(say ? -1 : clientnum, QMM_VARARGS("[QADMIN] The current map is: %s\n", QMM_GETSTRCVAR("mapname")));
	QMM_RETURN(say ? QMM_IGNORED : QMM_SUPERCEDE, 1);
}


int admin_timeleft(int clientnum, int access, std::vector<std::string> args, bool say) {
	intptr_t timelimit = g_syscall(G_CVAR_VARIABLE_INTEGER_VALUE, "timelimit");
	if (!timelimit) {
		ClientPrint(say ? -1 : clientnum, "[QADMIN] There is no time limit.\n");
		QMM_RETURN(say ? QMM_IGNORED : QMM_SUPERCEDE, 1);
	}
	time_t timeleft = (timelimit * 60) - (time(NULL) - g_mapstart);

	if (timeleft <= 0)
		ClientPrint(say ? -1 : clientnum, "[QADMIN] Time limit has been reached\n");
	else
		ClientPrint(say ? -1 : clientnum, QMM_VARARGS("[QADMIN] Time remaining: %lu minute(s) %lu second(s)\n", timeleft / 60, timeleft % 60));

	QMM_RETURN(say ? QMM_IGNORED : QMM_SUPERCEDE, 1);
}


void handle_vote_map(int winner, int winvotes, int totalvotes, void* param) {
	std::string map = *(std::string*)param;
	if (winner == 1) {
		ClientPrint(-1, QMM_VARARGS("[QADMIN] Vote to change map to %s was successful\n", map.c_str()));
		g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, QMM_VARARGS("map \"%s\"\n", map.c_str()));
	} else {
		ClientPrint(-1, QMM_VARARGS("[QADMIN] Vote to change map to %s has failed\n", map.c_str()));
	}
}


int admin_vote_map(int clientnum, int access, std::vector<std::string> args, bool say) {
	// this is static so that it still exists when passed to handle_vote_map as param
	static std::string map;

	int time = (int)QMM_GETINTCVAR("admin_vote_map_time");

	map = args[1];

	if (!is_valid_map(map)) {
		ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Unknown map '%s'\n", map.c_str()));
		QMM_RET_SUPERCEDE(1);
	}

	ClientPrint(-1, QMM_VARARGS("[QADMIN] A %d second vote has been started to changed map to %s\n", time, map.c_str()));
	ClientPrint(-1, "[QADMIN] Type 'castvote 1' for YES, or 'castvote 2' for NO\n");
	
	vote_start(clientnum, handle_vote_map, time, 2, (void*)&map);

	QMM_RET_SUPERCEDE(1);
}


void handle_vote_kick(int winner, int winvotes, int totalvotes, void* param) {
	int slotid = (int)(intptr_t)param;
	
	// user may have authed and gotten immunity during the vote,
	// but don't mention it, just make the vote fail
	if (player_has_access(slotid, ACCESS_IMMUNITY))
		winner = 0;

	if (winner == 1 && winvotes) {
		ClientPrint(-1, QMM_VARARGS("[QADMIN] Vote to kick %s was successful\n", g_playerinfo[slotid].name));
		KickClient(slotid, "Kicked due to vote.");
	} else {
		ClientPrint(-1, QMM_VARARGS("[QADMIN] Vote to kick %s has failed\n", g_playerinfo[slotid].name));
	}
}


int admin_vote_kick(int clientnum, int access, std::vector<std::string> args, bool say) {
	int slotid;
	int votetime = (int)QMM_GETINTCVAR("admin_vote_kick_time");

	std::string user = args[1];
	slotid = namematch(user);
	if (slotid < 0) {
		ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Ambiguous match or match not found for '%s'\n", user.c_str()));
		QMM_RET_SUPERCEDE(1);
	}
	
	// cannot votekick a user with immunity
	// the user's immunity is also checked in the vote
	// handler in case he auths before the vote ends
	if (player_has_access(slotid, ACCESS_IMMUNITY)) {
		ClientPrint(clientnum, QMM_VARARGS("[QADMIN] Cannot kick %s, user has immunity\n", g_playerinfo[slotid].name.c_str()));
		QMM_RET_SUPERCEDE(1);
	}

	ClientPrint(-1, QMM_VARARGS("[QADMIN] A %d second vote has been started to kick %s\n", votetime, g_playerinfo[slotid].name.c_str()));
	ClientPrint(-1, "[QADMIN] Type 'castvote 1' for YES, or 'castvote 2' for NO\n");
	vote_start(clientnum, handle_vote_kick, votetime, 2, (void*)(intptr_t)slotid);

	QMM_RET_SUPERCEDE(1);
}


int admin_vote_abort(int clientnum, int access, std::vector<std::string> args, bool say) {
	ClientPrint(-1, "[QADMIN] The current vote has been aborted\n");
	g_vote.inuse = false;

	QMM_RET_SUPERCEDE(1);
}


// say handler (need to handle subcommands)
int say(int clientnum, int access, std::vector<std::string> args, bool say) {
	if (g_playerinfo[clientnum].gagged) {
		ClientPrint(clientnum, "[QADMIN] Sorry, you have been gagged.\n");
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
			ClientPrint(clientnum, QMM_VARARGS("[QADMIN] You do not have access to that command: '%s'\n", saycmd.cmd));

			QMM_RET_SUPERCEDE(1);
		}
	}

	QMM_RET_IGNORED(0);
}


int castvote(int clientnum, int access, std::vector<std::string> args, bool say) {
	if (clientnum == SERVER_CONSOLE) {
		ClientPrint(clientnum, "[QADMIN] Trying to vote from the server console, eh?\n");
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
	{ "admin_banip",		admin_ban,			LEVEL_256,	1, "admin_banip <ip> [message]", "Bans the specified IP" },
	{ "admin_banslot",		admin_ban,			LEVEL_256,	1, "admin_banslot <index> [message]", "Bans the user in the specified slot" },
	{ "admin_cfg",			admin_cfg,			LEVEL_512,	1, "admin_cfg <file.cfg>", "Executes the given .cfg file on the server" },
	{ "admin_chat",			admin_chat,			LEVEL_64,	1, "admin_chat <text>", "Sends the message to all admins with admin_chat access" },
	{ "admin_csay",			admin_csay,			LEVEL_64,	1, "admin_csay <text>", "Displays message to all players in center of screen" },
	{ "admin_currentmap",	admin_currentmap,	LEVEL_0,	0, "admin_currentmap", "Displays current map" },
	{ "admin_fraglimit",	admin_fraglimit,	LEVEL_2,	1, "admin_fraglimit <value>", "Sets the server's fraglimit" },
	{ "admin_friendlyfire",	admin_friendlyfire,	LEVEL_32,	1, "admin_friendlyfire <value>", "Sets the server's friendlyfire" },
	{ "admin_gag",			admin_gag,			LEVEL_2048,	1, "admin_gag <name>", "Gags the specified player from speaking" },
	{ "admin_gagslot",		admin_gag,			LEVEL_2048,	1, "admin_gagslot <index>", "Gags the player in the specified slot from speaking" },
	{ "admin_gametype",		admin_gametype,		LEVEL_32,	1, "admin_gametype <value>", "Sets the server's gametype" },
	{ "admin_gravity",		admin_gravity,		LEVEL_32,	1, "admin_gravity <value>", "Sets the server's gravity" },
	{ "admin_help",			admin_help,			LEVEL_0,	0, "admin_help [start]", "Displays commands you have access to" },
	{ "admin_hostname",		admin_hostname,		LEVEL_512,	1, "admin_hostname <new name>", "Sets the server's hostname" },
	{ "admin_kick",			admin_kick,			LEVEL_128,	1, "admin_kick <name> [message]", "Kicks name from the server" },
	{ "admin_kickslot",		admin_kick,			LEVEL_128,	1, "admin_kickslot <index> [message]", "Kicks user with given slot from the server" },
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
	{ "admin_ungagslot",	admin_ungag,		LEVEL_2048,	1, "admin_ungagslot <index>", "Ungags the player in the specified slot" },
	{ "admin_userlist",		admin_userlist,		LEVEL_0,	0, "admin_userlist [name]", "Lists all users on the server that match 'name'" },
	{ "admin_vote_abort",	admin_vote_abort,	LEVEL_2,	1, "admin_vote_abort", "Aborts the current map or kick vote" },
	{ "admin_vote_kick",	admin_vote_kick,	LEVEL_1,	1, "admin_vote_kick <user>", "Initiates a vote to kick the user" },
	{ "admin_vote_map",		admin_vote_map,		LEVEL_1,	1, "admin_vote_map <map>", "Initiates a vote to change to the map" },
	{ "castvote",			castvote,			LEVEL_1,	1, "castvote <option>", "Places a vote for the given option" },

	{ "say",				say,				LEVEL_0,	0, NULL, NULL },

//	{ "admin_ban",		admin_ban,				LEVEL_256,	1, "admin_ban <name> ['guid'] [message]", "Bans the specified user by IP, unless 'id' is specified" },
//	{ "admin_banid",	admin_ban,				LEVEL_256,	1, "admin_banid <guid> [message]", "Bans the specified GUID" },
//	{ "admin_banip",	admin_ban,				LEVEL_256,	1, "admin_banip <ip> [message]", "Bans the specified IP" },
//	{ "admin_unban",	admin_unban,			LEVEL_256,	1, "admin_unban <guid|ip>", "Unbans the specified GUID or IP" },
//	{ "admin_unbanid",	admin_unban,			LEVEL_256,	1, "admin_unbanid <guid>", "Unbans the specified GUID" },
//	{ "admin_unbanip",	admin_unban,			LEVEL_256,	1, "admin_unbanip <ip>", "Unbans the specified IP" },
};

std::vector<admincmd_t> g_saycmds = {
	{ "admin_login",	admin_login,			LEVEL_0,	1, NULL, NULL },
	{ "castvote",		castvote,				LEVEL_1,	1, NULL, NULL },
	{ "currentmap",		admin_currentmap,		LEVEL_0,	0, NULL, NULL },
	{ "timeleft",		admin_timeleft,			LEVEL_0,	0, NULL, NULL },
};
