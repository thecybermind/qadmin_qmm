/*
QADMIN_QMM - Server Administration Plugin
Copyright 2004-2025
https://github.com/thecybermind/qadmin_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < k.m.masterson@gmail.com >

*/

#ifndef __QADMIN_QMM_UTIL_H__
#define __QADMIN_QMM_UTIL_H__

#include <vector>
#include <string>
#include <cstdint>

bool player_has_access(intptr_t clientnum, int reqaccess);
void player_clientprint(intptr_t clientnum, const char* msg, bool chat = false);
void player_kick(intptr_t clientnum, std::string message);
std::string strip_codes(std::string name);
std::vector<intptr_t> players_with_name(std::string find);
std::vector<intptr_t> players_with_ip(std::string find);
bool is_valid_map(std::string map);
std::string str_sanitize(std::string str);

int str_stristr(std::string haystack, std::string needle);
int str_stricmp(std::string s1, std::string s2);
int str_striequal(std::string s1, std::string s2);

std::vector<std::string> parse_str(std::string str, char sep = ' ');
std::vector<std::string> parse_args(int start);

std::string str_join(std::vector<std::string> arr, size_t start = 0, char delim = ' ');

#endif // __QADMIN_QMM_UTIL_H__
