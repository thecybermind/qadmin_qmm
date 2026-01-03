/*
QADMIN_QMM - Server Administration Plugin
Copyright 2004-2025
https://github.com/thecybermind/qadmin_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < k.m.masterson@gmail.com >

*/

#ifndef __QADMIN_QMM_VERSION_H__
#define __QADMIN_QMM_VERSION_H__

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x

#define QADMIN_QMM_VERSION_MAJOR	2
#define QADMIN_QMM_VERSION_MINOR	4
#define QADMIN_QMM_VERSION_REV		0

#define QADMIN_QMM_VERSION		STRINGIFY(QADMIN_QMM_VERSION_MAJOR) "." STRINGIFY(QADMIN_QMM_VERSION_MINOR) "." STRINGIFY(QADMIN_QMM_VERSION_REV)

#if defined(_WIN32)
#define QADMIN_QMM_OS             "Windows"
#ifdef _WIN64
#define QADMIN_QMM_ARCH           "x86_64"
#else
#define QADMIN_QMM_ARCH           "x86"
#endif
#elif defined(__linux__)
#define QADMIN_QMM_OS             "Linux"
#ifdef __LP64__
#define QADMIN_QMM_ARCH           "x86_64"
#else
#define QADMIN_QMM_ARCH           "x86"
#endif
#endif

#define QADMIN_QMM_VERSION_DWORD	QADMIN_QMM_VERSION_MAJOR , QADMIN_QMM_VERSION_MINOR , QADMIN_QMM_VERSION_REV , 0
#define QADMIN_QMM_COMPILE			__TIME__ " " __DATE__
#define QADMIN_QMM_BUILDER			"Kevin Masterson"

#endif // __QADMIN_QMM_VERSION_H__
