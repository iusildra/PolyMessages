#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "client.h"
#include "commandes.h"
#define usernameMaxLength 20

int NB_THREADS = 2;

struct values
{
  int socket;
  struct sockaddr_in sockaddr;
  socklen_t socklen;
  char *username;
} values;
int i = 2;

// 0 : int dS, 1 : struct sockaddr_in aS, 2 : socklen_t lgA
// sendThread : allows a client to send a message to the server
void *sendMsg(void *val)
{
  struct values *param = (struct values *)val;
  do
  {
    size_t size;
    char *fin = "fin";
    int maxSize = 1024;
    char msg[maxSize];
    char *fullMsg = malloc(sizeof(char) * (maxSize + strlen(param->username)));

    fgets(msg, maxSize, stdin);
    if (msg[0]=='/'){
      executer(msg,param->username);
    }
    strcpy(fullMsg, param->username);
    strcat(fullMsg, " -> ");
    strcat(fullMsg, msg);
    size = strlen(fullMsg) + 1;

    if (strcmp(msg, fin) == 0)
    {
      // free(msg);dS, aS, lgA)
      // break;
    }

    // Send the size of the message
    if (send(param->socket, &size, sizeof(size_t), 0) == -1)
    {
      perror("error sendto");
      exit(1);
    }

    // Send the message itself
    if (send(param->socket, fullMsg, size, 0) == -1)
    {
      perror("error sendto");
      exit(1);
    }

    free(fullMsg);
  } while (1);
}

// recvThread : allows a client to receive a message
void *receiveMsg(void *params)
{
  struct values *parameters = (struct values *)params;
  do
  {
    char *fin = "fin";
    size_t size;

    // Receive the size of the message
    if (recv(parameters->socket, &size, sizeof(size_t), 0) == -1)
    {
      perror("error recv server");
      exit(1);
    }

    char *msg = malloc(sizeof(char) * size);

    // Receive the message itself
    if (recv(parameters->socket, msg, sizeof(char) * size, 0) == -1)
    {
      perror("error recv");
      exit(1);
    }
    printf("%s", msg);

    if (strcmp(msg, fin) == 0)
    {
      // free(msg);
      // break;
    }

    free(msg);
  } while (1);
  pthread_exit(&i);
}

int main(int argc, char *argv[])
{
  printf("Début programme\n");
  if (strlen(argv[3]) >= usernameMaxLength)
  {
    perror("Username too long (must be < 20)");
    exit(1);
  }

  // Socket initialization
  int dS = socket(PF_INET, SOCK_STREAM, 0);
  if (dS == -1)
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
  connect(dS, (struct sockaddr *)&aS, lgA);
  printf("Socket Connecté\n");

  pthread_t sendThread;
  pthread_t recvThread;

  // Values to pass to the sendThread
  struct values val;
  val.socket = dS;
  val.sockaddr = aS;
  val.socklen = lgA;
  val.username = argv[3];

  // Create thread
  pthread_create(&sendThread, NULL, sendMsg, (void *)&val);
  pthread_create(&recvThread, NULL, receiveMsg, (void *)&val);

  // Wait for threads to finish
  pthread_join(sendThread, NULL);
  pthread_join(recvThread, NULL);

  if (close(dS) == -1)
  {
    perror("Erreur shutdown");
    exit(1);
  };

  printf("\nFin du programme\n");
}