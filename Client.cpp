#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

#pragma comment (lib, "ws2_32.lib") 

int main(int argc, char const* argv[])
{
    if (argc != 3)
    {
        std::cout << "Example: " << argv[0] << " <host> and <port>" << std::endl;
        return EXIT_FAILURE;
    }
    socket_wrapper::SocketWrapper sock_wrap;
    
    
    std::string host = argv[1];

    addrinfo hints =
    {
        .ai_flags = AI_CANONNAME,
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = 0
    };

    addrinfo* server_information = nullptr;
    
    if ((getaddrinfo(host.c_str(), nullptr, &hints, &server_information)) != 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    int socket_version;
    socklen_t addr_lenght;
    for (auto const* s = server_information; s != nullptr; s = s->ai_next)
    {
        if (AF_INET == s->ai_family) // type ipv4
        {
            socket_version = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (!socket_version)
            {
                std::cerr << sock_wrap.get_last_error_string() << std::endl;
                return EXIT_FAILURE;
            }
            in_addr ip_number{};
            int st = inet_pton(AF_INET, host.c_str(), &ip_number);
            if (st <= 0)
            {
                std::cerr << sock_wrap.get_last_error_string() << std::endl;
                return EXIT_FAILURE;
            }
            sockaddr_in addr =
            {
                .sin_family = PF_INET,
                .sin_port = htons(std::stoi(argv[2])),
            };
            addr.sin_addr = ip_number;
            addr_lenght = INET_ADDRSTRLEN;

            if (connect(socket_version, reinterpret_cast<const sockaddr*>(&addr), addr_lenght) != 0)
            {
                std::cerr << sock_wrap.get_last_error_string() << std::endl;
                return EXIT_FAILURE;
            }
        }
        else if (AF_INET6 == s->ai_family)                      // type ipv6
        {
            socket_version = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
            if (!socket_version)
            {
                std::cerr << sock_wrap.get_last_error_string() << std::endl;
                return EXIT_FAILURE;
            }
            in6_addr ip_number{};
            int st = inet_pton(AF_INET6, host.c_str(), &ip_number);
            if (st <= 0)
            {
                std::cerr << sock_wrap.get_last_error_string() << std::endl;
                return EXIT_FAILURE;
            }
            sockaddr_in6 addr =
            {
                .sin6_family = PF_INET6,
                .sin6_port = htons(std::stoi(argv[2])),
            };
            addr.sin6_addr = ip_number;
            addr_lenght = INET6_ADDRSTRLEN;

            if (connect(socket_version, reinterpret_cast<const sockaddr*>(&addr), addr_lenght) != 0)
            {
                std::cerr << sock_wrap.get_last_error_string() << std::endl;
                return EXIT_FAILURE;
            }
        }
        else
        {
            std::cout << s->ai_family << "\n";
        }
    }
    socket_wrapper::Socket sock = socket_version;
    ssize_t recv_len = 0;

    std::cout << "'exit' or 'file' \n";

    std::string message;
    char buffer[256];
    bool run = true;

    while (run)
    {
        std::getline(std::cin, message);
        send(sock, message.c_str(), message.size(), 0);
       
        std::cout << std::endl;

        if (message.find("file") == 0) 
        {
            while (true)
            {
                recv_len = recv(sock, buffer, sizeof(buffer) - 1, 0);
                buffer[recv_len] = '\0';
                std::string str = { buffer, 0, stic_cast<unsigned int>(recv_len) };
                if ("exit_file__" == str)
                {
                    break;
                }
                std::cout << str;
            }
            std::cout << std::endl;
            continue;
        }
        recv_len = recv(sock, buffer, sizeof(buffer) - 1, 0);

        if (recv_len > 0)
        {
            buffer[recv_len] = '\0';
            std::cout << buffer << '\n';
        }
        
        if (message == "exit") run = false;
    }
    sock.close();
    return EXIT_SUCCESS;
}

