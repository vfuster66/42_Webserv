#!/usr/bin/python3
print("Status: 200 OK")
print("Content-Type: text/html; charset=utf-8")
print("")

import requests

# Adresse du serveur
url = 'http://localhost:4000'

# Envoyer une requête GET
response = requests.get(url, cookies={'sessionId': 'valeurSessionId'})

# Afficher le cookie de session dans la réponse
print(response.cookies.get('sessionId'))

# Afficher le corps de la réponse
print(response.text)

