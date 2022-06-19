struct values
{
  int socket;
  char* ip;
  char* port;
  struct sockaddr_in sockaddr;
  socklen_t socklen;
  char *username;
  int idSalon;
  int filePort;
} values;

/**
 * @brief Detect if the user entered a client side command
 *
 * @param msg the message written
 * @param ip ip of the serveur
 * @param port port for the messge management 
 * @param sock socket of the server
 * @return int 0 if there was no commands, 1 otherwise
 */
int detectClientCommands(char *msg, char *ip, int port, int sock);

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
 * @param sig mandatory ¯\_(ツ)_/¯
 */
void terminateClient(int sig);

