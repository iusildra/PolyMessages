#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "commandes.h"

// Parameters needed to send/receive a message
/**
 * @brief Private message, only the sender and the given receiver will see it /mp <nom du destinataire> <message>
 *
 * @param user information about the sender
 * @param dest username of client to which the message is send
 * @param msg message to send
 * @return void*
 */
int messagePrive(struct userTuple** sockets, int nbClient, struct userTuple *user, char *dest, char *msg)
{

  // faire la recherche de l'identifiant de l'utilisateur dans la future collection
  int socket = -1;
  int i = 0;
  while (socket == -1 && i < nbClient)
  {
    if (sockets[i] != NULL && strcmp(sockets[i]->username, dest) == 0)
    {
      socket = sockets[i]->socket;
      break;
    }
    i++;
  }

  printf("Socket to send : %d", socket);
  if (socket == -1)
  {
    char *msgAlert = "Cet utilisateur n'existe pas";
    size_t fullSize = sizeof(char) * (strlen(msgAlert) + 1);

    if (send(user->socket, &fullSize, sizeof(size_t), 0) == -1)
    {
      perror("error send server size");
      exit(1);
    }
    if (send(user->socket, msgAlert, fullSize, 0) == -1)
    {
      perror("error send server msg");
      exit(1);
    }
  }

  else
  {
    char *delimiter = " -> ";
    size_t fullSize = sizeof(char) * (strlen(user->username) + strlen(delimiter) + strlen(msg) + 1);
    char *fullMsg = malloc(fullSize);
    strcpy(fullMsg, user->username);
    strcat(fullMsg, delimiter);
    strcat(fullMsg, msg);
    char *clients[2];

    if (send(socket, &fullSize, sizeof(size_t), 0) == -1)
    {
      perror("error sendto server size");
      exit(1);
    }
    if (send(socket, fullMsg, fullSize, 0) == -1)
    {
      perror("error sendto server msg");
      exit(1);
    }

    // FINIR L'ENVOI AUX 2 UTILISATEURS AVEC clients[]

    free(fullMsg);
  }
  return 0;
}

void *help(struct userTuple* socket)
{
  FILE *file;
  file = fopen("manuelFonct.txt", "r");
  char Buffer[128];
  size_t sizeBuff = 128;
  while (fgets(Buffer, 128, file)){
    if (send(socket->socket, &sizeBuff, sizeof(size_t), 0) == -1)
    {
      perror("error sendto server size");
      exit(1);
    }
    if (send(socket->socket, Buffer, sizeBuff, 0) == -1)
    {
      perror("error sendto server msg");
      exit(1);
    }
  }
  fclose(file);
}

void *receiveFile(struct userTuple* socket, char* filename)
{
  FILE *file;
  file = fopen(filename, "w");
  char Buffer[128];
  size_t sizeBuff = 128;
  while (fgets(Buffer, 128, file)){
    if (recv(socket->socket, &sizeBuff, sizeof(size_t), 0) == -1)
    {
      perror("error sendto server size");
      exit(1);
    }
    if (recv(socket->socket, Buffer, sizeBuff, 0) == -1)
    {
      perror("error sendto server msg");
      exit(1);
    }
    fprintf(file,"%s",Buffer);
  }
  fclose(file);
}

/**
 * @brief Launch command execution
 *
 * @param msg the message to send
 * @param user information about the sender
 * @param nbClient number of clients
 * @return void*
 */
void *executer(struct userTuple** sockets, int nbClient, char *msg, int position)
{
  // séparation et récupération de chaque élément de la commande

  char *motMsg = strtok(msg, " ");
  char listeMot[4][1024] = {""};
  int recognized = 0;
  strcpy(listeMot[0], motMsg);
  // redirection vers la commande de message privé
  if (strcmp(listeMot[0], "/mp") == 0)
  {
    recognized = 1;
    if (motMsg == NULL)
    {
      return NULL;
    }
    motMsg = strtok(NULL, " ");
    strcpy(listeMot[1], motMsg);
    size_t offset = strlen(listeMot[0]) + strlen(listeMot[1]) + 2; // Drop the command name and username (witht their whitespace...)
    char *msgToSend = malloc(sizeof(char) * (strlen(msg) - offset));
    msgToSend = strtok(NULL, "\0");

    messagePrive(sockets, nbClient, sockets[position], listeMot[1], msgToSend);
  }

  // redirection vers la commande du manuel
  if (strcmp(listeMot[0], "/help\n") == 0)
  {
    recognized = 1;
    help(sockets[position]);
  }

  if (strcmp(listeMot[0], "/send") == 0)
  {
    recognized = 1;
    receiveFile(sockets[position], listeMot[1]);
  }

  if (recognized == 0){
    char* nopMsg = "This command is not recognized\n";
    size_t nopSize = 31;
    if (send(sockets[position]->socket, &nopSize, sizeof(size_t), 0) == -1)
    {
      perror("error sendto server size");
      exit(1);
    }
    if (send(sockets[position]->socket, nopMsg, nopSize, 0) == -1)
    {
      perror("error sendto server msg");
      exit(1);
    }
  }
}