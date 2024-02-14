# Définition des variables
CC = c++
CFLAGS = -Wall -Werror -Wextra -std=c++98
LDFLAGS =
EXEC = webserv
SRC = $(wildcard *.cpp) $(wildcard srcs/*.cpp)
OBJ = $(SRC:.cpp=.o)
OBJ_DIR = objs
OBJS = $(addprefix $(OBJ_DIR)/,$(notdir $(OBJ)))
INCLUDE = -I includes

# Création du dossier objs
$(shell mkdir -p $(OBJ_DIR))

# Règle principale
all: $(EXEC)

# Compilation
$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDE)

$(OBJ_DIR)/%.o: srcs/%.cpp
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDE)

# Nettoyage
clean:
	rm -rf $(OBJ_DIR)/*.o

# Suppression de l'exécutable et du dossier objs
fclean: clean
	rm -rf $(EXEC)
	rm -rf $(OBJ_DIR)

# Pour recompiler
re: fclean all

