#ifndef COMM_H
#define COMM_H 1

#include "shared/inc/proto.h"

MensagemIn *monta_request(EstadoEstacionamento *e);
char *tranformar_request_em_string(MensagemIn *req);
MensagemOut *parse_string_resposta(char *res_str);

#endif