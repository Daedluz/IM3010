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
# include "ctpl_stl.h"

# define BUFFERLEN 4096

const int max_connection = 3;
const std::string serv_pk = "public_key";
// All registered usernames and their account balance
std::vector<std::vector<std::string>> register_list;
std::vector<std::vector<std::string>> online_list;
std::vector<int> online_sock;

int connection_cnt = 0;

struct Socket_connection
{
    int socket;
    std::string client_ip;
};

std::vector<std::string> split (std::string s, std::string delimiter) 
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

void handle_register (int socket, std::string username)
{
    std::string msg;
    for (int i=0; i<register_list.size(); i++)
    {
        if (username == register_list[i][0])
        {
            std::cout<<"\nUsername already registered\n";
            msg = "210 FAIL\n";
            send(socket, msg.c_str(), msg.size()+1, 0);
            return;
        }
    }
    std::vector<std::string> user;
    user.push_back(username);
    user.push_back("10000");
    register_list.push_back(user);
    msg = "100 OK\n";
    send(socket, msg.c_str(), msg.size()+1, 0);
    return;
}

void handle_list (int socket, std::string username)
{
    int acc_bal;
    for (int i=0; i<register_list.size(); i++)
    {
        if (username == register_list[i][0])
        {
            acc_bal = stoi(register_list[i][1]);
            break;
        }
    }
    std::string msg;
    msg = msg + std::to_string(acc_bal) + "\n";
    msg = msg + serv_pk + "\n";
    msg = msg + std::to_string(online_list.size()) + "\n";
    for (int i=0; i<online_list.size(); i++)
    {
        for (int j=0; j<online_list[0].size(); j++)
        {
            if (j==0)
                msg = msg + online_list[i][j];
            else
                msg = msg + "#" + online_list[i][j];
        }
        msg = msg + "\n";
    }
    send(socket, msg.c_str(), msg.size() + 1, 0);

    return;
}

std::string handle_login (int socket, std::string username, std::string ip_addr, int port_num)
// Return the account balance of user
{
    std::string msg = "";
    // If already logged in, can't log in
    for (int i=0; i<online_list.size(); i++)
    {
        if (username == online_list[i][0])
        {
            msg = "200 AUTH_FAIL\n";
            send(socket, msg.c_str(), msg.size()+1, 0);
            
            return "";
        }
    }


    for (int i=0; i<register_list.size(); i++)
    {
        if (username == register_list[i][0])
        {
            // Update online_user
            std::vector<std::string> user;
            user.push_back(username);
            user.push_back(ip_addr);
            user.push_back(std::to_string(port_num));
            online_list.push_back(user);

            // Send online list to client (in the ugly format)
            msg = msg + register_list[i][1] + "\n";
            msg = msg + serv_pk + "\n";
            msg = msg + std::to_string(online_list.size()) + "\n";
            for (int i=0; i<online_list.size(); i++)
            {
                for (int j=0; j<online_list[0].size(); j++)
                {
                    if (j==0)
                        msg = msg + online_list[i][j];
                    else
                        msg = msg + "#" + online_list[i][j];
                }
                msg = msg + "\n";
            }
            send(socket, msg.c_str(), msg.size() + 1, 0);
            
            return register_list[i][0];
        }
    }

    msg = "200 AUTH_FAIL\n";
    send(socket, msg.c_str(), msg.size()+1, 0);
    
    return "";
}

void handle_transaction (int socket, std::string peer, int value, std::string username)
{
    // Calculate account balance
    for (int i=0; i<register_list.size(); i++)
    {
        if (register_list[i][0] == username)
        {
            register_list[i][1] = std::to_string (stoi(register_list[i][1]) + value);
        }
        else if (register_list[i][0] == peer)
        {
            register_list[i][1] = std::to_string (stoi(register_list[i][1]) - value);
        }
    }

    // Send message to peer
    int id;
    for (int i=0; i<online_list.size(); i++)
    {
        if (online_list[i][0] == peer)
        {
            id = i;
            break;
        }
    }
    send(online_sock[id], "Transfer OK!\n", 14, 0);
    return;
}

void process_user (int id, Socket_connection connection)
// args stored the socket to client and the client's IP address
{
    int socket = connection.socket;
    std::string client_ip = connection.client_ip; 
    char buffer[BUFFERLEN] = {0};
    bool logged_in = false;
    std::string client_name;
    while (true)
    {
        // recv and process request from user
        int recv_len = read(socket, buffer, BUFFERLEN);
        std::cout<<buffer<<std::endl;
        std::vector<std::string> request = split(buffer, "#");
        if (request.size() == 3)
        {
            handle_transaction(socket, request[0], stoi(request[1]), request[2]);
        }
        else if (request[0] == "REGISTER")
        {
            handle_register(socket, request[1]);
        }
        else if (request.size() == 2 )
        // First argument is username, second argument is portNum
        {
            client_name = handle_login(socket, request[0], client_ip, stoi(request[1]));
            if (client_name != "")
            {
                logged_in = true;
            }
        }
        else if (request[0] == "List")
        {
            if (!logged_in)
            {
                std::cout<<"\nPlease login first !\n";
            }
            handle_list(socket, client_name);
        }
        else if (request[0] == "Exit")
        {
            // Remove user from online list
            int index;
            for (int i=0; i<online_list.size(); i++)
            {
                if (online_list[i][0] == client_name)
                {
                    index = i;
                }
            }
            online_list.erase(online_list.begin() + index);
            online_sock.erase(online_sock.begin() + index);

            connection_cnt = connection_cnt - 1;

            send(socket, "Bye\n", 4, 0);
            break;
        }
        else 
        {
            std::cout<<"\nCouldn't recognize command 😥\n";
        }

    }
    close(socket);
}



int main(int argc, char *argv[])
// Argument is port number
{
    // Create socket
    struct sockaddr_in addr, client_addr;
    int listen_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_sock <= 0)
    {
        std::cout << "\nCreate socket failed.\n";
        return 0;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)); 

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(atoi(argv[1]));
    inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
    bind(listen_sock, (struct sockaddr *)&addr, sizeof(addr));
    socklen_t addr_size = sizeof(addr);

    if (listen(listen_sock, max_connection) < 0)
    {
        std::cout<<"\nError listening to socket\n";
        return 0;
    }

    std::cout<<"Server listening on port "<<argv[1]<<std::endl;

    // create thread pool
    ctpl::thread_pool pool(max_connection);

    while (true)
    {
        if (connection_cnt >= max_connection)
        {
            // std::cout<<"Exceed max connection.\n";
            continue;
        }
        else
        {
            // Build a new connection
            int connection = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_size);
            if (connection < 0)
            {
                std::cout<<"\nFailed to establish connection\n";
                continue;
            }
            online_sock.push_back(connection);
            char ip_address[INET_ADDRSTRLEN]; 
            Socket_connection conn;
            conn.socket = connection;
            conn.client_ip = std::string(inet_ntop(AF_INET, &(client_addr.sin_addr), ip_address, INET_ADDRSTRLEN));
            pool.push(process_user, conn);
            
            connection_cnt ++;
            std::cout<<connection_cnt<<std::endl;
        }

    }

    close(listen_sock);

    return 0;
}