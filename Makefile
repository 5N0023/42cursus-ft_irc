# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hznagui <hznagui@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/02/18 10:34:59 by hznagui           #+#    #+#              #
#    Updated: 2024/03/17 14:00:04 by hznagui          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


NAME = ircserv
BOTNAME = bot

SERVERIP = "127.0.0.1"
PORT = 6697
PASSWORD = "password"

FLAGS = -Wall -Wextra -Werror  -std=c++98 -fsanitize=address -g

FILE = 	./srcs/main.cpp\
		./srcs/channel.cpp\
		./srcs/user.cpp\
		./srcs/utils.cpp\
		./srcs/server.cpp\
		./srcs/KICK.cpp\
		./srcs/TOPIC.cpp\
		./srcs/INVITE.cpp\
		./srcs/MODE.cpp\
		
BOTFILES = ./srcs/bot/bot.cpp\
		./srcs/bot/main.cpp\
		./srcs/bot/utils.cpp

HEADER = ./srcs/channel.hpp\
		./srcs/user.hpp\
		./srcs/utils.hpp\
		./srcs/replies.hpp\
		./srcs/server.hpp\

BOTHEADER = ./srcs/bot/bot.hpp\
		./srcs/bot/utils.hpp

OBJ_FILE = $(FILE:.cpp=.o)

COLOUR_GREEN=\033[1;32m

COLOUR_RED=\033[1;31m

all : $(NAME) $(BOTNAME)

$(BOTNAME) : $(BOTFILES) $(BOTHEADER)
	@c++  $(FLAGS) $(BOTFILES) -o $(BOTNAME)
	@echo "$(COLOUR_GREEN)--->[bonus part successfully created ✅]<---"

$(NAME) : $(OBJ_FILE) 	$(HEADER) 
	@c++  $(FLAGS) $(FILE) -o $(NAME)
	@echo "$(COLOUR_GREEN)--->[mandatory part successfully created ✅]<---"
	
%.o: %.cpp 	$(HEADER) 
	@c++  $(FLAGS) -c $< -o $@
	
clean :
	@rm -rf $(OBJ_FILE)
	@echo "$(COLOUR_RED)--->[obj file deleted successfully ✅]<---"

fclean: clean
	@rm -f $(NAME) $(BOTNAME)
	@echo "$(COLOUR_RED)--->[program deleted successfully ✅]<---"


re : fclean all

run:
	./ircserv $(PORT) $(PASSWORD) &> serverlog.txt 2>&1 &
	sleep 1
	./bot $(SERVERIP) $(PORT) $(PASSWORD) &> botlog.txt 2>&1 &

kill:
	kill $(shell pgrep server)
