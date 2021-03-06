#define MAX_NB_CLIENTS 10 

struct userTuple
{
	char *username;
	int idsalon;
	int socket;
};

struct salon_struct
{
  int connected; //utilisateurs connectés
  char* name;
  char* desc;
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
 * @brief Receive a file from a client and store it where the server program is
 * 
 * @param socket infos of the client that send the file
 * @param filename name of the file that the client send to the server
 * @return void*
 */
void *recvFile(int socket, char* filename);

/**
 * @brief Command listing files on the server
 *
 * @return void*
 */
void *ListeFichier(int socket);

/**
 * @brief Command listing channels on the server
 *
 * @return void*
 */
void* ListeSalon(int socket, int nbSalon, struct salon_struct* salons);

/**
 * @brief Send a file to a client and store it where the client program is
 * 
 * @param socket infos of the client that want to receive the file
 * @param filename name of the file that the client  want to receive from the server
 * @return void*
 */
void *sendFile(int socket, char* filename);

/**
 * @brief Create a textual channel and set its name and its description 
 * 
 * @param name 
 * @param desc 
 * @param salons 
 * @param size 
 * @return int 
 */
int creerSalon(char* name, char* desc, struct salon_struct* salons, int size);

/**
 * @brief Disconnect a user from a channel and connect them to the main channel
 * 
 * @param user 
 * @param salons 
 * @return void* 
 */
void* quitterSalon(struct userTuple* user, struct salon_struct* salons);

/**
 * @brief Get the NbSalons object
 * 
 * @param max 
 * @param salons 
 * @return int 
 */
int getNbSalons(int max,  struct salon_struct* salons);

/**
 * @brief Connects the user to the channel of their choice
 * 
 * @param sockets 
 * @param position 
 * @param salons 
 * @param size 
 * @return void* 
 */
void* connectClient(struct userTuple** sockets, int position, struct salon_struct* salons, int size);

/**
 * @brief Command allowing the user to delete the channel of their choice
 * 
 * @param sockets 
 * @param position 
 * @param salons 
 * @param size 
 * @return void* 
 */
void* deleteRoom(struct userTuple** sockets, int position, struct salon_struct* salons, int size);

/**
 * @brief Store the channels in a textual file to create them again at the start of the server
 * 
 * @param salons 
 * @return void* 
 */
void *saveRooms(struct salon_struct *salons);

/**
 * @brief Dispatch the requested command to the right function
 *
 * @param msg the message containing the command toe execute
 * @param position the position of the user in the array of sockets
 * @return void*
 */
void *executer(struct userTuple** sockets, int nbClient, char *msg, int position, struct salon_struct* salons, int size);
