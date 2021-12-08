# Socket Programming Phase 1

## Client Side

### Functions : 
- Register to server
- Log in
- Ask server for the newest :
    - Account balance
    - Online list
        - how many people is online
        - Username, IP address, and port number
    - Public Key
- Listen to server's response
- Transactions **between** clients (Talk to other client)
- Tell server before exiting the system

### How to use
`cd ./Socket_Programming/Phase1`  
`./client.out {server ip} {server port}`

#### Commands
- register : register a new user, the program will ask you what username you want.
- login : login as a user, the program will ask you what port number you want to listen on for peer messages.
- list : Request for the newest online list and server public key.
- transaction : transfer money to a peer. The prgram will ask you who you want to transfer money to, and how much money you want to transfer.
- exit : close connection.