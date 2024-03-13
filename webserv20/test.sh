#!/bin/bash

# Définition des couleurs
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Fonction pour effectuer des tests et afficher les résultats en couleur
perform_test() {
    # Exécution de la commande curl et capture du code de statut HTTP
    status=$(curl -s -o /dev/null -w "%{http_code}" "$@")

    # Affichage du test effectué
    echo -n "Test: $@ - Code: $status - "

    # Vérification du code de statut et affichage en conséquence
    if [ "$status" -eq 200 ] || [ "$status" -eq 204 ]; then
        echo -e "${GREEN}Réussi${NC}"
    else
        echo -e "${RED}Echec${NC}"
    fi
}

# Fonction pour effectuer des tests avec méthode spécifique et afficher les résultats en couleur
perform_cgi_test() {
    local method=$1
    local url=$2
    local test_name=$3

    echo -e "Test: $test_name - Méthode: $method - URL: $url\c"

    # Utilisation de la méthode spécifiée dans l'appel à curl
    if [ "$method" == "POST" ]; then
        response=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data "data=test" "$url")
    else
        response=$(curl -s -o /dev/null -w "%{http_code}" -X GET "$url")
    fi

    # Vérification du code de statut HTTP
    if [ "$response" -eq 200 ]; then
        echo -e "${GREEN} Réussi${NC}"
    else
        echo -e "${RED} Échec (Code: $response)${NC}"
    fi
}

perform_index_test() {
    local url=$1
    local expected_status=$2
    local test_name=$3
    status=$(curl -s -o /dev/null -w "%{http_code}" "$url")

    # Affichage du test effectué avec une nouvelle ligne avant et en jaune
    echo -e "Test: $test_name ($url) - Code attendu: $expected_status, Code obtenu: $status - \c"

    # Vérification du code de statut et affichage en conséquence
    if [ "$status" -eq "$expected_status" ]; then
        echo -e "${GREEN}Réussi${NC}"
    else
        echo -e "${RED}Échec${NC}"
    fi
}

# Fonction pour vérifier le contenu de la réponse
check_response_content() {
    local url=$1
    local expected_content=$2
    local test_name=$3

    response=$(curl -s "$url")

    echo -n "Test: $test_name ($url) - Contenu attendu: '$expected_content' - "
    echo "$response" | grep -q "$expected_content"
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Réussi${NC}"
    else
        echo -e "${RED}Échec${NC}"
    fi
}

# Fonction pour tester la redirection
test_redirection() {
    local url=$1
    local expected_url=$2
    local test_name=$3

    # Effectue une requête GET avec suivi de redirections et extrait l'URL finale
    final_url=$(curl -s -L -o /dev/null -w "%{url_effective}" "$url")

    # Affiche le test effectué
    echo -e "Test: $test_name - Redirection depuis: $url - URL finale attendue: $expected_url - \c"

    # Vérifie si l'URL finale correspond à l'URL attendue
    if [ "$final_url" == "$expected_url" ]; then
        echo -e "${GREEN}Réussi${NC}"
    else
        echo -e "${RED}Échec - URL finale obtenue: $final_url${NC}"
    fi
}

# Fonction pour tester le téléchargement de fichiers
test_download() {
    local url=$1
    local test_name=$2

    echo -e "Test: $test_name - Téléchargement depuis: $url\c"

    # Téléchargement du fichier dans le dossier temporaire
    curl -s -o "/tmp/downloaded_file" "$url"

    # Vérification de l'existence du fichier téléchargé
    if [ -f "/tmp/downloaded_file" ]; then
        echo -e "${GREEN}\nTest Réussi - Le fichier a été téléchargé avec succès.${NC}"
    else
        echo -e "${RED}Échec - Le fichier n'a pas été téléchargé.${NC}"
    fi

    # Nettoyage : Supprimer le fichier téléchargé après le test
    rm -f "/tmp/downloaded_file"
}

# Adresse du serveur web à tester sur le port 3500
SERVER_URL_3500="http://localhost:3500"

# Adresse du serveur web à tester sur le port 3000
SERVER_URL_3000="http://localhost:3000"

test_persistence() {
    SERVER_URL=$1
    PORT=$2
    COOKIE_FILE="cookies_${PORT}.txt"

    # Envoie une première requête pour initier une session et récupérer le cookie de session
    curl -c $COOKIE_FILE -i "$SERVER_URL" 2>/dev/null > /dev/null
    SESSION_COOKIE=$(grep 'sessionId' $COOKIE_FILE | awk '{print $7}')

    # Affichage du résultat de la récupération du cookie
    if [ -z "$SESSION_COOKIE" ]; then
        echo -e "${RED}Test échoué : Aucun cookie de session n'a été créé sur le port $PORT.${NC}"
    else
        echo -e "${GREEN}Test réussi : Cookie de session créé sur le port $PORT.${NC}"
    fi

    # Envoie une deuxième requête avec le cookie de session pour vérifier la persistance de la session
    curl -b $COOKIE_FILE -c $COOKIE_FILE -i "$SERVER_URL" 2>/dev/null > /dev/null
    SESSION_COOKIE_SECOND=$(grep 'sessionId' $COOKIE_FILE | awk '{print $7}')

    # Vérification de la persistance du cookie de session
    if [ "$SESSION_COOKIE" = "$SESSION_COOKIE_SECOND" ]; then
        echo -e "${GREEN}Test réussi : La session est persistante sur le port $PORT.${NC}"
    else
        echo -e "${RED}Test échoué : La session n'est pas persistante sur le port $PORT.${NC}"
    fi

    # Nettoyage
    rm $COOKIE_FILE
}

# Tests du site statique sur le port 3000
echo -e "${YELLOW}Test du site statique sur le port 3000${NC}"
perform_test -X GET localhost:3000/proxygirls.html
perform_test -X POST localhost:3000 -d "data=test"
perform_test -X POST localhost:3000 -d "@largefile.txt"
perform_test -X DELETE localhost:3000/delete.txt

# Tests CGI sur différents ports
echo -e "\n${YELLOW}Tests CGI sur différents ports${NC}"
# PHP
perform_cgi_test "GET" "http://localhost:4500/cgi-bin/cgi.php" "Test CGI PHP avec GET"
perform_cgi_test "POST" "http://localhost:4500/cgi-bin/cgi.php" "Test CGI PHP avec POST"

# Python
perform_cgi_test "GET" "http://localhost:4200/cgi-bin/cgi.py" "Test CGI Python avec GET"
perform_cgi_test "POST" "http://localhost:4200/cgi-bin/cgi.py" "Test CGI Python avec POST"

# Perl
perform_cgi_test "GET" "http://localhost:4100/cgi-bin/cgi.pl" "Test CGI Perl avec GET"
perform_cgi_test "POST" "http://localhost:4100/cgi-bin/cgi.pl" "Test CGI Perl avec POST"

# Tests de listing de répertoire et d'accès fichier sur le port 3200
echo -e "\n${YELLOW}Tests de listing de répertoire et d'accès fichier sur le port 3200${NC}"
check_response_content "http://localhost:3200/" "cgi-bin" "Listing du répertoire racine incluant cgi-bin"
check_response_content "http://localhost:3200/cgi-bin/" "cgi.php" "Existence de cgi.php dans cgi-bin"
perform_index_test "http://localhost:3200/blabla/" 404 "Inexistence du dossier blabla"
perform_index_test "http://localhost:3200/images/coucou.html" 404 "Inexistence du fichier coucou.html dans images"

# Test de redirection
base_url="http://localhost:3800"
echo -e "\n${YELLOW}Test des redirections sur le port 3800"${NC}
test_redirection "${base_url}/pages.html" "${base_url}/proxygirls.html" "Redirection /pages.html vers /proxygirls.html"

# Test de téléchargement
echo -e "\n${YELLOW}Test de telechargement de fichier sur le port 3500"${NC}
test_download "http://localhost:3500/fichiers/FormulaireBasique.pdf" "Téléchargement de FormulaireBasique.pdf"

# Exécution des tests de persistance de session pour les ports 3500 et 3000
echo -e "${YELLOW}\nDébut des tests de persistance de session sur les ports 3500 et 3000${NC}"
test_persistence $SERVER_URL_3500 3500
test_persistence $SERVER_URL_3000 3000

# Test de charge avec siege
echo -e "${YELLOW}\nLancement du test de charge avec siege${NC}"
# Récupération du résultat de siege
siege_result=$(siege -c50 -t5M -f urls.txt)
echo "$siege_result"

