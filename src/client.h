struct values
{
  int socket;
  struct sockaddr_in sockaddr;
  socklen_t socklen;
  char *username;
} values;

/**
 * @brief Allows a client to send a message
 *
 * @param val informations about the socket
 */
void *sendMsg(void *val);

/**
 * @brief Allows a client to receive a message
 *
 * @param params informations about the socket
 */
void *receiveMsg(void *params);

/**
 * @brief Allows a client to send it's username on login
 *
 * @param params informations about the socket
 */
void sendUsername(struct values *params);

/**
 * @brief Deconnect properly the client, sending a signal to the server he is deconnecting
 *
 * @param sig mandatory, don't know why ¯\_(ツ)_/¯
 */
void terminateClient(int sig);