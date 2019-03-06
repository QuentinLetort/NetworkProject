# NetworkProject

Simple Server/Client connection using sockets

## Usage

### Windows

Open the command prompt, change directory to the current one and launch the serveur by using:
```sh
❯ gcc -o server server.c -lws2_32
```
Then
```sh
❯ server
```
On another command prompt, launch the client by using:
```sh
❯ gcc -o client client.c -lws2_32
```
Then
```sh
❯ client
```

### Linux

TODO

## Steps to do 

#### Possibility Send and receive Messages (len max 280 characters by message) (client to server, client to client)
Ex: Simple chat 
https://medium.com/@yashitmaheshwary/simple-chat-server-using-sockets-in-c-f72fc8b5b24e
https://www.youtube.com/watch?v=dquxuXeZXgo
http://www.theinsanetechie.in/2014/01/a-simple-chat-program-in-c-tcp.html
#### Possibility to transfers files (jpeg) (client to client , client to server)
https://gist.github.com/ozcanovunc/203a6479dd5cbc322d69
https://github.com/zTrix/simple-ftp
https://www.go4expert.com/forums/simple-implementation-ftp-c-t14938/
ftp://ftp.cs.umass.edu/pub/net/pub/kurose/ftpserver.c
#### Possibility to Multi-Threads more clients(50) for a server
see C9 on brightspace
#### Server connection need a username and password 
see the tuto on brightspace in the project folder
#### Log file append when user connect(login, @IP, date, …)
#### Command line:

1. Exit
1. Help (list Command)L
1. ListU (list user in a server)
1. ListF (list files in a server)
1. TrfU ( transfert Upload file in a server)
1. TrfD ( transfert Download file in a server)
1. Private <user> (commute to private ) 
1. Public (commute to public )
1. Ring <user> (notification if user is connect)

    
