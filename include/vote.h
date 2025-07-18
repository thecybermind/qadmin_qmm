/*
QADMIN_QMM - Server Administration Plugin
Copyright 2004-2025
https://github.com/thecybermind/qadmin_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < cybermind@gmail.com >

*/

#ifndef __QADMIN_QMM_VOTE_H__
#define __QADMIN_QMM_VOTE_H__

#define MAX_CHOICES 9

#include <map>
#include <time.h>

typedef void (*pfnVoteFunc)(int winner, int winvotes, int totalvotes, void* param);

typedef struct voteinfo_s {
	bool inuse;
	time_t finishtime;
	pfnVoteFunc votefunc;
	int choices;
	void* param;
	intptr_t clientnum;
	std::map<int, int> votes;	// votecount[clientnum] = choice
} voteinfo_t;
extern voteinfo_t g_vote;

void vote_start(intptr_t clientnum, pfnVoteFunc callback, intptr_t seconds, int choices, void* param);
void vote_add(intptr_t clientnum, int vote);
void vote_finish();

#endif // __QADMIN_QMM_VOTE_H__
