#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shared/inc/proto.h"
#include "shared/inc/comm.h"
#include "shared/inc/shared_util.h"
#include "shared/inc/tcp_ip.h"

MensagemIn *monta_request(EstadoEstacionamento *e)
{
    MensagemIn *req = (MensagemIn *)malloc(sizeof(MensagemIn));
    return req;
}

char *tranformar_request_em_string(MensagemIn *req)
{
    char *frame = (char *)malloc(MAX_FRAME_SIZE * sizeof(char));
    memcpy(frame, &req, sizeof(frame));
    return frame;
}

MensagemOut *parse_string_resposta(char *res_str)
{
    MensagemOut *res = (MensagemOut *)malloc(sizeof(MensagemOut));
    memcpy(res_str, &res, sizeof(res_str));
    return res;
}
