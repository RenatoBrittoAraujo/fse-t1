#include <string.h>
#include <stdio.h>

#include "shared/inc/proto.h"
#include "shared/inc/shared_util.h"

EstadoEstacionamento *inicializar_estado(char *env_name, int ator_atual)
{
    log_print("incializar_estado\n", LEVEL_DEBUG);
    char buff[1000];

    sprintf(buff, "IP_%s", env_name);
    char *ip = read_env_str_index(buff, -1);

    sprintf(buff, "PORTA_%s", env_name);
    int porta = read_env_int_index(buff, -1);

    int num_andares = read_env_int_index("NUM_ANDARES", -1);
    int id_andar_da_entrada = read_env_int_index("ID_ANDAR_DA_ENTRADA", -1);

    EstadoEstacionamento *e = (EstadoEstacionamento *)malloc(sizeof(EstadoEstacionamento));

    e->ator_atual = ator_atual;

    Endereco *end = (Endereco *)malloc(sizeof(Endereco));

    end->ip = ip;
    end->porta = porta;
    e->endereco = end;

    // força o tempo inicial a sempre estar invalido
    e->tempo_ultima_execucao = -MCS_DESCONEXAO - 1;
    e->num_andares = num_andares;
    e->andares = (EstadoAndar **)malloc(sizeof(EstadoAndar *) *
                                        num_andares);
    e->sensor_de_descida_de_andar = (int *)malloc(sizeof(int) * (num_andares - 1));
    e->sensor_de_subida_de_andar = (int *)malloc(sizeof(int) * (num_andares - 1));
    e->estacionamento_fechado = 0;
    e->estacionamento_lotado = 0;

    EstadoEntrada *entr = e->entrada = (EstadoEntrada *)malloc(sizeof(EstadoEntrada));

    entr->id_andar = id_andar_da_entrada;

    for (int id_andar = 1; id_andar <= num_andares; id_andar++)
    {
        int num_vagas = read_env_int_index("NUM_VAGAS_ANDAR_", id_andar);
        char *ip_server_andar = read_env_str_index("IP_ANDAR_", id_andar);
        int porta_server_andar = read_env_int_index("PORTA_ANDAR_", id_andar);

        EstadoAndar *a = e->andares[id_andar - 1];
        e->andares[id_andar - 1] = a = (EstadoAndar *)malloc(sizeof(EstadoAndar));

        a->id_andar = id_andar;
        a->num_vagas = num_vagas;
        a->vagas = (int *)malloc(sizeof(int) * num_vagas);
        a->endereco = (Endereco *)malloc(sizeof(Endereco));
        a->endereco->ip = ip_server_andar;
        a->endereco->porta = porta_server_andar;
        a->andar_fechado = 0;
        a->andar_lotado = 0;

        EstadoEstacionamento *modificado_dep2;
    }

    e->t_dep_1 = e->t_dep_2 = e->t_main = NULL;

    return e;
}

EstadoEstacionamento *copiar_estado(EstadoEstacionamento *e)
{
    log_print("[PROTO.C] copiar_estado\n", LEVEL_DEBUG);
    EstadoEstacionamento *ne = (EstadoEstacionamento *)malloc(sizeof(EstadoEstacionamento));
    memcpy(ne, e, sizeof(EstadoEstacionamento));
    return ne;
}
