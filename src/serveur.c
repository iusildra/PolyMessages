#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "serveur.h"
#include "commandes.h"

#define MAX_NB_SALONS 10
struct salon_struct salons[MAX_NB_SALONS];

struct params
{
  pthread_t threads[MAX_NB_CLIENTS];
  int filesSocket[MAX_NB_CLIENTS];
  pthread_mutex_t mutex;
  int nbFileClient;
} fileParams;

void sendDisconnection(struct userTuple *user)
{
  char *end = "/DC";
  size_t size = sizeof(char) * (strlen(end) + 1);
  if (send(user->socket, &size, sizeof(size_t), 0) == -1)
  {
    perror("error sendto server");
    exit(1);
  }

  // Send the message itself
  if (send(user->socket, end, size, 0) == -1)
  {
    perror("error sendto server");
    exit(1);
  }
  printf("%s disconnected\n", user->username);
}

/**
 * @brief Verify that the given username doesn't already exists
 *
 * @param username the username to check
 * @return int 0 if the name already exist, 1 otherwise
 */
int checkUsername(char *username, int pos)
{
  int i = 0;
  while (i < connection.nbClients)
  {
    if (connection.socks[i] != NULL && connection.socks[i]->username != NULL && i != pos && strcmp(connection.socks[i]->username, username) == 0)
    {
      return 0;
    }
    i++;
  }
  return 1;
}

/**
 * @brief Get the client's username
 *
 * @param params information about the client who send the message (it's location in the array of client and the message's size)
 * @return char*
 */
char *getUsername(int position)
{
  size_t size;
  if (send(connection.socks[position]->socket, "Connection established", sizeof(char) * 23, 0) == -1)
  {
    perror("error sendto server");
    exit(1);
  }
  if (recv(connection.socks[position]->socket, &size, sizeof(int), 0) == -1)
  {
    perror("error recv serveur");
    exit(1);
  }
  char *msg = malloc(sizeof(char) * size);
  if (recv(connection.socks[position]->socket, msg, sizeof(char) * size, 0) == -1)
  {
    perror("error recv serveur");
    exit(1);
  }
  char *username = malloc(sizeof(char) * (size - 1));
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
void sendMsg(int position, char *msg)
{

  int idSalon = connection.socks[position]->idsalon;
  char *salon;
  char *nameSalon;
  if (idSalon != -1)
  {
    nameSalon = malloc(sizeof(char) * (strlen(salons[idSalon].name)));
    strcpy(nameSalon, salons[idSalon].name);
  }
  else
  {
    nameSalon = malloc(sizeof(char) * (strlen("\033[1;31mGénéral\033[0m") + 1));
    strcpy(nameSalon, "\033[1;31mGénéral\033[0m");
  }
  salon = malloc(sizeof(char) * (strlen(nameSalon) + 2));
  strcpy(salon, "[");
  strcat(salon, nameSalon);
  strcat(salon, "]");

  char *username = connection.socks[position]->username;
  char *delimiter = " -> ";
  for (int i = 0; i < connection.nbClients; i++)
  {
    if (connection.socks[i] == NULL || (connection.socks[i]->idsalon != idSalon))
    {
      continue;
    }
    if (connection.socks[i]->idsalon != connection.socks[position]->idsalon)
    {
      continue;
    }
    size_t fullSize = sizeof(char) * (strlen(salon) + strlen(username) + strlen(delimiter) + strlen(msg) + 1);
    char *fullMsg = malloc(fullSize);
    strcpy(fullMsg, salon);
    strcat(fullMsg, username);
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
 * @brief Manage the client by allowing it to send messages and to deconnect
 *
 * @param params information about the client who send the message (it's location in the array of client and the message's size)
 * @return void*
 */
void *clientManagement(void *params)
{

  int *position = (int *)params;

  connection.socks[*position]->username = getUsername(*position);
  if (checkUsername(connection.socks[*position]->username, *position) == 0)
  {
    printf("Cannot connect, this username already exists !\n");
    sendDisconnection(connection.socks[*position]);
    pthread_mutex_lock(&mutex);
    shutdown(connection.socks[*position]->socket, 2);
    connection.socks[*position] = NULL;
    pthread_mutex_unlock(&mutex);
    if (sem_post(&nbPlaces))
    {
      perror("sem post");
      exit(1);
    }
    free(params);
    pthread_exit(0);
  }
  printf("%s just connected !\n", connection.socks[*position]->username);
  connection.socks[*position]->idsalon = -1;

  // Loop as long as the client doesn't send '/DC'
  while (1)
  {
    char *end = "/DC\n";
    size_t size;
    // Receive the message's size
    if (recv(connection.socks[*position]->socket, &size, sizeof(size_t), 0) == -1)
    {
      perror("error recv server");
      exit(1);
    }

    char *msg = malloc(sizeof(char) * size);

    // Receive the message itself
    if (recv(connection.socks[*position]->socket, msg, sizeof(char) * size, 0) == -1)
    {
      perror("error recv server");
      exit(1);
    }
    if (strcmp(msg, end) == 0)
    {
      sendDisconnection(connection.socks[*position]);
      break;
    }
    if ((msg[0] == '/' || msg[0] == '@') && strcmp(msg, end) != 0)
    {
      executer(connection.socks, connection.nbClients, msg, *position, salons, MAX_NB_SALONS);
    }
    else
    {
      pthread_mutex_lock(&mutex);
      sendMsg(*position, msg);
      pthread_mutex_unlock(&mutex);
    }
    free(msg);
  }

  printf("End of transmission\n");
  // Close the socket and free the
  shutdown(connection.socks[*position]->socket, 2);
  connection.socks[*position] = NULL;
  if (sem_post(&nbPlaces))
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
  while (i < MAX_NB_CLIENTS && connection.socks[i] != NULL)
  {
    i++;
  }
  return i;
}

/**
 * @brief Get the File Index object
 *
 * @return int
 */
int getFileIndex()
{
  int i = 0;
  while (i < MAX_NB_CLIENTS && fileParams.filesSocket[i] != -1)
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
    if (sem_wait(&nbPlaces))
    {
      perror("sem wait");
      exit(1);
    };

    int *position = malloc(sizeof(int));
    struct userTuple *user = malloc(sizeof(struct userTuple));
    user->socket = accept(connection.dS, (struct sockaddr *)&connection.aC, &connection.lg);
    int i = getIndex();
    *position = i;
    // when a client connect and doesn't enter a username, if an another client connect, the server has a segmentation fault
    pthread_mutex_lock(&mutex);
    connection.socks[i] = user;
    pthread_mutex_unlock(&mutex);

    if (pthread_create(&thread[i], NULL, clientManagement, (void *)position) == -1)
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
 * @brief
 *
 * @param params
 * @return void*
 */
void *fileManagement(void *params)
{
  int *pos = (int *)params;
  char *command = malloc(sizeof(char) * 6);
  printf("file socket : %d\n", fileParams.filesSocket[*pos]);
  if (recv(fileParams.filesSocket[*pos], command, sizeof(char) * 6, 0) == -1)
  {
    perror("error recv file server");
    exit(1);
  }

  printf("Command is %s\n", command);

  if (strcmp(command, "@send") == 0)
  {
    size_t size;
    if (recv(fileParams.filesSocket[*pos], &size, sizeof(size_t), 0) == -1)
    {
      perror("error recv file server");
      exit(1);
    }
    char *name = malloc(size);
    if (recv(fileParams.filesSocket[*pos], name, size, 0) == -1)
    {
      perror("error recv file server");
      exit(1);
    }
    recvFile(fileParams.filesSocket[*pos], name);
  }
  else if (strcmp(command, "@recv") == 0)
  { // reception de fichier
    size_t size;
    ListeFichier(fileParams.filesSocket[*pos]);

    if (recv(fileParams.filesSocket[*pos], &size, sizeof(size_t), 0) == -1)
    {
      perror("error send file server");
      exit(1);
    }
    char *name = malloc(size);
    if (recv(fileParams.filesSocket[*pos], name, size, 0) == -1)
    {
      perror("error send file server");
      exit(1);
    }
    printf("Name is %s, size is %ld", name, size);
    sendFile(fileParams.filesSocket[*pos], name);
  }
  shutdown(fileParams.filesSocket[*pos], 2);
  fileParams.filesSocket[*pos] == -1;
  pthread_exit(0);
}

/**
 * @brief Manage client's requests for files
 *
 * @param dS Dedicated socket
 * @param acFiles sockaddr
 * @return void*
 */
void *fileClientLogin(int dS, struct sockaddr_in acFiles)
{
  for (int k = 0; k < MAX_NB_CLIENTS; k++)
  {
    fileParams.filesSocket[k] = -1;
  }
  socklen_t lg = sizeof(struct sockaddr_in);
  fileParams.nbFileClient = 0;
  do
  {
    int *pos = malloc(sizeof(int));
    int i = getFileIndex();
    // pthread_mutex_lock(&mutex);
    fileParams.filesSocket[i] = accept(dS, (struct sockaddr *)&acFiles, &lg);
    *pos = i;
    // pthread_mutex_unlock(&mutex);

    if (pthread_create(&fileParams.threads[i], NULL, fileManagement, (void *)pos) == -1)
    {
      perror("error fileManagement server");
      exit(1);
    }

    if (i == fileParams.nbFileClient)
    {
      fileParams.nbFileClient++;
    }
  } while (1);
}

void terminateEveryClient(int n)
{
  int i = 0;
  while (i < connection.nbClients)
  {
    struct userTuple *user = connection.socks[i];
    if (user != NULL)
    {
      sendDisconnection(user);
      i++;
    }
  }
  exit(0);
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

  if (argc != 3)
  {
    printf("Nombre d'arguments érroné ! ./serveur [port1] [port2] \n");
    exit(1);
  }

  printf("Début programme\n");
  if (sem_init(&nbPlaces, 0, MAX_NB_CLIENTS))
  {
    perror("Innit Semaphore");
    exit(1);
  }
  printf("Sémaphore crée\n");

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
  printf("Écoute client\n");

  struct sockaddr_in aC;
  connection.aC = aC;
  connection.lg = sizeof(struct sockaddr_in);

  int pid = fork();

  if (pid == 0)
  {
    signal(SIGINT, terminateEveryClient);
    userLogin(&nbPlaces);

    if (sem_destroy(&nbPlaces))
    {
      perror("sem destroy");
      exit(1);
    }
  }
  else
  {
    int dSFiles = socket(PF_INET, SOCK_STREAM, 0);
    if (dSFiles < 0)
    {
      perror("Couldn't connect");
      exit(1);
    }
    if (setsockopt(connection.dS, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
      perror("setsockopt(SO_REUSEADDR) failed");

    printf("Socket fichiers Créé\n");

    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons(atoi(argv[2]));
    if (bind(dSFiles, (struct sockaddr *)&ad, sizeof(ad)) == -1)
    {
      perror("error bind server");
      exit(1);
    };
    printf("Socket Nommé\n");

    if (listen(dSFiles, 7) == -1)
    {
      perror("error listen server");
      exit(1);
    }
    printf("Écoute fichiers\n");

    struct sockaddr_in aCFiles;
    fileClientLogin(dSFiles, aCFiles);
  }

  printf("\nFin du programme\n");
  return 0;
}