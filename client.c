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
#define PORT 23 
 
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
	char buf[280];
	char message[280];
	fd_set readfds;
	
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
			int bytesIn = recv(sock, buf, 280, 0);
			buf[bytesIn]=0;
			printf("%s",buf);			
			while(1) {				
                int fd_max = STDIN_FILENO;

				/* Set the bits for the file descriptors you want to wait on. */
				FD_ZERO(&readfds);
				FD_SET(STDIN_FILENO, &readfds);
				FD_SET(sock, &readfds);

				/* The select call needs to know the highest bit you set. */    
				if( sock > fd_max ) { fd_max = sock; }

				/* Wait for any of the file descriptors to have data. */
				int socketCount =select(fd_max+1, &readfds, NULL, NULL, NULL);
				
				if(FD_ISSET(sock,&readfds))
				{
					printf("%d",socketCount);
					printf("File Descriptor %d is ready to read!", sock);
					int bytesIn = recv(sock, message, 280, 0);
					message[bytesIn]=0;
					printf(message);
				}
				if(FD_ISSET(STDIN_FILENO,&readfds))
				{
					printf("%d",socketCount);
					char buffer[10];
					printf("stdin is ready.\n");
					read(STDIN_FILENO, buffer, 10);
					printf(buffer);
				}
				  
				
				
			}
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