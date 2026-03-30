SRC = ../Unity-master/src/unity.c MultipartTests.cpp $(shell find ../../  -name "*.cpp" | grep -v "main" | grep -v ".*Test.cpp")
OBJ = $(SRC:.cpp=.o)
OBJ := $(OBJ:.c=.o)
CXX = c++
CXXFLAGS = -g3 -Wall -Wextra -Werror -std=c++98
NAME = a.out
DATA :=  $(shell find ./HTTP/DefaultPages/Pages -name "*.htm")
DATA_OBJ := $(DATA:.htm=.htm.o)

all : $(NAME)

INCLUDES := $(shell find ../../ -name "*.hpp" -exec dirname {} \; | sort -u | awk '{printf "-I%s ", $$1}')

coverage : 
	-./a.out
	gcovr --html-details -r .. -o coverage/coverage.html
	@find .. \( \
		-name "*.gcno" -o \
		-name "*.gcda" \
	\) -delete

rm_coverage:
	find .. \( \
		-name "*.html" -o \
		-name "*.gcno" -o \
		-name "*.gcda" -o \
		-name "*.js"   -o \
		-name "*.css" \
	\) -delete

$(NAME): $(OBJ) $(DATA_OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OBJ) $(DATA_OBJ) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.htm.o: %.htm
	ld -r -b binary $< -o $@

clean :
	-@rm $(OBJ)

fclean : clean
	-@rm $(NAME)

cov: CXXFLAGS += -O0 -fprofile-arcs -ftest-coverage

cov: fclean $(NAME) coverage 

re : fclean $(NAME)

.PHONY: all clean fclean re coverage rm_coverage