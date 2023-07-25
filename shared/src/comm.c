#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <endian.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <arpa/inet.h>


#include "shared/inc/proto.h"
#include "shared/inc/comm.h"
#include "shared/inc/shared_util.h"
#include "shared/inc/tcp_ip.h"

char *transforma_estado_em_string(Estado *req)
{
    char *frame = (char *)malloc(MAX_FRAME_SIZE * sizeof(char));
    memcpy((void *)frame, (void *)req, sizeof(Estado));

    ((Estado *)frame)->ator_atual = htonl(req->ator_atual);
    ((Estado *)frame)->tempo_ultima_execucao = htonl(req->tempo_ultima_execucao);
    ((Estado *)frame)->num_andares = htonl(req->num_andares); // que é 2
    ((Estado *)frame)->estacionamento_fechado = htonl(req->estacionamento_fechado);
    ((Estado *)frame)->estacionamento_lotado = htonl(req->estacionamento_lotado);
    ((Estado *)frame)->num_vagas_andar_1 = htonl(req->num_vagas_andar_1);
    ((Estado *)frame)->andar_1_fechado = htonl(req->andar_1_fechado);
    ((Estado *)frame)->motor_cancela_entrada_ligado = htonl(req->motor_cancela_entrada_ligado);
    ((Estado *)frame)->motor_cancela_saida_ligado = htonl(req->motor_cancela_saida_ligado);
    ((Estado *)frame)->andar_1_lotado = htonl(req->andar_1_lotado);
    ((Estado *)frame)->vagas_andar_1 = htonl(req->vagas_andar_1);
    ((Estado *)frame)->num_vagas_andar_2 = htonl(req->num_vagas_andar_2);
    ((Estado *)frame)->andar_2_fechado = htonl(req->andar_2_fechado);
    ((Estado *)frame)->andar_2_lotado = htonl(req->andar_2_lotado);
    ((Estado *)frame)->vagas_andar_2 = htonl(req->vagas_andar_2);

    ((Estado *)frame)->sensor_de_presenca_entrada = htonl(req->sensor_de_presenca_entrada);
    ((Estado *)frame)->sensor_de_presenca_saida = htonl(req->sensor_de_presenca_saida);
    ((Estado *)frame)->sensor_de_passagem_entrada = htonl(req->sensor_de_passagem_entrada);
    ((Estado *)frame)->sensor_de_passagem_saida = htonl(req->sensor_de_passagem_saida);
    ((Estado *)frame)->sensor_de_subida_de_andar = htonl(req->sensor_de_subida_de_andar);
    ((Estado *)frame)->sensor_de_descida_de_andar = htonl(req->sensor_de_descida_de_andar);

    return frame;
}

Estado *parse_string_estado(char *res_str)
{
    Estado *e = (Estado *)res_str;
    Estado *res = (Estado *)malloc(sizeof(Estado));

    res->ator_atual = ntohl(e->ator_atual);
    res->tempo_ultima_execucao = ntohl(e->tempo_ultima_execucao);
    res->num_andares = ntohl(e->num_andares); // que é 2
    res->estacionamento_fechado = ntohl(e->estacionamento_fechado);
    res->estacionamento_lotado = ntohl(e->estacionamento_lotado);
    res->num_vagas_andar_1 = ntohl(e->num_vagas_andar_1);
    res->andar_1_fechado = ntohl(e->andar_1_fechado);
    res->motor_cancela_entrada_ligado = ntohl(e->motor_cancela_entrada_ligado);
    res->motor_cancela_saida_ligado = ntohl(e->motor_cancela_saida_ligado);
    res->andar_1_lotado = ntohl(e->andar_1_lotado);
    res->vagas_andar_1 = ntohl(e->vagas_andar_1);
    res->num_vagas_andar_2 = ntohl(e->num_vagas_andar_2);
    res->andar_2_fechado = ntohl(e->andar_2_fechado);
    res->andar_2_lotado = ntohl(e->andar_2_lotado);
    res->vagas_andar_2 = ntohl(e->vagas_andar_2);

    res->sensor_de_presenca_entrada = ntohl(e->sensor_de_presenca_entrada);
    res->sensor_de_presenca_saida = ntohl(e->sensor_de_presenca_saida);
    res->sensor_de_passagem_entrada = ntohl(e->sensor_de_passagem_entrada);
    res->sensor_de_passagem_saida = ntohl(e->sensor_de_passagem_saida);
    res->sensor_de_subida_de_andar = ntohl(e->sensor_de_subida_de_andar);
    res->sensor_de_descida_de_andar = ntohl(e->sensor_de_descida_de_andar);

    return res;
}
