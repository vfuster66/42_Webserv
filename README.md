# 42-Webserv

[![vfuster-'s 42 webserv Score](https://badge42.coday.fr/api/v2/cltpx5zqv531101p4bte37ts1/project/3528755)](https://github.com/Coday-meric/badge42)
le 13 mars 2024

## Sources HTTP

- https://fr.wikipedia.org/wiki/Hypertext_Transfer_Protocol
- https://www.w3.org/Protocols/
- https://www.it-connect.fr/le-protocole-http-pour-les-debutants/
- https://www.pierre-giraud.com/http-reseau-securite-cours/
- https://www.tutorialspoint.com/http/index.htm
- https://developer.mozilla.org/fr/docs/Web/HTTP
- https://www.youtube.com/watch?v=WGdOWtKL5nA
- https://www.youtube.com/watch?v=Qxs2S4agyxk

## Sources RFC

- https://www.rfc-editor.org/

## Sources Serveur Web

- https://fr.wikipedia.org/wiki/Serveur_web
- https://www.youtube.com/watch?v=msB9AvJ4bTM

## Sources Sockets

- https://en.wikipedia.org/wiki/Network_socket
- https://docs.oracle.com/javase/tutorial/networking/sockets/
- https://www.youtube.com/watch?v=LtXEMwSG5-8
- https://www.youtube.com/watch?v=mStnzIEprH8

## Sources Nginx

- https://fr.wikipedia.org/wiki/NGINX
- https://doc.ubuntu-fr.org/nginx

## Sources CGI

- https://fr.wikipedia.org/wiki/Common_Gateway_Interface
- https://www.tutorialspoint.com/python/python_cgi_programming.htm
- http://deptinfo.cnam.fr/Enseignement/CycleSpecialisation/IHM/pdf/Cgi.pdf

## Sources Modèles OSI

- https://www.youtube.com/watch?v=26jazyc7VNk

## Sources ports et protocoles

- https://www.youtube.com/watch?v=YSl6bordSh8

## Sources IO

- https://www.ibm.com/docs/en/i/7.4?topic=concepts-nonblocking-io
- https://www.ibm.com/docs/en/i/7.4?topic=concepts-io-multiplexingselect
- https://www.youtube.com/watch?v=dEHZb9JsmOU&list=WL&index=85&t=161s

## Fonctionnements

Initialisation et Configuration du Serveur :
La fonction main initialise le serveur en instanciant un objet Server avec un chemin vers le fichier de configuration,
un chemin vers le fichier de log et un niveau de log en paramètres.
Le constructeur Server lit et analyse la configuration du serveur à l'aide de ConfigParser::parse,
configure les sockets du serveur avec Server::setupServerSockets et configure le logging via le singleton Logger.

Fonctionnement du Serveur :
Server::start gère les connexions entrantes, accepte les nouvelles sockets clients et les ajoute à un ensemble 
principal pour surveillance. Il utilise select pour multiplexer entre les descripteurs de fichiers actifs.
Pour chaque descripteur de fichier client lisible, il extrait la requête, analyse les cookies et crée une 
session si nécessaire. Il construit ensuite un objet HttpRequest.

Gestion des Requêtes :
Le RequestHandler prend le HttpRequest, le valide et détermine le type de requête (CGI, GET, POST, DELETE). 
Pour les requêtes CGI, il utilise CgiHandler pour exécuter le script CGI et analyser sa sortie en un HttpResponse.
Pour les requêtes GET, POST et DELETE, RequestHandler traite la requête en conséquence, servant potentiellement 
des fichiers, gérant les données de formulaire ou exécutant des actions côté serveur.

Génération de Réponse :
Response::buildHttpResponse construit la chaîne de réponse HTTP à partir de l'objet HttpResponse, 
en définissant les en-têtes et le contenu appropriés en fonction du résultat du traitement de la requête.
Le serveur renvoie ensuite la réponse au client en utilisant Server::safeWrite.

Gestion des Sessions et des Cookies :
SessionManager gère la création, la validation et la clôture des sessions. 
Il génère des ID de session uniques et maintient une carte de session.
Cookies analyse et gère les cookies des en-têtes de requête HTTP, permettant au 
serveur de récupérer ou de définir des cookies liés aux sessions clients.

Logging :
La classe Logger fournit des fonctionnalités de logging dans toute l'application, 
écrivant des messages de log à la fois sur la console et dans un fichier avec des niveaux de log configurables.
