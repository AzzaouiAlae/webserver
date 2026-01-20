SRC = main.cpp 
OBJ = main.o
CXX = c++
CXXFLAGS = -g3 -Wall -Wextra -Werror -std=c++98 
NAME = websrv

all : $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean :
	rm $(OBJ)

fclean : clean
	rm $(NAME)

re : fclean $(NAME)

.PHONY: all clean fclean re