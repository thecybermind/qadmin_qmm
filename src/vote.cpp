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

#include <string.h>
#include <time.h>
#include "main.h"
#include "vote.h"
#include "util.h"

voteinfo_t g_vote;


// initiate a vote
void vote_start(int clientnum, pfnVoteFunc callback, intptr_t seconds, int choices, void* param) {
	if (g_vote.inuse) {
		player_clientprint(clientnum, "[QADMIN] A vote is already running\n");
		return;
	}
	g_vote.clientnum = clientnum;
	g_vote.votefunc = callback;
	g_vote.finishtime = g_leveltime + (seconds * 1000);
	g_vote.choices = choices;
	g_vote.param = param;
	g_vote.inuse = true;
	g_vote.votes.clear();
}


// someone has voted (vote should be 1-9)
void vote_add(int clientnum, int vote) {
	if (!g_vote.inuse) {
		player_clientprint(clientnum, "[QADMIN] There is no vote currently running\n");
		return;
	}

	if (g_vote.votes.count(clientnum)) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] You have already voted for %d\n", g_vote.votes[clientnum]));
		return;
	}

	if (vote < 1 || vote > g_vote.choices) {
		player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Invalid vote option, choose from 1-%d\n", g_vote.choices));
		return;
	}

	g_vote.votes[clientnum] = vote;
	player_clientprint(clientnum, QMM_VARARGS("[QADMIN] Vote counted for %d\n", vote));
}


// vote has ended
void vote_finish() {
	int winner = 0;
	int totalvotes = g_vote.votes.size();
	std::map<int, int> votecount;	// votecount[choice] = numvotes

	// tally up the votes
	for (auto& vote : g_vote.votes) {
		votecount[vote.second]++;
	}

	// figure out which choice won
	for (auto& vote : votecount) {
		if (vote.second > votecount[winner])
			winner = vote.first;
	}

	// if the winning votecount is 0 (no one voted), then give winner as 0
	g_vote.votefunc(winner, votecount[winner], totalvotes, g_vote.param);
	g_vote.inuse = 0;
}
