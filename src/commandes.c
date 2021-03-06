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
#define ROOM_BACKUP "../serverFiles/roomsBackup.txt"

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
    char *msgAlert = "Cet utilisateur n'existe pas\n";
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
  } else
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
  file = fopen(filepath, "wb");
  int TAILLE_BUFF = 2000;
  short int buffer[TAILLE_BUFF];
  int sizeBuff;
  while (1)
  {
    short int r = recv(socket, buffer, sizeof(short int) * TAILLE_BUFF, 0);
    if (r == -1)
    {
      perror("error recv client msg");
      exit(1);
    }
    if (r == 0) {
      break;
    }
    fwrite(buffer, sizeof(short int), r/2, file);
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
  // inspir?? d'un code trouv?? sur internet

  FILE *fp = popen("ls ../files | wc -l", "r");
  char sizeS[10];
  while (fgets(sizeS, sizeof(char) * 10, fp) != NULL)
  {
  }
  pclose(fp);
  int size = atoi(sizeS);
  if (send(socket, &size, sizeof(int), 0) == -1)
  {
    perror("error send files number");
    exit(1);
  }
  struct dirent *dir;
  // opendir() renvoie un pointeur de type DIR.
  DIR *d = opendir(PATH); // chemin pour acc??der au dossier
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
      printf("File %s\n", msg);
      free(msg);
    }
    char *end = "/end";
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

/**
 * @brief command listing channels on the server
 *
 * @return void*
 */
void* ListeSalon(int socket, int nbSalon, struct salon_struct* salons){
  char* nameSalon;
  char* descSalon;
  for (int i = 0; i < nbSalon; i++){
    nameSalon = salons[i].name;
    descSalon = salons[i].desc;
    char* msg = malloc(sizeof(char) * strlen(nameSalon) + strlen(descSalon) + 7);
    sprintf(msg, "\033[34m%d", i);
    strcat(msg, " -> ");
    strcat(msg, nameSalon);
    strcat(msg, " - ");
    strcat(msg, descSalon);
    strcat(msg, "\033[0m");
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
  int TAILLE_BUFF = 2000;
  FILE *file;
  short int buffer[TAILLE_BUFF];
  char *filepath = malloc(sizeof(char) * (strlen(PATH) + strlen(filename) + 1));
  strcpy(filepath, PATH);
  strcat(filepath, filename);
  file = fopen(filepath, "rb");
  int nb_val_lue;
  while ((nb_val_lue = fread(buffer, sizeof(short int), TAILLE_BUFF, file)) != 0)
  {
    if (send(socket, buffer, sizeof(short int)*nb_val_lue, 0) == -1)
    {
      perror("error sendto client msg");
      exit(1);
    }
  }
  fclose(file);
}

int creerSalon(char* name, char* desc, struct salon_struct* salons, int size){
  int i = 0;
  while (i < size && (salons[i].name) != NULL)
  {
    i++;
  }
  salons[i].desc = desc;
  salons[i].name = name;
  salons[i].connected++;
  return i;
}

void* quitterSalon(struct userTuple* user, struct salon_struct* salons){
  salons[user->idsalon].connected--;
  user->idsalon = -1;
}

int getNbSalons(int max,  struct salon_struct* salons)
{
  int i = 0;
  int res = 0;
  while (i < max)
  {
    if (salons[i].name != NULL){
      res++;
    }
    i++;
  }
  return res;
}

void* connectClient(struct userTuple** sockets, int position, struct salon_struct* salons, int size) {
  int n = 0;
  int answer = -1;
  if (recv(sockets[position]->socket, &n, sizeof(int), 0) == -1)
  {
    perror("error send client");
    exit(1);
  }
  if (n < size & salons[n].name != NULL) {
    sockets[position]->idsalon = n;
    printf("Room has been changed for user %s, now is %s\n", sockets[position]->username, salons[sockets[position]->idsalon].name);
  }
}

void* deleteRoom(struct userTuple** sockets, int position, struct salon_struct* salons, int size) {
  int n = 0;
  int answer = -1;
  if (recv(sockets[position]->socket, &n, sizeof(int), 0) == -1)
  {
    perror("error send client");
    exit(1);
  }
  if (n < size && salons[n].name != NULL) {//the room doesn't exist
    salons[sockets[position]->idsalon].name = NULL;
    if (sockets[position]->idsalon == n){
      sockets[position]->idsalon = -1;
    }
    printf("Room %s has been deleted\n", salons[sockets[position]->idsalon].name);
  }
}

/**
 * @brief Launch command execution
 *
 * @param msg the message to send
 * @param user information about the sender
 * @param nbClient number of clients
 * @return void*
 */
void *executer(struct userTuple **sockets, int nbClient, char *msg, int position, struct salon_struct* salons, int size)
{
  // s??paration et r??cup??ration de chaque ??l??ment de la commande

  char *motMsg = strtok(msg, " ");
  char listeMot[4][1024] = {""};
  int recognized = 0;
  strcpy(listeMot[0], motMsg);
  // redirection vers la commande de message priv??
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

  // redirection vers la commande de r??ception de fichier
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

  if (strcmp(listeMot[0], "/quitter\n") == 0){//quitter un salon
    recognized = 1;
    quitterSalon(sockets[position], salons);
  }

  if (strcmp(listeMot[0], "/creer") == 0) {//cr??ation d'un salon
    recognized = 1;
    size_t sizeName;//r??ception nom salon
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
    }//fin r??ception nom salon

    size_t sizeDesc;//r??ception description salon
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
    }//fin r??ception description salon

    printf("Desc is : %s", desc);

    //envoi id du salon, le cr??ateur du salon rentre dans le salon ?? la cr??ation
    int idSalon = creerSalon(name,desc, salons, size);
    sockets[position]->idsalon=idSalon;
    printf("Cr??ation salon d'id = %d\n", idSalon);
    
  }

  if (strcmp(listeMot[0], "/salons\n") == 0){
    recognized = 1;
    ListeSalon(sockets[position]->socket, getNbSalons(size,salons), salons);
  }

  if (strcmp(listeMot[0], "/send") == 0)
  {
    recognized = 1;
    recvFile(sockets[position]->socket, listeMot[1]);
  }

  if(strcmp(listeMot[0], "/connect") == 0) {
    recognized = 1;
    connectClient(sockets, position, salons, size);
  }

  if(strcmp(listeMot[0], "/delete") == 0) {
    recognized = 1;
    deleteRoom(sockets, position, salons, size);
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

void* saveRooms(struct salon_struct* salons) {
  FILE* file;
  file = fopen(ROOM_BACKUP, "w");
  for(int i = 0; i < 10; i++) {
    if (salons[i].name == NULL) continue;
    printf("Ecriture du salon %s en cours...\n", salons[i].name);
    fprintf(file, "%s/%s\n", salons[i].name, salons[i].desc);
    printf("Ecriture du salon %s temrin??e\n", salons[i].name);
  }
  fclose(file);
}

struct salon_struct* loadRooms() {
  FILE *file;
}