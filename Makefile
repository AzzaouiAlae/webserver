SRC = main.cpp Tokenizing/Tokenizing.cpp Parsing/Parsing.cpp Validation/Validation.cpp ErrorHandling/Error.cpp Singleton/Singleton.cpp
OBJ = main.o Tokenizing/Tokenizing.o Parsing/Parsing.o Validation/Validation.o ErrorHandling/Error.o Singleton/Singleton.o
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 
NAME = websrv.out

all : $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean :
	-rm $(OBJ)

fclean : clean
	-rm $(NAME)

re : fclean $(NAME)

.PHONY: all clean fclean re