#ifndef TCP_IP_H
#define TCP_IP_H 1

#define MAX_FRAME_SIZE 10000

#define SHARED_TCP_IP_ERROR_SOCKET_CREATION_FAILED 1000
#define SHARED_TCP_IP_ERROR_INVALID_ADDRESS 1001
#define SHARED_TCP_IP_ERROR_CONNECTION_FAILED 1002
#define SHARED_TCP_IP_ERROR_BIND_FAILED 1003
#define SHARED_TCP_IP_ERROR_SOCKOPT_FAILED 1004
#define SHARED_TCP_IP_ERROR_LISTEN_FAILED 1005
#define SHARED_TCP_IP_ERROR_ACCEPT_FAILED 1006

#include "shared/inc/errors.h"

t_error call_tcp_ip_port(char *request, size_t req_size, char *ip, int port, char *res_buff);

// get_response -> your handler, takes in a request and res_data, returns your response
// req          -> read only. pointer to request that is received
// res_data     -> write only. pointer to data that will be used to generate response.
//                 may be null. you can pass anything for your get_response function to use.
t_error listen_tcp_ip_port(char *ip, int port, char *(*get_response)(void *, void *, int *), void *req, void *res_data);

#endif