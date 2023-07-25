#ifndef PROTO_H
#define PROTO_H 1

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#include "shared/inc/threads.h"
#include "shared/inc/time.h"

#define RASP_ESTACIONAMENTO_1_3 1
#define SENSOR_DE_PASSAGEM_DESATIVADO 0
#define ESPERANDO_OUTRO_SENSOR 1

#define MCS_MAXIMO_CANCELA_ABERTA 10 * SECOND *MILLI
#define MCS_DESCONEXAO 10 * SECOND *MILLI
#define MCS_PERIODO_MINIMO_EXEC 100 * MILLI
#define MCS_DEADLINE_RESPOSTA_DEPENDENTE 500 * MILLI

#define ATOR_MAIN 1
#define ATOR_DEP1 2
#define ATOR_DEP2 3

#define DEBUG 1
#define IF_DEBUG if (DEBUG)

// ======= CONTROLE

struct Estado
{

    // ---------- comum
    int porta_andar_1;
    int porta_andar_2;
    char *ip_andar_1;
    char *ip_andar_2;

    ThreadState *t_main;
    ThreadState *t_dep_1;
    ThreadState *t_dep_2;
    // identificador de qual ator executou a ultima acao
    // do estado atual: ATOR_MAIN, ATOR_DEP1, ATOR_DEP2
    int ator_atual;

    // identificador de quando foi exectuado a ultima acao
    int tempo_ultima_execucao;

    // ---------- dados servidor principal
    int num_andares; // que é 2
    int estacionamento_fechado;
    int estacionamento_lotado;

    // quando sensor de presenca estiver ativado mas o de passagem nao
    // esse boleano eh settado

    // quando o sensor de presenca eh desativado e esse boleano esta
    // settado, significa que um carro entrou ou saiu

    int nova_presenca_entrada;
    int nova_presenca_saida;

    int timestamp_last_entrada;
    int timestamp_last_saida;
    
    int id_last_entrada;
    int id_last_saida;
    int entrada_pendente;
    int saida_pendente;

    int preco_pago_last_carro;
    int preco_por_segundo;

    int id_vagas[16];
    int entrada_time[16];
    int estacionou_na_vaga;
    int saiu_da_vaga;

    int last_entrada_i;
    int last_saida_i;

    // ---------- entradas para andar 1
    int num_vagas_andar_1;
    int andar_1_fechado;
    int motor_cancela_entrada_ligado;
    int motor_cancela_saida_ligado;
    unsigned long override_motor_cancela_entrada;
    unsigned long override_motor_cancela_saida;
    unsigned long override_motor_0_cancela_entrada;
    unsigned long override_motor_0_cancela_saida;

    // ---------- saidas do andar 1
    int andar_1_lotado;
    int vagas_andar_1;
    int last_vagas_andar_1;
    unsigned long sensor_de_presenca_entrada;
    unsigned long sensor_de_presenca_saida;
    unsigned long sensor_de_passagem_entrada;
    unsigned long sensor_de_passagem_saida;

    // ---------- saidas para andar 2
    int num_vagas_andar_2;
    int andar_2_fechado;

    // ---------- entradas do andar 2
    int andar_2_lotado;
    int vagas_andar_2;
    int last_vagas_andar_2;
    unsigned long sensor_de_subida_de_andar;
    unsigned long sensor_de_descida_de_andar;
} __attribute__((packed));
typedef struct Estado Estado;

// ======= INTERFACE
struct EstadoInterface
{
    Estado *e;
};
typedef struct EstadoInterface EstadoInterface;

Estado *inicializar_estado(char *env_name, int ator_atual);

Estado *copiar_estado(Estado *e);

void print_estado(Estado *e);

#endif
