#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int NB_THREADS = 2;

struct values {
  int socket;
  struct sockaddr_in sockaddr;
  socklen_t socklen;
}values;
int i = 2;

//0 : int dS, 1 : struct sockaddr_in aS, 2 : socklen_t lgA
void* sendMsg(void* val){
  struct values *param = (struct values*)val;
  do
  {
    
    int maxSize = 1024;
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

    if(sendto(param->socket, &size, sizeof(size_t), 0, (struct sockaddr *)&(param->sockaddr), param->socklen) == -1){
      perror("error sendto");
      exit(1);
    }

    if(sendto(param->socket, msg, size, 0, (struct sockaddr *)&param->sockaddr,param->socklen) == -1) {
      perror("error sendto");
      exit(1);
    }

    free(msg);
  } while (1);
}


void* receiveMsg(void* params){
  struct values *parameters = (struct values*) params;
  do
  {
    char *fin = "fin";
    size_t size;

    if (recv(parameters->socket, &size, sizeof(size_t), 0) == -1){
      perror("error recv server");
      exit(1);
    }

    char *msg = malloc(sizeof(char)*size);

    if (recv(parameters->socket, msg, sizeof(char)*size, 0) == -1){
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
  connect(dS, (struct sockaddr *) &aS, lgA);
  printf("Socket Connecté\n");

  pthread_t sendThread ;
  pthread_t recvThread;

  struct values val;
  val.socket = dS;
  val.sockaddr = aS;
  val.socklen = lgA;

  pthread_create(&sendThread, NULL, sendMsg, (void*)&val);
  pthread_create(&recvThread, NULL, receiveMsg, (void*)&val);

  pthread_join(sendThread, NULL);
  pthread_join(recvThread, NULL);

  if (close(dS) == -1)
  {
    perror("Erreur shutdown");
    exit(1);
  };

  printf("\nFin du programme\n");
}