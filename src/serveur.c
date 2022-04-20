#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "serveur.h"
#define MAX_NB_CLIENTS 10

// Parameters needed to send/receive a message
struct parametres_struct
{
  int dS;
  socklen_t lg;
  int nbClients;
  int socks[MAX_NB_CLIENTS];
  struct sockaddr_in aC;
};
struct parametres_struct connection;

struct clientParams
{
  size_t size;
  unsigned short position;
};

// Transfer Client 1's message to Client 2
void *clientManagement(void *params)
{

  struct clientParams *param = (struct clientParams *)params;

  while (1)
  {
    printf("Client : %d\n", connection.socks[param->position]);
    // Receive the message's size
    if (recv(connection.socks[param->position], &(param->size), sizeof(size_t), 0) == -1)
    {
      perror("error recv server");
      exit(1);
    }

    char *msg = malloc(sizeof(char) * (param->size));

    // Receive the message itself
    if (recv(connection.socks[param->position], msg, sizeof(char) * (param->size), 0) == -1)
    {
      perror("error recv server");
      exit(1);
    }

    // Send the message's size
    for (int i = 0; i < connection.nbClients; i++)
    {
      printf("Send to %d\n", connection.socks[i]);
      if (sendto(connection.socks[i], &(param->size), sizeof(size_t), 0, (struct sockaddr *)&(connection.aC), connection.lg) == -1)
      {
        perror("error sendto server");
        exit(1);
      }
      // Send the message itself
      if (sendto(connection.socks[i], msg, param->size, 0, (struct sockaddr *)&(connection.aC), connection.lg) == -1)
      {
        perror("error sendto server");
        exit(1);
      }
    }
    free(msg);
  }
  free(params);
  pthread_exit(0);
}

void *userLogin()
{
  pthread_t thread[MAX_NB_CLIENTS];
  do
  {
    struct clientParams* clientParams = malloc(sizeof(struct clientParams));
    connection.socks[connection.nbClients] = accept(connection.dS, (struct sockaddr *)&connection.aC, &connection.lg);
    clientParams->position = connection.nbClients;
    printf("Client added ? %d\n", pthread_create(&thread[clientParams->position], NULL, clientManagement, (void *)clientParams));
    connection.nbClients++;
  } while (connection.nbClients < MAX_NB_CLIENTS);

  for (int i = 0; i < MAX_NB_CLIENTS; i++)
  {
    pthread_join(thread[i], NULL);
  }
}

int main(int argc, char *argv[])
{
  printf("Début programme\n");

  connection.dS = socket(PF_INET, SOCK_STREAM, 0);
  printf("Socket Créé\n");

  struct sockaddr_in ad;
  ad.sin_family = AF_INET;
  ad.sin_addr.s_addr = INADDR_ANY;
  ad.sin_port = htons(atoi(argv[1]));
  bind(connection.dS, (struct sockaddr *)&ad, sizeof(ad));
  printf("Socket Nommé\n");

  listen(connection.dS, 7);
  printf("Mode écoute client\n");
  struct sockaddr_in aC;
  connection.aC = aC;
  connection.lg = sizeof(struct sockaddr_in);

  userLogin();

  // shutdown(dS2C, 2);
  // shutdown(dSC, 2);
  // shutdown(dS, 2);
  printf("Fin du programme");
}

// TODO : parametrize C1versC2 to make it more flexible and use 1 function for every client