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

#define PORT 8080
typedef struct client
{
	int client_name;
	bool private;
	int client_P;
} client;

int reinitializeSet(fd_set *readfds, SOCKET listening, client *clients,int max_clients)
{
	int max_sd=0;
	int sd;
	//efface le set de socket 
	FD_ZERO(readfds); 
	//ajoute listening socket au set 
	FD_SET(listening, readfds); 
	max_sd = listening; 
			
	//ajoute les sockets des clients au set 
	for (int i = 0 ; i < max_clients ; i++) 
	{ 
		sd = clients[i].client_name;					
		//verifie que la socket est valide et l'ajoute au set
		if(sd > 0) 
		{
			FD_SET( sd , readfds); 
		}								
		//cherche à obtenir le descripteur de fichier le plus élevé (utile pour la fonction select)
		if(sd > max_sd) 
		{
			max_sd = sd; 
		}
	}		
	return max_sd;
	
}
void addClient(SOCKET listening, client * clients, int max_clients)
{
	SOCKET new_socket;
	if ((new_socket = accept(listening,NULL, NULL))<0) 
	{ 
		printf("accept error"); 
	} 
	printf("Client %d arrive sur le chat\n", new_socket);
	//envoie un message de bienvenue au client
	char * welcomeMsg = "Bienvenue sur le chat!\r\n";
    send(new_socket, welcomeMsg, strlen(welcomeMsg), 0);							
					
	//ajoute new_socket au tableau des sockets clientes
	for (int i = 0; i < max_clients; i++) 
	{ 
		if( clients[i].client_name == 0 ) 
		{ 
			clients[i].client_name = new_socket;										
			break; 
		} 
	} 
}
bool isCommand(char * str)
{
	bool res=false;
	if(str[0]=='#')
	{
		res=true;
	}
	return res;
}
client findClient(int socket, client * clients,int max_clients)
{
	client res={-1,false,-1};
	for (int j = 0; j < max_clients; j++)
	{
		if(socket == clients[j].client_name && socket>0)
		{
			res = clients[j];
		}
	}
	return res;
}
void dealWithCommand(char * str, client * ptr_client, int listening, client * clients, int max_clients)
{		
	int sd= (*ptr_client).client_name;
	char delim[] = " ";
	char *cmd = strtok(str, delim);									
	if(strcmp(cmd,"exit")==0)
	{
		send(sd, "A bientot", 10, 0);
		closesocket(sd);
		printf("Client %d quitte le chat\n", sd);
		(*ptr_client).client_name=0;
		(*ptr_client).private=false;
		(*ptr_client).client_P=0;
	}
	else if(strcmp(cmd,"help")==0)
	{
		char help [1024]= "Voici la liste des commandes disponibles sur ce serveur:\n\n"
					"#listU : renvoie la liste des autres utilisateurs connectes (autres clients).\n"
                    "         Si il n'y en a aucun, le serveur renvoie : 'Aucun utilisateur actuellement en ligne.'\n"
                    "#listF : renvoie la liste des fichiers presents sur le serveur.\n"
                    "#trfU <> : permet de telecharger un fichier sur le serveur en mettant le nom de ce fichier a la place des <>.\n"
                    "#trfD <> : permet de tetecharger un fichier depuis le serveur en mettant le nom de ce fichier a la place des <>.\n"
                    "#private <> : permet d'activer la conversation prive avec un autre utilisateur. Pour se faire, il faut renseigner le nom de cet autre utilisateur a la place des <>.\n"
                    "#public : permet de repasser la conversation en mode publique si elle etait en prive.\n"
                    "#ring <> : suivi du nom de l'utilisateur concerne a la place des <> agit comme une commande ping, et permet de savoir si l'autre utilisateur est connecte.\n"
                    "Si la commande n'est pas reconnu, un message d'erreur sera renvoye.";
		send(sd, help, strlen(help), 0);
	}
	else if(strcmp(cmd,"listU")==0)
	{
		char strOut[1024];
		strOut[0]=0;
		strcat(strOut, "List users:\n");
		int compteur=0;
		for (int j = 0; j < max_clients; j++)
		{
			SOCKET sock = clients[j].client_name;
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
			strcat(strOut, "Aucun utilisateur actuellement en ligne\n");
		}
		strOut[strlen(strOut)-1]=0;
		send(sd, strOut, strlen(strOut), 0);
	}
	else if(strcmp(cmd,"listF")==0)
	{
	//TODO: list files of server 
	//Without a renseigned user, search file on the server 
	}
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
		char * user = strtok(NULL, delim);
		if(user != NULL)
		{
			int user_private = atoi(user);
			client dest= findClient(user_private,clients, max_clients);
			if(dest.client_name!=-1)
			{
				(*ptr_client).private=true;
			
				(*ptr_client).client_P=user_private;
				char validation[320];

				sprintf(validation,"vous etes maintenant en communication prive avec %d",user_private);
				send(sd, validation, strlen(validation), 0);	
			}
			else
			{
				char *error = "Veuillez specifier un utilisateur actuellement connecte.";
				send(sd, error, strlen(error), 0);
			}
															
		}
		else 
		{
			char *error = "Veuillez specifier l'utilisateur avec qui vous voulez passer en mode prive.";
			send(sd, error, strlen(error), 0);												
		}
	}
	else if(strcmp(cmd,"public")==0)
	{
		(*ptr_client).private=false;
		(*ptr_client).client_P=0;
		char *validation = "Retour en communication public";
		send(sd, validation, strlen(validation), 0);
	}
	else if (strcmp(cmd,"ring")==0)
	{
		char * user = strtok(NULL, delim);
		if(user != NULL)
		{
			int userToRing = atoi(user);
			client dest= findClient(userToRing,clients, max_clients);
			if(dest.client_name!=-1)
			{
				char validation[300];
				sprintf(validation,"l'utilisateur %d est connecte et a recu le ring.",userToRing);
				send(sd, validation, strlen(validation),0);
				char ringsent[300];
				sprintf(ringsent,"l'utilisateur %d vous a envoye un ring.",sd);
				send(userToRing, ringsent, strlen(ringsent),0);	
				printf("USER:%d> envoie un ring a %d\n",sd,userToRing);
			}
			else
			{
				char *notfound = "L'utilisateur que vous essayez de sonner n'est pas connecte.";
				send(sd, notfound, strlen(notfound), 0);
			}			
		}
		else
		{
			char *error = "Veuillez specifier l'utilisateur que vous voulez sonner.";
			send(sd, error, strlen(error), 0);
		}
	}
	else
	{
		char *error = "la commande specifie n'est pas reconnu. Veuillez utiliser la commande #help pour plus d'informations";
		send(sd, error, strlen(error), 0);
	}
}
void sendMessageToAll(char * msg, int listening, int src_socket, client * clients, int max_clients)
{
	char strOut[320];
	for (int i = 0; i < max_clients; i++)
	{
		if(clients[i].client_name!=0)
		{
			SOCKET outSock = clients[i].client_name;
			if (outSock != listening && outSock != src_socket)
			{                               
				sprintf(strOut,"USER:%d> %s",src_socket,msg);
				send(outSock, strOut, strlen(strOut), 0);
			}
		}
	}
}

void SendMessageToOne(char * msg, client * this_client) 
{
	char strOut[320];
	sprintf(strOut,"(private)USER:%d> %s",(*this_client).client_name,msg);
	send((*this_client).client_P, strOut, strlen(strOut), 0);
}
void getCommunication(fd_set *ptr_readfds, int listening, client * clients, int max_clients)
{	
	int sd=0;
	char buf[280];
	for (int i = 0; i < max_clients; i++) 
	{ 
		sd = clients[i].client_name;
		if (FD_ISSET( sd , ptr_readfds)) 
		{
			// Message reçu dans buf
			int bytesIn = recv(sd, buf, 280, 0);
			buf[bytesIn]=0;
			//Cas où le client quitte le chat
			if (bytesIn <= 0)
			{
				//Ferme la socket client et remplace par 0 dans la liste des clients
				closesocket(sd);
				printf("Client %d quitte le chat\n", sd);
				clients[i].client_name=0;
				clients[i].private=false;
				clients[i].client_P=0;
			} 										
			else
			{ 
				if(isCommand(buf))
				{
					char * str = (buf+1);
					dealWithCommand(str,&clients[i], listening, clients,max_clients);
				}
				
				else
				{
					client this_client=findClient(sd,clients,max_clients);					
					if(!this_client.private)
					{
						//message de la part d'un client
						printf("USER:%d> %s\n",sd,buf);
						//Envoie le message à tous les autres clients                       
						sendMessageToAll(buf,listening,sd,clients,max_clients);
					}
					else
					{
						printf("USER:%d> en communication prive avec %d\n",sd,this_client.client_P);
						SendMessageToOne(buf,&this_client);
					}
				}
			}									
		}
	} 
	buf[0]=0;
}

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
	client *clients=(client*) malloc(sizeof(client)*max_clients);	
	int max_sd, sd;
	int activity;	
	fd_set readfds;
				
	//initialise tous les sockets clients à 0
	for (int i = 0; i < max_clients; i++)   
	{   
		client new_client = {0, false, 0};
		clients[i] = new_client;
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
						max_sd=reinitializeSet(&readfds,listening,clients,max_clients);
						
						//attend pour une activité d'une socket du set  
						activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL); 
					
						if (activity < 0) 
						{ 
							printf("select error"); 
						} 
							
						//si l'activité provient de la listening socket, il s'agit d'un nouveau client
						if (FD_ISSET(listening, &readfds)) 
						{ 
							addClient(listening, clients, max_clients);
						} 
						//sinon il s'agit d'une communication de la part d'un client
						else{
							getCommunication(&readfds,listening,clients,max_clients);
						}
					}
				}
				else
					printf("listen error");
			}
			else
				printf("bind error");
		}
		else
			printf("socket error");
	}
	#if defined (WIN32)
		WSACleanup();
	#endif
	
    return EXIT_SUCCESS;
}
