#define MAX_NB_CLIENTS 10 

// Parameters needed to send/receive a message
struct parametres_struct
{
  int dS;
  socklen_t lg;
  int nbClients;
  struct userTuple *socks[MAX_NB_CLIENTS];
  struct sockaddr_in aC;
};
struct parametres_struct connection;
sem_t nbPlaces;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Send a disconnection signal to the user (used to properly diconnect every client when the server shuts down)
 * 
 * @param user the user to disconnect
 */
void sendDisconnection(struct userTuple *user);

/**
 * @brief Verify that the given username doesn't already exists
 *
 * @param username the username to check
 * @return int 0 if the name already exist, 1 otherwise
 */
int checkUsername(char *username, int pos);

/**
 * @brief Get the username of a new client
 * 
 * @param position the position of the client's socket in the socket array
 * @return char* the username the client gave
 */
char *getUsername(int position);

/**
 * @brief Broadcast a message to the people in the same room as the sender
 *
 * @param position location in the array of client and the message's size (used to get the room and socket)
 * @param msg the message to be send
 */
void sendMsg(int position, char *msg);

/**
 * @brief Manage the client by allowing it to send messages and to deconnect
 *
 * @param params information about the client who send the message (it's location in the array of client and the message's size)
 * @return void*
 */
void *clientManagement(void *params);

/**
 * @brief Get the first index available in the array of client
 *
 * @return int
 */
int getIndex();

/**
 * @brief Get the first index available in the array of fileClient
 * 
 * @return int 
 */
int getFileIndex();

/**
 * @brief Manage client login by allowing it to connect and adding it to the array of client
 *
 * @return void*
 */
void *userLogin();

/**
 * @brief Allows to receive/send a file
 *
 * @param params position in the array of file client
 * @return void*
 */
void *fileManagement(void *params);

/**
 * @brief Manage client's requests for files
 *
 * @param dS Dedicated socket
 * @param acFiles sockaddr
 * @return void*
 */
void *fileClientLogin(int dS, struct sockaddr_in acFiles);

/**
 * @brief Free the place of a client when he
 *
 * @param n index of the client we want to deconnect
 */
void terminateEveryClient(int n);
