#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "shared/inc/time.h"
#include "shared/inc/threads.h"
#include "shared/inc/shared_util.h"
#include "shared/inc/tcp_ip.h"
#include "shared/inc/proto.h"
#include "shared/inc/comm.h"
#include "shared/inc/errors.h"

#define DEBUG 0

#define PERIODO_MINIMO_ENTRE_EXECUCOES 100 * MILLI

#define TEMPO_MINIMO_CANCELA_ABERTA 2 * SECOND * MILLI

// ========== COMEÇA CONFIG DE PINOS =============
#define RASP_ESTACIONAMENTO_1_3 1
#ifdef RASP_ESTACIONAMENTO_1_3

#define OUT_A1_ENDERECO_01 22
#define OUT_A1_ENDERECO_02 26
#define OUT_A1_ENDERECO_03 19
#define INP_A1_SENSOR_VAGA 18
#define OUT_A1_SINAL_DE_LOTADO_FECHADO 27
#define INP_A1_SENSOR_PRESENCA_CANCELA_ENTRADA 23
#define INP_A1_SENSOR_PASSAGEM_CANCELA_ENTRADA 24
#define INP_A1_SENSOR_PRESENCA_CANCELA_SAIDA 25
#define INP_A1_SENSOR_PASSAGEM_CANCELA_SAIDA 12
#define OUT_A1_MOTOR_CANCELA_ENTRADA 10
#define INP_A1_SENSOR_PASSAGEM_GPIO 12
#define OUT_A1_MOTOR_CANCELA_SAIDA 17

#define OUT_A2_ENDERECO_01 13
#define OUT_A2_ENDERECO_02 6
#define OUT_A2_ENDERECO_03 5
#define INP_A2_SENSOR_VAGA 20
#define OUT_A2_SINAL_DE_LOTADO_FECHADO 8
#define INP_A2_SENSOR_PRESENCA_CANCELA 16
#define INP_A2_SENSOR_PASSAGEM_CANCELA 21

#define INP_SENSOR_DE_PASSAGEM_1 16
#define INP_SENSOR_DE_PASSAGEM_2 21

#else

// NOTE QUE ESSES AQUI SÃO OS PINOS ERRADOS DA PLACA 4
// [TODO] define os pinos para a outra rasp de estacionamento
#define OUT_A1_ENDERECO_01 22
#define OUT_A1_ENDERECO_02 26
#define OUT_A1_ENDERECO_03 19
#define INP_A1_SENSOR_VAGA 18
#define OUT_A1_SINAL_DE_LOTADO_FECHADO 27
#define INP_A1_SENSOR_PRESENCA_CANCELA_ENTRADA 23
#define INP_A1_SENSOR_PASSAGEM_CANCELA_ENTRADA 24
#define OUT_A1_MOTOR_CANCELA_ENTRADA 10
#define INP_A1_SENSOR_PRESENCA_CANCELA_SAIDA 25
#define INP_A1_SENSOR_PASSAGEM_CANCELA_SAIDA 12
#define OUT_A1_MOTOR_CANCELA_SAIDA 17

#define OUT_A2_ENDERECO_01 13
#define OUT_A2_ENDERECO_02 6
#define OUT_A2_ENDERECO_03 5
#define INP_A2_SENSOR_VAGA 20
#define OUT_A2_SINAL_DE_LOTADO_FECHADO 8
#define INP_A2_SENSOR_PRESENCA_CANCELA 16
#define INP_A2_SENSOR_PASSAGEM_CANCELA 21

#define INP_SENSOR_DE_PASSAGEM_1 16
#define INP_SENSOR_DE_PASSAGEM_2 21

#endif
// ========== TERMINA CONFIG DE PINOS =============

#include <bcm2835.h>

#include "shared/inc/time.h"

#define TEMPO_ESPERA_DETECTOR_VAGA 10

int id_andar;

void configura_pinos()
{
    IF_DEBUG log_print("[DEP] configura_pinos\n", LEVEL_DEBUG);
    bcm2835_gpio_fsel(OUT_A1_ENDERECO_01, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OUT_A1_ENDERECO_02, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OUT_A1_ENDERECO_03, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(INP_A1_SENSOR_VAGA, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(OUT_A1_SINAL_DE_LOTADO_FECHADO, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(INP_A1_SENSOR_PRESENCA_CANCELA_ENTRADA, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(INP_A1_SENSOR_PASSAGEM_CANCELA_ENTRADA, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(OUT_A1_MOTOR_CANCELA_ENTRADA, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(INP_A1_SENSOR_PRESENCA_CANCELA_SAIDA, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(INP_A1_SENSOR_PASSAGEM_CANCELA_SAIDA, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(OUT_A1_MOTOR_CANCELA_SAIDA, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OUT_A2_ENDERECO_01, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OUT_A2_ENDERECO_02, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OUT_A2_ENDERECO_03, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(INP_A2_SENSOR_VAGA, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(OUT_A2_SINAL_DE_LOTADO_FECHADO, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(INP_A2_SENSOR_PRESENCA_CANCELA, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(INP_A2_SENSOR_PASSAGEM_CANCELA, BCM2835_GPIO_FSEL_INPT);
}

void handle_interruption(int sinal)
{
    IF_DEBUG log_print("[DEP] handle_interruption\n", LEVEL_DEBUG);
    bcm2835_close();
    IF_DEBUG printf("[DEP] interruption! %d\n", sinal);
    exit(0);
}

char *handle_request_servidor_principal(void *c_request, void *estado_dep, int *res_size)
{
    IF_DEBUG log_print("[DEP] handle_request_servidor_principal()", LEVEL_DEBUG);
    IF_DEBUG printf("c_request: %p len=%u\n", c_request, strlen(c_request));fflush(NULL);

    Estado *e_novo = parse_string_estado(c_request);
    
    IF_DEBUG log_print("[DEP] novo estado recebido do servidor principal", LEVEL_DEBUG);

    IF_DEBUG printf("I GOT ESTACIONAMENTO ESTADO:\n");
    IF_DEBUG print_estado(e_novo);

    Estado *e = (Estado *)estado_dep;

    if (e_novo->ator_atual != ATOR_MAIN)
    {
        IF_DEBUG log_print("[DEP] IGNOREI ESTADO PORQUE ATOR NÃO É MAIN!", LEVEL_ERROR);
        IF_DEBUG printf("[DEP] ator = %d\n", e_novo->ator_atual);
        IF_DEBUG log_print("[DEP] RETORNANDO ECHO DO ESTADO ATUAL!", LEVEL_ERROR);
        IF_DEBUG fflush(NULL);
        char *resposta = transforma_estado_em_string(e);
        return resposta;
    }

    e->num_vagas_andar_1 = e_novo->num_vagas_andar_1;

    e->andar_1_fechado = e_novo->andar_1_fechado;

    e->motor_cancela_entrada_ligado = e_novo->motor_cancela_entrada_ligado;

    e->motor_cancela_saida_ligado = e_novo->motor_cancela_saida_ligado;

    e->num_vagas_andar_2 = e_novo->num_vagas_andar_2;

    e->andar_2_fechado = e_novo->andar_2_fechado;

    free(e_novo);

    IF_DEBUG log_print("[DEP] estado antigo sobrescrito", LEVEL_DEBUG);

    char *resposta = transforma_estado_em_string(e);

    *res_size = sizeof(Estado);

    return resposta;
}

void escuta_main(ThreadState *ts, void *args)
{
    IF_DEBUG log_print("[DEP] escuta_main\n", LEVEL_DEBUG);

    Estado *e = (Estado *)args;

    int ator_atual = e->ator_atual;

    int porta;
    char *ip;

    if (ator_atual == ATOR_DEP1)
    {
        porta = e->porta_andar_1;
        ip = e->ip_andar_1;
    }
    else
    {
        porta = e->porta_andar_2;
        ip = e->ip_andar_2;
    }

    char* request = malloc(MAX_FRAME_SIZE);

    log_print("ABRINDO PORTA PRA ESCUTAR A MAIN", LEVEL_DEBUG);
    t_error err = listen_tcp_ip_port(ip, porta, handle_request_servidor_principal, request, e);

    if (err)
    {
        IF_DEBUG printf("[DEP] erro %lu no listen_tcp_ip_port() do dependente\n", err);
        fflush(NULL);
        pthread_exit(NULL);
    }

    IF_DEBUG printf("Fechando servidor que estava na porta %s:%d\n", ip, porta);

    free(request);

    pthread_exit(NULL);
}

ThreadState *cria_thread_listen(Estado *e)
{
    IF_DEBUG log_print("[DEP] ThreadState* t = cria_thread_listen\n", LEVEL_DEBUG);
    ThreadState *t = create_thread_state(-1);
    e->t_main = t;
    t->routine = escuta_main;
    t->args = e;
    start_thread(t);
    return t;
}

int atualiza_tempo(unsigned long *attr, int atualizar)
{
    // IF_DEBUG log_print("[DEP] atualiza_tempo\n", LEVEL_DEBUG);
    if (atualizar)
        *attr = get_time_mcs();
    return atualizar;
}

Estado *le_aplica_estado(Estado *e, int id_andar)
{
    // printf("\033[2J\033[1;1H");

    IF_DEBUG log_print("[DEP] le_aplica_estado iniciando\n", LEVEL_DEBUG);

    uint8_t end1, end2, end3;

    if (id_andar == 1)
    {
        end1 = OUT_A1_ENDERECO_01;
        end2 = OUT_A1_ENDERECO_02;
        end3 = OUT_A1_ENDERECO_03;
    }
    else
    {
        end1 = OUT_A2_ENDERECO_01;
        end2 = OUT_A2_ENDERECO_02;
        end3 = OUT_A2_ENDERECO_03;
    }

    IF_DEBUG log_print("[DEP] le_aplica_estado verificando vagas\n", LEVEL_DEBUG);

    int new_vagas = 0;

     printf("\n========= SERVIDOR DEPENDENTE %d (%s) ===========\n", id_andar, id_andar==1?"entrada":"piso superior");
    printf("timestamp: | %lu\n", get_time_mcs());
     printf("          | -------------------------------  \n");
     printf("vagas:    |  1 | 2 | 3 | 4 | 5 | 6 | 7 | 8   \n");
     printf("          | -------------------------------  \n");
     printf("ocupacao: | ");

    // Itera por todas as vagas
    for (int i = 0; i < 8; i++)
    {
        int vend1 = LOW, vend2 = LOW, vend3 = LOW;
        if ((1 << 0) & i)
            vend1 = HIGH;
        if ((1 << 1) & i)
            vend2 = HIGH;
        if ((1 << 2) & i)
            vend3 = HIGH;

        bcm2835_gpio_write(end1, vend1);
        bcm2835_gpio_write(end2, vend2);
        bcm2835_gpio_write(end3, vend3);

        fflush(NULL);
        wait_micro(10*MILLI);

        int readv;
        if (id_andar == 1)
        {
            readv = bcm2835_gpio_lev(INP_A1_SENSOR_VAGA);
        }
        else
        {
            readv = bcm2835_gpio_lev(INP_A2_SENSOR_VAGA);
            
        }
        int ocupado = readv > LOW;
        if (ocupado)
        {
            new_vagas = new_vagas | (1 << i);
        }
         printf(" %d", ocupado);
        if (i < 7)printf(" |");
    }

    printf("\n--------- | newvagas: %d ------------------------------- \n", new_vagas);

    int lotado, fechado;
    if (id_andar == 1)
    {
        e->vagas_andar_1 = new_vagas;
        fechado = e->andar_1_fechado;
        lotado = e->andar_1_lotado = 0x00FF == new_vagas;
    }

    if (id_andar == 2)
    {
        e->vagas_andar_2 = new_vagas;
        fechado = e->andar_2_fechado;
        lotado = e->andar_2_lotado = 0x00FF == new_vagas;
    }

    printf("lotado:   | %s (%d)    \n", lotado ? "está lotado":"não está lotado" ,lotado);
    printf("fechado:  | %s (%d)   \n",fechado ? "está fechado":"não está fechado", fechado);

        int sinal_ligado = e->estacionamento_fechado || e->estacionamento_lotado;
        if (id_andar == 1)
        {
            sinal_ligado = sinal_ligado || e->andar_1_fechado || e->andar_1_lotado;
            bcm2835_gpio_write(OUT_A1_SINAL_DE_LOTADO_FECHADO, sinal_ligado);
        }
        else
        {
            sinal_ligado = sinal_ligado || e->andar_2_fechado || e->andar_2_lotado;
            bcm2835_gpio_write(OUT_A2_SINAL_DE_LOTADO_FECHADO, sinal_ligado);
        }

    // entrada
    if (id_andar == 1)
    {
        printf("-------------------------------------------\n");

        IF_DEBUG log_print("[DEP LE APLICA ESTADO] analisando entrada\n", LEVEL_DEBUG);

        if (bcm2835_gpio_lev(INP_A1_SENSOR_PRESENCA_CANCELA_ENTRADA))
            e->sensor_de_presenca_entrada = get_time_mcs();

        if (bcm2835_gpio_lev(INP_A1_SENSOR_PASSAGEM_CANCELA_ENTRADA))
            e->sensor_de_passagem_entrada = get_time_mcs();

        if (bcm2835_gpio_lev(INP_A1_SENSOR_PRESENCA_CANCELA_SAIDA))
            e->sensor_de_presenca_saida = get_time_mcs();

        if (bcm2835_gpio_lev(INP_A1_SENSOR_PASSAGEM_CANCELA_SAIDA))
            e->sensor_de_passagem_saida = get_time_mcs();

        int now = get_time_mcs();
        
        if (now - e->sensor_de_presenca_entrada < TEMPO_MINIMO_CANCELA_ABERTA || 
        now - e->sensor_de_passagem_entrada < TEMPO_MINIMO_CANCELA_ABERTA)
        {
            e->motor_cancela_entrada_ligado = 1;
        }
        else{
            e->motor_cancela_entrada_ligado = 0;
        }

        if (now - e->sensor_de_presenca_saida < TEMPO_MINIMO_CANCELA_ABERTA || 
        now - e->sensor_de_passagem_saida < TEMPO_MINIMO_CANCELA_ABERTA)
        {
            e->motor_cancela_saida_ligado = 1;
        }
        else{
            e->motor_cancela_saida_ligado = 0;
        }

        bcm2835_gpio_write(OUT_A1_MOTOR_CANCELA_ENTRADA, e->motor_cancela_entrada_ligado);

        bcm2835_gpio_write(OUT_A1_MOTOR_CANCELA_SAIDA, e->motor_cancela_saida_ligado); 

        printf("motor_cancela_entrada:      | %s (%d) \n", e->motor_cancela_entrada_ligado ? "abrindo" : "fechando", e->motor_cancela_entrada_ligado);
        printf("motor_cancela_saida:        | %s (%d)   \n", e->motor_cancela_saida_ligado ? "abrindo" : "fechando", e->motor_cancela_saida_ligado);
        printf("sensor_de_presenca_entrada: | %lu   \n", e->sensor_de_presenca_entrada);
        printf("sensor_de_passagem_entrada: | %lu   \n", e->sensor_de_passagem_entrada);
        printf("sensor_de_presenca_saida:   | %lu   \n", e->sensor_de_presenca_saida);
        printf("sensor_de_passagem_saida:   | %lu   \n", e->sensor_de_passagem_saida);
    }

    // não é entrada
    if (id_andar == 2)
    {
        printf("-------------------------------------------\n");

        IF_DEBUG log_print("[DEP LE APLICA ESTADO] analisando passagem de andar\n", LEVEL_DEBUG);

        if (bcm2835_gpio_lev(INP_SENSOR_DE_PASSAGEM_1))
            e->sensor_de_subida_de_andar = get_time_mcs();

        if (bcm2835_gpio_lev(INP_SENSOR_DE_PASSAGEM_2))
            e->sensor_de_descida_de_andar = get_time_mcs();

        printf("sensor_de_subida_de_andar:  | %lu  \n", e->motor_cancela_saida_ligado);
        printf("sensor_de_descida_de_andar: | %lu  \n", e->motor_cancela_saida_ligado);
    }



    e->tempo_ultima_execucao = get_time_mcs();
    e->ator_atual = id_andar == 1 ? ATOR_DEP1 : ATOR_DEP2;
        fflush(NULL);

    return e;
}

int main()
{
    set_level(LEVEL_DEBUG);

    IF_DEBUG log_print("[DEP MAIN] incializando\n", LEVEL_INFO);

    if (!bcm2835_init())
        exit(1);
    IF_DEBUG log_print("[DEP MAIN] bcm iniciado\n", LEVEL_INFO);

    configura_pinos();
    IF_DEBUG log_print("[DEP MAIN] pinos configurados\n", LEVEL_INFO);

    signal(SIGINT, handle_interruption);
    IF_DEBUG log_print("[DEP MAIN] signal settado\n", LEVEL_INFO);

    id_andar = read_env_int_index("ID_ANDAR", -1);
    int ator_atual = id_andar == 2 ? ATOR_DEP2 : ATOR_DEP1;

    char BUFF[1000];
    IF_DEBUG sprintf(BUFF, "%s%d", "ANDAR_", id_andar);
    IF_DEBUG log_print(BUFF, 1);

    IF_DEBUG log_print("[DEP MAIN] estado inicializado\n", LEVEL_INFO);
    Estado *e = inicializar_estado(BUFF, ator_atual);

    IF_DEBUG log_print("[DEP MAIN] abrindo porta\n", LEVEL_DEBUG);
    ThreadState *t = cria_thread_listen(e);
    IF_DEBUG log_print("[DEP MAIN] servidor dependente rodando\n", LEVEL_DEBUG);

    unsigned long last_exec = 0;
    char *ip;
    int porta;
    if (id_andar == 1)
    {
        ip = e->ip_andar_1;
        porta = e->porta_andar_1;
    }
    else
    {
        ip = e->ip_andar_2;
        porta = e->porta_andar_2;
    }

    while (1)
    {

        IF_DEBUG printf("rodando em %s:%d\n", ip, porta);
        e = le_aplica_estado(e, id_andar);

        unsigned long wait_mcs  = PERIODO_MINIMO_ENTRE_EXECUCOES - get_time_mcs() + last_exec;

        if (PERIODO_MINIMO_ENTRE_EXECUCOES > get_time_mcs() - last_exec)
            wait_micro(wait_mcs);
        printf("\e[1;1H\e[2J");

        last_exec = get_time_mcs();
    }
}
