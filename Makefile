
NAME				:=	webserv
CC					:=	c++
CFLAGS				:=	-Wall -Wextra -std=c++11 -pedantic

################################################################################
# COLORS

BOLD 				:=	\e[1m
RESET 				:=	\e[0m
LIGHT_GREEN 		:=	\e[92m
LIGHT_CYAN 			:=	\e[96m

################################################################################
# DIRECTORIES

INCL_DIR			:=	include
SRC_DIR				:=	src
CGI_DIR				:=	cgi
CONFIG_DIR			:=	config
SOCKET_DIR			:=	socket
HTTP_DIR			:=	http
UTILS_DIR			:=	utils
OBJ_DIR				:=	obj

################################################################################
# SOURCES

MAIN				:=	main.cpp

CGI_SRCS			:=	CgiHandler.cpp

CONFIG_SRCS			:=	Config.cpp			\
						Location.cpp		\
						Server.cpp			\
						Utils.cpp

HTTP_SRCS			:= 	HttpServer.cpp		\
						HttpResponse.cpp	\
						HttpRequest.cpp		\
						HttpParser.cpp	    \
						EventHandler.cpp

# SOCKET_SRCS			:=

UTILS_SRCS			:=	KqueueUtils.cpp

SRCS				:= $(MAIN)
SRCS				+= $(addprefix $(CGI_DIR)/, $(CGI_SRCS))
SRCS				+= $(addprefix $(CONFIG_DIR)/, $(CONFIG_SRCS))
SRCS				+= $(addprefix $(SOCKET_DIR)/, $(SOCKET_SRCS))
SRCS				+= $(addprefix $(HTTP_DIR)/, $(HTTP_SRCS))
SRCS				+= $(addprefix $(UTILS_DIR)/, $(UTILS_SRCS))

SRCP				:= $(addprefix $(SRC_DIR)/, $(SRCS))

################################################################################
# INCLUDES

CGI_INCS			:=	CgiHandler.hpp

CONFIG_INCS			:=	Config.hpp			\
						Location.hpp		\
						Server.hpp			\
						Utils.hpp

# SOCKET_INCS			:=

HTTP_INCS			:=	HttpServer.hpp		\
						EventHandler.hpp

UTILS_INCS			:=	KqueueUtils.hpp						

INCS				+= $(addprefix $(CGI_DIR)/, $(CGI_INCS))
INCS				+= $(addprefix $(CONFIG_DIR)/, $(CONFIG_INCS))
INCS				+= $(addprefix $(SOCKET_DIR)/, $(SOCKET_INCS))
INCS				+= $(addprefix $(HTTP_DIR)/, $(HTTP_INCS))
INCS				+= $(addprefix $(UTILS_DIR)/, $(UTILS_INCS))

INC_DIRS			+= $(addprefix -I$(INCL_DIR)/, $(CGI_DIR)/)
INC_DIRS			+= $(addprefix -I$(INCL_DIR)/, $(CONFIG_DIR)/)
INC_DIRS			+= $(addprefix -I$(INCL_DIR)/, $(SOCKET_DIR)/)
INC_DIRS			+= $(addprefix -I$(INCL_DIR)/, $(HTTP_DIR)/)
INC_DIRS			+= $(addprefix -I$(INCL_DIR)/, $(UTILS_DIR)/)

INCLUDE				:= $(INC_DIRS)

INCP				:= $(addprefix $(INCL_DIR)/, $(INCS))

HEADERS				:= $(INCP)

################################################################################
# OBJECTS

OBJS := $(SRCS:.cpp=.o)
OBJP := $(addprefix $(OBJ_DIR)/, $(OBJS))

#################################################################################

all: $(NAME)

print:
	@echo Sources: $(SRCS)
	@echo Source path: $(SRCP)
	@echo Objects: $(OBJS)
	@echo Object path: $(OBJP)
	@echo Headers: $(HEADERS)
	@echo include: $(INCLUDE)
	@echo OBJ dir: $(OBJ_DIR)
	@echo SRC dir: $(SRC_DIR)

clean:
	@rm -rf $(OBJ_DIR)
	@echo "Done cleaning $(CURDIR)/$(OBJ_DIR)"

fclean: clean
	@rm -f $(NAME)
	@echo "Done cleaning executable $(CURDIR)/$(NAME)"

re: fclean
	@$(MAKE)

$(NAME):	$(OBJP)
	@printf "$(LIGHT_CYAN)$(BOLD)make$(RESET)   [$(LIGHT_GREEN)$(NAME)$(RESET)] : "
	$(CC) $(OBJP) $(CFLAGS) -o $(NAME)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(@D)
	@$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@
	@printf "$(LIGHT_CYAN)$(BOLD)make$(RESET)   [$(LIGHT_GREEN)$(NAME)$(RESET)] : "
	@printf "$(notdir $(basename $@)) created\n"

.PHONY: all clean fclean re
