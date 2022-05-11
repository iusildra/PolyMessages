#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "clientCommands.h"
#define PATH "../files/"

/**
 * @brief Send a file
 * 
 * @param path path of the file to send
 * @return int 0 if success, -1 otherwise
 */
int sendFile(char *path, int socket) {
  FILE *file;
  file = fopen(path, "r");
  char Buffer[128];
  size_t sizeBuff = 128;
  while (fgets(Buffer, 128, file)){
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

/**
 * @brief Choose the file to send
 * 
 * @param max 
 * @return char* the path of the file
 */
char* chooseFile(char** files, int max) {
  int n = -1;
  do
  {
    printf("Please enter a number from (1-%d) : ", max);
    scanf("%d", &n);
  } while (n <= 0 && n > max);
  printf("%s\n", files[n-1]);
  return files[n - 1];
}

/**
 * @brief List files in the specified directory and ask for a choice
 * 
 * @return char* the path of the file to send
 */
char* listFiles() {
  DIR *d;
  FILE *fp;
  struct dirent *dir;
  d = opendir(PATH);
  char **files;
  if (d)
  {
    char sizeS[10];
    fp = popen("ls ../files | wc -l", "r");
    while (fgets(sizeS, sizeof(char)*10, fp) != NULL) {}
    pclose(fp);
    int size = atoi(sizeS);
    files = malloc(sizeof(char *) * size);
    printf("Select the file you wish to send :\n");
    int i = 1;
    while ((dir = readdir(d)) != NULL)
    {
      if (strcmp(dir->d_name, ".") == 0) continue;
      if (strcmp(dir->d_name, "..") == 0) continue;
      files[i - 1] = malloc(sizeof(char)*(strlen(PATH) + strlen(dir->d_name)+1));
      strcpy(files[i - 1], PATH);
      strcat(files[i - 1], dir->d_name);
      printf("%d\t%s\n", i, files[i - 1]);
      i++;
    }
    closedir(d);
    return chooseFile(files, i - 1);
  }
}