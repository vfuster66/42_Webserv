# Définition des variables
CC = c++
CFLAGS = -Wall -Werror -Wextra -std=c++98
LDFLAGS =
EXEC = webserv
SRC = $(wildcard *.cpp) $(wildcard srcs/*.cpp)
OBJ = $(SRC:.cpp=.o)
INCLUDE = -I includes

# Règle principale
all: $(EXEC)

# Compilation
$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDE)

# Nettoyage
clean:
	rm -rf *.o
	rm -rf src/*.o

# Suppression de l'exécutable
fclean: clean
	rm -rf $(EXEC)

