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
 * @param msg the messag writtent
 * @return int 0 if there was no commands, 1 otherwise
 */
int detectClientCommands(char* msg, int socket) {
  int recognized = 0;
  if (strcmp(msg, "/send\n") == 0)
  {
    recognized = 1;
    char *name = listFiles();
    char* path = malloc(sizeof(char)*(strlen(PATH) + strlen(name)+1));
    strcpy(path, PATH);
    strcat(path, name);
    printf("%s\n", path);
    int pid = fork();
    if (pid != 0)
    {
      //Open a new socket
      sendFile(path, name, socket);
    }
  }

  if (strcmp(msg, "/recv\n") == 0) {
    recognized = 1;
    int pid = fork();
    if (pid == 0) {
      // recvFile(socket);
    }
  }
  if (recognized != 0)
    return 1;
  else
    return 0;
}

/**
 * @brief Allows a client to send a message
 *
 * @param val informations about the socket
 * @return void*
 */
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
    if (size == 2) { continue; } //size == 2 <=> msg = "\n\0"

    if (detectClientCommands(msg, param->fileSocket) == 1) {
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

/**
 * @brief Allows a client to receive a message
 *
 * @param params informations about the socket
 * @return void*
 */
void *receiveMsg(void *params)
{
  char *end = "/DC";
  struct values *parameters = (struct values *)params;
  do
  {
    size_t size;
    if (recv(parameters->socket, &size, sizeof(size_t), 0) == -1)
    {
      perror("error recv");
      exit(1);
    }

    char *msg = malloc(sizeof(char) * size);
    if (recv(parameters->socket, msg, sizeof(char) * size, 0) == -1)
    {
      perror("error recv");
      exit(1);
    }
    if (strcmp(end, msg) == 0)
    {
      printf("Disconnection complete\n");
      exit(0);
    }

    printf("%s", msg);
    free(msg);
  } while (1);
  pthread_exit(0);
}

/**
 * @brief Allows a client to send it's username on login
 *
 * @param params informations about the socket
 * @return void*
 */
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

  printf("Enter your username (length must be <= %d): ", usernameMaxSize);
  msg = malloc(sizeof(char) * usernameMaxSize);
  do {
    fgets(msg, usernameMaxSize, stdin);
  } while (strlen(msg) == 1); //strlen(msg==1) <=> msg = "\n\0"

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
 * @brief Main function of the client
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
  printf("Début programme\n");
  // Socket initialization
  int dS = socket(PF_INET, SOCK_STREAM, 0);
  if (dS == -1)
  {
    perror("NO CONNECTION TO SERVER");
    exit(1);
  }
  printf("Socket Créé\n");
  int dSFile = socket(PF_INET, SOCK_STREAM, 0);
  if (dSFile == -1)
  {
    perror("NO CONNECTION TO SERVER");
    exit(1);
  }
  printf("Socket Créé\n");

  
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

  struct sockaddr_in aSFile;
  aSFile.sin_family = AF_INET;
  inet_pton(AF_INET, argv[1], &(aSFile.sin_addr));
  aSFile.sin_port = htons(atoi(argv[3]));
  socklen_t lgAFile = sizeof(struct sockaddr_in);
  if (connect(dSFile, (struct sockaddr *)&aSFile, lgAFile) == -1)
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
  values.fileSocket = dSFile;

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

  printf("\nFin du programme\n");
  return 0;
}