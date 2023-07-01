#ifndef __G_NETWORKING_SOCKET_WRAPPER_H__
#define __G_NETWORKING_SOCKET_WRAPPER_H__

#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif

/* socket */
#ifdef _WIN32
typedef SOCKET g_socket_t;
#else
typedef int g_socket_t;
#endif

#ifdef _WIN32
#define g_socket_close closesocket
#else
#define g_socket_close close
#endif

#ifdef _WIN32
#define g_INVALID_SOCKET (INVALID_SOCKET)
#else
#define g_INVALID_SOCKET (-1)
#endif

int g_socket_env_init(){
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return -1;
    }
	
	return 0;
#else
	return 0;
#endif
}

void g_socket_env_destroy(){
#ifdef _WIN32
	WSACleanup();
#endif
}

#define g_socket_socket \
	socket
	
#define g_socket_listen \
	listen
	
#define g_socket_accept(sock, addr, addrLen) \
	accept(sock, (struct sockaddr*)&(addr), &(addrLen))

#define g_socket_bind(sock, addr) \
	bind(sock, (struct sockaddr*)&(addr), sizeof(addr))
	
#define g_socket_connect(sock, serverAddr) \
	connect(sock, (struct sockaddr*)&(serverAddr), sizeof(serverAddr))
	
#define g_socket_build_addr(addr, addr_sin_family, ip, port) \
	do{                                                      \
		memset(&(addr), 0, sizeof(addr));                    \
		addr.sin_family = (addr_sin_family);                 \
		addr.sin_port = htons(port);                         \
		addr.sin_addr.s_addr = inet_addr(ip);                \
	}while(0)
	
#define g_socket_build_addr_any(addr, addr_sin_family, port)   \
	do{                                                        \
		memset(&(addr), 0, sizeof(addr));                      \
		addr.sin_family = (addr_sin_family);                   \
		addr.sin_port = htons(port);                           \
		addr.sin_addr.s_addr = htonl(INADDR_ANY);              \
	}while(0)

#define g_socket_send(socket, msg, msgLen) \
	send(socket, msg, (msgLen), 0)
	
#define g_socket_recv(socket, buf, bufLen) \
	recv(socket, buf, (bufLen), 0)

#endif
