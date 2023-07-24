NAME = ircserv

SRC_DIR = Src/

OBJ_DIR = .obj/

INCLUDE_DIR = Include/

CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++98 -g

SRC = main.cpp Server.cpp Socket.cpp User.cpp Channels.cpp message.cpp utils.cpp mode.cpp

OBJP = $(addprefix $(OBJ_DIR), $(notdir $(SRC:.cpp=.o)))

VPATH = $(SRC_DIR)

TOTAL_FILES := $(words $(SRC))

COMPILED_FILES = 0

all: $(NAME)

$(OBJ_DIR)%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@
	@$(eval COMPILED_FILES=$(shell echo $$(($(COMPILED_FILES)+1))))
	@$(call print_progress)

$(NAME): $(OBJP)
	@$(CC) $(CFLAGS) $(OBJP) -o $(NAME)
	@echo "\033[32m\nCompiled ✅\033[0;37m"

clean:
	@echo "\033[0;31mCleaning objects🧹"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "\033[0;31mRemoving $(NAME) 🗑\033[0;37m"
	@rm -rf $(NAME)

re: fclean all


print_progress = @printf "\r\033[K\033[32mCompiling... [%-10s]\033[0m \033[32m%d/%d\033[0m" "$(shell perl -e 'printf "=" x (int($(COMPILED_FILES) * 10 / $(TOTAL_FILES)))')" "$(COMPILED_FILES)" "$(TOTAL_FILES)"

.PHONY: all clean fclean re

