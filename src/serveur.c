#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#define NB_THREADS 2

struct parametres_struct {
  int socks [50];
  size_t size;
  socklen_t lg;
  struct sockaddr_in aC;
};

void * C1versC2(void * params){

  struct parametres_struct *param =(struct parametres_struct *)params;

  while (1){

    if (recv(param -> socks[0], &(param -> size), sizeof(size_t), 0) == -1)
    {
      perror("error recv server");
      exit(1);
    }
    char *msg = malloc(sizeof(char)*(param -> size));

    // if (strcmp(msg, "fin") == 0){
    //   shutdown(dS, 2);
    //   shutdown(dS2, 2);
    //   listen(dS, 7);
    //   listen(dS2, 7);
    // }

    if (recv(param -> socks[0], msg, sizeof(char)*(param -> size), 0) == -1){
      perror("error recv server");
      exit(1);
    }

    printf("Taille = %ld & Message = %s", param -> size, msg);
    if (sendto(param -> socks[1], &(param -> size), sizeof(size_t), 0, (struct sockaddr *)&(param -> aC), param -> lg) == -1)
    {
      perror("error sendto server");
      exit(1);
    }

    if (sendto(param -> socks[1], msg, param -> size, 0, (struct sockaddr *)&(param -> aC), param -> lg) == -1)
    {
      perror("error sendto server");
      exit(1);
    }
    free(msg);
  }
  pthread_exit(0);
}



void * C2versC1(void * params){

  struct parametres_struct *param =(struct parametres_struct *)params;

  while (1){
    if (recv(param -> socks[1], &(param -> size), sizeof(size_t), 0) == -1) {
      perror("error recv");
      exit(1);
    }
    
    char *msg = malloc(sizeof(char)*(param -> size));

    if (recv(param -> socks[1], msg, sizeof(char)*(param -> size), 0) == -1){
      perror("error recv server");
      exit(1);
    }
    if (sendto(param -> socks[0], &(param -> size), sizeof(size_t), 0, (struct sockaddr *)&(param -> aC), param -> lg) == -1)
    {
      perror("error sendto server");
      exit(1);
    }
    if (sendto(param -> socks[0], msg, param -> size, 0, (struct sockaddr *)&(param -> aC), param -> lg) == -1)
    {
      perror("error sendto server");
      exit(1);
    }
    free(msg);
  }
  pthread_exit(0);
}

int main(int argc, char *argv[]){

  printf("Début programme\n");

  pthread_t thread[NB_THREADS];

  struct parametres_struct param;

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
  param.aC = aC;
  param.lg = sizeof(struct sockaddr_in);
  param.socks[0] = accept(dS, (struct sockaddr *)&aC, &param.lg);
  printf("Client 1 Connecté\n");

  int dS2 = socket(PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in ad2;
  ad.sin_family = AF_INET;
  ad.sin_addr.s_addr = INADDR_ANY;
  ad.sin_port = htons(atoi(argv[1]));
  bind(dS2, (struct sockaddr *)&ad2, sizeof(ad2));

  listen(dS2, 7);
  printf("Mode écoute client 2\n");

  struct sockaddr_in aC2;
  socklen_t lg2 = sizeof(struct sockaddr_in);
  param.socks[1] = accept(dS, (struct sockaddr *)&aC2, &lg2);
  printf("Client 2 Connecté\n");

  pthread_create(&thread[0], NULL, C1versC2, (void *)&param);
  pthread_create(&thread[1], NULL, C2versC1, (void *)&param);

  pthread_join(thread[0], NULL);
  pthread_join(thread[1], NULL);

  // shutdown(dS2C, 2);
  //shutdown(dSC, 2);
  //shutdown(dS, 2);
  printf("Fin du programme");
}