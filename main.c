#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

// Structure pour stocker les informations des clients
typedef struct s_client {
  int fd, id, new;
  struct s_client *next;
  struct s_client *prev;
} t_client;

// Définition des variables globales
int sockfd;     // Socket du serveur
ssize_t fd_max; // Nombre maximum de clients

char tokenBuff[4096]; // buff pour la fonction ft_strtok

fd_set pool_set;  // Ensemble de sockets en lecture
fd_set read_set;  // Ensemble de sockets en écriture
fd_set write_set; // Ensemble de sockets en lecture

t_client *clients; // Liste des clients connectés

// Fonction pour afficher un message d'erreur en cas d'argument invalide
void Wrong() {
  char err[] = "Wrong number of arguments\n";
  write(2, err, strlen(err));
  exit(1);
}

// Fonction pour afficher un message d'erreur en cas d'erreur fatale
void Fatal() {
  char err[] = "Fatal error\n";
  write(2, err, strlen(err));
  exit(1);
}

char *ft_strchr(char *s, int c) {
  while (*s != '\0' && *s != c) {
    s++;
  }
  if (*s == c)
    return (s);
  return NULL;
}

char *ft_strtok(char *s, char *delim) {
  // déclaration d'une variable statique pour stocker la position de début du
  // prochain token
  static char *next = NULL;
  // déclaration d'un compteur d'indice pour le tampon de caractères
  int i = 0;

  // effacer le tampon de caractères
  bzero(tokenBuff, 4096);
  // si un pointeur vers une chaîne est fourni, initialiser la variable "next" à
  // cette adresse
  if (s != NULL)
    next = s;
  // si "next" est NULL ou pointe vers la fin de la chaîne, retourner NULL
  if (next == NULL || *next == '\0')
    return NULL;
  // tant que le caractère courant n'est pas un caractère de délimitation et que
  // la fin de la chaîne n'a pas été atteinte, stocker le caractère dans le
  // tampon de caractères, incrémenter l'indice et passer au caractère suivant
  while (*next != '\0' && ft_strchr(delim, *next) == NULL) {
    tokenBuff[i++] = *next;
    next++;
  }
  // si le caractère courant est un caractère de délimitation,
  // stocker le caractère dans le tampon, incrémenter l'indice, stocker le
  // caractère nul '\0' et passer au caractère suivant
  if (*next != '\0') {
    tokenBuff[i++] = *next++;
    *next = '\0';
  }
  // retourner le pointeur vers le début du token
  return tokenBuff;
}

// Fonction pour obtenir le socket avec le plus grand numéro de descripteur
int get_fd_max() {
  int fd = 0;
  for (t_client *it = clients; it; it = it->next) {
    if (fd < it->fd)
      fd = it->fd;
  }
  if (fd == 0)
    return sockfd;
  return fd;
}

// fd est utilisé pour s'assurer que le message n'est pas envoyé au
// client émetteur
void send_all(char *s, int fd) {
  int len = strlen(s);
  for (t_client *it = clients; it; it = it->next) {
    if (fd != it->fd && FD_ISSET(it->fd, &write_set)) {
      if (send(it->fd, s, len, 0) < 0)
        Fatal();
    }
  }
}

void addClient(t_client *cli) {
  // Si la liste est vide, ajoutez le client en tant que premier élément
  if (clients == NULL) {
    clients = cli;
    return;
  }
  // Parcours la liste des clients existants
  for (t_client *it = clients; it; it = it->next) {
    // Si l'élément suivant est nul, ajoutez le client à la fin de la liste
    if (it->next == NULL) {
      it->next = cli;
      cli->prev = it;
      break;
    }
  }
}

void add_client() {
  char prompt[100];
  t_client *cli = malloc(sizeof(t_client));

  if (cli == NULL)
    Fatal();
  // Vérifie si le client peut être alloué
  bzero(prompt, 100);
  // Accepte une nouvelle connexion
  if ((cli->fd = accept(sockfd, NULL, NULL)) < 0)
    Fatal();
  // Initialise les valeurs du nouveau client
  cli->id = fd_max;
  cli->new = 1;
  cli->next = NULL;
  cli->prev = NULL;
  fd_max++;
  // Ajoutez le nouveau client à la liste et définit son fd dans le pool_set
  FD_SET(cli->fd, &pool_set);
  // Génère le message d'arrivée du client
  sprintf(prompt, "server: client %d just arrived\n", cli->id);
  // Envoi message nouvelle connection au client connectés
  send_all(prompt, cli->fd);
  addClient(cli);
}

t_client *get_client(int fd) {
  for (t_client *it = clients; it; it = it->next) {
    if (it->fd == fd) {
      return it;
    }
  }
  return NULL;
}

// Fonction pour gérer les messages reçus des clients
int msg(int fd) {
  char prompt[100];
  char buff[4096];
  t_client *cli = get_client(fd);

  if (cli == NULL)
    return (1);
  bzero(prompt, 100);
  bzero(buff, 4096);
  if (recv(cli->fd, buff, 4096, 0) <= 0)
    return (1);
  sprintf(prompt, "client %d: ", cli->id);
  char *token = ft_strtok(buff, "\n");
  for (; token != NULL; token = ft_strtok(NULL, "\n")) {
    if (cli->new)
      send_all(prompt, fd);
    send_all(token, fd);
    cli->new = (token[(strlen(token) - 1)] == '\n');
  }
  return (0);
}

void remove_client(int fd) {
  t_client *remove = NULL;
  char prompt[100];
  bzero(prompt, 100);
  for (t_client *it = clients; it; it = it->next) {
    if (it->fd == fd) {
      remove = it;
      if (it->next)
        it->next->prev = it->prev;
      if (it->prev)
        it->prev->next = it->next;
      if (remove == clients)
        clients = clients->next;
      break;
    }
  }
  sprintf(prompt, "server: client %d just left\n", remove->id);
  send_all(prompt, fd);
  FD_CLR(fd, &pool_set);
  close(fd);
  free(remove);
}

int main(int ac, char **av) {
  struct sockaddr_in servaddr;
  clients = NULL;
  sockfd = 0;
  fd_max = 0;
  if (ac != 2)
    Wrong();
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    printf("socket creation failed...\n");
    exit(0);
  } else
    printf("Socket successfully created..\n");
  bzero(&servaddr, sizeof(servaddr));
  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(2130706433); // 127.0.0.1
  servaddr.sin_port = htons(atoi(av[1]));

  // Binding newly created socket to given IP and verification
  if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0) {
    printf("socket bind failed...\n");
    exit(0);
  } else
    printf("Socket successfully binded..\n");
  if (listen(sockfd, 10000) < 0) {
    printf("cannot listen\n");
    exit(0);
  }

  FD_ZERO(&pool_set);
  FD_ZERO(&read_set);
  FD_ZERO(&write_set);
  FD_SET(sockfd, &pool_set);
  while (1) {
    int max = get_fd_max() + 1;
    read_set = pool_set;
    write_set = pool_set;
    if (select(max, &read_set, &write_set, NULL, NULL) < 0) {
      continue;
    }
    for (int i = 0; i < max; ++i) {
      if (FD_ISSET(i, &read_set)) {
        if (i == sockfd) {
          add_client();
          break;
        } else {
          if (msg(i)) {
            remove_client(i);
          }
        }
      }
    }
  }
  return 0;
}
