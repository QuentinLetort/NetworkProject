//Fichier standard au sytème d'exploitation (windows ou linux)
#if defined (WIN32)
    #include <winsock2.h>
	#include <unistd.h>
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
	int max_clients=50;
	int client_socket[max_clients];	
	int max_sd, sd;
	int activity;
	SOCKET new_socket;
	fd_set readfds;
				
	//initialise tous les sockets clients à 0
	for (int i = 0; i < max_clients; i++)   
	{   
		client_socket[i] = 0;   
	}
    
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
					int compteur=0;
					while(true) 
					{ 
						//efface le set de socket 
						FD_ZERO(&readfds); 
					
						//ajoute listening socket au set 
						FD_SET(listening, &readfds); 
						max_sd = listening; 
						
						//ajoute les sockets des clients au set 
						for (int i = 0 ; i < max_clients ; i++) 
						{ 
							sd = client_socket[i]; 							
							//verifie que la socket est valide et l'ajoute au set
							if(sd > 0) 
							{
								FD_SET( sd , &readfds); 
							}								
							//cherche à obtenir le descripteur de fichier le plus élevé (utile pour la fonction select)
							if(sd > max_sd) 
							{
								max_sd = sd; 
							}
						} 
					
						//attend pour une activité d'une socket du set  
						activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL); 
					
						if (activity < 0) 
						{ 
							perror("select"); 
						} 
							
						//si l'activité provient de la listening socket, il s'agit d'un nouveau client
						if (FD_ISSET(listening, &readfds)) 
						{ 
							if ((new_socket = accept(listening,NULL, NULL))<0) 
							{ 
								perror("accept"); 
							} 
							printf("Client %d arrive sur le chat\n", new_socket);
							//envoie un message de bienvenue au client
							char * welcomeMsg = "Bienvenue sur le chat!\r\n";
                            send(new_socket, welcomeMsg, strlen(welcomeMsg), 0);							
								
							//ajoute new_socket au tableau des sockets clientes
							for (int i = 0; i < max_clients; i++) 
							{ 
								if( client_socket[i] == 0 ) 
								{ 
									client_socket[i] = new_socket;										
									break; 
								} 
							} 
						} 
						else
						{	
							//sinon il s'agit d'une communication de la part d'un client
							for (int i = 0; i < max_clients; i++) 
							{ 
								sd = client_socket[i];
								
								if (FD_ISSET( sd , &readfds)) 
								{ 
									char buf[280];

									// Message reçu dans buf
									int bytesIn = recv(sd, buf, 280, 0);
									buf[bytesIn]=0;
									//Cas où le client quitte le chat
									if (bytesIn <= 0)
									{
										//Ferme la socket client et remplace par 0 dans la liste des clients
										closesocket(sd);
										printf("Client %d quitte le chat\n", sd);
										client_socket[i]=0;
									} 										
									else
									{ 
										if(buf[0]=='#')
										{
											char * str = (buf+1);
											char delim[] = " ";

											char *cmd = strtok(str, delim);										
											if(strcmp(cmd,"exit")==0)
											{
												send(sd, "A bientot", 10, 0);
												closesocket(sd);
												printf("Client %d quitte le chat\n", sd);
												client_socket[i]=0;
											}
											else if(strcmp(cmd,"help")==0)
											{
												//TODO: show commands
											}
											else if(strcmp(cmd,"listU")==0)
											{
												char strOut[]="List users:\n";
												int compteur=0;
												for (int j = 0; j < max_clients; j++)
												{
													SOCKET sock = client_socket[j];
													if (sock != listening && sock != sd && sock>0)
													{                               
														char strUser[30];
														sprintf(strUser,"User: %d\n",sock);
														strcat(strOut, strUser);
														compteur++;
													}
												}
												if(compteur==0)
												{
													printf("%d",compteur);
													strcat(strOut, "Aucun utilisateur actuellement en ligne\n");
												}
												strOut[strlen(strOut)-1]=0;
												send(sd, strOut, strlen(strOut), 0);
											}
											else if(strcmp(cmd,"listF")==0)
											{
												char strOut[]="List users:\n";
												int compteur=0;
												for (int j = 0; j < max_clients; j++)
												{
													SOCKET sock = client_socket[j];
													if (sock != listening && sock != sd && sock>0)
													{                               
														char strUser[30];
														sprintf(strUser,"User: %d\n",sock);
														strcat(strOut, strUser);
														compteur++;
													}
												}
												if(compteur==0)
												{
													printf("%d",compteur);
													strcat(strOut, "Aucun utilisateur actuellement en ligne\n");
												}
												strOut[strlen(strOut)-1]=0;
												send(sd, strOut, strlen(strOut), 0);
											}
											//TODO: list files of server 
												//Without a renseigned user, search file on the server 
											
											else if(strcmp(cmd,"trfU")==0)
											{
												//TODO: upload a file to the server
											}
											else if(strcmp(cmd,"trfD")==0)
											{
												//TODO: Download a file from the server
											}
											else if(strcmp(cmd,"private")==0)
											{
												//TODO: commute to private conversation with user 
											}
											else if(strcmp(cmd,"public")==0)
											{
												//TODO: commute to public if client was in private conversation
											}
											else if (strcmp(cmd,"ring")==0)
											{
												//TODO: send to user concerned that a client ring him and to client if user is connect
											}
											else
											{
												//TODO: send to client>cmd incorrect
											}											
										}
										else
										{
											//message de la part d'un client
											printf("USER:%d> %s\n",sd,buf);
											//Envoie le message à tous les autres clients                       
											for (int j = 0; j < max_clients; j++)
											{
												if(client_socket[j]!=0)
												{
													SOCKET outSock = client_socket[j];
													if (outSock != listening && outSock != sd)
													{                               
														char strOut[320];
														sprintf(strOut,"USER:%d> %s",sd,buf);
														send(outSock, strOut, strlen(strOut), 0);
													}
												}
												
											}
										}
										
									}									
								}
							} 
						}
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
