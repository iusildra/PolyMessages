#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
// #include "commandes.h"
#define MAX_NB_CLIENTS 10

/* paramètres nécessaires à l'utilisation des fonctions */
struct userTuple
{
  char *username;
  int socket;
};
// Parameters needed to send/receive a message
struct parametres_struct
{
  int dS;
  socklen_t lg;
  int nbClients;
  struct userTuple *socks[MAX_NB_CLIENTS];
  struct sockaddr_in aC;
};

struct clientParams
{
  size_t size;
  unsigned short position;
};

/*
Fonction qui lance l'éxécution de la bonne fonctionnalité
*/
void* executer(char* msg, struct clientParams user, struct parametres_struct connection){



  // séparation et récupération de chaque élément de la commande

  char* motMsg = strtok(msg, " ");
  char listeMot[4][1024] = {""};
  strcpy(listeMot[0], motMsg);
  int i = 1;
  while (motMsg != NULL){
    motMsg = strtok(NULL, " ");
    strcpy(listeMot[i], motMsg);
    i = i + 1;
  }

  // redirection vers la commande de message privé
  if (listeMot[0]=="/mp"){
    messagePrive(connection.socks[user.position],listeMot[1],listeMot[2])
  }

  //redirection vers la commande du manuel
  if (listeMot[0]=="/manuel"){
    manuelFonc(connection.socks[user.position])
  }
}

/* 
Fonctionnalité de message privé :
Prends en paramètre l'utilisateur à qui envoyer le mp ainsi que le message à envoyer
Envoie un message que seule la personne en paramètre et la personne qui a envoyé le message peut voir
Utilisation de la commande : /mp <nom du destinataire> <message>
*/
void* messagePrive(struct userTuple* user, char* dest, char* msg){

    //faire la recherche de l'identifiant de l'utilisateur dans la future collection
    int socket = -1;
    int i = 0;
    while(socket == -1 || i < connection.nbClients ){
      if (connection.socks[i]->username == dest){
        socket = connection.socks[i]->socket;
      }
      i=i+1;
    }

    if (socket ==-1){
      char *msgAlert = "Cet utilisateur n'existe pas";
      size_t fullSize = sizeof(char) * (strlen(msgAlert) + 1);

      if (send(user->socket, fullSize, sizeof(size_t), 0))
      {
        perror("error sendto server");
        exit(1);
      }
      if (send(user->socket, "Cet utilisateur n'existe pas", fullSize, 0)
      {
        perror("error sendto server");
        exit(1);
      }
      free(msg);
    }

    else{
      char *username = user->username;
      char *delimiter = " -> ";
      size_t fullSize = sizeof(char) * (strlen(username) + strlen(delimiter) + strlen(msg) + 1);
      char *fullMsg = malloc(fullSize);
      strcpy(fullMsg, username);
      strcat(fullMsg, delimiter);
      strcat(fullMsg, msg);
      char *clients[2];

      if (send(socket, fullSize, sizeof(size_t), 0))
      {
        perror("error sendto server");
        exit(1);
      }
      if (send(socket, msg, fullSize, 0))
      {
        perror("error sendto server");
        exit(1);
      }

      // FINIR L'ENVOI AUX 2 UTILISATEURS AVEC clients[]

      free(msg);
    }
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
