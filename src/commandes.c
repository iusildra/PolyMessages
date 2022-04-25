#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "commandes.h"
#define MAX_NB_CLIENTS 10

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
void* executer(char* msg, char* user){
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
    messagePrive(user,atoi(listeMot[1]),listeMot[2])
  }
  if (listeMot[0]=="/manuel"){
    manuelFonc(user)
  }
}

/* 
Fonctionnalité de message privé :
Prends en paramètre l'utilisateur à qui envoyer le mp ainsi que le message à envoyer
Envoie un message que seule la personne en paramètre et la personne qui a envoyé le message peut voir
Utilisation de la commande : /mp <nom du destinataire> <message>
*/
void* messagePrive(char* expe, char* dest, char* msg){
    int numdest;
    int i;

    //faire la recherche de l'identifiant de l'utilisateur dans la future collection

    for (i = 0; i < MAX_NB_CLIENTS; i++){
        if (param->connection.socks[i]==dest){
            numdest = i;
        }
    }
    if (send(param->connection.socks[numdest], msg, param->size, 0)
    {
      perror("error sendto server");
      exit(1);
    }
    if (send(param->connection.socks[numdest], msg, param->size, 0)
    {
      perror("error sendto server");
      exit(1);
    }
    free(msg);
}

/*
Fonctionnalité de manuel :
Ne prends pas de paramètres
Lorsqu'un utilisateur se sert de cette fonction, liste les fonctionnalités disponibles, stockées dans un fichier texte
Utilisation de la commande : /manuel
*/
void * manuelFonc(char* user){
  
}


/* 
Fonctionnalité de déconnexion :
Ne prends pas de paramètre
Lorsqu'un utilisateur utilise cette fonction, le serveur enlève le client de la liste, ferme la connexion et met fin au thread de cet utilisateur
Utilisation de la commande : /DC
*/
