#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

  int nbOctet = 0;
  int maxSize = 128;
  char *fin = "fin";
  char *m = malloc(sizeof(char)*maxSize);
  size_t size;
  do
  {
    printf("Message -> ");
    fgets(m, maxSize, stdin);
    size = strlen(m);

    if(sendto(dS, &size, sizeof(int), 0, (struct sockaddr *)&aS, lgA) == -1) {
      perror("error sendto");
      exit(1);
    }
    
    if(sendto(dS, m, size, 0, (struct sockaddr *)&aS, lgA) == -1) {
      perror("error sendto");
      exit(1);
    }

    printf("Envoi confirmé\n\n");
  } while (1);

  if (close(dS) == -1)
  {
    perror("Erreur shutdown");
    exit(1);
  };

  printf("\nFin du programme");
}