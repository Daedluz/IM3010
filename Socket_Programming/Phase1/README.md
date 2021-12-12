# Socket Programming Phase 1
## B08705051 陳旻浚

## Environment
Ubuntu 20.04 LTS

## How to compile
`g++ g++ client.cpp -o client.out -pthread`

## Functions : 
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

## How to use

`cd ./Socket_Programming/Phase1`  
`./client.out {server ip} {server port}`

- register : register a new user, the program will ask you what username you want.
- login : login as a user, the program will ask you what port number you want to listen on for peer messages.
- list : Request for the newest online list and server public key.
- transaction : transfer money to a peer. The prgram will ask you who you want to transfer money to, and how much money you want to transfer.
- exit : close connection.

## References
https://www.youtube.com/watch?v=WDn-htpBlnU&ab_channel=SloanKelly  
https://www.geeksforgeeks.org/socket-programming-cc/