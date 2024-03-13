#!/bin/bash

# Adresse du serveur web à tester sur le port 3500
SERVER_URL_3500="http://localhost:3500"

# Adresse du serveur web à tester sur le port 3000
SERVER_URL_3000="http://localhost:3000"

# Test pour le port 3500
echo "Test sur le port 3500:"
# Envoie une première requête pour initier une session et récupérer le cookie de session
response=$(curl -c cookies_3500.txt -i "$SERVER_URL_3500" 2>/dev/null)
sessionCookie3500=$(grep 'sessionId' cookies_3500.txt | awk '{print $7}')
echo "Session Cookie récupéré sur le port 3500: $sessionCookie3500"

# Envoie une deuxième requête avec le cookie de session pour vérifier la persistance de la session
response=$(curl -b cookies_3500.txt -c cookies_3500.txt -i "$SERVER_URL_3500" 2>/dev/null)
sessionCookie3500Second=$(grep 'sessionId' cookies_3500.txt | awk '{print $7}')
echo "Session Cookie après la deuxième requête sur le port 3500: $sessionCookie3500Second"

if [ "$sessionCookie3500" = "$sessionCookie3500Second" ]; then
    echo "Test réussi : La session est persistante sur le port 3500."
else
    echo "Test échoué : La session n'est pas persistante sur le port 3500."
fi

# Test pour le port 3000
echo "Test sur le port 3000:"
# Envoie une première requête pour initier une session et récupérer le cookie de session
response=$(curl -c cookies_3000.txt -i "$SERVER_URL_3000" 2>/dev/null)
sessionCookie3000=$(grep 'sessionId' cookies_3000.txt | awk '{print $7}')
echo "Session Cookie récupéré sur le port 3000: $sessionCookie3000"

# Envoie une deuxième requête avec le cookie de session pour vérifier la persistance de la session
response=$(curl -b cookies_3000.txt -c cookies_3000.txt -i "$SERVER_URL_3000" 2>/dev/null)
sessionCookie3000Second=$(grep 'sessionId' cookies_3000.txt | awk '{print $7}')
echo "Session Cookie après la deuxième requête sur le port 3000: $sessionCookie3000Second"

if [ "$sessionCookie3000" = "$sessionCookie3000Second" ]; then
    echo "Test réussi : La session est persistante sur le port 3000."
else
    echo "Test échoué : La session n'est pas persistante sur le port 3000."
fi

# Nettoyage
rm cookies_3500.txt cookies_3000.txt


