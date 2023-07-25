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
    log_print("transforma_estado_em_string v3.0", LEVEL_DEBUG);

    char *frame = (char *)malloc(MAX_FRAME_SIZE * sizeof(char));
    memcpy((void *)frame, (void *)req, sizeof(Estado));

    ((Estado *)frame)->ator_atual = hton32(req->ator_atual);
    ((Estado *)frame)->tempo_ultima_execucao = hton32(req->tempo_ultima_execucao);
    ((Estado *)frame)->num_andares = hton32(req->num_andares); // que é 2
    ((Estado *)frame)->estacionamento_fechado = hton32(req->estacionamento_fechado);
    ((Estado *)frame)->estacionamento_lotado = hton32(req->estacionamento_lotado);
    ((Estado *)frame)->num_vagas_andar_1 = hton32(req->num_vagas_andar_1);
    ((Estado *)frame)->andar_1_fechado = hton32(req->andar_1_fechado);
    ((Estado *)frame)->motor_cancela_entrada_ligado = hton32(req->motor_cancela_entrada_ligado);
    ((Estado *)frame)->motor_cancela_saida_ligado = hton32(req->motor_cancela_saida_ligado);
    ((Estado *)frame)->andar_1_lotado = hton32(req->andar_1_lotado);
    ((Estado *)frame)->vagas_andar_1 = hton32(req->vagas_andar_1);
    ((Estado *)frame)->num_vagas_andar_2 = hton32(req->num_vagas_andar_2);
    ((Estado *)frame)->andar_2_fechado = hton32(req->andar_2_fechado);
    ((Estado *)frame)->andar_2_lotado = hton32(req->andar_2_lotado);
    ((Estado *)frame)->vagas_andar_2 = hton32(req->vagas_andar_2);

    ((Estado *)frame)->sensor_de_presenca_entrada = hton64(req->sensor_de_presenca_entrada);
    ((Estado *)frame)->sensor_de_presenca_saida = hton64(req->sensor_de_presenca_saida);
    ((Estado *)frame)->sensor_de_passagem_entrada = hton64(req->sensor_de_passagem_entrada);
    ((Estado *)frame)->sensor_de_passagem_saida = hton64(req->sensor_de_passagem_saida);
    ((Estado *)frame)->sensor_de_subida_de_andar = hton64(req->sensor_de_subida_de_andar);
    ((Estado *)frame)->sensor_de_descida_de_andar = hton64(req->sensor_de_descida_de_andar);

    return frame;
}

Estado *parse_string_estado(char *res_str)
{
    log_print("parse_string_estado v3.0", LEVEL_DEBUG);

    Estado *e = (Estado *)res_str;
    Estado *res = (Estado *)malloc(sizeof(Estado));

    res->ator_atual = ntoh32(e->ator_atual);
    res->tempo_ultima_execucao = ntoh32(e->tempo_ultima_execucao);
    res->num_andares = ntoh32(e->num_andares); // que é 2
    res->estacionamento_fechado = ntoh32(e->estacionamento_fechado);
    res->estacionamento_lotado = ntoh32(e->estacionamento_lotado);
    res->num_vagas_andar_1 = ntoh32(e->num_vagas_andar_1);
    res->andar_1_fechado = ntoh32(e->andar_1_fechado);
    res->motor_cancela_entrada_ligado = ntoh32(e->motor_cancela_entrada_ligado);
    res->motor_cancela_saida_ligado = ntoh32(e->motor_cancela_saida_ligado);
    res->andar_1_lotado = ntoh32(e->andar_1_lotado);
    res->vagas_andar_1 = ntoh32(e->vagas_andar_1);
    res->num_vagas_andar_2 = ntoh32(e->num_vagas_andar_2);
    res->andar_2_fechado = ntoh32(e->andar_2_fechado);
    res->andar_2_lotado = ntoh32(e->andar_2_lotado);
    res->vagas_andar_2 = ntoh32(e->vagas_andar_2);

    res->sensor_de_presenca_entrada = ntoh64(e->sensor_de_presenca_entrada);
    res->sensor_de_presenca_saida = ntoh64(e->sensor_de_presenca_saida);
    res->sensor_de_passagem_entrada = ntoh64(e->sensor_de_passagem_entrada);
    res->sensor_de_passagem_saida = ntoh64(e->sensor_de_passagem_saida);
    res->sensor_de_subida_de_andar = ntoh64(e->sensor_de_subida_de_andar);
    res->sensor_de_descida_de_andar = ntoh64(e->sensor_de_descida_de_andar);

    return res;
}
