#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <fstream>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

#pragma comment (lib, "ws2_32.lib")

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper socket_wrap;
    const int port { std::stoi(argv[1]) };
    socket_wrapper::Socket listenf = {AF_INET, SOCK_STREAM, IPPROTO_TCP};

    std::cout << "Starts on port: " << port << "\n";

    if (!listenf)
    {
        std::cerr << socket_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    sockaddr_in addr =
    {
        .sin_family = PF_INET,
        .sin_port = htons(port),
    };

    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenf, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        std::cerr << socket_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    if (listen(listenf, SOMAXCONN) != 0)
    {
        std::cerr << socket_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    char buffer[256];

    sockaddr_in client_address = {0};
    socklen_t client_address_len = sizeof(sockaddr_in);
    ssize_t recv_len = 0;
    std::cout << "Server to run\n";
    char client_address_buf[INET_ADDRSTRLEN];

    socket_wrapper::Socket socket = accept(listenf, reinterpret_cast<sockaddr*>(&client_address), &client_address_len);
    if (!socket)
    {
        std::cerr << socket_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }
    listenf.close();
    bool run = true;

    while (run)
    {
        recv_len = recv(socket, buffer, sizeof(buffer) - 1, 0);

        if (recv_len > 0)
        {
            buffer[recv_len] = '\0';

            std::cout
                << "Address: "
                << inet_ntop(AF_INET, &client_address.sin_addr, client_address_buf, sizeof(client_address_buf) / sizeof(client_address_buf[0]))
                << ":" << ntohs(client_address.sin_port)
                << "\n"
                << buffer
                << "\n\n";
    
            char hbuf[NI_MAXHOST];
            if ((getnameinfo(reinterpret_cast<sockaddr*>(&client_address), client_address_len, hbuf, sizeof(hbuf) - 1, nullptr, 0, NI_NAMEREQD)) != 0)
            {
                std::cout << WSAGetLastError() << '\n';
                std::cerr << "could not resolve hostname" << std::endl;
            }
            else
                std::cout << hbuf << '\n';

            std::string command_string = { buffer, 0, static_cast<unsigned int>(recv_len) };
            if (command_string.find("file") == 0)
            {
                std::string file_path(command_string, 5, recv_len);
                std::cout << file_path;
                std::ifstream ifs(file_path);
                std::string line;
                if (ifs.is_open())
                {
                    while (getline(ifs, line))
                    {
                        send(socket, line.c_str(), line.size(), 0);
                    }
                    line = "exit_file__";
                    ifs.close();
                }
                else
                {
                    line = "error open file \n";
                    send(socket, line.c_str(), line.size(), 0);
                    line = "exit_file__";
                }
                send(socket, line.c_str(), line.size(), 0);
                continue;
            }

            send(socket, buffer, recv_len, 0);
            if ("exit" == command_string)
                run = false;
        }

        std::cout << std::endl;
    }
    socket.close();
    return EXIT_SUCCESS;
}

