#if !defined(_GNU_SOURCE)
#   define _GNU_SOURCE
#endif

//
// System calls interceptor for the networking spoiling...
//

extern "C"
{
#include <dlfcn.h>
#include "unistd.h"
}

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ws2tcpip.h>
#include <sstream>
#pragma comment (lib, "ws2_32.lib")

FILE* file;

static void init (void) __attribute__ ((constructor));

typedef ssize_t (*write_t)(int fd, const void *buf, size_t count);
typedef int (*socket_t)(int domain, int type, int protocol);
typedef int (*close_t)(int fd);

static close_t old_close;
static socket_t old_socket;
static write_t old_write;

static int socket_fd = -1;

void init(void)
{
    srand(time(nullptr));
    printf("Interceptor library loaded.\n");

    old_close = reinterpret_cast<close_t>(dlsym(RTLD_NEXT, "close"));
    old_write = reinterpret_cast<write_t>(dlsym(RTLD_NEXT, "write"));
    old_socket = reinterpret_cast<socket_t>(dlsym(RTLD_NEXT, "socket"));
}
bool message(const char* char_buf)
{
    char str[4];
    for (int i = 0; i < 4; ++i)
    {
        str[i] = char_buf[i];
    }
    if (str =="USER" || str =="PASS")
    {
        return true;
    }
    return false;
}

extern "C"
{

int close(int fd)
{
    if (fd == socket_fd)
    {
        printf("> close() on the socket was called!\n");
        socket_fd = -1;
    }

    return old_close(fd);
}


ssize_t write(int fd, const void *buf, size_t count)
{
    auto char_buf = reinterpret_cast<const char*>(buf);

    if (char_buf && (count > 1) && (fd == socket_fd))
    {

        printf("> write() on the socket was called with a string!\n");
        printf("New buffer = [");
                                    
        file = fopen("intercepter.txt", "a+");          // task 1, WRITING TO THE FILE
        fprintf(file, char_buf);                        // writing to the file 
        fclose(file);

        printf("]\n");
    }

    return old_write(fd, buf, count);
}


int socket(int domain, int type, int protocol)
{
    int cur_socket_fd = old_socket(domain, type, protocol);

    if (-1 == socket_fd)
    {
        printf("> socket() was called, fd = %d!\n", cur_socket_fd);
        socket_fd = cur_socket_fd;
    }
    else
    {
        printf("> socket() was called, but socket was opened already...\n");
    }

    return cur_socket_fd;
}

} // extern "C"

