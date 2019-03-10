//Fichier standard au sytème d'exploitation (windows ou linux)
#if defined (WIN32)
    #include <winsock2.h>

    typedef int socklen_t;
    //stocke la taille d'une structure de type sockaddr_in

#elif defined (linux)

    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>

    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close (s)

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#endif

//Fichiers standards au deux OS
#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>
#include <string.h>


#define PORT 23

int main(void)
{
    // Si la plateforme est Windows
    #if defined (WIN32)
    WSADATA WSAData;
    int erreur = WSAStartup(MAKEWORD(2,2), &WSAData);
    #else
    int erreur = 0;
    #endif
    
    /* Socket et contexte d'adressage du serveur */
    SOCKET listening;    
    /*Contexte d'adressage avec:
        struct sockaddr_in
        {
            short sin_family; //format d'adresse (AF_INET: adresse format internet)
            unsigned short sin_port; //port utilisé
            struct in_addr sin_addr; //adresse IP utilisée
            char sin_zero[8]; //non utilisés
        };
    */
    SOCKADDR_IN sin;
    
    int sock_err;
    
    if(!erreur)
    {
        /* Création d'une socket */
        listening = socket(AF_INET, SOCK_STREAM, 0);
        
        /* Si la socket est valide */
        if(listening != INVALID_SOCKET)
        {
            printf("La socket %d est maintenant ouverte en mode TCP/IP\n", listening);
            
            /* Configuration */
            sin.sin_addr.s_addr = htonl(INADDR_ANY);  /* Adresse IP automatique */
            sin.sin_family = AF_INET;                 /* Protocole familial (IP) */
            sin.sin_port = htons(PORT);               /* Listage du port */
            sock_err = bind(listening, (SOCKADDR*)&sin, sizeof(sin));
            
            /* Si la socket fonctionne */
            if(sock_err != SOCKET_ERROR)
            {
                /* Démarrage du listage (mode server) */
                sock_err = listen(listening, 5);
                printf("Listage du port %d...\n", PORT);
                
                /* Si la socket fonctionne */
                if(sock_err != SOCKET_ERROR)
                {
                    // Creation du set de descripteur de fichiers
                    fd_set master;
                    FD_ZERO(&master);

                    // Add our first socket that we're interested in interacting with; the listening socket!
                    // It's important that this socket is added for our server or else we won't 'hear' incoming
                    // connections 
                    FD_SET(listening, &master);

                    // this will be changed by the \quit command (see below, bonus not in video!)
                    bool running = true; 

                    while (running)
                    {
                    // Make a copy of the master file descriptor set, this is SUPER important because
                    // the call to select() is _DESTRUCTIVE_. The copy only contains the sockets that
                    // are accepting inbound connection requests OR messages. 

                    // E.g. You have a server and it's master file descriptor set contains 5 items;
                    // the listening socket and four clients. When you pass this set into select(), 
                    // only the sockets that are interacting with the server are returned. Let's say
                    // only one client is sending a message at that time. The contents of 'copy' will
                    // be one socket. You will have LOST all the other sockets.

                    // SO MAKE A COPY OF THE MASTER LIST TO PASS INTO select() !!!

                        fd_set copy = master;

                         // See who's talking to us
                        int socketCount = select(0, &copy, NULL, NULL, NULL);

                        // Loop through all the current connections / potential connect
                        for (int i = 0; i < socketCount; i++)
                        {
                             // Makes things easy for us doing this assignment
                            SOCKET clsock = copy.fd_array[i];

                            // Is it an inbound communication?
                            if (clsock == listening)
                            {
                                // Accept a new connection
                                SOCKET client = accept(listening, NULL, NULL);
                                printf("Client %d arrive sur le chat\n", client);

                                // Add the new connection to the list of connected clients
                                FD_SET(client, &master);

                                // Send a welcome message to the connected client
                                char * welcomeMsg = "Bienvenue sur le chat!\r\n";
                                send(client, welcomeMsg, strlen(welcomeMsg), 0);
                            }
                            else // It's an inbound message
                            {
                                char buf[280];
                                ZeroMemory(buf, 280);

                                // Receive message
                                int bytesIn = recv(clsock, buf, 280, 0);
                                if(!(buf[0]=='\r'&& buf[1]=='\n' && bytesIn==2))
                                {
                                    if (bytesIn <= 0)
                                    {
                                        // Drop the client
                                        closesocket(clsock);
                                        printf("Client %d quit the chat server\n", clsock);
                                        FD_CLR(clsock, &master);
                                    }
                                    else
                                    {
										if(buf[0]=='#')
										{
											char * cmd;
											cmd=(buf+1);
											if(cmd=="exit")
											{
												closesocket(clsock);
												printf("Client %d quit the chat server\n", clsock);
												FD_CLR(clsock, &master);
											}
											else if(cmd=="help")
											{
												//TODO: show commands
											}
											else if(cmd=="listU")
											{
												//TODO: list user on the server
											}
											else if(cmd=="listF")
											{
												//TODO: list files of server 
											}
											else if(cmd=="trfU")
											{
												//TODO: upload a file to the server
											}
											else if(cmd=="trfD")
											{
												//TODO: Download a file from the server
											}
											else if(cmd=="private")
											{
												//TODO: commute to private conversation with user 
											}
											else if(cmd=="public")
											{
												//TODO: commute to public if client was in private conversation
											}
											else if (cmd=="ring")
											{
												//TODO: send to user concerned that a client ring him and to client if user is connect
											}
											else
											{
												//TODO: send to client>cmd incorrect
											}
											printf("%s:%d",cmd,strlen(cmd));
										}
										else
										{
											printf("USER:%d> %s\n",clsock,buf);
											// Send message to other clients, and definiately NOT the listening socket                        
											for (int i = 0; i < master.fd_count; i++)
											{
												SOCKET outSock = master.fd_array[i];
												if (outSock != listening && outSock != clsock)
												{                               
													char strOut[320];
													sprintf(strOut,"USER:%d> %s",clsock,buf);
													send(outSock, strOut, strlen(strOut), 0);
												}
											}
										}
                                    }

                                }
                            }
                        }
                    }
                    // Remove the listening socket from the master file descriptor set and close it
                    // to prevent anyone else trying to connect.
                    FD_CLR(listening, &master);
                    closesocket(listening);

                    // Message to let users know what's happening.
                    char* msg = "Server is shutting down. Goodbye\r\n";

                    while (master.fd_count > 0)
                    {
                    // Get the socket number
                        SOCKET clsock = master.fd_array[0];

                    // Send the goodbye message
                        send(clsock, msg, strlen(msg)+ 1, 0);

                    // Remove it from the master file list and close the socket
                        FD_CLR(clsock, &master);
                        closesocket(clsock);
                    }
                }
                else
                    perror("listen");
            }
            else
                perror("bind");
        }
        else
            perror("socket");

        #if defined (WIN32)
            WSACleanup();
        #endif

    }
    return EXIT_SUCCESS;
}
