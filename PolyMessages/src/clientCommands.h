/**
 * @brief Send a file
 * 
 * @param path path of the file to send
 * @param name name of the file to send
 * @param socket socket to the file server
 * @return int 0 if success, -1 otherwise
 */
int sendFile(char* path, char* name, int socket);

/**
 * @brief Ask for a file
 * 
 * @param path path where the received file will be stored
 * @param name name of the received file 
 * @param socket socket to the file server
 * @return int 0 if success, -1 otherwise
 */
int recvFile(char* path, char* name, int socket);

/**
 * @brief Choose the file to send
 * 
 * @param max 
 * @return char* the path of the file
 */
char* chooseFile(char** files, int max);

/**
 * @brief List files in the specified directory and ask for a choice
 * 
 * @return char* the path of the file to send
 */
char *listFiles();

/**
 * @brief List files on the server and ask for a choice
 * 
 * @param socket socket to the file server
 * @return char* the path of the file to send
 */
char* listServFiles(int socket);

char* nameSalon(int socket);

char* descSalon(int socket);

void* creerSalon(int socket);

void *connectToRoom(int socket);