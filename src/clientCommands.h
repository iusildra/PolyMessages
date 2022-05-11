/**
 * @brief Send a file
 * 
 * @param path path of the file to send
 * @return int 0 if success, -1 otherwise
 */
int sendFile(char* path, int socket);


/**
 * @brief List files in the specified directory and ask for a choice
 * 
 * @return char* the path of the file to send
 */
char *listFiles();