/*
QADMIN_QMM - Server Administration Plugin
Copyright 2004-2025
https://github.com/thecybermind/qadmin_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < cybermind@gmail.com >

*/

#ifndef __QADMIN_QMM_UTIL_H__
#define __QADMIN_QMM_UTIL_H__

#include <vector>
#include <string>

bool has_access(int, int);
int player_with_ip(std::string find, int start_after = -1);
void ClientPrint(int, const char*, bool = 0);
void KickClient(int slotid, std::string message);
void setcvar(std::string cvar, std::string value);
std::string stripcodes(std::string name);
int namematch(std::string find, bool ret_first = 0, int start_after = -1);
bool is_valid_map(std::string map);
std::string sanitize(std::string str);

bool InfoString_Validate(const char*);

int str_stristr(std::string haystack, std::string needle);
int str_stricmp(std::string s1, std::string s2);
int str_striequal(std::string s1, std::string s2);

std::vector<std::string> parse_str(std::string str, char sep = ' ');
std::vector<std::string> parse_args(int start, int end = -1);

std::string str_join(std::vector<std::string> arr, size_t start = 0, char delim = ' ');

#endif // __QADMIN_QMM_UTIL_H__
