NAME		=	webserv

# apply all rules on the following directories
VPATH		=	$(addprefix $(_SRC), \
					core \
					http \
					config \
					error \
				)

# shell commands
RM			=	rm -f
RMDIR		=	$(RM) -r
MKDIR		=	mkdir

# compiler and flags
CXX			=	c++ -std=c++98
CXXFLAGS	=	-Wall -Werror -Wextra -O2
DBGFLAGS	=	-Wall -Werror -Wextra -Wshadow -g -DDEBUG_MODE

# dir
_SRC		=	srcs/
_OBJ		=	objs/
_INC		=	includes/
_BIN		=	bin/

# files
SRCS		=	$(addsuffix .cpp, \
					main \
					HTTPServer \
					HTTPSocks \
					HTTPEpoll \
					HTTPReq \
					err \
					Mediator \
				)
OBJS		=	$(addprefix $(_OBJ), $(patsubst %.cpp, %.o, $(SRCS)))
INCS		=	-I ./$(_INC) $(addprefix -I./, $(VPATH))

# rules
all: $(NAME)

debug: CXXFLAGS = $(DBGFLAGS)
debug: $(NAME)

info:
	@printf "\
	 CXX		=  $(CXX)\n\
	 CXXFLAGS	=  $(CXXFLAGS)\n\
	 DBGFLAGS	=  $(DBGFLAGS)\n\
	 INCS		=$(patsubst -I%, %, $(INCS))\n\
	 VPATH		=  $(VPATH)\n\
	 SRCS		=  $(SRCS)\n"

$(NAME) : $(_BIN)$(NAME)
	
$(_BIN)$(NAME): $(_BIN) $(_OBJ) $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(@) $(INCS)

$(_OBJ)%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(<) -o $(@) $(INCS)

%/:
	$(MKDIR) $(@)

clean:
	$(RMDIR) $(_OBJ)

fclean: clean
	$(RMDIR) $(_BIN)

re: fclean all

rdebug: fclean debug

.PHONY: all debug info clean fclean re rdebug
