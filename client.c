#if defined (WIN32)
    #include <winsock2.h>
	#include <unistd.h>
    typedef int socklen_t;
#elif defined (linux)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
    typedef int SOCKET;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct sockaddr SOCKADDR;
#endif
 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>

#define PORT 8080
#define MSG_SIZE 280
#define INCOMING_FILE "INCOMING-FILE"
#define NAME_SIZE 30
#define FILE_SIZE_CHAR 30 //le nom et la taille max d'un fichier ne doivent pas exceder 30 caracteres
#define BUFFER_SIZE 1024

void * receive_message(void * sockfd) {
	SOCKET sock=(intptr_t)sockfd;
    char message[MSG_SIZE];
    while (1) {
        int bytesIn = recv(sock, message, MSG_SIZE, 0);
		message[bytesIn]=0;
		if (strcmp(message, INCOMING_FILE) == 0) {
			// on reçoit un fichier
			send(sock, "ok", 5,0);
			char file_size[FILE_SIZE_CHAR];
			char file_name[NAME_SIZE];
			
			int bytesIn1=recv(sock, file_size, FILE_SIZE_CHAR,0);
			send(sock, "ok", 5,0);

			file_size[bytesIn1]=0;
			printf("\rTaille du fichier: %s\n", file_size);

			
			int bytesIn2=recv(sock, file_name, NAME_SIZE, 0);
			send(sock, "ok", 5,0);

			file_name[bytesIn2]=0;
			printf("%s fait %s octets\n", file_name, file_size);

			char * filenameNewFile="download.jpg";
			FILE * local_file = fopen(filenameNewFile, "wb+");
			if(local_file == NULL) {
				printf("Erreur à l'ouverture du nouveau fichier");
			}

			// recevoir fichier
			char buffer[BUFFER_SIZE];
			int r = 0, w = 0, total = 0;
			
			while(total < atoi(file_size)) {
				r = recv(sock, buffer, BUFFER_SIZE, 0);
				w = fwrite(buffer, 1, r, local_file);
				total += r;
				if(w < 0) {
					printf("Erreur lors de l'ecriture du fichier");					
					fclose(local_file);					
				}
			}
			printf("Fichier telecharge\n");
			fclose(local_file);
			printf("\r%s", "> ");
			fflush(stdout);
			message[0]=0;
		}
		else
		{
			if(bytesIn<=0)
			{		
				printf("\rServeur deconnecte\n");			
				break;			
			}
			else
			{
				printf("\r%s\n", message);  
				printf("\r%s", "> ");
				fflush(stdout);
			}			
		
		}
	}
	return 0;
}

void * send_message(void * sockfd) {
	SOCKET sock=(intptr_t)sockfd;
    char message[MSG_SIZE];
    while (1) {
		printf("\r%s", "> ");
		fflush(stdout);
		fgets(message, MSG_SIZE, stdin);
		message[strlen(message)-1]=0;
		send(sock, message, strlen(message), 0);	
		message[0]=0;
	}
	return 0;
}


int main(void)
{
    #if defined (WIN32)
        WSADATA WSAData;
        int erreur = WSAStartup(MAKEWORD(2,2), &WSAData);
    #else
        int erreur = 0;
    #endif
 
    SOCKET sock;
    SOCKADDR_IN sin;
	char buf[50];
	
    if(!erreur)
    {
        /* Création de la socket */
        sock = socket(AF_INET, SOCK_STREAM, 0);
 
        /* Configuration de la connexion */
        sin.sin_addr.s_addr = inet_addr("127.0.0.1");
        sin.sin_family = AF_INET;
        sin.sin_port = htons(PORT);
		
        /* Si le client arrive à se connecter */
        if(connect(sock, (SOCKADDR*)&sin, sizeof(sin)) != SOCKET_ERROR)
		{			
			int bytesIn = recv(sock, buf, 50, 0);
			buf[bytesIn]=0;
			printf("%s",buf);
			pthread_t thread[2];
			if (pthread_create(&thread[0], NULL, &send_message, (void *)(intptr_t)sock) != 0) {
				printf ("Impossible de créer le thread send\n");				
			}
			if (pthread_create(&thread[1], NULL, &receive_message, (void *)(intptr_t)sock) != 0) {
				printf ("Impossible de créer le thread recv\n");
			}
			
			pthread_join (thread[1], NULL);

		}
        else
		{
            printf("Impossible de se connecter\n");
		}
        /* On ferme la socket précédemment ouverte */
        closesocket(sock);
 
        #if defined (WIN32)
            WSACleanup();
        #endif
    }
 
    return EXIT_SUCCESS;
}
