#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "shared/inc/time.h"
#include "shared/inc/threads.h"
#include "shared/inc/shared_util.h"
#include "shared/inc/tcp_ip.h"

#define PERIODO_MINIMO_ENTRE_EXECUCOES 100 // milissegundos

// ========== COMEÇA CONFIG DE PINOS =============
#define RASP_ESTACIONAMENTO_1_3 1
#ifdef RASP_ESTACIONAMENTO_1_3

#define OUT_A1_ENDERECO_01 22
#define OUT_A1_ENDERECO_02 26
#define OUT_A1_ENDERECO_03 19
#define INP_A1_SENSOR_VAGA 18
#define OUT_A1_SINAL_DE_LOTADO_FECHADO 27
#define INP_A1_SENSOR_ABERTURA_CANCELA_ENTRADA 23
#define INP_A1_SENSOR_FECHAMENTO_CANCELA_ENTRADA 24
#define OUT_A1_MOTOR_CANCELA_ENTRADA 10
#define INP_A1_SENSOR_ABERTURA_CANCELA_SAIDA 25
#define INP_A1_SENSOR_FECHAMENTO_CGPIO .setmodeANCELA_SAIDA 12
#define OUT_A1_MOTOR_CANCELA_SAIDA 17

#define OUT_A2_ENDERECO_01 13
#define OUT_A2_ENDERECO_02 6
#define OUT_A2_ENDERECO_03 5
#define INP_A2_SENSOR_VAGA 20
#define OUT_A2_SINAL_DE_LOTADO_FECHADO 8
#define INP_A2_SENSOR_ABERTURA_CANCELA 16
#define INP_A2_SENSOR_FECHAMENTO_CANCELA 21

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
#define INP_A1_SENSOR_ABERTURA_CANCELA_ENTRADA 23
#define INP_A1_SENSOR_FECHAMENTO_CANCELA_ENTRADA 24
#define OUT_A1_MOTOR_CANCELA_ENTRADA 10
#define INP_A1_SENSOR_ABERTURA_CANCELA_SAIDA 25
#define INP_A1_SENSOR_FECHAMENTO_CANCELA_SAIDA 12
#define OUT_A1_MOTOR_CANCELA_SAIDA 17

#define OUT_A2_ENDERECO_01 13
#define OUT_A2_ENDERECO_02 6
#define OUT_A2_ENDERECO_03 5
#define INP_A2_SENSOR_VAGA 20
#define OUT_A2_SINAL_DE_LOTADO_FECHADO 8
#define INP_A2_SENSOR_ABERTURA_CANCELA 16
#define INP_A2_SENSOR_FECHAMENTO_CANCELA 21

#define INP_SENSOR_DE_PASSAGEM_1 16
#define INP_SENSOR_DE_PASSAGEM_2 21

#endif
// ========== TERMINA CONFIG DE PINOS =============

// ========== COMEÇA CONFIG DE AMBIENTE =============
#ifdef TEST_MODE

// CONFIG DE TESTE
// #include <test_time_util.h>

#define BCM2835_GPIO_FSEL_OUTP 1
#define BCM2835_GPIO_FSEL_INPT 0

void bcm2835_gpio_fsel(uint8_t pin, uint8_t mode) { log_print("[BCM MOCK] bcm2835_gpio_fsel()\n", LEVEL_DEBUG); }
int bcm2835_close(void) { log_print("[BCM MOCK] bcm2835_close()\n", LEVEL_DEBUG); }
int bcm2835_init(void)
{
    log_print("[BCM MOCK] bcm2835_init()\n", LEVEL_DEBUG);
    return 1;
}
int bcm2835_gpio_lev(uint8_t pin) { log_print("[BCM MOCK] bcm2835_gpio_lev()\n", LEVEL_DEBUG); }
void bcm2835_gpio_write(uint8_t pin, uint8_t on) { log_print("[BCM MOCK] bcm2835_gpio_write()\n", LEVEL_DEBUG); }

#else

// CONFIG DE PROD

#include <bcm2835.h>

#include "shared/inc/time_util.h"

#endif
// ========== TERMINA CONFIG DE AMBIENTE =============

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
    bcm2835_gpio_fsel(INP_A1_SENSOR_ABERTURA_CANCELA_ENTRADA, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(INP_A1_SENSOR_FECHAMENTO_CANCELA_ENTRADA, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(OUT_A1_MOTOR_CANCELA_ENTRADA, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(INP_A1_SENSOR_ABERTURA_CANCELA_SAIDA, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(INP_A1_SENSOR_FECHAMENTO_CANCELA_SAIDA, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(OUT_A1_MOTOR_CANCELA_SAIDA, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OUT_A2_ENDERECO_01, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OUT_A2_ENDERECO_02, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(OUT_A2_ENDERECO_03, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(INP_A2_SENSOR_VAGA, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(OUT_A2_SINAL_DE_LOTADO_FECHADO, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(INP_A2_SENSOR_ABERTURA_CANCELA, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(INP_A2_SENSOR_FECHAMENTO_CANCELA, BCM2835_GPIO_FSEL_INPT);
}

void handle_interruption(int sinal)
{
    log_print("[DEP] handle_interruption\n", LEVEL_DEBUG);
    bcm2835_close();
    printf("interruption! %d\n", sinal);
    exit(0);
}

EstadoEstacionamento *request = NULL;
char *resposta = NULL;
char *get_resposta(void *c_request)
{
    char *n_request = NULL;
    log_print("[DEP] get_resposta\n", LEVEL_DEBUG);
    if (c_request != NULL)
    {
        log_print("[DEP] get_resposta memcpy\n", LEVEL_DEBUG);
        n_request = (char *)malloc(sizeof(EstadoEstacionamento));
        memcpy(n_request, c_request, sizeof(EstadoEstacionamento));
        log_print("[DEP] request setada\n", LEVEL_DEBUG);
    }
    if (request != NULL)
    {
        log_print("[DEP] limpa antiga request\n", LEVEL_DEBUG);
        free(request);
    }
    log_print("[DEP] seta nova request\n", LEVEL_DEBUG);
    request = c_request;
    log_print("[DEP] request feita\n", LEVEL_DEBUG);

    log_print("[DEP] get_resposta\n", LEVEL_DEBUG);
    return resposta;
}

void set_resposta(EstadoEstacionamento *e)
{
    log_print("[DEP] set_resposta\n", LEVEL_DEBUG);
    if (resposta == NULL)
    {
        log_print("[DEP] aloca resposta\n", LEVEL_DEBUG);
        resposta = (char *)malloc(sizeof(EstadoEstacionamento));
    }
    if (e != NULL)
    {

        log_print("[DEP] memcpy resposta\n", LEVEL_DEBUG);
        memcpy(resposta, e, sizeof(EstadoEstacionamento));
    }
    log_print("[DEP] fim set_resposta\n", LEVEL_DEBUG);
}

void escuta_main(ThreadState *ts, void *args)
{
    log_print("[DEP] escuta_main\n", LEVEL_DEBUG);
    EstadoEstacionamento *e = (EstadoEstacionamento *)args;
    printf("ABRINDO IP:PORTA %s:%d\n", e->endereco->ip, e->endereco->porta);
    fflush(NULL);
    log_print("[DEP] deu parse do estacionamento\n", LEVEL_DEBUG);
    listen_tcp_ip_port(get_resposta, e->endereco->ip, e->endereco->porta);
    log_print("[DEP] escutando porta!\n", LEVEL_DEBUG);
}

void abrir_thread_server_dependente(EstadoEstacionamento *e)
{
    log_print("[DEP] abrir_thread_server_dependente\n", LEVEL_DEBUG);

    ThreadState *t = create_thread_state();
    e->t_main = t;
    t->routine = escuta_main;
    t->args = e;
    start_thread(t);
}

int atualiza_tempo(time_t *attr, int atualizar)
{
    log_print("[DEP] atualiza_tempo\n", LEVEL_DEBUG);
    if (atualizar)
        *attr = get_timestamp_now();
    return atualizar;
}

EstadoEstacionamento *le_aplica_estado(EstadoEstacionamento *e, int id_andar)
{
    log_print("[DEP] le_aplica_estado\n", LEVEL_DEBUG);
    if (request != NULL)
    {
        log_print("[DEP] tem request1!!\n", LEVEL_DEBUG);
        memcpy(e, request, sizeof(EstadoEstacionamento));
        log_print("[DEP] tem request2!!\n", LEVEL_DEBUG);
        free(request);
        log_print("[DEP] tem request3!!\n", LEVEL_DEBUG);
        request = NULL;
        log_print("[DEP] tem request4!!\n", LEVEL_DEBUG);
    }
    log_print("[DEP] le_aplica_estado iniciando\n", LEVEL_DEBUG);
    printf("e->num_andares = %d\n", e->num_andares);

    if (e->num_andares < id_andar - 1)
    {
        log_print("[ERROR] [DEP] le_aplica_estado numero da andar < id do andar atual\n", LEVEL_ERROR);
    }

    EstadoAndar *a = e->andares[id_andar - 1];
    int andar_entrada = read_env_int_index("ID_ANDAR_DA_ENTRADA", -1);

    log_print("[DEP] le_aplica_estado iniciando\n", LEVEL_DEBUG);
    uint8_t end1, end2, end3;
    if (id_andar == 1 || 1)
    {
        end1 = OUT_A1_ENDERECO_01;
        end2 = OUT_A1_ENDERECO_02;
        end3 = OUT_A1_ENDERECO_03;
    }
    else
    {
        end1 = OUT_A2_ENDERECO_03;
        end2 = OUT_A2_ENDERECO_02;
        end3 = OUT_A2_ENDERECO_01;
    }

    log_print("[DEP] le_aplica_estado verificando vagas\n", LEVEL_DEBUG);
    // Itera por todas as vagas
    for (int i = 0; i < a->num_vagas; i++)
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

        // claramente essa solução não suporta mais que 8 vagas, mas foi oq deu
        wait(5);
        int read = bcm2835_gpio_lev(INP_A1_SENSOR_VAGA);
        a->vagas[i] = read;
        printf("vaga %d é %d!\n", i + 1, read);
        fflush(NULL);
    }

    if (id_andar == andar_entrada)
    {
        log_print("[DEP LE APLICA ESTADO] analisando entrada\n", LEVEL_DEBUG);

        atualiza_tempo(&e->entrada->sensor_de_presenca_entrada, bcm2835_gpio_lev(INP_A1_SENSOR_ABERTURA_CANCELA_ENTRADA));
        atualiza_tempo(&e->entrada->sensor_de_passagem_saida, bcm2835_gpio_lev(INP_A1_SENSOR_FECHAMENTO_CANCELA_SAIDA));
        bcm2835_gpio_write(OUT_A1_MOTOR_CANCELA_ENTRADA, e->entrada->motor_entrada_ligado);

        atualiza_tempo(&e->entrada->sensor_de_passagem_entrada, bcm2835_gpio_lev(INP_A1_SENSOR_FECHAMENTO_CANCELA_ENTRADA));
        atualiza_tempo(&e->entrada->sensor_de_presenca_saida, bcm2835_gpio_lev(INP_A1_SENSOR_ABERTURA_CANCELA_SAIDA));
        bcm2835_gpio_write(OUT_A1_MOTOR_CANCELA_SAIDA, e->entrada->motor_saida_ligado);

        bcm2835_gpio_write(OUT_A1_SINAL_DE_LOTADO_FECHADO, e->estacionamento_fechado || e->estacionamento_lotado);
    }
    else
    {
        log_print("[DEP LE APLICA ESTADO] analisando passagem de andar\n", LEVEL_DEBUG);
        int id_sensores = id_andar;
        if (andar_entrada < id_andar)
        {
            id_sensores--;
        }

        log_print("le sensor de entrada \n", LEVEL_DEBUG);
        if (bcm2835_gpio_lev(INP_SENSOR_DE_PASSAGEM_1))
        {
            e->sensor_de_subida_de_andar[id_sensores - 1]++;
            e->tempo_sensores_de_andar_atualizados = get_timestamp_now();
        }

        if (bcm2835_gpio_lev(INP_SENSOR_DE_PASSAGEM_2))
        {
            e->sensor_de_descida_de_andar[id_sensores - 1]++;
            e->tempo_sensores_de_andar_atualizados = get_timestamp_now();
        }

        bcm2835_gpio_write(OUT_A2_SINAL_DE_LOTADO_FECHADO, e->estacionamento_fechado || e->estacionamento_lotado);
    }

    return e;
}

int main()
{
    set_env();
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
    char BUFF[1000];
    sprintf(BUFF, "%s%d", "ANDAR_", id_andar);
    log_print(BUFF, 1);
    EstadoEstacionamento *e = inicializar_estado(BUFF);

    log_print("[DEP MAIN] estado inicializado\n", LEVEL_INFO);

    abrir_thread_server_dependente(e);
    log_print("[DEP MAIN] thread de abrir porta chamada\n", LEVEL_DEBUG);

    time_t last_exec = 0;

    while (1)
    {
        log_print("[DEP MAIN] chama le aplica estado\n", LEVEL_DEBUG);
        e = le_aplica_estado(e, id_andar);
        log_print("[DEP MAIN] le aplica estado retornou!!!\n", LEVEL_DEBUG);
        set_resposta(e);

        printf("ESCUTANDO %s:%d\n", e->endereco->ip, e->endereco->porta);
        if (is_newer(PERIODO_MINIMO_ENTRE_EXECUCOES + last_exec))
        {
            time_t wait_time = PERIODO_MINIMO_ENTRE_EXECUCOES + last_exec - get_timestamp_now();
            printf("Waiting %lu ms\n", wait_time);
            wait(wait_time);
        }
        last_exec = get_timestamp_now();
    }
}
