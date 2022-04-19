#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#define MAX_NB_CLIENTS 2

// Parameters needed to send/receive a message
struct params
{
  struct parametres_struct
  {
    int socks[MAX_NB_CLIENTS];
    int nbClients;
    socklen_t lg;
    struct sockaddr_in aC;
  } connection;
  size_t size;
};

// Transfer Client 1's message to Client 2
void *C1versC2(void *params)
{

  struct params *param = (struct params *)params;

  while (1)
  {
    // Receive the message's size
    if (recv(param->connection.socks[0], &(param->size), sizeof(size_t), 0) == -1)
    {
      perror("error recv server");
      exit(1);
    }
    char *msg = malloc(sizeof(char) * (param->size));

    // if (strcmp(msg, "fin") == 0){
    //   shutdown(dS, 2);
    //   shutdown(dS2, 2);
    //   listen(dS, 7);
    //   listen(dS2, 7);
    // }
    // Receive the message itself
    if (recv(param->connection.socks[0], msg, sizeof(char) * (param->size), 0) == -1)
    {
      perror("error recv server");
      exit(1);
    }

    // Send the message's size
    if (sendto(param->connection.socks[1], &(param->size), sizeof(size_t), 0, (struct sockaddr *)&(param->connection.aC), param->connection.lg) == -1)
    {
      perror("error sendto server");
      exit(1);
    }
    // Send the message itself
    if (sendto(param->connection.socks[1], msg, param->size, 0, (struct sockaddr *)&(param->connection.aC), param->connection.lg) == -1)
    {
      perror("error sendto server");
      exit(1);
    }
    free(msg);
  }
  pthread_exit(0);
}

// Transfer Client 2's message to Client 1
void *C2versC1(void *params)
{

  struct params *param = (struct params *)params;

  while (1)
  {
    // Receive the message's size
    if (recv(param->connection.socks[1], &(param->size), sizeof(size_t), 0) == -1)
    {
      perror("error recv");
      exit(1);
    }

    char *msg = malloc(sizeof(char) * (param->size));
    // Receive the message itself
    if (recv(param->connection.socks[1], msg, sizeof(char) * (param->size), 0) == -1)
    {
      perror("error recv server");
      exit(1);
    }

    // Send the message's size
    if (sendto(param->connection.socks[0], &(param->size), sizeof(size_t), 0, (struct sockaddr *)&(param->connection.aC), param->connection.lg) == -1)
    {
      perror("error sendto server");
      exit(1);
    }
    // Send the message itself
    if (sendto(param->connection.socks[0], msg, param->size, 0, (struct sockaddr *)&(param->connection.aC), param->connection.lg) == -1)
    {
      perror("error sendto server");
      exit(1);
    }
    free(msg);
  }
  pthread_exit(0);
}

int main(int argc, char *argv[])
{

  printf("Début programme\n");
  pthread_t thread[MAX_NB_CLIENTS];

  struct params param;

  int dS = socket(PF_INET, SOCK_STREAM, 0);
  printf("Socket Créé\n");

  struct sockaddr_in ad;
  ad.sin_family = AF_INET;
  ad.sin_addr.s_addr = INADDR_ANY;
  ad.sin_port = htons(atoi(argv[1]));
  bind(dS, (struct sockaddr *)&ad, sizeof(ad));
  printf("Socket Nommé\n");

  listen(dS, 7);
  printf("Mode écoute client 1\n");

  struct sockaddr_in aC;
  param.connection.aC = aC;
  param.connection.lg = sizeof(struct sockaddr_in);
  param.connection.socks[0] = accept(dS, (struct sockaddr *)&param.connection.aC, &param.connection.lg);
  printf("Client 1 Connecté\n");

  param.connection.socks[1] = accept(dS, (struct sockaddr *)&param.connection.aC, &param.connection.lg);
  printf("Client 2 Connecté\n");

  pthread_create(&thread[0], NULL, C1versC2, (void *)&param);
  pthread_create(&thread[1], NULL, C2versC1, (void *)&param);

  pthread_join(thread[0], NULL);
  pthread_join(thread[1], NULL);

  // shutdown(dS2C, 2);
  // shutdown(dSC, 2);
  // shutdown(dS, 2);
  printf("Fin du programme");
}

// TODO : parametrize C1versC2 to make it more flexible and use 1 function for every client