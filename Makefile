
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
