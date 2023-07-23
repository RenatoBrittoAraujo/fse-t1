#ifndef COMM_H
#define COMM_H 1

#include "shared/inc/proto.h"

char *tranformar_request_em_string(EstadoEstacionamento *req);
EstadoEstacionamento *parse_string_resposta(char *res_str);

#endif