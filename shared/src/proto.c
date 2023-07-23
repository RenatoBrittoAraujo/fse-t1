#include <string.h>
#include <stdio.h>

#include "shared/inc/proto.h"
#include "shared/inc/shared_util.h"

Estado *inicializar_estado(char *env_name, int ator_atual)
{
    Estado *e = (Estado *)malloc(sizeof(Estado));

    e->ator_atual = ator_atual;
    e->tempo_ultima_execucao = -MCS_DESCONEXAO - 1;
    e->num_andares = 2;

    e->estacionamento_fechado = 0;
    e->estacionamento_lotado = 0;

    e->ip_andar_1 = read_env_str_index("IP_ANDAR_1", -1);
    e->porta_andar_1 = read_env_int_index("PORTA_ANDAR_1", -1);

    e->ip_andar_2 = read_env_str_index("IP_ANDAR_2", -1);
    e->porta_andar_2 = read_env_int_index("PORTA_ANDAR_2", -1);

    e->estacionamento_fechado = 0;
    e->estacionamento_lotado = 0;
    e->num_vagas_andar_1 = 8;
    e->vagas_andar_1 = 0;
    e->andar_1_fechado = 0;
    e->andar_1_lotado = 0;

    e->motor_cancela_entrada_ligado = 0;
    e->motor_cancela_saida_ligado = 0;
    e->sensor_de_presenca_entrada = 0;
    e->sensor_de_presenca_saida = 0;
    e->sensor_de_passagem_entrada = 0;
    e->sensor_de_passagem_saida = 0;

    e->num_vagas_andar_2 = 0;
    e->vagas_andar_2 = 0;
    e->andar_2_fechado = 0;
    e->sensor_de_subida_de_andar = 0;
    e->sensor_de_descida_de_andar = 0;
    e->andar_2_lotado = 0;

    e->t_dep_1 = e->t_dep_2 = e->t_main = NULL;

    return e;
}

Estado *copiar_estado(Estado *e)
{
    log_print("[PROTO.C] copiar_estado\n", LEVEL_DEBUG);
    Estado *ne = (Estado *)malloc(sizeof(Estado));
    memcpy(ne, e, sizeof(Estado));
    return ne;
}
