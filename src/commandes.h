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
 * @return void*
 */
void *help();
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