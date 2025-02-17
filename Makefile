SRC = Sources/Client.cpp \
	Sources/Server.cpp \
	Sources/Process.cpp\
	Sources/Tools.cpp \
	Sources/TestConfig.cpp \
	Sources/main.cpp \
	Sources/Parser/Parser.cpp \
	Sources/Parser/Utils.cpp \
	Sources/Parser/LocationParser.cpp \
	Sources/Parser/ServerParser.cpp \
	Sources/Parser/HttpParser.cpp \
	

OBJ = $(SRC:.cpp=.o)
OBJS=$(OBJ)

FLAGS = c++ -Wall -Wextra -Werror --std=c++98 -g -fsanitize=address

NAME = WebServ

all : $(NAME)

$(NAME): $(OBJS)
	$(FLAGS) $(OBJS) -o $(NAME)

clean:
			rm -f $(OBJ)

fclean : clean
			rm -f $(NAME)

re : fclean all
