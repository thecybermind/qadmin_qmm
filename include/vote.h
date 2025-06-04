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

typedef void (*pfnVoteFunc)(int winner, int winvotes, int totalvotes, void* param);

typedef struct voteinfo_s {
	bool inuse;
	int finishtime;
	pfnVoteFunc votefunc;
	int choices;
	void* param;
	int clientnum;
	int votes[MAX_CLIENTS];
} voteinfo_t;
extern voteinfo_t g_vote;

void vote_start(int, pfnVoteFunc, int, int, void*);
void vote_add(int, int);
void vote_finish();

#endif // __QADMIN_QMM_VOTE_H__
