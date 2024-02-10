#pragma once


#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct MatchInfo {
    std::string league;
    std::string homeTeam;
    std::string awayTeam;
    std::string matchTime;
};

std::vector<MatchInfo> getMatches();