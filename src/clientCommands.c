#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "clientCommands.h"
#define PATH "../files/"

/**
 * @brief Send a file to the server
 *
 * @param path path of the file to send
 * @return int 0 if success, -1 otherwise
 */
int sendFile(char *path, char *name, int socket)
{
  // send signal for file managemnt
  if (send(socket, "@send", sizeof(char) * 6, 0) == -1)
  {
    perror("error send client");
    exit(1);
  }
  // send name of the file
  size_t size = sizeof(char) * (strlen(name) + 1);
  if (send(socket, &size, sizeof(size_t), 0) == -1)
  {
    perror("error send client");
    exit(1);
  }
  if (send(socket, name, size, 0) == -1)
  {
    perror("error send client");
    exit(1);
  }
    
  FILE *file;
  FILE *file2;
  file = fopen(path, "rb");
  int TAILLE_BUF = 2000;
  short int buffer[TAILLE_BUF];
  int nb_val_lue;
  while ((nb_val_lue = fread(buffer, sizeof(short int), TAILLE_BUF, file)) > 0)
  {
    if (send(socket, buffer, sizeof(short int)*nb_val_lue, 0) == -1)
    {
      perror("error sendto client msg");
      exit(1);
    }
  }

  fclose(file);
  exit(0);
}

/**
 * @brief Ask for a file
 *
 * @param socket socket to the file server
 * @return int 0 if success, -1 otherwise
 */
int recvFile(char *path, char *filename, int socket)
{
  size_t size = sizeof(char) * (strlen(filename) + 1);
  if (send(socket, &size, sizeof(size_t), 0) == -1)
  {
    perror("error send client");
    exit(1);
  }
  if (send(socket, filename, size, 0) == -1)
  {
    perror("error send client");
    exit(1);
  }
  FILE *file;
  int TAILLE_BUF = 2000;
  file = fopen(path, "wb");
  short int buffer[TAILLE_BUF];
  int sizeBuff;
  while (1)
  {
    short int r = recv(socket, buffer, sizeof(short int) * TAILLE_BUF, 0);
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
 * @brief Choose the file to send
 *
 * @param max
 * @return char* the path of the file
 */
char *chooseFile(char **files, int max)
{
  int n = -1;
  do
  {
    printf("\033[34mPlease enter a number from (1-%d) : \033[0m", max);
    scanf("%d", &n);
  } while (n <= 0 && n > max);
  printf("\033[34m%s\033[0m\n", files[n-1]);
  return files[n - 1];
}

/**
 * @brief List files in the specified directory and ask for a choice
 *
 * @return char* the path of the file to send
 */
char *listFiles()
{
  DIR *d;
  FILE *fp;
  struct dirent *dir;
  d = opendir(PATH);
  char **files;
  if (d)
  {
    char sizeS[10];
    fp = popen("ls ../files | wc -l", "r");
    while (fgets(sizeS, sizeof(char) * 10, fp) != NULL)
    {
    }
    pclose(fp);
    int size = atoi(sizeS);
    files = malloc(sizeof(char *) * size);
    printf("\033[34mSelect the file you wish to send :\033[0m\n");
    int i = 1;
    while ((dir = readdir(d)) != NULL)
    {
      if (strcmp(dir->d_name, ".") == 0)
        continue;
      if (strcmp(dir->d_name, "..") == 0)
        continue;
      files[i - 1] = malloc(sizeof(char) * (strlen(dir->d_name) + 1));
      strcpy(files[i - 1], dir->d_name);
      printf("\033[1;34m%d\t%s\033[0m\n", i, files[i - 1]);
      i++;
    }
    closedir(d);
    char *name = chooseFile(files, i - 1);
    char *nameToSend = (char *)malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(nameToSend, name);
    free(files);
    return nameToSend;
  }
}

char *listServFiles(int socket)
{
  if (send(socket, "@recv", sizeof(char) * 6, 0) == -1)
  {
    perror("error send @file client");
    exit(1);
  }

  size_t sizeName;
  char *m;
  int i = 0;
  int nbFiles = 0;
  if (recv(socket, &nbFiles, sizeof(int), 0) == -1)
  {
    perror("error receive server files number");
    exit(1);
  }
  char **files = malloc(sizeof(char *) * nbFiles);
  while (1)
  {
    if (recv(socket, &sizeName, sizeof(size_t), 0) == -1)
    {
      perror("error recv size client");
      exit(1);
    }

    m = malloc(sizeof(char) * sizeName);
    if (recv(socket, m, sizeName, 0) == -1)
    {
      perror("error recv file client");
      exit(1);
    }

    if (strcmp(m, "@end") == 0)
    {
      break;
    }
    files[i] = m;
    printf("\033[1;34m%d\t%s\033[0m", i, m);
    i++;
  }

  char *filename = chooseFile(files, i - 1);
  free(files);
  return filename;
}

char* nameSalon(int socket){
  printf("\033[34mEntrez un nom pour votre salon (de taille 20 max)\033[0m\n");
  char* msg = malloc(20);
  scanf("%s",msg);
  size_t size = sizeof(char) * (strlen(msg));
  
  if (send(socket, &size, sizeof(size_t), 0) == -1)
  {
    perror("error send client");
    exit(1);
  }
  if (send(socket, msg, size, 0) == -1)
  {
    perror("error send client");
    exit(1);
  }
  
}

char* descSalon(int socket){
  printf("\033[34mEntrez une description pour le salon (de taille 200 max)\033[0m\n");
  char* msg = malloc(200);
  scanf("%s",msg);
  size_t size = sizeof(char) * (strlen(msg));
  
  if (send(socket, &size, sizeof(size_t), 0) == -1)
  {
    perror("error send client");
    exit(1);
  }
  if (send(socket, msg, size, 0) == -1)
  {
    perror("error send client");
    exit(1);
  }
}

void* quitterSalon(int socket){
  char* msg = "@qsal";
  size_t size = sizeof(char) * (strlen(msg) + 1);
  if (send(socket, &size, sizeof(size_t), 0) == -1)
  {
    perror("error send client");
    exit(1);
  }
  if (send(socket, msg, size, 0) == -1)
  {
    perror("error send client");
    exit(1);
  }
}

int creerSalon(int socket){
  char* msg = "@csal";
  size_t size = sizeof(char) * (strlen(msg) + 1);
  if (send(socket, &size, sizeof(size_t), 0) == -1)
  {
    perror("error send client");
    exit(1);
  }
  if (send(socket, msg, size, 0) == -1)
  {
    perror("error send client");
    exit(1);
  }

  char* name = nameSalon(socket);
  char* desc = descSalon(socket);
  int idSalon;
  if (recv(socket, &idSalon, sizeof(int), 0) == -1)
  {
    perror("error receive idSalon");
    exit(1);
  }
  printf("\033[31mreception id salon\033[0m\n");
  if (idSalon == -1){
    printf("\033[34mLe salon n'a pas pu être créé\033[0m\n");
  }else {
    printf("\033[34mLe salon a été créé!\033[0m\n");
  }
  return idSalon;
}