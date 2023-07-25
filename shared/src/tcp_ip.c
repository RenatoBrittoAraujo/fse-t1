#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <shared/inc/shared_util.h>
#include <shared/inc/tcp_ip.h>

#define DEBUG 0
#define IF_DEBUG if(DEBUG)

// res_buff may be NULL, response will not be set
t_error call_tcp_ip_port(char *request,size_t req_size, char *ip, int port, char *res_buff)
{
    IF_DEBUG log_print("[shared.tcp_ip] [call_tcp_ip_port] call_tcp_ip_port()\n", LEVEL_DEBUG);
    IF_DEBUG log_print("[shared.tcp_ip] [call_tcp_ip_port] ():1\n", LEVEL_DEBUG);

    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[MAX_FRAME_SIZE];
    IF_DEBUG log_print("[shared.tcp_ip] [call_tcp_ip_port] ():2\n", LEVEL_DEBUG);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        IF_DEBUG log_print("[shared.tcp_ip] [call_tcp_ip_port] ():ERRO1\n", LEVEL_DEBUG);
        return handle_error(SHARED_TCP_IP_ERROR_SOCKET_CREATION_FAILED, "[shared.tcp_ip] [call_tcp_ip_port] socket creation failed");
    }

    IF_DEBUG log_print("[shared.tcp_ip] [call_tcp_ip_port] ():3\n", LEVEL_DEBUG);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    IF_DEBUG log_print("[shared.tcp_ip] [call_tcp_ip_port] ():4\n", LEVEL_DEBUG);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
    {
        IF_DEBUG log_print("[shared.tcp_ip] [call_tcp_ip_port] ():ERRO2\n", LEVEL_DEBUG);
        return handle_error(SHARED_TCP_IP_ERROR_INVALID_ADDRESS, "[shared.tcp_ip] [call_tcp_ip_port] address not found / invalid address");
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        IF_DEBUG log_print("[shared.tcp_ip] [call_tcp_ip_port] ():ERRO3\n", LEVEL_DEBUG);
        return SHARED_TCP_IP_ERROR_CONNECTION_FAILED;
    }

    IF_DEBUG printf("SENDING BYTES: %u\n", req_size);
    IF_DEBUG printf("SENDING: %s\n", request);

    IF_DEBUG log_print("[shared.tcp_ip] [call_tcp_ip_port] ():5\n", LEVEL_DEBUG);
    send(sock, request, req_size, 0);
    IF_DEBUG log_print("[shared.tcp_ip] [call_tcp_ip_port] ():6\n", LEVEL_DEBUG);
    valread = read(sock, buffer, MAX_FRAME_SIZE);
    IF_DEBUG log_print("[shared.tcp_ip] [call_tcp_ip_port] ():7\n", LEVEL_DEBUG);
    if (buffer != NULL)
    {
        IF_DEBUG log_print("[shared.tcp_ip] [call_tcp_ip_port] ():8\n", LEVEL_DEBUG);
        IF_DEBUG printf("copiando res_buffer no endereco %p\n", res_buff);
        fflush(NULL);
        memcpy(res_buff, buffer, MAX_FRAME_SIZE);
    }
    IF_DEBUG log_print("[shared.tcp_ip] [call_tcp_ip_port] ():9\n", LEVEL_DEBUG);
    return NO_ERROR;
}

t_error listen_tcp_ip_port(char *ip, int port, char *(*get_response)(void *, void *), void *req, void *res_data)
{
    IF_DEBUG log_print("[shared.tcp_ip] [listen_tcp_ip_port] listen_tcp_ip_port\n", LEVEL_DEBUG);

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[MAX_FRAME_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        return handle_error(SHARED_TCP_IP_ERROR_SOCKET_CREATION_FAILED, "[shared.tcp_ip] [listen_tcp_ip_port] socket creation failed");
    }
    IF_DEBUG log_print("[shared.tcp_ip] [listen_tcp_ip_port] socket opened\n", LEVEL_DEBUG);

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEADDR, &opt, sizeof(opt)))
    {
        return handle_error(SHARED_TCP_IP_ERROR_SOCKOPT_FAILED, "[shared.tcp_ip] [listen_tcp_ip_port] setsockopt failed");
    }
    IF_DEBUG log_print("[shared.tcp_ip] [listen_tcp_ip_port] socket options set\n", LEVEL_DEBUG);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        return handle_error(SHARED_TCP_IP_ERROR_BIND_FAILED, "[shared.tcp_ip] [listen_tcp_ip_port] bind failed");
    }
    IF_DEBUG log_print("[shared.tcp_ip] [listen_tcp_ip_port] socket binded\n", LEVEL_DEBUG);

    if (listen(server_fd, 3) < 0)
    {
        return handle_error(SHARED_TCP_IP_ERROR_LISTEN_FAILED, "[shared.tcp_ip] [listen_tcp_ip_port] listen failed");
    }
    IF_DEBUG log_print("[shared.tcp_ip] [listen_tcp_ip_port] listen worked!\n", LEVEL_DEBUG);

    while (1)
    {
        IF_DEBUG log_print("[shared.tcp_ip] [listen_tcp_ip_port] waiting for connection\n", LEVEL_DEBUG);
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            return handle_error(SHARED_TCP_IP_ERROR_ACCEPT_FAILED, "[shared.tcp_ip] [listen_tcp_ip_port] accept failed");
        }
        IF_DEBUG log_print("[shared.tcp_ip] [listen_tcp_ip_port] request received\n", LEVEL_DEBUG);

        valread = read(new_socket, buffer, MAX_FRAME_SIZE);

        IF_DEBUG log_print("[shared.tcp_ip] [listen_tcp_ip_port] valread\n", LEVEL_DEBUG);

        IF_DEBUG printf("[shared.tcp_ip] [listen_tcp_ip_port] read %d bytes!\n", valread);
        IF_DEBUG fflush(NULL);

        memcpy(req, buffer, MAX_FRAME_SIZE);

        char *response = get_response(req, res_data);
        IF_DEBUG log_print("[shared.tcp_ip] [listen_tcp_ip_port] response created\n", LEVEL_DEBUG);

        send(new_socket, &response, strlen(response), 0);
        IF_DEBUG log_print("[shared.tcp_ip] [listen_tcp_ip_port] response sent\n", LEVEL_DEBUG);

        free(response);
        close(new_socket);
    }

    return NO_ERROR;
}
