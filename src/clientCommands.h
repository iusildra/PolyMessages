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
 * @param socket socket to the file server
 * @return int 0 if success, -1 otherwise
 */
int recvFile(int socket);


/**
 * @brief List files in the specified directory and ask for a choice
 * 
 * @return char* the path of the file to send
 */
char *listFiles();