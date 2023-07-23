#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shared/inc/proto.h"
#include "shared/inc/comm.h"
#include "shared/inc/shared_util.h"
#include "shared/inc/tcp_ip.h"

char *tranformar_request_em_string(EstadoEstacionamento *req)
{
    char *frame = (char *)malloc(MAX_FRAME_SIZE * sizeof(char));
    memcpy(frame, &req, sizeof(frame));
    return frame;
}

EstadoEstacionamento *parse_string_resposta(char *res_str)
{
    EstadoEstacionamento *res = (EstadoEstacionamento *)malloc(sizeof(EstadoEstacionamento));
    memcpy(res_str, &res, sizeof(res_str));
    return res;
}
