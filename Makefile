DATA :=  $(shell find ./HTTP/DefaultPages/Pages -name "*.htm")
DATA_OBJ := $(DATA:.htm=.htm.o)

SRC := $(shell find . -name "*.cpp" | grep -v "./mainAlae.cpp" | grep -v ".*Test.cpp" | grep -v ".*Cgi.cpp")
OBJ = $(SRC:%.cpp=%.o)

INCLUDES := $(shell find . -name "*.hpp" -exec dirname {} \; | sort -u | awk '{printf "-I%s ", $$1}')

CXX = c++
CXXFLAGS = $(INCLUDES)  -Wall -Wextra -Werror -std=c++98  -g3 #-Ofast
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