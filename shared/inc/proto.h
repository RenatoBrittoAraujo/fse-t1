#ifndef PROTO_H
#define PROTO_H 1

#include <stdlib.h>

#include "shared/inc/threads.h"
#include "shared/inc/time.h"

#define RASP_ESTACIONAMENTO_1_3 1
#define SENSOR_DE_PASSAGEM_DESATIVADO 0
#define ESPERANDO_OUTRO_SENSOR 1

#define MCS_MAXIMO_CANCELA_ABERTA 10 * SECOND *MILLI
#define MCS_DESCONEXAO 10 * SECOND *MILLI
#define MCS_PERIODO_MINIMO_EXEC 50 * MILLI
#define MCS_DEADLINE_RESPOSTA_DEPENDENTE 500 * MILLI

#define ATOR_MAIN 1
#define ATOR_DEP1 2
#define ATOR_DEP2 3

// ======= CONTROLE

struct EstadoEstacionamento
{
    // identificador de qual ator executou a ultima acao
    // do estado atual: ATOR_MAIN, ATOR_DEP1, ATOR_DEP2
    int ator_atual;

    // identificador de quando foi exectuado a ultima acao
    int tempo_ultima_execucao;

    // dados servidor principal
    int num_andares; // que é 2
    int estacionamento_fechado;
    int estacionamento_lotado;

    // entradas para andar 1
    int num_vagas_andar_1;
    int andar_1_fechado;
    int motor_cancela_entrada_ligado;
    int motor_cancela_saida_ligado;

    // saidas do andar 1
    int andar_1_lotado;
    int vagas_andar_1;
    time_t sensor_de_presenca_entrada;
    time_t sensor_de_presenca_saida;
    time_t sensor_de_passagem_entrada;
    time_t sensor_de_passagem_saida;

    // entradas para andar 2
    int num_vagas_andar_2;
    int andar_2_fechado;

    // entradas do andar 2
    int andar_2_lotado;
    int vagas_andar_2;
    time_t sensor_de_subida_de_andar;
    time_t sensor_de_descida_de_andar;

    // comum
    int porta_andar_1;
    int porta_andar_2;
    char *ip_andar_1;
    char *ip_andar_2;

    ThreadState *t_main;
    ThreadState *t_dep_1;
    ThreadState *t_dep_2;
};
typedef struct EstadoEstacionamento EstadoEstacionamento;

// ======= INTERFACE
struct EstadoInterface
{
    EstadoEstacionamento *e;
};
typedef struct EstadoInterface EstadoInterface;

EstadoEstacionamento *inicializar_estado(char *env_name, int ator_atual);

EstadoEstacionamento *copiar_estado(EstadoEstacionamento *e);

#endif