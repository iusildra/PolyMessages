#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int i = 2;

//0 : int dS, 1 : struct sockaddr_in aS, 2 : socklen_t lgA
void* sendMsg(void* values){
  do
  {
    int maxSize = 128;
    char *fin = "fin";
    size_t size;

    char *msg = malloc(sizeof(char)*maxSize);
    printf("Message -> ");
    fgets(msg, maxSize, stdin);
    size = strlen(msg)+1;

    if (strcmp(msg, fin) == 0)
    {
      // free(msg);dS, aS, lgA)
      // break;
    }

    if(sendto((int)values[0], &size, sizeof(size_t), 0, (struct sockaddr *)&values[1], (socklen_t)values[2]) == -1) {
      perror("error sendto");
      exit(1);
    }

    if(sendto((int)values[0], msg, size, 0, (struct sockaddr *)&values[1],(socklen_t)values[2]) == -1) {
      perror("error sendto");
      exit(1);
    }

    free(msg);
  } while (1);
}


void* receiveMsg(void* dS){
  do
  {
    char *fin = "fin";
    size_t size;

    if (recv(dS, &size, sizeof(size_t), 0) == -1){
      perror("error recv server");
      exit(1);
    }

    char *msg = malloc(sizeof(char)*size);

    if (recv(dS, msg, sizeof(char)*size, 0) == -1){
      perror("error recv");
      exit(1);
    }
    printf("Client 2 -> %s", msg);

    if (strcmp(msg, fin) == 0)
    {
      // free(msg);
      // break;
    }

    free(msg);
  } while (1);
  pthread_exit(&i);
}

int main(int argc, char *argv[]) {
  printf("Début programme\n");
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
  connect(dS, (struct sockaddr *) &aS, lgA) ;
  printf("Socket Connecté\n");

  pthread_t idSend;
  void* values = [dS, aS, lgA] ;
  pthread_create(&idSend, NULL, sendMsg, values);

  pthread_t idRcv;
  pthread_create(&idRcv, NULL, receiveMsg, &dS);

  if (close(dS) == -1)
  {
    perror("Erreur shutdown");
    exit(1);
  };

  printf("\nFin du programme");
}