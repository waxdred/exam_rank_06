#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct s_client {
  int fd, id, new;
  struct s_client *next;
  struct s_client *prev;
} t_client;

int sockfd;
ssize_t fd_max;
fd_set pool_set;
fd_set read_set;
fd_set write_set;

t_client *clients;
char prompt[100];
char tok[1024];

void freelst(t_client **cli) {
  if (*cli == NULL)
    return;
  freelst(&(*cli)->next);
  free(*cli);
  *cli = NULL;
}

void Wrong() {
  char err[] = "Wrong number of arguments\n";
  write(2, err, strlen(err));
  exit(1);
}

void Fatal() {
  char err[] = "Fatal error\n";
  write(2, err, strlen(err));
  freelst(&clients);
  exit(1);
}

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

void send_all(char *s, int fd) {
  int len = strlen(s);
  for (t_client *it = clients; it; it = it->next) {
    if (fd != it->fd && FD_ISSET(it->fd, &write_set)) {
      if (send(it->fd, s, len, 0) < 0)
        Fatal();
    }
  }
}

t_client *get_client(int fd) {
  for (t_client *it = clients; it; it = it->next) {
    if (it->fd == fd)
      return it;
  }
  return (NULL);
}

void addClient(t_client *cli) {
  if (clients == NULL) {
    clients = cli;
    return;
  }
  for (t_client *it = clients; it; it = it->next) {
    if (it->next == NULL) {
      it->next = cli;
      cli->prev = it;
      break;
    }
  }
}

void remove_client(int fd) {
  t_client *remove = NULL;

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
    }
  }
  sprintf(prompt, "server: client %d just left\n", remove->id);
  send_all(prompt, fd);
  FD_CLR(fd, &pool_set);
  close(fd);
  free(remove);
}

void add_client() {
  t_client *cli = malloc(sizeof(t_client));

  if (cli == NULL)
    Fatal();
  bzero(prompt, 100);
  if ((cli->fd = accept(sockfd, NULL, NULL)) < 0)
    Fatal();
  cli->id = fd_max;
  cli->new = 1;
  cli->next = NULL;
  cli->prev = NULL;
  fd_max++;
  FD_SET(cli->fd, &pool_set);
  sprintf(prompt, "server: client %d just arrived\n", cli->id);
  send_all(prompt, cli->fd);
  addClient(cli);
}

char *ft_strtok(char *s, int c) {
  static char *next;
  int i = 0;

  bzero(tok, 1024);
  if (s != NULL)
    next = s;
  if (*next == '\0' || next == NULL)
    return NULL;
  while (*next != '\0' && *next != c) {
    tok[i++] = *next;
    next++;
  }
  if (*next != '\0') {
    tok[i++] = *next;
    *next++ = '\0';
  }
  return tok;
}

int msg(int fd) {
  char buff[1048];
  t_client *cli = get_client(fd);

  bzero(prompt, 100);
  bzero(buff, 1048);
  if (cli == NULL)
    return (1);
  if (recv(fd, buff, 200, 0) <= 0)
    return (1);
  sprintf(prompt, "client %d: ", cli->id);
  char *token = ft_strtok(buff, '\n');
  for (; token != NULL; token = ft_strtok(NULL, '\n')) {
    if (cli->new)
      send_all(prompt, fd);
    send_all(token, fd);
    cli->new = (token[(strlen(token) - 1)] == '\n');
  }
  return 0;
}

int main(int ac, char **av) {
  struct sockaddr_in servaddr;
  clients = NULL;
  sockfd = 0;
  fd_max = 0;
  if (ac != 2)
    Wrong();
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
    exit(0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(2130706433); // 127.0.0.1
  servaddr.sin_port = htons(atoi(av[1]));

  if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
    exit(0);
  if (listen(sockfd, 10000) < 0) {
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
    if (max > 6)
      Fatal();
    if (select(max, &read_set, &write_set, NULL, NULL) < 0)
      continue;
    for (int i = 0; i < max; ++i) {
      if (FD_ISSET(i, &read_set)) {
        if (i == sockfd) {
          add_client();
          break;
        } else {
          if (msg(i))
            remove_client(i);
        }
      }
    }
  }
  return (0);
}
