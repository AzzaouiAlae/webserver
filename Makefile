SRC = main.cpp $(shell find . -name "*.cpp" | grep -v "main" | grep -v ".*Test.cpp")
OBJ = $(SRC:.cpp=.o)
CXXFLAGS = -g3 -Wall -Wextra -Werror -std=c++98 
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