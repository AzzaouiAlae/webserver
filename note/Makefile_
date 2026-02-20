DATA := $(shell find ./DefaultPages/Pages -name "*.htm")
DATA_OBJ := $(DATA:.htm=.htm.o)

SRC := $(shell find . -name "*.cpp" | grep -v "./main.cpp" | grep -v ".*Test.cpp")
OBJ = $(SRC:%.cpp=%.o)

CXX = c++
CXXFLAGS =  -Wall -Wextra -Werror -std=c++98 -g3 #-Ofast
NAME = websrv.out

all : $(NAME)

$(NAME): $(OBJ) $(DATA_OBJ)
	$(CXX) $(CXXFLAGS) $(DATA_OBJ) $(OBJ) -o $(NAME)

%.htm.o: %.htm
	ld -r -b binary $< -o $@

clean :
	-rm $(OBJ) $(DATA_OBJ)

fclean : clean
	-rm $(NAME)

re : fclean $(NAME)

.PHONY: all clean fclean re