SRC		=	srcs/water.cpp

OBJ		=	$(SRC:.cpp=.o)

NAME	=	water

LIB		=	-lSDL

CXX		=	g++

CXXFLAGS	=	-W -Wall -Wextra -Werror -std=c++11 -fopenmp -O3

RM		=	rm -f

all:	$(NAME)

$(NAME):	$(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ) $(LIB)

clean:
	$(RM) $(OBJ)

fclean:	clean
	$(RM) $(NAME)

re:	fclean all

.PHONY:	all clean fclean re
