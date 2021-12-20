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

const int max_connection = 10;

void process_user (int id)
{
    while (true)
    {
        // recv and process request from user
    }
}

int main(int argc, char *argv[])
// Argument is port number
{
    int current_user = 0;
    // Create socket
    struct sockaddr_in addr;
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
        if (current_user >= max_connection)
        {
            continue;
        }
        int connection = accept(listen_sock, (struct sockaddr *)&addr, &addr_size);
        if (connection < 0)
        {
            std::cout<<"\nFailed to establish connection\n";
            continue;
        }

        pool.push();

        current_user ++;

    }

    close(listen_sock);

    return 0;
}