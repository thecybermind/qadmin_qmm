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

#include <vector>
#include <string>
#include <cstdint>
#include "main.h"
#include "util.h"

#ifdef WIN32
 #ifndef vsnprintf
  #define vsnprintf _vsnprintf
 #endif
#endif

bool player_has_access(intptr_t clientnum, int reqaccess) {
	if (clientnum == SERVER_CONSOLE)
		return true;
	
	if (!g_playerinfo.count(clientnum))
		return false;

	int defaccess = (int)QMM_GETINTCVAR("admin_default_access");

	int access = g_playerinfo[clientnum].access | defaccess;

	return (access & reqaccess) == reqaccess;
}


void player_clientprint(intptr_t clientnum, const char* msg, bool chat) {
	if (clientnum == SERVER_CONSOLE)
		g_syscall(G_PRINT, msg);
	else {
#ifdef GAME_NO_SEND_SERVER_COMMAND
		g_syscall(G_CPRINTF, clientnum, PRINT_HIGH, "%s", msg);
#else
		if (chat)
			g_syscall(G_SEND_SERVER_COMMAND, clientnum, QMM_VARARGS("chat \"%s\"", msg));
		else
			g_syscall(G_SEND_SERVER_COMMAND, clientnum, QMM_VARARGS("print \"%s\"", msg));
#endif
	}
}


void player_kick(intptr_t clientnum, std::string message) {
#ifdef GAME_CLIENT_ENT_PTRS
	edict_t* ent = (edict_t*)clientnum;
	clientnum = ent->s.number;
#endif
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
		if (str_striequal(find, playerinfo.second.name) || str_striequal(find, playerinfo.second.stripname)) {
			ret.clear();
			ret.push_back(playerinfo.first);
			return ret;
		}
		else if (str_stristr(find, playerinfo.second.name) || str_stristr(find, playerinfo.second.stripname)) {
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
	fileHandle_t fmap;
#ifdef GAME_MOHAA
	intptr_t mapsize = g_syscall(G_FS_FOPEN_FILE_QMM, QMM_VARARGS("maps\\%s", map.c_str()), &fmap, FS_READ);
	g_syscall(G_FS_FCLOSE_FILE_QMM, fmap);
#else
	int mapsize = (int)g_syscall(G_FS_FOPEN_FILE, QMM_VARARGS("maps\\%s", map.c_str()), &fmap, FS_READ);
	g_syscall(G_FS_FCLOSE_FILE, fmap);
#endif
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

	return ret;
}


std::vector<std::string> parse_args(int start, int end) {
	if (end < 0)
		end = (int)g_syscall(G_ARGC);

	char temp[MAX_STRING_LENGTH];
	std::string s;

	for (int i = start; i <= end; i++) {
		QMM_ARGV(i, temp, sizeof(temp));
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
