#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{

	printf("Début programme\n");

	int dS = socket(PF_INET, SOCK_STREAM, 0);
	printf("Socket Créé\n");

	struct sockaddr_in ad;
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = htons(atoi(argv[1]));
	bind(dS, (struct sockaddr *)&ad, sizeof(ad));
	printf("Socket Nommé\n");

	listen(dS, 7);
	printf("Mode écoute client 1\n");

	struct sockaddr_in aC;
	socklen_t lg = sizeof(struct sockaddr_in);
	int dSC = accept(dS, (struct sockaddr *)&aC, &lg);
	printf("Client 1 Connecté\n");

	int dS2 = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in ad2;
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = htons(atoi(argv[1]));
	bind(dS2, (struct sockaddr *)&ad2, sizeof(ad2));

	listen(dS2, 7);
	printf("Mode écoute client 2\n");

	struct sockaddr_in aC2;
	socklen_t lg2 = sizeof(struct sockaddr_in);
	int dS2C = accept(dS, (struct sockaddr *)&aC2, &lg2);
	printf("Client Connecté\n");

	size_t size;
	while (1)
	{

		/**
		 * @brief Receive client 1's message
		 * 
		 */
		if (recv(dSC, &size, sizeof(size_t), 0) == -1)
		{
			perror("error recv server");
			exit(1);
		}
		char *msg = malloc(sizeof(char)*size);
		if (recv(dSC, msg, sizeof(char)*size, 0) == -1){
			perror("error recv server");
			exit(1);
		}

		/**
		 * @brief Check if the communication should end
		 * 
		 */
		if (strcmp(msg, "fin") == 0){
			shutdown(dS, 2);
			shutdown(dS2, 2);
			listen(dS, 7);
			listen(dS2, 7);
		}

		/**
		 * @brief Send client 1's message to client 2
		 *
		 */
		if (sendto(dS2C, &size, sizeof(size_t), 0, (struct sockaddr *)&aC, lg) == -1)
		{
			perror("error sendto server");
			exit(1);
		}
		if (sendto(dS2C, msg, size, 0, (struct sockaddr *)&aC, lg) == -1)
		{
			perror("error sendto server");
			exit(1);
		}

		/**
		 * @brief Receive client 2's message
		 * 
		 */
		if (recv(dS2C, &size, sizeof(size_t), 0) == -1) {
			perror("error recv");
			exit(1);
		}
		if (recv(dS2C, msg, sizeof(char)*size, 0) == -1){
			perror("error recv server");
			exit(1);
		}

		/**
		 * @brief Send client 2's message to client 1
		 * 
		 */
		if (sendto(dSC, &size, sizeof(size_t), 0, (struct sockaddr *)&aC, lg) == -1)
		{
			perror("error sendto server");
			exit(1);
		}
		if (sendto(dSC, msg, size, 0, (struct sockaddr *)&aC, lg) == -1)
		{
			perror("error sendto server");
			exit(1);
		}

		/**
		 * @brief Check if the communication should end
		 * 
		 */
		if (strcmp(msg, "fin") == 0){
			shutdown(dS, 2);
			shutdown(dS2, 2);
			listen(dS, 7);
			listen(dS2, 7);
		}

		free(msg);
	}

	// shutdown(dS2C, 2);
	shutdown(dSC, 2);
	shutdown(dS, 2);
	printf("Fin du programme");
}

