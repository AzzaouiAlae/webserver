SRC = ../Unity-master/src/unity.c ../../Tokenizing/Tokenizing.cpp ../../ErrorHandling/Error.cpp TokenizingTest.cpp
OBJ = ../Unity-master/src/unity.o ../../Tokenizing/Tokenizing.o ../../ErrorHandling/Error.o TokenizingTest.o
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