SRC = $(CURDIR)/../Unity-master/src/unity.c $(CURDIR)/../../Tokenizing/Tokenizing.cpp $(CURDIR)/../../ErrorHandling/Error.cpp TokenizingTest.cpp
OBJ = $(SRC:.cpp=.o)
OBJ := $(OBJ:.c=.o)
CXX = c++
CXXFLAGS = -g3 -Wall -Wextra -Werror -std=c++98 --coverage
NAME = a.out

all : $(NAME) coverage

coverage : 
	-./a.out
	gcovr --html-details -r ../.. -o coverage/coverage.html
	@find ../.. \( \
		-name "*.gcno" -o \
		-name "*.gcda" \
	\) -delete

rm_coverage:
	find ../.. \( \
		-name "*.html" -o \
		-name "*.gcno" -o \
		-name "*.gcda" -o \
		-name "*.js"   -o \
		-name "*.css" \
	\) -delete

$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	-@rm $(OBJ)

fclean : clean
	-@rm $(NAME)

re : fclean all

.PHONY: all clean fclean re coverage rm_coverage