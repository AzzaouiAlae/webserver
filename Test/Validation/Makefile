SRC = ../Unity-master/src/unity.c ValidationTest.cpp ../../Parsing/Parsing.cpp ../../TokensTypes/TokensTypes.cpp\
		../../Validation/Validation.cpp ../../Tokenizing/Tokenizing.cpp ../../ErrorHandling/Error.cpp
OBJ = $(SRC:.cpp=.o)
OBJ := $(OBJ:.c=.o)
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