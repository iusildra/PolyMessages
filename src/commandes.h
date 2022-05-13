struct userTuple
{
	char *username;
	int socket;
};

/**
 * @brief Private message, only the sender and the given receiver will see it /mp <nom du destinataire> <message>
 *
 * @param user information about the sender
 * @param dest username of client to which the message is send
 * @param msg message to send
 * @return void*
 */
int messagePrive(struct userTuple** sockets, int nbClient, struct userTuple *user, char *dest, char *msg);

/**
 * @brief Man page : list every commands available
 * 
 * @param socket infos of the client to send the man page
 * @return void*
 */
void *help(int socket);

/**
 * @brief Receive a message from a client and store it where the server program is
 * 
 * @param socket infos of the client that send the file
 * @param filename name of the file that the client send to the server
 * @return void*
 */
void *recvFile(int socket, char* filename);

/**
 * @brief Dispatch the requested command to the right function
 *
 * @param msg the message containing the command toe execute
 * @param position the position of the user in the array of sockets
 * @return void*
 */
void *executer(struct userTuple** sockets, int nbClient, char *msg, int position);

/**
 * @brief Deconnect a client
 *
 * @param user
 */
void deco(int user);