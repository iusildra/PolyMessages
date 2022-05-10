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

void sendDisconnection(struct userTuple *user);

/**
 * @brief Verify that the given username doesn't already exists
 *
 * @param username the username to check
 * @return int 0 if the name already exist, 1 otherwise
 */
int checkUsername(char *username, int pos);

/**
 * @brief Get the client's username
 *
 * @param params information about the client who send the message (it's location in the array of client and the message's size)
 * @return char
 */
char *getUsername(int position);

/**
 * @brief Broadcast a message
 *
 * @param param information about the client who send the message (it's location in the array of client and the message's size)
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
 * @brief Free the place of a client when he
 *
 * @param n index of the client we want to deconnect
 */
void terminateEveryClient(int n);