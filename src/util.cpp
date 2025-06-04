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

#include <stdarg.h>
#include <stdio.h>
#include "main.h"
#include "util.h"

#ifdef WIN32
 #ifndef vsnprintf
  #define vsnprintf _vsnprintf
 #endif
#endif

bool has_access(int clientnum, int reqaccess) {
	if (clientnum == SERVER_CONSOLE)
		return 1;
	
	if (clientnum < 0 || clientnum >= MAX_CLIENTS)
		return 0;

	return ((g_playerinfo[clientnum].access & reqaccess) == reqaccess);
}

// returns the first slot id with given ip starting after the slot id specified in 'start_after'
int user_with_ip(const char* ip, int start_after) {
	for (int i = start_after + 1; i < MAX_CLIENTS; ++i) {
		if (!strcmp(ip, g_playerinfo[i].ip))
			return i;
	}

	return -1;
}

int arrsize(admincmd_t* arr) {
	int i;
	for (i = 0; arr[i].cmd && arr[i].func; ++i);
	return i;
}

void ClientPrint(int clientnum, const char* msg, bool chat) {
	// remove any QMM_VARARGS() conflicts by making a temp string
	static char temp[MAX_STRING_LENGTH];
	strncpy(temp, msg, MAX_STRING_LENGTH);
	msg = (const char*)temp;

	if (clientnum == SERVER_CONSOLE)
		g_syscall(G_PRINT, msg);
	else {
		if (chat)
			g_syscall(G_SEND_SERVER_COMMAND, clientnum, QMM_VARARGS("chat \"%s\"", msg));
		else
			g_syscall(G_SEND_SERVER_COMMAND, clientnum, QMM_VARARGS("print \"%s\"", msg));
	}
}

char* concatargs(int min) {
	static char text[MAX_DATA_LENGTH];
	char arg[MAX_DATA_LENGTH];
	int max = g_syscall(G_ARGC);
	size_t x = 1;
	text[0] = '\0';
	for (int i = min; i < max; ++i) {
		QMM_ARGV(i, arg, sizeof(arg));
		strncat(text, arg, sizeof(text) - x);
		x += strlen(arg);
		strncat(text, " ", sizeof(text) - x);
		++x;
	}
	
	text[sizeof(text)-1] = '\0';

	return text;
}

bool is_valid_map(const char* map) {
	fileHandle_t fmap;
	int mapsize = g_syscall(G_FS_FOPEN_FILE, QMM_VARARGS("maps\\%s", map), &fmap, FS_READ);
	g_syscall(G_FS_FCLOSE_FILE, fmap);
	return mapsize ? 1 : 0;
}

char** tok_parse(const char* str, char split) {
	if (!str || !*str)
		return NULL;

	size_t i, index, slen = strlen(str);
	// toks is 1 to allocate the NULL terminating pointer
	int toks = 1;
	char* copy = (char*)malloc(slen + 1);
	if (!copy)
		return NULL;
	char* tokstart = copy;
	memcpy(copy, str, slen + 1);

	for (i = 0; i <= slen; ++i) {
		if (copy[i] == split || !copy[i])
			++toks;
	}

	char** arr = (char**)malloc(sizeof(char*) * toks);
	for (i = 0, index = 0; i <= slen; ++i) {
		if (copy[i] == split || !copy[i]) {
			arr[index++] = tokstart;
			tokstart = &copy[i+1];
			copy[i] = '\0';
		}
	}
	arr[index] = NULL;

	return arr;	
}

void tok_free(char** arr) {
	if (arr) {
		if (arr[0])
			free((void*)(arr[0]));
		free((void*)arr);
	}
}

void setcvar(const char* cvar, int datanum) {
	char value[MAX_DATA_LENGTH];
	QMM_ARGV(datanum, value, sizeof(value));
	g_syscall(G_CVAR_SET, cvar, value);
}

const char* StripCodes(const char* name) {
	static char temp[MAX_NETNAME];

	size_t slen = strlen(name);
	if (slen >= MAX_NETNAME)
		slen = MAX_NETNAME - 1;

	for (size_t i = 0, j = 0; i < slen; ++i) {
		if (name[i] == Q_COLOR_ESCAPE) {
			if (name[i+1] != Q_COLOR_ESCAPE)
				++i;
			continue;
		}
		temp[j++] = name[i];
	}

	return temp;
}

// cycling array of buffers
const char* lcase(const char* string) {
	static char buf[8][1024];
	static int index = 0;
	int i = index;

	int j = 0;
	for (j = 0; string[j]; ++j) {
		buf[i][j] = tolower(string[j]);
	}
	buf[i][j] = '\0';

	index = (index + 1) & 7;
	return buf[i];
}

// returns index of matching name
// -1 when ambiguous or not found
int namematch(const char* string, bool ret_first, int start_after) {
	char lstring[MAX_NAME_LENGTH];
	// copied so the lcase-local static buffer is not overwritten in the big loop below
	strncpy(lstring, lcase(string), sizeof(lstring));
	lstring[sizeof(lstring) - 1] = '\0';

	const char* lfullname = NULL;
	const char* lstripname = NULL;

	int matchid = -1;

	for (int i = start_after + 1; i < MAX_CLIENTS; ++i) {
		if (!g_playerinfo[i].connected)
			continue;

		// on a complete match, return
		if (!strcmp(string, g_playerinfo[i].name))
			return i;

		// try partial matches, if we have 2, cancel
		lfullname = lcase(g_playerinfo[i].name);
		lstripname = lcase(g_playerinfo[i].stripname);

		if (strstr(lfullname, lstring) || strstr(lstripname, lstring)) {
			if (ret_first)
				return i;

			if (matchid == -1)
				matchid = i;
			else
				return -1;
		}
	}
	
	return matchid;
}

qboolean Info_Validate(const char* s) {
	return (strchr(s, '\"') || strchr(s, ';')) ? qfalse : qtrue;
}
