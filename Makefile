# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: daalmeid <daalmeid@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/04/26 16:48:15 by daalmeid          #+#    #+#              #
#    Updated: 2022/05/20 11:00:25 by daalmeid         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

################## Programs #################

SNAME	= 		server
CNAME	=		client

################## COLORS ##################

--GRN	=		\033[32m
--RED	=		\033[31m
--WHT	=		\033[39m

################## TERMINAL ################

RM		=		rm -f

################## COMPILER ################

CMP		=		c++
FLAGS	=		-g -Wall -Werror -Wextra -std=c++98
#INC_FT	=		-I ./headers/headers_ft/ -I ./headers/utils
#INC_STL =		-I ./headers/header_stl

################## FILES ###################

SRVFLES	=		server.cpp

CLTFLES	=		client.cpp

SRVSRCS	=		$(addprefix srcs/server/, $(SRVFLES))

CLTSRCS	=		$(addprefix srcs/client/, $(CLTFLES))

SRVOBJS	=		$(patsubst srcs/server/%.cpp, srcs/server/%.o, $(SRVSRCS))

CLTOBJS	=		$(patsubst srcs/client/%.cpp, srcs/client/%.o, $(CLTSRCS))


################## RULES ###################

all: $(SNAME) $(CNAME)

srcs/server/%.o : srcs/server/%.cpp
	$(CMP) $(FLAGS) -c $< -o $@

srcs/client/%.o : srcs/client/%.cpp
	$(CMP) $(FLAGS) -c $< -o $@

$(SNAME): $(SRVOBJS)
	$(CMP) $(FLAGS) $(SRVOBJS) -o $(SNAME)

$(CNAME): $(CLTOBJS)
	$(CMP) $(FLAGS) $(CLTOBJS) -o $(CNAME)

################## CLEAN ###################

clean:
	$(RM) $(SRVOBJS)$(CLTOBJS)
fclean: clean
	$(RM) $(SNAME) $(CNAME)

re: fclean all

.PHONY: all clean fclean re