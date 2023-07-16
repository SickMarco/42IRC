NAME = ircserv

SRC = main.cpp Server.cpp User.cpp Irc.cpp

OBJ_DIR = ./.obj/

CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++98 -g

OBJP = $(addprefix $(OBJ_DIR),$(notdir $(SRC:.cpp=.o)))

all: $(NAME)

$(OBJ_DIR)%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJP)
	@$(CC) $(CFLAGS) $(OBJP) -o $(NAME)
	@echo "\033[32mCompiled ✅\033[0;37m"

clean:
	@echo "\033[0;31mCleaning objects🧹"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "\033[0;31mRemoving $(NAME) 🗑\033[0;37m"
	@rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re
