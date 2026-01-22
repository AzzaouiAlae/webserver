SRC = ../Unity-master/src/unity.c ParsingTest.cpp ../../Parsing/Parsing.cpp ../../TokensTypes/TokensTypes.cpp
OBJ = ../Unity-master/src/unity.o ParsingTest.o ../../Parsing/Parsing.o ../../TokensTypes/TokensTypes.o
CXX = c++
CXXFLAGS = -g3 -Wall -Wextra -Werror -std=c++98 
NAME = a.out

all : $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean :
	-rm $(OBJ)

fclean : clean
	-rm $(NAME)

re : fclean $(NAME)

.PHONY: all clean fclean re