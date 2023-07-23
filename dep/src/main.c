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

#define PERIODO_MINIMO_ENTRE_EXECUCOES 100 * MILLI

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
#define INP_A1_SENSOR_PASSAGEM_CANCELA_SAIDA 25
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
    log_print("[DEP] configura_pinos\n", LEVEL_DEBUG);
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
    log_print("[DEP] handle_interruption\n", LEVEL_DEBUG);
    bcm2835_close();
    printf("[DEP] interruption! %d\n", sinal);
    exit(0);
}

char *handle_request_servidor_principal(void *c_request, void *estado_dep)
{
    log_print("[DEP] handle_request_servidor_principal()", LEVEL_DEBUG);

    EstadoEstacionamento *e_novo = parse_string_resposta(c_request);

    log_print("[DEP] novo estado recebido do servidor principal", LEVEL_DEBUG);

    EstadoEstacionamento *e = (EstadoEstacionamento *)estado_dep;

    e->num_vagas_andar_1 = e_novo->num_vagas_andar_1;

    e->andar_1_fechado = e_novo->andar_1_fechado;

    e->motor_cancela_entrada_ligado = e_novo->motor_cancela_entrada_ligado;

    e->motor_cancela_saida_ligado = e_novo->motor_cancela_saida_ligado;

    e->num_vagas_andar_2 = e_novo->num_vagas_andar_2;

    e->andar_2_fechado = e_novo->andar_2_fechado;

    log_print("[DEP] estado antigo sobrescrito", LEVEL_DEBUG);

    char *resposta = tranformar_request_em_string(e);

    return resposta;
}

void escuta_main(ThreadState *ts, void *args)
{
    log_print("[DEP] escuta_main\n", LEVEL_DEBUG);

    EstadoEstacionamento *e = (EstadoEstacionamento *)args;

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

    t_error err = listen_tcp_ip_port(ip, porta, handle_request_servidor_principal, NULL, args);

    if (err)
    {
        printf("[DEP] erro %lu no listen_tcp_ip_port() do dependente\n", err);
        fflush(NULL);
        pthread_exit(NULL);
    }

    printf("Servidor dependende rodando no endereco %s:%d\n", ip, porta);

    pthread_exit(NULL);
}

ThreadState *cria_thread_listen(EstadoEstacionamento *e)
{
    log_print("[DEP] ThreadState* t = cria_thread_listen\n", LEVEL_DEBUG);
    ThreadState *t = create_thread_state(-1);
    e->t_main = t;
    t->routine = escuta_main;
    t->args = e;
    start_thread(t);
    return t;
}

int atualiza_tempo(time_t *attr, int atualizar)
{
    log_print("[DEP] atualiza_tempo\n", LEVEL_DEBUG);
    if (atualizar)
        *attr = get_time_mcs();
    return atualizar;
}

EstadoEstacionamento *le_aplica_estado(EstadoEstacionamento *e, int id_andar)
{
    log_print("[DEP] le_aplica_estado iniciando\n", LEVEL_DEBUG);

    uint8_t end1, end2, end3;

    end1 = OUT_A1_ENDERECO_01;
    end2 = OUT_A1_ENDERECO_02;
    end3 = OUT_A1_ENDERECO_03;

    log_print("[DEP] le_aplica_estado verificando vagas\n", LEVEL_DEBUG);

    int new_vagas = 0;

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

        printf("%d %d %d\n",
               bcm2835_gpio_lev(end3),
               bcm2835_gpio_lev(end2),
               bcm2835_gpio_lev(end1));

        fflush(NULL);
        wait_micro(5);

        int read = bcm2835_gpio_lev(INP_A1_SENSOR_VAGA);

        if (read != LOW)
        {
            new_vagas = new_vagas | (1 << i);
            printf("vaga %d está ocupada!\n", i + 1);
        }
        else
        {
            printf("vaga %d está livre!\n", i + 1);
        }

        fflush(NULL);
    }

    if (id_andar == 1)
    {
        e->vagas_andar_1 = new_vagas;
        e->andar_1_lotado = 0x00FF == new_vagas;
    }

    if (id_andar == 2)
    {
        e->vagas_andar_2 = new_vagas;
        e->andar_2_lotado = 0x00FF == new_vagas;
    }

    // entrada
    if (id_andar == 1)
    {
        log_print("[DEP LE APLICA ESTADO] analisando entrada\n", LEVEL_DEBUG);

        atualiza_tempo(&e->sensor_de_presenca_entrada, bcm2835_gpio_lev(INP_A1_SENSOR_PRESENCA_CANCELA_ENTRADA));

        atualiza_tempo(&e->sensor_de_passagem_saida, bcm2835_gpio_lev(INP_A1_SENSOR_PASSAGEM_CANCELA_SAIDA));

        bcm2835_gpio_write(OUT_A1_MOTOR_CANCELA_ENTRADA, e->motor_cancela_entrada_ligado);

        atualiza_tempo(&e->sensor_de_passagem_entrada, bcm2835_gpio_lev(INP_A1_SENSOR_PASSAGEM_CANCELA_ENTRADA));

        atualiza_tempo(&e->sensor_de_presenca_saida,
                       bcm2835_gpio_lev(INP_A1_SENSOR_PRESENCA_CANCELA_SAIDA));

        bcm2835_gpio_write(OUT_A1_MOTOR_CANCELA_SAIDA, e->motor_cancela_saida_ligado);

        bcm2835_gpio_write(OUT_A1_SINAL_DE_LOTADO_FECHADO, e->estacionamento_fechado || e->estacionamento_lotado);
    }

    // não é entrada
    if (id_andar == 2)
    {
        log_print("[DEP LE APLICA ESTADO] analisando passagem de andar\n", LEVEL_DEBUG);

        if (bcm2835_gpio_lev(INP_SENSOR_DE_PASSAGEM_1))
        {
            e->sensor_de_subida_de_andar = get_time_mcs();
        }

        if (bcm2835_gpio_lev(INP_SENSOR_DE_PASSAGEM_2))
        {
            e->sensor_de_descida_de_andar = get_time_mcs();
        }

        bcm2835_gpio_write(OUT_A2_SINAL_DE_LOTADO_FECHADO, e->estacionamento_fechado || e->estacionamento_lotado);
    }

    e->tempo_ultima_execucao = get_time_mcs();
    e->ator_atual = id_andar == 1 ? ATOR_DEP1 : ATOR_DEP2;

    return e;
}

int main()
{
    set_level(LEVEL_DEBUG);
    set_time_wait_ignore(0);

    log_print("[DEP MAIN] incializando\n", LEVEL_INFO);

    if (!bcm2835_init())
        exit(1);
    log_print("[DEP MAIN] bcm iniciado\n", LEVEL_INFO);

    configura_pinos();
    log_print("[DEP MAIN] pinos configurados\n", LEVEL_INFO);

    signal(SIGINT, handle_interruption);
    log_print("[DEP MAIN] signal settado\n", LEVEL_INFO);

    id_andar = read_env_int_index("ID_ANDAR", -1);
    int ator_atual = id_andar == 2 ? ATOR_DEP2 : ATOR_DEP1;

    char BUFF[1000];
    sprintf(BUFF, "%s%d", "ANDAR_", id_andar);
    log_print(BUFF, 1);

    log_print("[DEP MAIN] estado inicializado\n", LEVEL_INFO);
    EstadoEstacionamento *e = inicializar_estado(BUFF, ator_atual);

    log_print("[DEP MAIN] abrindo porta\n", LEVEL_DEBUG);
    ThreadState *t = cria_thread_listen(e);
    log_print("[DEP MAIN] servidor dependente rodando\n", LEVEL_DEBUG);

    time_t last_exec = 0;

    while (1)
    {
        log_print("[DEP MAIN] chama le aplica estado\n", LEVEL_DEBUG);
        e = le_aplica_estado(e, id_andar);

        if (is_newer(PERIODO_MINIMO_ENTRE_EXECUCOES + last_exec))
        {
            time_t wait_time = PERIODO_MINIMO_ENTRE_EXECUCOES + last_exec - get_time_mcs();
            printf("Waiting %lu ms\n", wait_time);
            wait_micro(wait_time);
        }
        last_exec = get_time_mcs();
    }
}
