#include <stdio.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "serveur.h"
#include "commandes.c"
#define MAX_NB_CLIENTS 3

struct userTuple
{
  char *username;
  int socket;
};
// Parameters needed to send/receive a message
struct parametres_struct
{
  int dS;
  socklen_t lg;
  int nbClients;
  struct userTuple *socks[MAX_NB_CLIENTS];
  struct sockaddr_in aC;
};
struct parametres_struct connection;

struct clientParams
{
  size_t size;
  unsigned short position;
};

sem_t* nbPlaces;




/**
 * @brief Get the client's username
 *
 * @param params information about the client who send the message (it's location in the array of client and the message's size)
 * @return char*
 */
char *getUsername(struct clientParams *params)
{
  if (send(connection.socks[params->position]->socket, "Connection established", sizeof(char) * 23, 0) == -1)
  {
    perror("error sendto server");
    exit(1);
  }
  if (recv(connection.socks[params->position]->socket, &(params->size), sizeof(int), 0) == -1)
  {
    perror("error recv serveur");
    exit(1);
  }
  char *msg = malloc(sizeof(char) * params->size);
  if (recv(connection.socks[params->position]->socket, msg, sizeof(char) * params->size, 0) == -1)
  {
    perror("error recv serveur");
    exit(1);
  }
  char *username = malloc(sizeof(char) * (params->size - 1));
  memcpy(username, msg, strlen(msg) - 1);
  free(msg);
  return username;
}

/**
 * @brief Broadcast a message
 *
 * @param param information about the client who send the message (it's location in the array of client and the message's size)
 * @param msg the message to be send
 */
void sendMsg(struct clientParams *param, char *msg)
{
  char *username = connection.socks[param->position]->username;
  char *delimiter = " -> ";
  for (int i = 0; i < connection.nbClients; i++)
  {
    if (connection.socks[i] == NULL)
    {
      continue;
    }
    size_t fullSize = sizeof(char) * (strlen(username) + strlen(delimiter) + strlen(msg) + 1);
    char *fullMsg = malloc(fullSize);
    strcpy(fullMsg, username);
    strcat(fullMsg, delimiter);
    strcat(fullMsg, msg);

    if (send(connection.socks[i]->socket, &fullSize, sizeof(size_t), 0) == -1)
    {
      perror("error sendto server");
      exit(1);
    }

    // Send the message itself
    if (send(connection.socks[i]->socket, fullMsg, fullSize, 0) == -1)
    {
      perror("error sendto server");
      exit(1);
    }
    free(fullMsg);
  }
}

/**
 * @brief Receive a message and broadcast it to everyone
 *
 * @param param information about the client who send the message (it's location in the array of client and the message's size)
 * @return int
 */
int recvSend(struct clientParams *param)
{
  char *end = "/DC\n";
  // Receive the message's size
  if (recv(connection.socks[param->position]->socket, &(param->size), sizeof(size_t), 0) == -1)
  {
    perror("error recv server");
    exit(1);
  }

  char *msg = malloc(sizeof(char) * (param->size));

  // Receive the message itself
  if (recv(connection.socks[param->position]->socket, msg, sizeof(char) * (param->size), 0) == -1)
  {
    perror("error recv server");
    exit(1);
  }
  if (strcmp(msg, end) == 0)
  {
    printf("%s disconnect\n", connection.socks[param->position]->username);
    return 0;
  }
  if (msg[0] == '/' && strcmp(msg, fin) != 0)
    {
      // executer(msg, *(param), connection);
    }
  else{
    sendMsg(param, msg);
    }
  free(msg);
  return 1;
}

/**
 * @brief Manage the client by allowing it to send messages and to deconnect
 *
 * @param params information about the client who send the message (it's location in the array of client and the message's size)
 * @return void*
 */
void *clientManagement(void *params)
{

  struct clientParams *param = (struct clientParams *)params;

  connection.socks[param->position]->username = getUsername(param);
  printf("%s just connected !\n", connection.socks[param->position]->username);

  // Loop as long as the client doesn't send '/DC'
  while (recvSend(param))
  {
  }

  // Close the socket and free the
  shutdown(connection.socks[param->position]->socket, 2);
  connection.socks[param->position] = NULL;
  if (sem_post(nbPlaces))
  {
    perror("sem post");
      exit(1);
  }
  free(params);
  pthread_exit(0);
}

/**
 * @brief Get the first index available in the array of client
 *
 * @return int
 */
int getIndex()
{
  int i = 0;
  while (i < connection.nbClients && connection.socks[i] != NULL)
  {
    i++;
  }
  return i;
}

/**
 * @brief Manage client login by allowing it to connect and adding it to the array of client
 *
 * @return void*
 */
void *userLogin()
{
  pthread_t thread[MAX_NB_CLIENTS];
  do
  {
    if (sem_wait(nbPlaces))
    {
      perror("sem wait");
      exit(1);
    };

    struct clientParams *clientParams = malloc(sizeof(struct clientParams));
    struct userTuple *user = malloc(sizeof(struct userTuple));
    user->socket = accept(connection.dS, (struct sockaddr *)&connection.aC, &connection.lg);
    int i = getIndex();
    clientParams->position = i;
    connection.socks[i] = user;

    if (pthread_create(&thread[i], NULL, clientManagement, (void *)clientParams) == -1)
    {
      perror("error clientManagement server");
      exit(1);
    }
    if (i == connection.nbClients)
    { // Used to make sure the received message is transmitted to every client. If nbClient was decreased when a client deconnect, the last clients wouldn't receive the message.
      connection.nbClients++;
    }
  } while (1);
}

/**
 * @brief Runs the server
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
  printf("Début programme\n");

  if (sem_init(nbPlaces, 0, MAX_NB_CLIENTS))
  {
    perror("Innit Semaphore");
    exit(1);
  }

  connection.dS = socket(PF_INET, SOCK_STREAM, 0);
  if (connection.dS < 0)
  {
    perror("Couldn't connect");
    exit(1);
  }
  if (setsockopt(connection.dS, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    perror("setsockopt(SO_REUSEADDR) failed");

  printf("Socket Créé\n");

  struct sockaddr_in ad;
  ad.sin_family = AF_INET;
  ad.sin_addr.s_addr = INADDR_ANY;
  ad.sin_port = htons(atoi(argv[1]));
  if (bind(connection.dS, (struct sockaddr *)&ad, sizeof(ad)) == -1)
  {
    perror("error bind server");
    exit(1);
  };
  printf("Socket Nommé\n");

  if (listen(connection.dS, 7) == -1)
  {
    perror("error listen server");
    exit(1);
  }
  printf("Mode écoute client\n");
  struct sockaddr_in aC;
  connection.aC = aC;
  connection.lg = sizeof(struct sockaddr_in);

  userLogin(&nbPlaces);

  if (sem_destroy(nbPlaces))
  {
    perror("sem destroy");
    exit(1);
  }

  // shutdown(dS2C, 2);
  // shutdown(dSC, 2);
  // shutdown(dS, 2);
  printf("Fin du programme");
  return 0;
}

// TODO : parametrize C1versC2 to make it more flexible and use 1 function for every client