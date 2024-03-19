#define RPL_WELCOME(nick, hostname) ":" + hostname + " 001 " + nick + " :Welcome " + nick + " to the ft_irc server !\r\n"
#define RPL_YOURHOST(nick, hostname) ":" + hostname + " 002 " + nick + " :Your host is " + hostname + " running version 0.0.0.1 !\r\n"
#define RPL_CREATED(nick, hostname) ":" + hostname + " 003 " + nick + " :This server was created by mlektaib && hznagui!\r\n"
#define RPL_MYINFO(nick, hostname) ":" + hostname + " 004 " + nick + " :Host: " + hostname + ", Version: 0.0.0.1\r\n"

#define ERR_PASSWDMISMATCH(target, hostname) ":" + hostname + " 464 " + target + " :Password incorrect ! Please try again by using PASS command\r\n"
#define ERR_ALREADYREGISTERED(nick, hostname) ":" + hostname + " 462 " + nick + " :You already registered !\r\n"
#define ERR_NOTREGISTERED(nick, hostname) ":" + hostname + " 451 " + nick + " :You have not registered !\r\n"

#define ERR_ERRONEUSNICKNAME(nick, hostname) ":" + hostname + " 432 " + nick + " :Erroneus nickname !\r\n"
#define ERR_NONICKNAMEGIVEN(nick, hostname) ":" + hostname + " 431 " + nick + " :No nickname given !\r\n"
#define ERR_NICKNAMEINUSE(nick, hostname) ":" + hostname + " 433 " + nick + " :Nickname is already in use ! you can chose other nickname by using NICK command\r\n"
#define RPL_NICKCHANGE(oldNick, nick, hostname) ":" + oldNick + " NICK " + nick + "\r\n"

#define ERR_NEEDPASS(nick, hostname) ":" + hostname + " 461 " + nick + " : you need to enter a password by using PASS command\r\n"

#define ERR_BADCHANNELNAME(nick, hostname, channelname) ":" + hostname + " 476 " + nick + " " + channelname + " :Invalid channel name." + "\r\n"
#define ERR_CHANNELISFULL(nick, channelName) ":" + nick + " 471 " + channelName + " :Cannot join channel (+l)\r\n"
#define ERR_BADCHANNELKEY(nick, hostname, channelName) ":" + hostname + " 475 " + nick + " " + channelName + " :Cannot join channel (incorrect channel key)\r\n"
#define ERR_INVITEONLY(nick, channelName) ":" + nick + " 473 " + channelName + " :Cannot join channel (+i)\r\n"
#define ERR_NEEDMOREPARAMS(clientIP, hostname, command) ":" + hostname + " 461 " + clientIP + " " + command + " :Not enough parameters\r\n"


#define RPL_JOIN(nick, username, channelname, ipaddress) ":" + nick + "!~" + username + "@" + ipaddress + " JOIN " + channelname + "\r\n"
#define RPL_YOUPART(nick,userName , hostName,channelName) ":" + nick + "!~" + userName + "@" + hostName + " PART " + channelName + " :it was fun to having you here\r\n"
#define RPL_TOPICDISPLAY(hostname, nick, channel, topic) ":" + hostname + " 332 " + nick + " " + channel + " :" + topic + "\r\n"
#define RPL_TOPIC(hostname, nick, channel, setter, topic) ":" + hostname + " 332 " + nick + " " + channel + " :" + setter + " has set a new topic: " + topic + "\r\n"

#define RPL_NAMREPLY(hostname, clients, channelname, nick) ":" + hostname + " 353 " + nick + " = " + channelname + " :" + clients + "\r\n"

#define RPL_INVITE(nick, username, clienthostname, invited, channel) ":" + nick + "!" + username + "@" + clienthostname + " INVITE " + invited + " :" + channel + "\r\n"
#define ERR_NOTONCHANNEL(hostname, channel) ":" + hostname + " 442 " + channel + " " + ":You're not on that channel\r\n"
#define ERR_NOSUCHNICK(hostname, nick) ":" + hostname + " 401 " + nick +" :No such nick\r\n"
#define ERR_NOSUCHCHANNEL(hostname, channel,nick) ":" + hostname + " 403 " + nick + channel + " " + ":No such channel\r\n"// 
#define ERR_CANNOTSENDTOCHAN(hostname, channel) ":" + hostname + " 404 " + channel + " :Cannot send to channel\r\n"
#define ERR_USERONCHANNEL(hostname, channel, nick) ":" + hostname + " 443 " + nick + " " + channel + "  :is already on channel\r\n"

#define ERR_INPUTTOOLONG(nick, hostname) ":" + hostname + " 417 " + nick + " :Input line was too long !\r\n"

#define RPL_SETTOPIC(nick, hostname, channel, topic) ":" + nick + "!" + nick + "@" + hostname + " TOPIC " + channel + " :" + topic + "\r\n"

#define RPL_UMODEIS(hostname, channelname) ":" + hostname + " MODE " + channelname + " +nt\r\n"

#define ERR_UNKNOWNMODE(nick, hostname, channel, character) ":" + hostname + " 472 " + nick + " " + channel + " " + character + " :is unknown mode char to me\r\n"
#define RPL_YOUREOPER(hostname, nick) ":" + hostname + " 381 " + nick + ":You are now an IRC operator\r\n"
#define RPL_KICK(kicker, username, host, channel, targetuser, reason) ":" + kicker + "!" + username + "@" + host + " KICK " + channel + " " + targetuser + " :" + reason + "\r\n"
#define PRIVMSG_FORMAT(senderNick, senderUsername, senderHostname, receiver, message) ":" + senderNick + "!~" + senderUsername + "@" + senderHostname + " PRIVMSG " + receiver + " :" + message + "\r\n"
#define ERR_UNKNOWNCOMMAND(nick, hostname, command) ":" + hostname + " 421 " + nick + " " + command + " :Unknown command\r\n"
#define ERR_CHANOPRIVSNEEDED(hostname,channel)  ":" + hostname + " 482 " + channel + " :You're not channel operator\r\n"
#define RPL_NOTOPIC(hostname,channel,nick) (":" + hostname + " 331 " + nick + " " + channel +  " :No topic is set.\r\n")
#define RPL_CHANNELMODEIS(nick,server_ip,channel,mode) ":" + nick + "!~" + nick + "@" + server_ip + " MODE " + channel + " :" + mode + "\r\n"