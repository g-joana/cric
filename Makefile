NAME	= ircserv

SRC		= main.cpp Server.cpp Client.cpp CommandParser.cpp Channel.cpp
OBJ		= $(SRC:.cpp=.o)

CXX		= c++
CXXFLAGS= -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

distclean: fclean
	rm -f *.dwo *.pdb test/*.log

re: fclean all

.PHONY: all clean fclean distclean re
