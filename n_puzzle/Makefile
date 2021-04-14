# ___COMPILATION___

C_COMPILER= clang++
FLAGS= -Wall -Wextra -Werror -std=c++11 -g3 -Ofast
SRCS= npuzzle.cpp

NAME= npuzzle
OBJS= ${SRCS:.cpp=.o}
INCLUDE_DIRS = -I .

.cpp.o:
	${C_COMPILER} ${FLAGS} ${INCLUDE_DIRS} -c $< -o ${<:.cpp=.o}

all: ${NAME}

${NAME}: ${OBJS}
	${C_COMPILER} ${FLAGS} ${INCLUDE_DIRS} ${OBJS} -o ${NAME}

start: all
	@./${NAME}

valgrind_start: all
	@valgrind --leak-check=full --show-leak-kinds=definite ./${NAME}

clean:
	rm -rf ${OBJS}

fclean: clean
	rm -rf ${NAME}

re: fclean all

.PHONY: all clean fclean re start