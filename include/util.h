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

bool player_has_access(int clientnum, int reqaccess);
int player_with_ip(std::string find, int start_after = -1);
void player_clientprint(int clientnum, const char* msg, bool chat = false);
void player_kick(int slotid, std::string message);
std::string strip_codes(std::string name);
int player_with_name(std::string find, bool ret_first = 0, int start_after = -1);
bool is_valid_map(std::string map);
std::string str_sanitize(std::string str);

int str_stristr(std::string haystack, std::string needle);
int str_stricmp(std::string s1, std::string s2);
int str_striequal(std::string s1, std::string s2);

std::vector<std::string> parse_str(std::string str, char sep = ' ');
std::vector<std::string> parse_args(int start, int end = -1);

std::string str_join(std::vector<std::string> arr, size_t start = 0, char delim = ' ');

#endif // __QADMIN_QMM_UTIL_H__
