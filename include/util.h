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
int user_with_ip(const char*, int = -1);
int arrsize(admincmd_t*);
void ClientPrint(int, const char*, bool = 0);
void KickClient(int, const char*);
char* concatargs(int);
char** tok_parse(const char*, char = ' ');
void tok_free(char**);
void setcvar(const char*, int);
const char* StripCodes(const char*);
const char* lcase(const char*);
int namematch(const char*, bool = 0, int = -1);
bool is_valid_map(const char*);

bool InfoString_Validate(const char*);

int str_stristr(std::string haystack, std::string needle);
int str_stricmp(std::string s1, std::string s2);
int str_striequal(std::string s1, std::string s2);

std::vector<std::string> parse_str(std::string str, char sep = ' ');
std::vector<std::string> parse_args(int start, int end = -1);

#endif // __QADMIN_QMM_UTIL_H__
