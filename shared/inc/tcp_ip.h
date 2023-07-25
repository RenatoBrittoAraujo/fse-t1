#ifndef TCP_IP_H
#define TCP_IP_H 1

#define MAX_FRAME_SIZE 10000

#include "shared/inc/errors.h"

t_error call_tcp_ip_port(char *request, size_t req_size, char *ip, int port, char *res_buff);

// get_response -> your handler, takes in a request and res_data, returns your response
// req          -> read only. pointer to request that is received
// res_data     -> write only. pointer to data that will be used to generate response.
//                 may be null. you can pass anything for your get_response function to use.
t_error listen_tcp_ip_port(char *ip, int port, char *(*get_response)(void *, void *), void *req, void *res_data);

#endif