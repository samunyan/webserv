# Executable
NAME		=	webserv

# Directories
OBJ_DIR			= 	./objs
SRC_DIR			=	./srcs
HEAD_DIRS		=	./include

# Files
FILES		=	main.cpp \
                servers/Server.cpp \
                servers/Webserv.cpp \
				utils/webserv_utils.cpp \
				utils/server_utils.cpp \
				utils/utils.cpp \
				utils/UrlParser.cpp \
				messages/Request.cpp \
				messages/Response.cpp
SRCS	= 	$(addprefix $(SRC_DIR)/, $(FILES))
OBJS	= 	$(addprefix $(OBJ_DIR)/, $(FILES:.cpp=.o))

# Compiler
SYSTEM		 := $(shell uname)
ifeq ($(SYSTEM),Linux)
CXX			=	clang++
else
CXX			= 	c++
endif
CXXFLAGS	=	-Wall -Werror -Wextra -std=c++98 $(HEAD_DIRS:%=-I %)
ifeq ($(SANITIZER), 1)
		CXXFLAGS	+=	-g -fsanitize=address
		LDFLAGS		+=	-g -fsanitize=address
endif
ifeq ($(DEBUG), 1)
		CXXFLAGS	+=	-g
		LDFLAGS		+=	-g
endif

.PHONY:		all clean fclean re sanitizer debug

all:	    $(NAME)

$(NAME):	$(OBJS)
			@$(CXX) $(CXXFLAGS) $(SRCS) $(LDFLAGS) -o $(NAME)
			@test -z '$(filter %.o,$?)' || (echo ✅ $(BBlue) [$(NAME)]"\t"$(BGreen)Compilation done. \
				Usage: ./$(NAME) [ optional: \<filename\>.conf ] && \
				echo $(White)"\t\t\t"Compiler flags: $(CXXFLAGS)$(Color_Off))

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.cpp
			@mkdir -p $(dir $@)
			@echo ⌛ $(BBlue)[$(NAME)]"\t"$(Yellow)Compiling $<$(Color_Off)
			@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
			@if [ -d $(OBJ_DIR) ]; then \
  					rm -rf $(OBJ_DIR); \
  					echo 🗑$(BBlue)[$(NAME)]"\t"$(BGreen)Object files removed.$(Color_Off); \
  			fi

fclean:		clean
			@rm -rf webserv.log
			@if [ -f $(NAME) ]; then \
					rm -rf $(NAME); \
					rm -rf *.dSYM; \
					echo 🗑$(BBlue)[$(NAME)]"\t"$(BGreen)Executable removed.$(Color_Off); \
			fi

re:			fclean all

sanitizer:
			@SANITIZER=1 make re

debug:
			@DEBUG=1 make re

# Colors
## Reset
Color_Off='\033[0m'       # Text Reset
## Regular Colors
Black='\033[0;30m'        # Black
Red='\033[0;31m'          # Red
Green='\033[0;32m'        # Green
Yellow='\033[0;33m'       # Yellow
Blue='\033[0;34m'         # Blue
Purple='\033[0;35m'       # Purple
Cyan='\033[0;36m'         # Cyan
White='\033[0;37m'        # White
## Bold
BBlack='\033[1;30m'       # Black
BRed='\033[1;31m'         # Red
BGreen='\033[1;32m'       # Green
BYellow='\033[1;33m'      # Yellow
BBlue='\033[1;34m'        # Blue
BPurple='\033[1;35m'      # Purple
BCyan='\033[1;36m'        # Cyan
BWhite='\033[1;37m'       # White
## Underline
UBlack='\033[4;30m'       # Black
URed='\033[4;31m'         # Red
UGreen='\033[4;32m'       # Green
UYellow='\033[4;33m'      # Yellow
UBlue='\033[4;34m'        # Blue
UPurple='\033[4;35m'      # Purple
UCyan='\033[4;36m'        # Cyan
UWhite='\033[4;37m'       # White
## Background
On_Black='\033[40m'       # Black
On_Red='\033[41m'         # Red
On_Green='\033[42m'       # Green
On_Yellow='\033[43m'      # Yellow
On_Blue='\033[44m'        # Blue
On_Purple='\033[45m'      # Purple
On_Cyan='\033[46m'        # Cyan
On_White='\033[47m'       # White
## Bold High Intensity
BIBlack='\033[1;90m'      # Black
BIRed='\033[1;91m'        # Red
BIGreen='\033[1;92m'      # Green
BIYellow='\033[1;93m'     # Yellow
BIBlue='\033[1;94m'       # Blue