# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hznagui <hznagui@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/02/18 10:34:59 by hznagui           #+#    #+#              #
#    Updated: 2024/02/18 11:29:13 by hznagui          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


NAME = ircserv

FLAGS = -Wall -Wextra -Werror #-fsanitize=address -g

FILE = 	./srcs/main.cpp\
		./srcs/channel.cpp\
		./srcs/user.cpp\
		./srcs/utils.cpp\
		./srcs/server.cpp\
		

HEADER = ./srcs/channel.hpp\
		./srcs/user.hpp\
		./srcs/utils.hpp\
		./srcs/replies.hpp\
		./srcs/server.hpp\
		
OBJ_FILE = $(FILE:.cpp=.o)

COLOUR_GREEN=\033[1;32m

COLOUR_RED=\033[1;31m

all : $(NAME)

$(NAME) : $(OBJ_FILE) 	$(HEADER) 
	@c++ -std=c++98 $(FLAGS) $(FILE) -o $(NAME)
	@echo "$(COLOUR_GREEN)--->[mandatory part successfully created ✅]<---"
	
%.o: %.cpp 	$(HEADER) 
	@c++ -std=c++98 $(FLAGS) -c $< -o $@
	
clean :
	@rm -rf $(OBJ_FILE)
	@echo "$(COLOUR_RED)--->[obj file deleted successfully ✅]<---"

fclean: clean
	@rm -f $(NAME)
	@echo "$(COLOUR_RED)--->[program deleted successfully ✅]<---"


re : fclean all

# default:
# 	c++ ./srcs/*.cpp -o server
# 	c++ ./srcs/bot/*.cpp -o bot

# run:
# 	./server 6697 password
