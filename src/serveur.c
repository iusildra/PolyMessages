#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "serveur.h"
#define MAX_NB_CLIENTS 10
#define DEFAULT_SOCKET -1

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
  char *end = "Marvin -> /DC\n"; //Used for testing. The user must be named Marvin (case sensitive :D) 


  //Send to say to the client he is connected
  char* co;
  sprintf(co, "Bonjour %d", param->position);
  printf("%s",co);
  size_t first_size = sizeof(co);
  if (send(connection.socks[param->position], &first_size, sizeof(size_t), 0) == -1){
    perror("error first sendto size server");
    exit(1);
  }

  if (send(connection.socks[param->position], co, first_size, 0) == -1){
    perror("error first sendto server");
    exit(1);
  }
  printf("%s",co);

  while (1)
  {
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
    printf("Msg -> %s", msg);
    if (strcmp(msg, end) == 0)
    {
      printf("Client %d disconnect\n", connection.socks[param->position]);
      break;
    }

    // Send the message's size
    for (int i = 0; i < connection.nbClients; i++)
    {
      if (connection.socks[i] == DEFAULT_SOCKET)
      {
        continue;
      }
      if (send(connection.socks[i], &(param->size), sizeof(size_t), 0) == -1)
      {
        perror("error sendto server");
        exit(1);
      }
      // Send the message itself
      if (send(connection.socks[i], msg, param->size, 0) == -1)
      {
        perror("error sendto server");
        exit(1);
      }
    }
    free(msg);
  }
  shutdown(connection.socks[param->position], 2);
  connection.socks[param->position] = DEFAULT_SOCKET;
  free(params);
  pthread_exit(0);
}

int getIndex()
{
  int i = 0;
  while (i < connection.nbClients && connection.socks[i] != DEFAULT_SOCKET)
  {
    i++;
  }
  return i;
}

void *userLogin()
{
  pthread_t thread[MAX_NB_CLIENTS];
  do
  {
    struct clientParams *clientParams = malloc(sizeof(struct clientParams));
    int i = getIndex();
    connection.socks[i] = accept(connection.dS, (struct sockaddr *)&connection.aC, &connection.lg);
    clientParams->position = i;
    printf("Client logs in\n");
    if (pthread_create(&thread[i], NULL, clientManagement, (void *)clientParams)==-1){
      perror("error clientManagement server");
      exit(1);
    }
    if (i == connection.nbClients)
    { // Used to make sure the received message is transmitted to every client. If nbClient was decreased when a client deconnect, the last clients wouldn't receive the message.
      connection.nbClients++;
    }
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
  if (bind(connection.dS, (struct sockaddr *)&ad, sizeof(ad))==-1) {
    perror("error bind server");
    exit(1);
  };
  printf("Socket Nommé\n");

  if (listen(connection.dS, 7)==-1){
    perror("error listen server");
    exit(1);  
  }
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