#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shared/inc/proto.h"
#include "shared/inc/comm.h"
#include "shared/inc/shared_util.h"
#include "shared/inc/tcp_ip.h"

char *transforma_estado_em_string(Estado *req)
{
    char *frame = (char *)malloc(MAX_FRAME_SIZE * sizeof(char));
    memcpy(frame, &req, sizeof(frame));
    return frame;
}

Estado *parse_string_estado(char *res_str)
{
    Estado *res = (Estado *)malloc(sizeof(Estado));
    memcpy(res_str, &res, sizeof(res_str));
    return res;
}
