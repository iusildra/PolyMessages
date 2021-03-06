#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "clientCommands.h"
#include "client.h"
#define PATH "../files/"
int NB_THREADS = 2;

/**
 * @brief Detect if the user entered a client side command
 *
 * @param msg the messag written
 * @return int 0 if there was no commands, 1 otherwise
 */
int detectClientCommands(char *msg, char* ip, int port, int sock)
{
  int recognized = 0;
  if ((strcmp(msg, "/send\n")) == 0)
  {
    recognized = 1;
    char *name = listFiles();
    char *path = malloc(sizeof(char) * (strlen(PATH) + strlen(name) + 1));
    strcpy(path, PATH);
    strcat(path, name);
    int pid = fork();
    if (pid == 0)
    {
      // Open a new socket
      int dS = socket(PF_INET, SOCK_STREAM, 0);
      if (dS == -1)
      {
        perror("NO CONNECTION TO SERVER");
        exit(1);
      }
      struct sockaddr_in aS;
      aS.sin_family = AF_INET;
      inet_pton(AF_INET,ip, &(aS.sin_addr));
      aS.sin_port = htons(port);
      socklen_t lgA = sizeof(struct sockaddr_in);
      if (connect(dS, (struct sockaddr *)&aS, lgA) == -1)
      {
        perror("error connect server");
        exit(1);
      }
      sendFile(path, name, dS);
      exit(0);
    }
  }

  if ((strcmp(msg, "/recv\n") == 0)) {
    recognized = 1;
    int dS = socket(PF_INET, SOCK_STREAM, 0);
    if (dS == -1)
    {
      perror("NO CONNECTION TO SERVER");
      exit(1);
    }
    struct sockaddr_in aS;
    aS.sin_family = AF_INET;
    inet_pton(AF_INET,ip, &(aS.sin_addr));
    aS.sin_port = htons(port);
    socklen_t lgA = sizeof(struct sockaddr_in);
    if (connect(dS, (struct sockaddr *)&aS, lgA) == -1)
    {
      perror("error connect server");
      exit(1);
    }
    char *name = listServFiles(dS);
    char *nameToSend = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(nameToSend, name);

    char *path = malloc(sizeof(char) * (strlen(PATH) + strlen(name) + 1));
    strcpy(path, PATH);
    strcat(path, name);
    printf("\033[34m%s\n\033[0m", path);
    int pid = fork();
    if (pid == 0)
    {
      recvFile(path, nameToSend, dS);
      free(nameToSend);
      exit(0);
    }
  }

  // redirection vers la commande de cr??ation de salon
  if ((strcmp(msg, "/creer\n")) == 0){
    recognized = 1;
    if (values.idSalon == -1){
      creerSalon(sock);
    }else {
      printf("\033[34mVous ??tes d??j?? dans un channel\033[0m\n");
    }
  }


  if (strcmp(msg, "/files\n") == 0)
  {
    recognized = 1;
    int dS = socket(PF_INET, SOCK_STREAM, 0);
      if (dS == -1)
      {
        perror("NO CONNECTION TO SERVER");
        exit(1);
      }
      struct sockaddr_in aS;
      aS.sin_family = AF_INET;
      inet_pton(AF_INET,ip, &(aS.sin_addr));
      aS.sin_port = htons(port);
      socklen_t lgA = sizeof(struct sockaddr_in);
      if (connect(dS, (struct sockaddr *)&aS, lgA) == -1)
      {
        perror("error connect server");
        exit(1);
      }
    listServFiles(dS);
  }

  if (strcmp(msg, "/connect\n") == 0) {
    recognized = 1;
    connectToRoom(sock);
  }
  if (strcmp(msg, "/delete\n") == 0) {
    recognized = 1;
    deleteRoom(sock);
  }
  if (recognized != 0)
    return 1;
  else
    return 0;
}

void *sendMsg(void *val)
{
  // 0 : int dS, 1 : struct sockaddr_in aS, 2 : socklen_t lgA
  struct values *param = (struct values *)val;
  char *fin = "/DC\n";
  do
  {
    size_t size;
    int maxSize = 1024;
    char *msg = malloc(sizeof(char) * maxSize);

    fgets(msg, maxSize, stdin);
    size = strlen(msg) + 1;
    if (size == 2)
    {
      continue;
    } // size == 2 <=> msg = "\n\0"

    if (detectClientCommands(msg, param->ip, param->filePort, param->socket) == 1) {
      free(msg);
      continue;
    }

    // Send the size of the message
    if (send(param->socket, &size, sizeof(size_t), 0) == -1)
    {
      perror("error sendto");
      exit(1);
    }

    // Send the message itself
    if (send(param->socket, msg, size, 0) == -1)
    {
      perror("error sendto");
      exit(1);
    }

    if (strcmp(msg, fin) == 0)
    {
      free(msg);
      break;
    }

    free(msg);
  } while (1);
}

void *receiveMsg(void *params)
{
  char *end = "/DC";
  struct values *parameters = (struct values *)params;
  do
  {
    size_t size;
    if (recv(parameters->socket, &size, sizeof(size_t), 0) == -1)
    {
      perror("error recv size");
      exit(1);
    }

    char *msg = malloc(sizeof(char) * size);
    if (recv(parameters->socket, msg, sizeof(char) * size, 0) == -1)
    {
      perror("error recv msg");
      exit(1);
    }
    if (strcmp(end, msg) == 0)
    {
      printf("\033[34mDisconnection complete\033[0m\n");
      exit(0);
    }

    printf("%s", msg);
    free(msg);
  } while (1);
  pthread_exit(0);
}

void sendUsername(struct values *params)
{
  int usernameMaxSize = 12;

  char *msg = malloc(sizeof(char) * 23);

  // Receive the message itself
  if (recv(params->socket, msg, sizeof(char) * 23, 0) == -1)
  {
    perror("error recv confirm server");
    exit(1);
  }

  printf("%s\n", msg);
  free(msg);

  printf("\033[34mEnter your username (length must be <= %d: \033[0m", usernameMaxSize-2);
  msg = malloc(sizeof(char) * usernameMaxSize);
  do
  {
    fgets(msg, usernameMaxSize, stdin);
  } while (strlen(msg) == 1); // strlen(msg==1) <=> msg = "\n\0"

  // Send the size of the message
  if (send(params->socket, &usernameMaxSize, sizeof(int), 0) == -1)
  {
    perror("error send username");
    exit(1);
  }

  // Send the message itself
  if (send(params->socket, msg, sizeof(char) * usernameMaxSize, 0) == -1)
  {
    perror("error send username");
    exit(1);
  }

  free(msg);
}

void terminateClient(int sig)
{
  // Send the size of the message
  char *end = "/DC\n";
  size_t size = sizeof(char) * (strlen(end) + 1);
  if (send(values.socket, &size, sizeof(size_t), 0) == -1)
  {
    perror("error send username");
    exit(1);
  }

  // Send the message itself
  if (send(values.socket, end, size, 0) == -1)
  {
    perror("error send username");
    exit(1);
  }
  exit(0);
}

/**
 * @brief Main function
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    printf("Nombre d'arguments ??rron?? ! ./client [addresse IP serveur] [port1] [port2] \n");
    exit(1);
  }

  printf("\033[34mD??but programme\033[0m\n");
  // Socket initialization
  int dS = socket(PF_INET, SOCK_STREAM, 0);
  if (dS == -1)
  {
    perror("NO CONNECTION TO SERVER");
    exit(1);
  }
  printf("\033[34mFile Socket Cr????\033[0m\n");

  struct sockaddr_in aS;
  aS.sin_family = AF_INET;
  inet_pton(AF_INET, argv[1], &(aS.sin_addr));
  aS.sin_port = htons(atoi(argv[2]));
  socklen_t lgA = sizeof(struct sockaddr_in);
  if (connect(dS, (struct sockaddr *)&aS, lgA) == -1)
  {
    perror("error connect server");
    exit(1);
  }

  pthread_t sendThread;
  pthread_t recvThread;

  // Values to pass to the sendThread
  values.socket = dS;
  values.ip = argv[1];
  values.port = argv[2];
  values.sockaddr = aS;
  values.socklen = lgA;
  values.idSalon = -1;
  values.filePort = atoi(argv[3]);

  sendUsername(&values);

  signal(SIGINT, terminateClient);
  // Create thread
  pthread_create(&sendThread, NULL, sendMsg, (void *)&values);
  pthread_create(&recvThread, NULL, receiveMsg, (void *)&values);

  // Wait for threads to finish
  pthread_join(recvThread, NULL);
  pthread_join(sendThread, NULL);

  if (close(dS) == -1)
  {
    perror("Erreur shutdown");
    exit(1);
  };

  printf("\n\033[34mFin du programme\033[0m\n");
  return 0;
}