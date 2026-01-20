/*
QADMIN_QMM - Server Administration Plugin
Copyright 2004-2026
https://github.com/thecybermind/qadmin_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < k.m.masterson@gmail.com >

*/

#define _CRT_SECURE_NO_WARNINGS 1

#include <qmmapi.h>

#include "version.h"
#include "game.h"

#include <vector>
#include <string>
#include <cstdint>
#include "main.h"
#include "util.h"


bool player_has_access(intptr_t clientnum, int reqaccess) {
	if (clientnum == SERVER_CONSOLE)
		return true;
	
	if (!g_playerinfo.count(clientnum))
		return false;

	int defaccess = (int)QMM_GETINTCVAR(PLID, "admin_default_access");

	int access = g_playerinfo[clientnum].access | defaccess;

	return (access & reqaccess) == reqaccess;
}


void player_clientprint(intptr_t clientnum, const char* msg, bool chat) {
	if (clientnum == SERVER_CONSOLE) {
		g_syscall(G_PRINT, msg);
		return;
	}
#ifdef GAME_NO_SEND_SERVER_COMMAND
	if (clientnum == -1) {
		for (auto& p : g_playerinfo) {
			g_syscall(G_CPRINTF, p.first, PRINT_HIGH, "%s", msg);
		}
		return;
	}
	g_syscall(G_CPRINTF, clientnum, PRINT_HIGH, "%s", msg);
#else
	if (chat)
		g_syscall(G_SEND_SERVER_COMMAND, clientnum, QMM_VARARGS(PLID, "chat \"%s\"", msg));
	else
		g_syscall(G_SEND_SERVER_COMMAND, clientnum, QMM_VARARGS(PLID, "print \"%s\"", msg));
#endif
}


void player_kick(intptr_t clientnum, std::string message) {
	g_syscall(G_DROP_CLIENT, clientnum, message.c_str());
}


std::string strip_codes(std::string name) {
#ifdef GAME_NO_NAME_COLOR
	return name;
#else
	std::string ret;
	for (size_t i = 0; i < name.size(); ++i) {
		if (name[i] == Q_COLOR_ESCAPE) {
			if (name[i + 1] != Q_COLOR_ESCAPE)
				i++;
			continue;
		}
		ret += name[i];
	}

	return ret;
#endif
}


// returns vector of indexes of partial or full matching name
std::vector<intptr_t> players_with_name(std::string find) {
	std::vector<intptr_t> ret;

	for (auto& playerinfo : g_playerinfo) {
		// for exact match, return just this player
		if (str_striequal(playerinfo.second.name, find) || str_striequal(playerinfo.second.stripname, find)) {
			ret.clear();
			ret.push_back(playerinfo.first);
			return ret;
		}
		else if (str_stristr(playerinfo.second.name, find) || str_stristr(playerinfo.second.stripname, find)) {
			ret.push_back(playerinfo.first);
		}
	}

	return ret;
}


// returns vector of indexes with matching ip
std::vector<intptr_t> players_with_ip(std::string find) {
	std::vector<intptr_t> ret;

	for (auto& playerinfo : g_playerinfo) {
		if (playerinfo.second.ip == find)
			ret.push_back(playerinfo.first);
	}

	return ret;
}


bool is_valid_map(std::string map) {
// games that don't have readability into pak/pk3 files, just return true
#ifdef GAME_MOHAA
	return true;
#else
	if (G_FS_FOPEN_FILE < 0)
		return true;
#endif

	fileHandle_t fmap;

	intptr_t mapsize = (int)g_syscall(G_FS_FOPEN_FILE, QMM_VARARGS(PLID, "maps/%s.bsp", map.c_str()), &fmap, FS_READ);
	// doesn't exist, return immediately
	if (mapsize < 0)
		return false;
	// if file was 0 bytes, we still need to close, but return false
	g_syscall(G_FS_FCLOSE_FILE, fmap);
	return mapsize ? true : false;
}


std::string str_sanitize(std::string str) {
	size_t sep = str.find_first_of("\";\\");
	while (sep != std::string::npos) {
		str[sep] = ' ';
		sep = str.find_first_of("\";\\");
	}
	return str;
}


int str_stristr(std::string haystack, std::string needle) {
	for (auto& c : haystack)
		c = (char)std::tolower((unsigned char)c);
	for (auto& c : needle)
		c = (char)std::tolower((unsigned char)c);

	return haystack.find(needle) != std::string::npos;
}


int str_stricmp(std::string s1, std::string s2) {
	for (auto& c : s1)
		c = (char)std::tolower((unsigned char)c);
	for (auto& c : s2)
		c = (char)std::tolower((unsigned char)c);

	return s1.compare(s2);
}


int str_striequal(std::string s1, std::string s2) {
	return str_stricmp(s1, s2) == 0;
}


std::vector<std::string> parse_str(std::string str, char sep) {
	std::vector<std::string> ret;
	
	size_t f = str.find(sep);
	while (f != std::string::npos) {
		ret.push_back(str.substr(0, f));
		str = str.substr(f + 1);
		f = str.find(sep);
	}
	if (str.size())
		ret.push_back(str);

	return ret;
}


// append all args to a single string, separated by spaces
// this is done so say commands which have the entire text in arg1 will still parse correctly
std::vector<std::string> parse_args(int start) {
	int end = (int)g_syscall(G_ARGC) - 1;

	char temp[MAX_STRING_LENGTH];
	std::string s;

	for (int i = start; i <= end; i++) {
		QMM_ARGV(PLID, i, temp, sizeof(temp));
		if (i != start)
			s += " ";
		s += temp;
	}

	return parse_str(s, ' ');
}


std::string str_join(std::vector<std::string> arr, size_t start, char delim) {
	bool first = true;
	std::string ret;
	for (size_t i = start; i < arr.size(); i++) {
		if (!first)
			ret += delim;
		ret += arr[i];
		first = false;
	}

	return ret;
}
