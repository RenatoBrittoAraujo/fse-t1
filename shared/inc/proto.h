#ifndef PROTO_H
#define PROTO_H 1

#include <stdlib.h>

#include "shared/inc/threads.h"
#include "shared/inc/time.h"

#define RASP_ESTACIONAMENTO_1_3 1
#define SENSOR_DE_PASSAGEM_DESATIVADO 0
#define ESPERANDO_OUTRO_SENSOR 1

#define TEMPO_MAXIMO_CANCELA_ABERTA 10 * SECOND
#define TEMPO_MAXIMO_DE_DESCONEXAO 10 * SECOND
#define PERIODO_MINIMO_EXEC 50

// ======= CONTROLE
struct Endereco
{
    char *ip;
    int porta;
};
typedef struct Endereco Endereco;

struct EstadoAndar
{
    int id_andar;

    int *vagas;
    int num_vagas;

    int andar_lotado;
    int andar_fechado;

    Endereco *endereco;
};
typedef struct EstadoAndar EstadoAndar;

struct EstadoEntrada
{
    int id_andar;

    time_t sensor_de_presenca_entrada;
    time_t sensor_de_presenca_saida;
    time_t sensor_de_passagem_entrada;
    time_t sensor_de_passagem_saida;

    int motor_entrada_ligado;
    int motor_saida_ligado;

    Endereco *endereco;
};
typedef struct EstadoEntrada EstadoEntrada;

struct EstadoEstacionamento
{
    int num_andares;
    int tempo_ultima_execucao;

    int estacionamento_fechado;
    int estacionamento_lotado;

    // note que o número de sensores desse tipo é
    // exatamente 1 menor que o número de andares
    // quando o valor no indice i é diferente
    // entre cada um dos vetores, significa que
    // o número maior está aguardando o menor.
    // em teoria, essa diferença deveria ser
    // apenas 1 devido a natureza do problema
    int *sensor_de_subida_de_andar;
    int *sensor_de_descida_de_andar;
    time_t tempo_sensores_de_andar_atualizados;

    EstadoAndar **andares;
    EstadoEntrada *entrada;

    Endereco *endereco;

    ThreadState *t_main;
    ThreadState *t_dep_1;
    ThreadState *t_dep_2;
};
typedef struct EstadoEstacionamento EstadoEstacionamento;

// ======= COMUNICACAO
struct MensagemOut
{
    EstadoEstacionamento *e;
};
typedef struct MensagemOut MensagemOut;

struct MensagemIn
{
    EstadoEstacionamento *e;
};
typedef struct MensagemIn MensagemIn;


// ======= INTERFACE
struct EstadoInterface
{
    EstadoEstacionamento *e;
};
typedef struct EstadoInterface EstadoInterface;

EstadoEstacionamento *inicializar_estado(char* env_name);
EstadoEstacionamento *copiar_estado(EstadoEstacionamento* e);

#endif