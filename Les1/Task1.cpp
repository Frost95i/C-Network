#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#pragma comment (lib, "ws2_32.lib")
// ������

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

// Trim from end (in place).
static inline std::string& rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) { return !std::isspace(c); }).base());
    return s;
}


int main(int argc, char const *argv[])
{
  

    socket_wrapper::SocketWrapper sock_wrap;
    const int port = 1230;
    socket_wrapper::Socket sock = {AF_INET, SOCK_DGRAM, IPPROTO_UDP};

    std::cout << "Starting echo server on the port " << port << "...\n";

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    sockaddr_in addr =
    {
        .sin_family = PF_INET,
        .sin_port = htons(port),
    };

    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        // Socket will be closed in the Socket destructor.
        return EXIT_FAILURE;
    }

    char buffer[256];

    // socket address used to store client address
    sockaddr_in client_address = {0};
    socklen_t client_address_len = sizeof(sockaddr_in);
    ssize_t recv_len = 0;
    std::cout << "Running echo server...\n" << std::endl;
    char client_address_buf[INET_ADDRSTRLEN];

    while (true)
    {
        // Read content into buffer from an incoming client.
        recv_len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                            reinterpret_cast<sockaddr *>(&client_address),
                            &client_address_len);

        if (recv_len > 0)
        {
            buffer[recv_len] = '\0';

            std::cout
                << "Client with address "
                << inet_ntop(AF_INET, &client_address.sin_addr, client_address_buf, sizeof(client_address_buf) / sizeof(client_address_buf[0]))
                << ":" << ntohs(client_address.sin_port)
                << " sent datagram "
                << "[length = "
                << recv_len
                << "]:\n'''\n"
                << buffer
                << "\n'''"
                << std::endl;
                
            sockaddr* addr_server = (sockaddr*)&(client_address.sin_addr);
            
            socklen_t addrlen = INET_ADDRSTRLEN;
            char r_buffer[NI_MAXHOST];
            if ((getnameinfo(addr_server, client_address_len, r_buffer, sizeof(r_buffer) - 1, nullptr, 0, NI_NAMEREQD)) != 0)
            {
                std::cout << WSAGetLastError() << '\n';             
                std::cerr << "could not resolve hostname" << std::endl;
            }
            else
                std::cout << r_buffer << '\n';

            unsigned int r_len = recv_len;
            std::string command_string = { buffer, 0, r_len };
            rtrim(command_string);
            
            if ("exit" == command_string) run = false;
                        // Send same content back to the client ("echo").
            sendto(sock, buffer, recv_len, 0, reinterpret_cast<const sockaddr *>(&client_address),
                   client_address_len);
        }

        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}

