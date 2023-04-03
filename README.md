# 42 exam_rank_06
## Introduction
Ce projet est une implémentation d'un chatroom. Le serveur accepte des connexions des clients et leur permet de communiquer entre eux.

Le programme est écrit en C et utilise les sockets pour la communication en réseau. Le code est modulaire et contient plusieurs fonctions pour gérer les connexions et les messages entrants.

## usage
Compilation est execution du programme sur le port```8888```:
```fish
gcc -Wall -Wextra -Werror main.c -o server && ./server 8888
```
Connection au server
```fish
nc localhost 8888
```

## Fonctionnement
L'exécution du serveur se fait avec la commande ./server port où port est le numéro du port sur lequel le serveur doit écouter les connexions entrantes.

Une fois lancé, le serveur se met en attente de connexions des clients. Les clients peuvent se connecter en utilisant la commande telnet localhost port. Une fois connecté, les clients peuvent envoyer des messages qui seront envoyés à tous les autres clients connectés.

## Architecture du code
Le code est divisé en plusieurs parties :

## Structures
Le code utilise une structure t_client pour stocker les informations des clients connectés. La structure contient les champs suivants :

- ```fd``` : le descripteur de fichier correspondant à la socket du client.
- ```id``` : l'identifiant du client, qui est équivalent à son numéro d'ordre de connexion.
- ```new``` : un indicateur pour savoir si le client est nouveau ou non. Utile pour afficher des messages différents pour les nouveaux clients.
- ```next``` et prev : des pointeurs vers les clients suivants et précédents dans la liste doublement chaînée.

## Variables globales
Le code utilise plusieurs variables globales pour gérer les connexions et les messages :

- ```sockfd``` : le descripteur de fichier correspondant à la socket du serveur.
- ```fd_max``` : le numéro maximum de descripteurs de fichiers utilisés. C'est utile pour déterminer la valeur de retour de la fonction ```select()```.
- ```tokenBuff``` : un tampon utilisé par la fonction ```ft_strtok()``` pour stocker le jeton analysé.
- ```pool_set```, read_set et write_set : des ensembles de sockets utilisés par la fonction ```select()``` pour détecter les activités de lecture et d'écriture.
- ```clients``` : un pointeur vers la liste doublement chaînée des clients connectés.

## Fonctions
Le code utilise plusieurs fonctions pour gérer les connexions et les messages :

- ```Wrong()``` : fonction pour afficher un message d'erreur en cas d'argument invalide.
- ```Fatal()``` : fonction pour afficher un message d'erreur en cas d'erreur fatale.
- ```ft_strchr()``` : fonction pour rechercher un caractère dans une chaîne.
- ```ft_strtok()``` : fonction pour extraire des jetons d'une chaîne.
- ```get_fd_max()``` : fonction pour obtenir le numéro maximum de descripteurs de fichiers utilisés.
- ```send_all()``` : fonction pour envoyer un message à tous les clients connectés, sauf à l'émetteur.
- ```addClient()``` : fonction pour ajouter un client à la liste doublement chaînée des clients connectés.
- ```add_client()``` : fonction pour ajouter un nouveau client à la liste des clients connectés.
- ```get_client()``` : fonction pour obtenir un pointeur vers un client à partir de son descripteur de fichier.
- ```msg()``` : fonction pour gérer les messages entrants des clients.
