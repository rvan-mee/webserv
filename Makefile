NAME				:=	webserv
CC					:=	c++
export LOG_ENABLED	:=	1
CFLAGS				:=	-Wall -Wextra -std=c++11 -pedantic -D LOG_ENABLED=$(LOG_ENABLED) -D PATH_TO_PYTHON=\"$(shell which python3)\"
################################################################################
# EXTRA FLAGS
ifdef PROD
# Used for evals
CFLAGS				+=	-Werror
else
# Used for development
CFLAGS				+=	-g3 -fsanitize=address
endif
################################################################################
# COLORS
BOLD 				:=	\e[1m
RESET 				:=	\e[0m
LIGHT_GREEN 		:=	\e[92m
LIGHT_CYAN 			:=	\e[96m
################################################################################
# DIRECTORIES
INCL_DIR			:=	include
LOG_DIR				:=	logs
SRC_DIR				:=	src
OBJ_DIR				:=	obj
VPATH 				:=	$(subst $(space),:,$(shell find $(SRC_DIR) -type d))
################################################################################
# SOURCES / OBJECTS
MAIN				:=	littlewebserver.cpp
export SRCS			:=	

TEST_OBJS			:=	$(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))
MAIN				:=	littlewebserver.cpp
MAIN_OBJ			:=	$(addprefix $(OBJ_DIR)/, $(MAIN:.cpp=.o))
OBJS				:=	$(TEST_OBJS) $(MAIN_OBJ)
################################################################################
# ARGS FOR DEV
ARGS				:= 

all: $(NAME)

$(NAME):	$(OBJ_DIR) $(LOG_DIR) $(OBJS)
	@printf "$(LIGHT_CYAN)$(BOLD)make$(RESET)   [$(LIGHT_GREEN)$(NAME)$(RESET)] : "
	$(CC) $(OBJS) $(CFLAGS) -o $(NAME)

$(OBJ_DIR)/%.o: $(notdir %.cpp)
	@$(CC) $(CFLAGS) -c $< -I$(INCL_DIR) -o $@
	@printf "$(LIGHT_CYAN)$(BOLD)make$(RESET)   [$(LIGHT_GREEN)$(NAME)$(RESET)] : "
	@printf "$(notdir $(basename $@)) created\n"

# TODO:
prod: export PROD=1
prod: $(NAME)
	@echo $(CFLAGS)
	./$(NAME) $(ARGS)

run: $(NAME) deletelogs
	./$(NAME) $(ARGS)

lldb: $(NAME)
	lldb $(NAME) -- $(ARGS)

clean:
	@rm -rf $(OBJ_DIR)

fclean:	clean
	@rm -f $(NAME)

re: fclean all

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(LOG_DIR):
	@mkdir -p $(LOG_DIR)

deletelogs:
	@rm -rf $(LOG_DIR)
	@mkdir $(LOG_DIR)
	@printf "$(LIGHT_GREEN)$(BOLD)Deleted Logs!$(RESET)\n"

testPages:
	./tests/errorpages.sh

# For debugging makefile
echo: export LOG_ENABLED=0
echo:
	@echo $(CFLAGS)

	# @$(MAKE) -C $(TEST_DIR) echo


.PHONY: all clean fclean re lldb
