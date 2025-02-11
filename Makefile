<<<<<<< HEAD
NAME		= webserv

CC			= c++
FLAGS		= -Wall -Wextra -Werror -std=c++98 -g
RM			= rm -rf

OBJDIR = .objFiles

FILES		= 	parser \

SRC			= $(FILES:=.cpp)
OBJ			= $(addprefix $(OBJDIR)/, $(FILES:=.o))
HEADER		= parser.hpp \


#Colors:
GREEN		=	\e[92;5;118m
YELLOW		=	\e[93;5;226m
GRAY		=	\e[33;2;37m
RESET		=	\e[0m
CURSIVE		=	\e[33;3m

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJ) $(HEADER)
	@$(CC) $(OBJ) $(FLAGS) -o $(NAME)
	@printf "$(_SUCCESS) $(GREEN)- Executable ready.\n$(RESET)"

$(OBJDIR)/%.o: %.cpp $(HEADER)
	@mkdir -p $(dir $@)
	@$(CC) $(FLAGS) -c $< -o $@

clean:
	@$(RM) $(OBJDIR) $(OBJ)
	@printf "$(YELLOW)    - Object files removed.$(RESET)\n"

fclean: clean
	@$(RM) $(NAME)
	@printf "$(YELLOW)    - Executable removed.$(RESET)\n"

re: fclean all
=======

SRC = Sources/Client.cpp \
	Sources/Server.cpp \
	Sources/Process.cpp\
	Sources/Tools.cpp \
	Sources/TestConfig.cpp \
	Sources/main.cpp \
		

OBJ = $(SRC:.cpp=.o)
OBJS=$(OBJ)

FLAGS = g++ -Wall -Wextra -Werror --std=c++98 -g -fsanitize=address

NAME = WebServ

all : $(NAME)

$(NAME): $(OBJS)
	$(FLAGS) $(OBJS) -o $(NAME)

clean:
			rm -f $(OBJ)

fclean : clean
			rm -f $(NAME)

re : fclean all
>>>>>>> cad8d49879d7c48f37e6eea83ae0a4708d19a562
