#include "utils.hpp"

std::vector<MatchInfo> extractMatchInfo(const std::string &jsonResponse)
{
    std::vector<MatchInfo> matches;
    size_t pos = 0;
    int count = 0;
    while ((pos = jsonResponse.find("\"competition\":{", pos)) != std::string::npos)
    {
        MatchInfo match;
        size_t leagueStart = jsonResponse.find("\"name\":\"", pos) + 8;
        size_t leagueEnd = jsonResponse.find("\"", leagueStart);
        match.league = jsonResponse.substr(leagueStart, leagueEnd - leagueStart);

        // Ensure we move to the "homeTeam" section after "competition" to avoid capturing competition names again
        size_t homeTeamPos = jsonResponse.find("\"homeTeam\":{", leagueEnd);
        size_t homeTeamStart = jsonResponse.find("\"name\":\"", homeTeamPos) + 8;
        size_t homeTeamEnd = jsonResponse.find("\"", homeTeamStart);
        match.homeTeam = jsonResponse.substr(homeTeamStart, homeTeamEnd - homeTeamStart);

        // Ensure we move to the "awayTeam" section immediately after "homeTeam" to capture the correct away team name
        size_t awayTeamPos = jsonResponse.find("\"awayTeam\":{", homeTeamEnd);
        size_t awayTeamStart = jsonResponse.find("\"name\":\"", awayTeamPos) + 8;
        size_t awayTeamEnd = jsonResponse.find("\"", awayTeamStart);
        match.awayTeam = jsonResponse.substr(awayTeamStart, awayTeamEnd - awayTeamStart);

        size_t matchTimeStart = jsonResponse.find("\"utcDate\":\"", awayTeamEnd) + 11;
        size_t matchTimeEnd = jsonResponse.find("\"", matchTimeStart);
        match.matchTime = jsonResponse.substr(matchTimeStart, matchTimeEnd - matchTimeStart);

        matches.push_back(match);
        pos = matchTimeEnd; // Move past this match for the next iteration
        count++;
        if (count == 100)
        {
            break;
        }
    }
    return matches;
}

std::string httpRequest(const std::string &host, const std::string &path)
{
    int port = 80; // HTTP port
    // token "X-Auth-Token: 7437b3fe243e4b9092b4dd01d6508400"
    std::string request = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\nX-Auth-Token: 7437b3fe243e4b9092b4dd01d6508400\r\n\r\n";
    std::string response;

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "ERROR opening socket" << std::endl;
        return "";
    }

    // Get server IP
    struct hostent *server = gethostbyname(host.c_str());
    if (server == nullptr)
    {
        std::cerr << "ERROR, no such host" << std::endl;
        close(sockfd);
        return "";
    }

    // Connect to server
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "ERROR connecting" << std::endl;
        close(sockfd);
        return "";
    }

    // Send request
    if (write(sockfd, request.c_str(), request.length()) < 0)
    {
        std::cerr << "ERROR writing to socket" << std::endl;
        close(sockfd);
        return "";
    }

    // Receive response
    char buffer[4096];
    while (true)
    {
        memset(buffer, 0, 4096);
        int bytes_read = read(sockfd, buffer, 4095);
        if (bytes_read < 0)
        {
            std::cerr << "ERROR reading response from socket" << std::endl;
            close(sockfd);
            return "";
        }
        if (bytes_read == 0)
            break;
        response += std::string(buffer, bytes_read);
    }

    close(sockfd);
    // remove the headers
    size_t bodyStart = response.find("\r\n\r\n") + 4;
    response = response.substr(bodyStart);
    return response;
}

std::vector<MatchInfo> getMatches()
{
    // This example won't work with HTTPS endpoints, only HTTP.
    // system
    std::string path = "https://api.football-data.org/v4/matches";
    std::string response = httpRequest("api.football-data.org", path);
    std::cerr << response << std::endl;
    std::vector<MatchInfo> matches = extractMatchInfo(response);
    return matches;
}
