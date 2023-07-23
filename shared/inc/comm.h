#ifndef COMM_H
#define COMM_H 1

#include "shared/inc/proto.h"

char *transforma_estado_em_string(Estado *req);
Estado *parse_string_estado(char *res_str);

#endif