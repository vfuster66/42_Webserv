# Définition des variables
CC = c++
CFLAGS = -Wall -Werror -Wextra -std=c++98 -Wshadow
LDFLAGS =
EXEC = webserv
SRC = $(wildcard *.cpp) $(wildcard srcs/*.cpp)
OBJ = $(SRC:.cpp=.o)
OBJ_DIR = objs
OBJS = $(addprefix $(OBJ_DIR)/,$(notdir $(OBJ)))
INCLUDE = -I includes

# Règle principale
all: $(EXEC)

# Correction: Ajoutez une règle pour créer le dossier objs avant la compilation
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compilation
$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Assurez-vous que la création du dossier objs est une dépendance pour la compilation des objets
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDE)

$(OBJ_DIR)/%.o: srcs/%.cpp | $(OBJ_DIR)
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDE)

# Nettoyage
clean:
	rm -rf $(OBJ_DIR)/*.o

# Suppression de l'exécutable et du dossier objs
fclean: clean
	rm -rf $(EXEC) $(OBJ_DIR)
	rm -f *.log
# Pour recompiler
re: fclean all

