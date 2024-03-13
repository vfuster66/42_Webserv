# Définition des variables
CC = c++
CFLAGS = -g -Wall -Werror -Wextra -std=c++98 #-fsanitize=address
LDFLAGS = #-lasan
EXEC = webserv
SRC = $(wildcard *.cpp) $(wildcard srcs/*.cpp)
OBJ = $(SRC:.cpp=.o)
OBJ_DIR = objs
OBJS = $(addprefix $(OBJ_DIR)/,$(notdir $(OBJ)))
INCLUDE = -I includes

# Marque les cibles n'ayant pas de fichier associé
.PHONY: all set-permissions clean fclean re

# Règle principale
all: set-permissions $(EXEC)

# Ajoutez une règle pour créer le dossier objs avant la compilation
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compilation de l'exécutable
$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compilation des fichiers objets, avec création du dossier objs si nécessaire
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDE)

$(OBJ_DIR)/%.o: srcs/%.cpp | $(OBJ_DIR)
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDE)

# Changement des permissions
set-permissions:
	chmod 755 test.sh
	chmod 755 *.txt
	find ./www/cgi-bin -type f -name "cgi.*" -exec chmod 755 {} +

# Nettoyage des fichiers objets
clean:
	rm -rf $(OBJ_DIR)/*.o

# Suppression de l'exécutable, du dossier objs, et des fichiers .log
fclean: clean
	rm -rf $(EXEC) $(OBJ_DIR)
	rm -f *.log

# Pour recompiler
re: fclean all
