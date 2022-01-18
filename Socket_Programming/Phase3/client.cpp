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
# include <sys/types.h>
# include <stdlib.h>
# include <openssl/ssl.h>
# include <openssl/err.h>

# define BUFFERLEN 4096

using namespace std;

void load_cert(SSL_CTX* ctx, string option)
{
    string pid = to_string(getpid());
    char key_path[1024];
    char cert_path[1024];
    sprintf(key_path, "./cert/%s_%s.key", pid.c_str(), option.c_str());
    sprintf(cert_path, "./cert/%s_%s.crt", pid.c_str(), option.c_str());

    if (SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    if ( SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    if ( !SSL_CTX_check_private_key(ctx) )
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}

string configure_ctx (SSL_CTX* ctx, string option)
{
    string pid = to_string(getpid());
    cout<< pid;
    char cmd[1024];
    sprintf(cmd, "openssl req -x509 -sha256 -nodes -days 365 -newkey rsa:2048 -keyout ./cert/%s_%s.key -out ./cert/%s_%s.crt -subj \"/C=TW/ST=Taiwan/L=TPE/O=NTU/OU=IM/CN=Michael/emailAddress=\"", pid.c_str(), option.c_str(), pid.c_str(), option.c_str());

    system(cmd);
    cout<<"Certificate generated.";

    load_cert(ctx, option);

    return pid;
}

struct args 
{
    SSL *serv_ssl;
    int port_num;
};

void * listen_peer_message (void * arg)
// arg would store 1.ssl to server 2.port number the client will be listening on
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    // SSL *peer_ssl;
    const SSL_METHOD *meth = SSLv23_server_method();
    SSL_CTX *ctx = SSL_CTX_new (meth);
    // peer_ssl = SSL_new (ctx);
    // DONE TODO configure cert
    configure_ctx(ctx, "serv");


    struct args *params = (struct args*)arg;
    struct sockaddr_in addr;
    int recv_len=0;
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
    listen(listening, 10);
    cout<<"Listening on port "<<params->port_num<<endl;

    while(true)
    {
        int client_sock = accept(listening, (struct sockaddr *)&addr, &addr_size);
        if (client_sock < 0)
        {
            cout<<"\nFailed to establish connection\n";
            continue;
        }

        // SSL *client_ssl;
        // client_ssl = SSL_new(ctx);
        

        // if(!SSL_set_fd(client_ssl, client_sock))
        // {
        //     cout<<"Set_fd failed\n";
        // }
        // else
        // {
        //     cout<<"Set_fd successfully\n";
        // }
        // int err = SSL_accept(client_ssl);
        // cout<<endl<<err<<endl;
        // if (err <= 0)
        // {
        //     cout<<"Create ssl connection to peer failed.\n";
        //     ERR_print_errors_fp(stderr);
        //     close(client_sock);
        //     continue;
        // }
        // else
        // {
        //     cout<<"Connected to peer, receiving message.\n";
        // }

        cout<<"\nYou have a transaction request : ";
        // recv_len = SSL_read(client_ssl, buffer, BUFFERLEN);
        recv_len = read(client_sock, buffer, BUFFERLEN);
        cout<<buffer<<"\n> ";
        SSL_write(params->serv_ssl, buffer, recv_len+1);
        close(client_sock);
        // SSL_shutdown(client_ssl);
        // SSL_free(client_ssl);
    }
    
    SSL_CTX_free(ctx);

    return NULL;
}

void user_transaction(int sock, char buffer[BUFFERLEN], string userName, vector<vector<string>> user_list)
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
    // Create peer socket
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

    // DONE TODO Create SSL connection
    SSL *peer_ssl;
    const SSL_METHOD *meth = SSLv23_client_method();
    SSL_CTX *ctx = SSL_CTX_new (meth);

    // DONE TODO Configure context with key and certificate
    configure_ctx(ctx, "peer");

    peer_ssl = SSL_new (ctx);

    // if (!peer_ssl) {
    //     printf("Error creating SSL.\n");
    //     return;
    // }

    // int err = SSL_set_fd(peer_ssl, peer_sock);
    // if (!err)
    // {
    //     cout<<"Error getting socket file descriptor\nAborting...\n";
    //     return;
    // }

    // err = SSL_connect(peer_ssl);
    // if (err < 0)
    // {
    //     cout<<"Error when creating SSL connection: "<< err <<endl;
    //     cout<<"Aborting...";
    //     return;
    // }


    // send message to peer
    cout<<msg<<endl;
    // SSL_write(peer_ssl, msg.c_str(), msg.size()+1);
    send(peer_sock, msg.c_str(), msg.size()+1, 0);
    // Close SSL connection
    close(peer_sock);
    SSL_shutdown(peer_ssl);
    SSL_free(peer_ssl);
    

    // receive confirm message from server
    // int bytes_recv = SSL_read(sock, buffer, BUFFERLEN);
    int bytes_recv = recv(sock, buffer, BUFFERLEN,0);
    cout<<buffer<<endl;
    memset(buffer, 0, BUFFERLEN);
    return;
}

int user_register(SSL *sock, char buffer[BUFFERLEN])
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

    SSL_write(sock, userName.c_str(), userName.size()+1);
    int bytes_recv = SSL_read(sock, buffer, BUFFERLEN);
    if (bytes_recv > 0) // get response
        copy(buffer, buffer+3, status);
    // cout<<buffer;
    memset(buffer, 0, BUFFERLEN); // clear buffer

    return stoi(string(status));
}

int userLogin(SSL *sock, char buffer[BUFFERLEN], string userName, int listen_port)
{
    string req = userName + "#" + to_string(listen_port);

    SSL_write(sock, req.c_str(), req.size()+1);
    int bytes_recv = SSL_read(sock, buffer, BUFFERLEN);
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
    SSL *ssl;
    int sock = 0, ssl_sock = 0;
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

    // REF https://stackoverflow.com/questions/41229601/openssl-in-c-socket-connection-https-client
    // Initialize SSL library and register algorithms
    SSL_library_init();
    // Basically the same with the last one
    OpenSSL_add_all_algorithms();
    // Register error strings for `libcrypto` and `libssl`
    SSL_load_error_strings();
    // General SSL/TLS methods
    const SSL_METHOD *meth = SSLv23_client_method();
    // Hold default value for later SSL connection creation
    SSL_CTX *ctx = SSL_CTX_new (meth);
    ssl = SSL_new (ctx);

    // DONE TODO Configure context with key and certificate
    configure_ctx(ctx, "client");

    if (!ssl) {
        printf("Error creating SSL.\n");
        return -1;
    }

    ssl_sock = SSL_get_fd(ssl);
    int err = SSL_set_fd(ssl, sock);
    if (!err)
    {
        cout<<"Error getting socket file descriptor\nAborting...\n";
        return -1;
    }

    err = SSL_connect(ssl);
    if (err < 0)
    {
        ERR_print_errors_fp(stderr);
        cout<<"Error when creating SSL connection: "<< err <<endl;
        cout<<"Aborting...";
        return -1;
    }

    cout<<"SSL connection established:\n"<<SSL_get_cipher(ssl)<<endl;

    
    // Listen to user inputs to perform actions

    while (true)
    {
        userInput = "";
        cout<<"> ";
        getline(cin, userInput);
        // QUESTION Can't use string in switch cases 😢
        if (userInput == "register")
        {
            int status = user_register(ssl, buffer);
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
            int res_len = userLogin(ssl, buffer, userName, stoi(port_num));
            if (res_len > 0) // if login successful
            {
                // DONE TODO Create a thread to listen to other user's message
                pthread_t tid;
                pthread_attr_t attr;

                pthread_attr_init (&attr);
                struct args param = {ssl, stoi(port_num)};
                
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
            SSL_write(ssl, "List", 5);
            int res_len = SSL_read(ssl, buffer, BUFFERLEN);
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
            SSL_write(ssl, "Exit", 5);
            SSL_read(ssl, buffer, BUFFERLEN);
            cout<<"\n"<<buffer;
            cout<<"\nClosing connection\n";
            close(sock);
            memset(buffer, 0, BUFFERLEN); // clear buffer
            SSL_shutdown(ssl);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            break;
        }
        else
        {
            cout<<"Couldn't recognize command :(\n";
        }
    }

    return 0;
}