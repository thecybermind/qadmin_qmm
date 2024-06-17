/*
QADMIN_QMM - Server Administration Plugin
Copyright 2004-2024
https://github.com/thecybermind/nocrash_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < cybermind@gmail.com >

*/

#ifndef __UTIL_H__
#define __UTIL_H__

bool has_access(int, int);
int user_with_ip(const char*, int = -1);
int arrsize(admincmd_t*);
void ClientPrint(int, const char*, bool = 0);
char* concatargs(int);
char** tok_parse(const char*, char = ' ');
void tok_free(char**);
void setcvar(char*, int);
const char* StripCodes(const char*);
const char* lcase(const char*);
int namematch(const char*, bool = 0, int = -1);
bool is_valid_map(const char*);

char* Info_ValueForKey(const char*, const char*);
qboolean Info_Validate(const char*);

#endif //__UTIL_H__
