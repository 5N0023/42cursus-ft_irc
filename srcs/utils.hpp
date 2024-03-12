/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hznagui <hznagui@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 11:08:47 by hznagui           #+#    #+#             */
/*   Updated: 2024/03/12 10:44:11 by hznagui          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "server.hpp"
class user;
size_t ToSize_T(std::string &);
bool has_char(std::string);
user getUser_str(std::string,std::vector<user>);
std::string getreason(std::vector<std::string>,size_t);
std::vector<std::string> split(const std::string& input, char delimiter);
std::vector<std::string> splitCommand(std::string command);
std::map<std::string, std::string> parseChannels(std::string channels,std::string keys);
std::string getLocalIP();