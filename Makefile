SRC = Sources/HTTP_request_parser.cpp \
	Sources/HTTP_response_maker.cpp \
	Sources/Client.cpp \
	Sources/Server.cpp \
	Sources/Process.cpp \
	Sources/Tools.cpp \
	Sources/Parser/Parser.cpp \
	Sources/Parser/Utils.cpp \
	Sources/Parser/LocationParser.cpp \
	Sources/Parser/ServerParser.cpp \
	Sources/Parser/HttpParser.cpp \
	Sources/Methods.cpp \
	Sources/cgi_php_handler.cpp

OBJ = $(SRC:.cpp=.o)
OBJS = $(OBJ)

FLAGS = -Wall -Wextra -Werror --std=c++98 -g #-fsanitize=address -D_GLIBCXX_DEBUG
CC = g++

NAME = WebServ

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
