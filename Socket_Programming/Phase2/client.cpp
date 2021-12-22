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

# define BUFFERLEN 4096

using namespace std;

struct args 
{
    int* serv_sock;
    int port_num;
};

void * listen_peer_message (void * arg)
// arg would store 1.sock to server 2.port number the client will be listening on
{
    struct args *params = (struct args*)arg;
    struct sockaddr_in addr;
    int new_sock=0, recv_len=0;
    char buffer[BUFFERLEN]={0};
    int listening = socket(PF_INET, SOCK_STREAM, 0);
    if (listening <= 0)
    {
        cout << "\nCreate listening socket failed.\n";
        return NULL;
    }
    int opt = 1;
    setsockopt(listening, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)); 

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(params->port_num);
    inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
    bind(listening, (struct sockaddr *)&addr, sizeof(addr));
    socklen_t addr_size = sizeof(addr);

    while(true)
    {
        listen(listening, 10);
        // cout<<"Listening on port "<<params->port_num<<endl;
        new_sock = accept(listening, (struct sockaddr *)&addr, &addr_size);
        cout<<"\nYou have a transaction request : ";
        recv_len = read(new_sock, buffer, BUFFERLEN);
        cout<<buffer<<"\n> ";
        send(*params->serv_sock, buffer, recv_len+1, 0);
        close(new_sock);
    }

    return NULL;
}

void user_transaction(int &sock, char buffer[BUFFERLEN], string userName, vector<vector<string>> user_list)
{
    // DONE FIXME Can't connect to peer 
    string target, value, ip_addr, port, msg;
    cout<<"\nWho do you wanna transfer money to ?\n> ";
    getline(cin, target);
    cout<<"\nHow much money do you want to transfer ?\n> ";
    getline(cin, value);
    if (target == "" || value == "")
    {
        cout<<"Empty input !\n";
        return;
    }
    bool found = false;
    for (int i=0; i<user_list.size(); i++)
    {
        if (target == user_list[i][0])
        {
            ip_addr = user_list[i][1];
            port = user_list[i][2];
            found = true;
            break;
        }
    }
    if (!found)
    {
        cout<<"\nPeer not found, try \"list\" command and try again.\n";
        return;
    }
    msg = userName + "#" + value + "#" + target;
    // cout<<msg<<endl;

    // Connect to peer socket
    struct sockaddr_in peer_addr;
    int peer_sock = socket(PF_INET, SOCK_STREAM, 0);
    if( peer_sock < 0)
    {
        cout<<"\n Peer socket creation error \n";
        return ;
    }
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(stoi(port));
    inet_pton(AF_INET, "0.0.0.0", &peer_addr.sin_addr); // ip_addr.c_str()
    int connect_result = connect(peer_sock, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
    if( connect_result < 0)
    {
        cout<<"\n Peer connection error \n";
        return ;
    }

    // send message to peer
    send(peer_sock, msg.c_str(), msg.size()+1, 0);
    close(peer_sock);
    // receive confirm message from server
    int bytes_recv = recv(sock, buffer, BUFFERLEN, 0);
    cout<<buffer<<endl;
    memset(buffer, 0, BUFFERLEN);
    return;
}

int user_register(int &sock, char buffer[BUFFERLEN])
{
    string userName;
    char status[3];
    cout << "What username do you want ? \n> ";
    getline(cin, userName);
    if (userName == "")
    {
        cout<<"Empty input !\n";
        return 210;
    }
    userName = "REGISTER#" + userName;

    send(sock, userName.c_str(), userName.size()+1, 0);
    int bytes_recv = recv(sock, buffer, BUFFERLEN, 0);
    if (bytes_recv > 0) // get response
        copy(buffer, buffer+3, status);
    // cout<<buffer;
    memset(buffer, 0, BUFFERLEN); // clear buffer

    return stoi(string(status));
}

int userLogin(int &sock, char buffer[BUFFERLEN], string userName, int listen_port)
{
    string req = userName + "#" + to_string(listen_port);

    send(sock, req.c_str(), req.size()+1, 0);
    int bytes_recv = recv(sock, buffer, BUFFERLEN, 0);
    // REVIEW Is it a legit way to check response ?
    if (bytes_recv == 14)
    {
        cout<<"\nLogin failed.\n";
        bytes_recv = 0;
    }
    else 
        cout<<"\nLogin Successful !\n";
    return bytes_recv;
}


// for string delimiter
vector<string> split (string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

int main(int argc, char *argv[])
// argv[1] is server ip address, argv[2] is server port number
{
    int sock = 0;
    char buffer[BUFFERLEN] = {0};
    string userInput, userName;
    struct sockaddr_in serv_addr;
    bool login = false;

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
    int serv_port = atoi(argv[2]);
    serv_addr.sin_port = htons(serv_port);

    if ( inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0 )
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
            if (login)
            {
                cout<<"You've already logged in as "<<userName<<endl;
                continue;
            }
            string port_num;
            cout<<"Username : \n> ";
            getline(cin, userName);
            cout<<"Port number : \n> ";
            getline(cin, port_num);
            if (userName == "" || port_num == "")
            {
                cout<<"Empty input !\n";
                continue;
            }
            int res_len = userLogin(sock, buffer, userName, stoi(port_num));
            if (res_len > 0) // if login successful
            {
                // DONE TODO Create a thread to listen to other user's message
                pthread_t tid;
                pthread_attr_t attr;

                pthread_attr_init (&attr);
                struct args param = {&sock, stoi(port_num)};
                
                pthread_create (&tid, &attr, listen_peer_message, &param);


                // Read the list
                char str[res_len];
                copy (buffer, buffer+res_len, str);
                string tmp_str = string(str);
                // cout<<str;
                vector<string> tmp = split(tmp_str, "\n");
                acc_bal = stoi(tmp[0]);
                serv_pk = tmp[1];
                online_num = stoi(tmp[2]);
                for (int i=3; i<3+online_num; i++)
                {
                    user_list.push_back(split(tmp[i], "#"));
                }

                cout<<"\nAccount balance is : "<<acc_bal<<endl;
                cout<<"Server's public key is : "<<serv_pk<<endl;
                cout<<online_num<<endl;
                cout<<"User list :"<<endl;
                for (int i=0; i<user_list.size(); i++)
                {
                    for (int j=0; j<user_list[0].size(); j++)
                    {
                        cout<< user_list[i][j]<<" ";
                    }
                    cout<<endl;
                }
                cout<<endl;

                memset(buffer, 0, BUFFERLEN); // clear buffer
                login = true;
            }
            
        }
        else if (userInput == "transaction")
        {
            user_transaction(sock, buffer, userName, user_list);
        }

        else if (userInput == "list")
        {
            if (login == false)
            {
                cout<<"\nPlease login first !\n";
                continue;
            }
            send(sock, "List", 5, 0);
            int res_len = recv(sock, buffer, BUFFERLEN, 0);
            // TODO Make a function to read user_list
            user_list.clear();
            char str[res_len];
            copy (buffer, buffer+res_len, str);
            string tmp_str = string(str);
            vector<string> tmp = split(tmp_str, "\n");
            acc_bal = stoi(tmp[0]);
            serv_pk = tmp[1];
            online_num = stoi(tmp[2]);
            for (int i=3; i<3+online_num; i++)
            {
                user_list.push_back(split(tmp[i], "#"));
            }

            cout<<"\nAccount balance is : "<<acc_bal<<endl;
            cout<<"Server's public key is : "<<serv_pk<<endl;
            cout<<online_num<<endl;
            cout<<"User list :"<<endl;
            for (int i=0; i<user_list.size(); i++)
            {
                for (int j=0; j<user_list[0].size(); j++)
                {
                    cout<< user_list[i][j]<<" ";
                }
                cout<<endl;
            }
            cout<<endl;

            memset(buffer, 0, BUFFERLEN); // clear buffer
        }
        
        else if (userInput == "exit")
        {
            send(sock, "Exit", 5, 0);
            recv(sock, buffer, BUFFERLEN, 0);
            cout<<"\n"<<buffer;
            cout<<"\nClosing connection\n";
            close(sock);
            memset(buffer, 0, BUFFERLEN); // clear buffer
            break;
        }
        else
        {
            cout<<"Couldn't recognize command :(\n";
        }
    }

    return 0;
}