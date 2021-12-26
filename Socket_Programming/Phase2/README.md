# Socket Programming Phase 2
## B08705051 陳旻浚

## Environment
Ubuntu 20.04 LTS

## How to Use
Use `make` to compile server and client  
then run `./server.out ${Port}` to start server  
and `./client.out ${Server IP} ${Server Port}` to run the client.

## Commands (for client)
- `register`: Register a username.
- `login`: Login with a username and a port to listen to.
- `list`: Request the latest online list from the server.
- `transaction`: Transfer money to a peer.
- `exit`: End the client program.

## ToDo

- [x] Listen to assigned port
- [x] Create a thread pool
- [x] Assign each user to a thread  
    Inside the thread :
    - [x] Register
    - [x] Login
    - [x] Transaction
    - [x] List
    - [x] Exit
- [x] Set max connection to 3 and test it

