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
// #include "commandes.c"
#define MAX_NB_CLIENTS 10

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

sem_t nbPlaces;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Private message, only the sender and the given receiver will see it /mp <nom du destinataire> <message>
 * 
 * @param user information about the sender
 * @param dest username of client to which the message is send
 * @param msg message to send
 * @return void* 
 */
void *messagePrive(struct userTuple *user, char *dest, char *msg)
{

  // faire la recherche de l'identifiant de l'utilisateur dans la future collection
  int socket = -1;
  int i = 0;
  while (socket == -1 || i < connection.nbClients)
  {
    if (connection.socks[i] != NULL && connection.socks[i]->username == dest)
    {
      socket = connection.socks[i]->socket;
    }
    i++;
  }
  printf("Socket to send = %d", socket);

  if (socket == -1)
  {
    char *msgAlert = "Cet utilisateur n'existe pas";
    size_t fullSize = sizeof(char) * (strlen(msgAlert) + 1);

    if (send(user->socket, &fullSize, sizeof(size_t), 0))
    {
      perror("error sendto server");
      exit(1);
    }
    if (send(user->socket, "Cet utilisateur n'existe pas", fullSize, 0))
    {
      perror("error sendto server");
      exit(1);
    }
  }

  else
  {
    char *delimiter = " -> ";
    size_t fullSize = sizeof(char) * (strlen(user->username) + strlen(delimiter) + strlen(msg) + 1);
    char *fullMsg = malloc(fullSize);
    strcpy(fullMsg, user->username);
    strcat(fullMsg, delimiter);
    strcat(fullMsg, msg);
    char *clients[2];

    if (send(socket, &fullSize, sizeof(size_t), 0))
    {
      perror("error sendto server");
      exit(1);
    }
    if (send(socket, fullMsg, fullSize, 0))
    {
      perror("error sendto server");
      exit(1);
    }

    // FINIR L'ENVOI AUX 2 UTILISATEURS AVEC clients[]

    free(fullMsg);
  }
}

/*
Fonctionnalité de manuel :
Ne prends pas de paramètres
Lorsqu'un utilisateur se sert de cette fonction, liste les fonctionnalités disponibles, stockées dans un fichier texte
Utilisation de la commande : /manuel
*/
void *manuelFonc()
{
  FILE *file;
  file = fopen("manuelFonct.txt", "r");
  char Buffer[128];
  while (fgets(Buffer, 128, file))
    printf("%s", Buffer);
  fclose(file);
}

/**
 * @brief Launch command execution
 *
 * @param msg the message to send
 * @param user information about the sender
 * @param nbClient number of clients
 * @return void*
 */
void *executer(char *msg, int position)
{
  // séparation et récupération de chaque élément de la commande

  char *motMsg = strtok(msg, " ");
  char listeMot[4][1024] = {""};
  strcpy(listeMot[0], motMsg);
  printf("Substring=%s\n", listeMot[0]);
  // redirection vers la commande de message privé
  if (strcmp(listeMot[0], "/mp") == 0)
  {
    if (motMsg == NULL)
    {
      return NULL;
    }
    motMsg = strtok(NULL, " ");
    strcpy(listeMot[1], motMsg);
    size_t offset = strlen(listeMot[0]) + strlen(listeMot[1]) + 2; // Drop the command name and username (witht their whitespace...)
    char *msgToSend = malloc(sizeof(char) * strlen(msg));
    // printf("Username5 : %s\n", connection.socks[position]->username);
    memcpy(msgToSend, msg + offset, strlen(msg) - offset);

    messagePrive(connection.socks[position], listeMot[1], msgToSend);
  }

  // redirection vers la commande du manuel
  if (strcmp(listeMot[0], "/help") == 0)
  {
    manuelFonc(connection.socks[position]);
  }
}

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
    if (connection.socks[i] != NULL && i != pos && strcmp(connection.socks[i]->username, username) == 0)
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
  char *username = connection.socks[position]->username;
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
 * @brief Manage the client by allowing it to send messages and to deconnect
 *
 * @param params information about the client who send the message (it's location in the array of client and the message's size)
 * @return void*
 */
void *clientManagement(void *params)
{

  int* position = (int *)params;

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
    if (msg[0] == '/' && strcmp(msg, end) != 0)
    {
      executer(msg, *position);
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
    if (sem_wait(&nbPlaces))
    {
      perror("sem wait");
      exit(1);
    };

    int* position = malloc(sizeof(int));
    struct userTuple *user = malloc(sizeof(struct userTuple));
    user->socket = accept(connection.dS, (struct sockaddr *)&connection.aC, &connection.lg);
    int i = getIndex();
    *position = i;
    //when a client connect and doesn't enter a username, if an another client connect, the server has a segmentation fault
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
  printf("Mode écoute client\n");
  struct sockaddr_in aC;
  connection.aC = aC;
  connection.lg = sizeof(struct sockaddr_in);

  signal(SIGINT, terminateEveryClient);

  userLogin(&nbPlaces);

  if (sem_destroy(&nbPlaces))
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