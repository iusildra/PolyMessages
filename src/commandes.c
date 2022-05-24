#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <dirent.h>
#include "commandes.h"
#define PATH "../serverFiles/"

/**
 * @brief Private message, only the sender and the given receiver will see it /mp <nom du destinataire> <message>
 *
 * @param user information about the sender
 * @param dest username of client to which the message is send
 * @param msg message to send
 * @return void*
 */
int messagePrive(struct userTuple **sockets, int nbClient, struct userTuple *user, char *dest, char *msg)
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

    free(fullMsg);
  }
  return 0;
}

/**
 * @brief Print the manuel page of every commands
 *
 * @param socket the user who requested the man page
 * @return void*
 */
void *help(int socket)
{
  FILE *file;
  file = fopen("manuelFonct.txt", "r");
  char Buffer[128];
  size_t sizeBuff = 128;
  while (fgets(Buffer, 128, file))
  {
    if (send(socket, &sizeBuff, sizeof(size_t), 0) == -1)
    {
      perror("error sendto server size");
      exit(1);
    }
    if (send(socket, Buffer, sizeBuff, 0) == -1)
    {
      perror("error sendto server msg");
      exit(1);
    }
  }
  fclose(file);
}

void *recvFile(int socket, char *filename)
{
  FILE *file;
  char *filepath = malloc(sizeof(char) * (strlen(PATH) + strlen(filename) + 1));
  strcpy(filepath, PATH);
  strcat(filepath, filename);
  file = fopen(filepath, "w");
  char Buffer[128];
  size_t sizeBuff = 128;
  while (1)
  {
    if (recv(socket, &sizeBuff, sizeof(size_t), 0) == -1)
    {
      perror("error sendto server size");
      exit(1);
    }
    if (recv(socket, Buffer, sizeBuff, 0) == -1)
    {
      perror("error sendto server msg");
      exit(1);
    }
    if (strcmp(Buffer, "@end") == 0)
      break;
    fprintf(file, "%s", Buffer);
  }
  fclose(file);
}

/**
 * @brief command listing file on the server
 *
 * @return void*
 */
void *ListeFichier(int socket)
{
  // inspiré d'un code trouvé sur internet

  /* à modifier pour que ça récupère dans le dossier sur le serveur */

  struct dirent *dir;
  // opendir() renvoie un pointeur de type DIR.
  DIR *d = opendir(PATH); // chemin pour accéder au dossier
  if (d)
  {
    while ((dir = readdir(d)) != NULL)
    {
      if (strcmp(dir->d_name, ".") == 0)
        continue;
      if (strcmp(dir->d_name, "..") == 0)
        continue;
      char *msg = malloc(sizeof(char) * (strlen(dir->d_name) + 1));
      strcpy(msg, dir->d_name);

      size_t msgSize = sizeof(char) * (strlen(msg) + 1);
      if (send(socket, &msgSize, sizeof(size_t), 0) == -1)
      {
        perror("error sendto server size");
        exit(1);
      }
      if (send(socket, msg, msgSize, 0) == -1)
      {
        perror("error sendto server msg");
        exit(1);
      }
    }
    char *end = "@end";
    size_t size = sizeof(char) * (strlen(end) + 1);
    if (send(socket, &size, sizeof(size_t), 0) == -1)
    {
      perror("error sendto server size");
      exit(1);
    }
    if (send(socket, end, size, 0) == -1)
    {
      perror("error sendto server msg");
      exit(1);
    }
    closedir(d);
  }
}

void* ListeSalon(int socket){
  char* nameSalon;
  char* descSalon;
  for (int i = 0; i < nbSalon; i++){
    nameSalon = salons[i].name;
    descSalon = salons[i].desc;
    char* msg = malloc(sizeof(char) * strlen(nameSalon) + strlen(descSalon) + 5);
    strcat(msg, nameSalon);
    strcat(msg, " - ");
    strcat(msg, descSalon);
    strcat(msg, "\n");
    size_t msgSize = sizeof(char) * (strlen(msg) + 1);
    if (send(socket, &msgSize, sizeof(size_t), 0) == -1)
    {
      perror("error sendto server size");
      exit(1);
    }
    if (send(socket, msg, msgSize, 0) == -1)
    {
      perror("error sendto server msg");
      exit(1);
    }
    free(msg);
  }
}

/**
 * @brief command sending a file from the server to a user
 *
 * @param Filename file name
 * @return void*
 */
void *sendFile(int socket, char *filename)
{
  void *buffer;
  FILE *file;
  char *filepath = malloc(sizeof(char) * (strlen(PATH) + strlen(filename) + 1));
  strcpy(filepath, PATH);
  strcat(filepath, filename);
  file = popen(filepath, "r");
  size_t sizeBuffer = 128;

  while (fgets(buffer, 128, file))
  {
    if (send(socket, &sizeBuffer, sizeof(size_t), 0) == -1)
    {
      perror("error sendto client size");
      exit(1);
    }
    if (send(socket, buffer, sizeBuffer, 0) == -1)
    {
      perror("error sendto client msg");
      exit(1);
    }
  }
  fclose(file);

  char *end = "@end";
  size_t size = sizeof(char) * (strlen(end) + 1);
  if (send(socket, &size, sizeof(size_t), 0) == -1)
  {
    perror("error sendto client");
    exit(1);
  }
  if (send(socket, end, size, 0) == -1)
  {
    perror("error sendto client");
    exit(1);
  }
}

int creerSalon(char* name, char* desc, int socket){
  int i = 0;
  while (i < MAX_NB_SALONS && salons[i].name != NULL)
  {
    i++;
  }
  salons[i].admin = socket;
  salons[i].desc = desc;
  salons[i].name = name;
  salons[i].connected++;
  return i;
}

void* quitterSalon(struct userTuple* user){
  salons[user->idsalon].connected--;
  user->idsalon = -1;
}

/**
 * @brief Launch command execution
 *
 * @param msg the message to send
 * @param user information about the sender
 * @param nbClient number of clients
 * @return void*
 */
void *executer(struct userTuple **sockets, int nbClient, char *msg, int position)
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
    help(sockets[position]->socket);
  }

  // redirection vers la commande de réception de fichier
  if (strcmp(listeMot[0], "/rcvFile\n") == 0)
  {
    recognized = 1;
    if (motMsg == NULL)
    {
      return NULL;
    }
    motMsg = strtok(NULL, "\0");
    strcpy(listeMot[1], motMsg);
    recvFile(sockets[position]->socket, listeMot[1]);
  }

  // redirection vers la commande listant les fichiers présents sur le serveur
  if (strcmp(listeMot[0], "/Files\n") == 0)
  {
    recognized = 1;
    ListeFichier(sockets[position]->socket);
  }

  if (strcmp(listeMot[0], "@qsal") == 0){//quitter un salon
    recognized = 1;
    quitterSalon(sockets[position]);
  }

  if (strcmp(listeMot[0], "@csal") == 0) {//création d'un salon
    recognized = 1;
    size_t sizeName;//réception nom salon
    if (recv(sockets[position]->socket, &sizeName, sizeof(size_t), 0) == -1)
    {
      perror("error recv salon name server");
      exit(1);
    }
    char *name = malloc(sizeName);
    if (recv(sockets[position]->socket, name, sizeName, 0) == -1)
    {
      perror("error recv salon name server");
      exit(1);
    }//fin réception nom salon

    size_t sizeDesc;//réception description salon
    if (recv(sockets[position]->socket, &sizeDesc, sizeof(size_t), 0) == -1)
    {
      perror("error recv salon desc server");
      exit(1);
    }
    char *desc = malloc(sizeDesc);
    if (recv(sockets[position]->socket, desc, sizeDesc, 0) == -1)
    {
      perror("error recv salon desc server");
      exit(1);
    }//fin réception description salon

    //envoi id du salon, le créateur du salon rentre dans le salon à la création
    int idSalon = creerSalon(name,desc,sockets[position]->socket);
    sockets[position]->idsalon=idSalon;
    printf("Création salon d'id = %d\n", idSalon);
    
    if (send(sockets[position]->socket, &idSalon, sizeof(int), 0) == -1)
    {
      perror("error send idSalon server");
      exit(1);
    }
    printf("envoi salon d'id = %d\n", idSalon);
    
  }

  if (strcmp(listeMot[0], "@send") == 0)
  {
    recognized = 1;
    recvFile(sockets[position]->socket, listeMot[1]);
  }

  if (recognized == 0)
  {
    char *nopMsg = "This command is not recognized\n";
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
