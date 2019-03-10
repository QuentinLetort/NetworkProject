# NetworkProject

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
