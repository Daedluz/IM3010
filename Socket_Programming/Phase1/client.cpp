# include <iostream>
# include <sys/types.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <netdb.h>
# include <pthread.h>
# include <cstring>
# include <string>
# include <vector>

# define PORT 8080
# define BUFFERLEN 4096

using namespace std;

int user_register(int &sock, char buffer[BUFFERLEN])
{
    string userName;
    char status[3];
    cout << "What username do you want ? \n> ";
    getline(cin, userName);
    userName = "REGISTER#" + userName;

    send(sock, userName.c_str(), userName.size()+1, 0);
    int bytes_recv = recv(sock, buffer, BUFFERLEN, 0);
    if (bytes_recv > 0) // get response
        copy(buffer, buffer+3, status);
    memset(buffer, 0, BUFFERLEN); // clear buffer

    return stoi(string(status));
}

int userLogin(int &sock, char buffer[BUFFERLEN])
{
    string userName;
    cout<<"Username : \n> ";
    getline(cin, userName);
    string req = userName + "#" + to_string(PORT);

    send(sock, req.c_str(), req.size()+1, 0);
    int bytes_recv = recv(sock, buffer, BUFFERLEN, 0);
    // REVIEW Is it a legit way to check response ?
    if (bytes_recv == 14)
        cout<<"\nLogin failed.\n";
    return bytes_recv;
}

int main()
{
    int sock = 0;
    char buffer[BUFFERLEN] = {0};
    string userInput;
    struct sockaddr_in serv_addr;

    int acc_bal = 0, online_num = 0;
    string serv_pk;
    // [Account name, IP address, Port number]
    vector<vector<string>> user_list;

    // Create socket
    if( (sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout<<"\n Socket creation error \n";
        return -1;
    }

    // Define server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if ( inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0 )
    {
        cout<<"\nInvalid address\n";
        return -1;
    }

    // Connect to server
    int connectRes = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (connectRes < 0)
    {
        cout<<"\nConnection failed.\n";
        return -1;
    }

    // TODO Create a thread to listen to other user's message
    
    // Listen to user inputs to perform actions

    while (true)
    {
        userInput = "";
        cout<<"> ";
        getline(cin, userInput);
        // QUESTION Can't use string in switch cases 😢
        if (userInput == "register")
        {
            int status = user_register(sock, buffer);
            if (status == 100)
                cout<<"Register successful !\n";
            else
                cout<<"Register failed\n";
        }
        else if (userInput == "login")
        {
            int res_len = userLogin(sock, buffer);
            // Read the list

        }
        else if (userInput == "exit")
        {
            cout<<"\nClosing connection\n";
            close(sock);
            break;
        }
        else
        {
            cout<<"Couldn't recognize command :(\n";
        }
    }


    return 0;
}