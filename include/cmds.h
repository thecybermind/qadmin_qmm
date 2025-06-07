/*
QADMIN_QMM - Server Administration Plugin
Copyright 2004-2025
https://github.com/thecybermind/qadmin_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < cybermind@gmail.com >

*/

#ifndef __QADMIN_QMM_CMD_H__
#define __QADMIN_QMM_CMD_H__

#include <vector>
#include <string>

#include "main.h"

void reload();
int handlecommand(int, int);
int handlecommand(int, std::vector<std::string>);
int admin_adduser(addusertype_t type);

#endif // __QADMIN_QMM_UTIL_H__
