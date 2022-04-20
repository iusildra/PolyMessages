#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "commandes.h"
#define MAX_NB_CLIENTS 2

/* paramètres nécessaires à l'utilisation des fonctions */

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

/*
Fonction d'éxécution qui lance l'éxécution de la bonne fonctionnalité
*/
void* executer(char* msg, String user){
  char* motMsg = strtok(msg, " ");
  char listeMot[4][1024] = {""}
  strcpy(listeMot[0], motMsg)
  int i = 1;
  while (motMsg != NULL){
    motMsg = strtok(NULL, " ");
    strcpy(listeMot[i], motMsg);
    i = i + 1;
  }
  if (listeMot[0]=="/mp"){

    //faire la recherche de l'identifiant de l'utilisateur dans la future collection

    messagePrive(/*identifiant*/,atoi(listeMot[1]),listeMot[2])
  }
}

/* 
Fonctionnalité de message privé
Prends en paramètre l'utilisateur à qui envoyer le mp ainsi que le message à envoyer
Envoie un message que seule la personne en paramètre et la personne qui a envoyé le message peut voir
Utilisation de la commande : /mp *nom d'utilisateur* *message*
*/
void* messagePrive(int expe, int dest, char* msg){
    int numdest;
    int i;
    for (i = 0; i < MAX_NB_CLIENTS; i++){
        if (param->connection.socks[i]==dest){
            numdest = i;
        }
    }
    if (sendto(param->connection.socks[numdest], msg, param->size, 0, (struct sockaddr *)&(param->connection.aC), param->connection.lg) == -1)
    {
      perror("error sendto server");
      exit(1);
    }
    free(msg);
}

/* 
Fonctionnalité de déconnexion
Ne prends pas de paramètre
Lorsqu'un utilisateur utilise cette fonction, le serveur enlève le client de la liste, ferme la connexion et met fin au thread de cet utilisateur
Utilisation de la commande : /deconnexion dans le fil de discussion
*/
