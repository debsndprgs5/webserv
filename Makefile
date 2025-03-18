SRC = Sources/HTTP_request_parser.cpp \
	Sources/HTTP_response_maker.cpp \
	Sources/Client.cpp \
	Sources/Server.cpp \
	Sources/Process.cpp\
	Sources/Tools.cpp \
	Sources/Parser/Parser.cpp \
	Sources/Parser/Utils.cpp \
	Sources/Parser/LocationParser.cpp \
	Sources/Parser/ServerParser.cpp \
	Sources/Parser/HttpParser.cpp \
	Sources/Methods.cpp \



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
