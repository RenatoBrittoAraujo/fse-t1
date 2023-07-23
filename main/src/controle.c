#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "shared/inc/proto.h"
#include "shared/inc/time.h"
#include "shared/inc/shared_util.h"
#include "shared/inc/threads.h"
#include "shared/inc/tcp_ip.h"
#include "shared/inc/comm.h"

void set_entradas_inicial()
{
    log_print("[MAIN] set_entradas_inicial()\n", LEVEL_DEBUG);
}

bool is_nova_conexao(Estado *e)
{
    log_print("[MAIN] is_nova_conexao()\n", LEVEL_DEBUG);
    if (e == NULL)
    {
        return true;
    }
    if (e->tempo_ultima_execucao < get_time_mcs() - MCS_DESCONEXAO)
    {
        return true;
    }
    return false;
}

char *get_response(void *req, void *res_data)
{
    // le a resposta do servidor dependente
    Estado *request_dependente = parse_string_resposta((char *)req);
    Estado *novo_estado_dependente = (Estado *)request_dependente;
    memcpy(req, novo_estado_dependente, sizeof(Estado));

    // envia a resposta do servidor principal
    Estado *res = monta_request((Estado *)res_data);
    char *res_str = tranformar_request_em_string(res);
    return res_str;
}

void escuta_dependente(ThreadState *ts, void *args)
{
    log_print("[MAIN THREAD] escuta_dependente()\n", LEVEL_DEBUG);

    Estado *estado_main = (Estado *)args;
    Estado *estado_dep = NULL;

    char *ip;
    int porta;

    int dependente = -1;
    dependente = ts->thread_id == estado_main->t_dep_1->thread_id ? ATOR_DEP1 : dependente;
    dependente = ts->thread_id == estado_main->t_dep_2->thread_id ? ATOR_DEP2 : dependente;

    switch (dependente)
    {
    case ATOR_DEP1:
        ip = estado_main->ip_andar_1;
        porta = estado_main->porta_andar_1;
        break;
    case ATOR_DEP2:
        ip = estado_main->ip_andar_2;
        porta = estado_main->porta_andar_2;
        break;
    default:
        printf("[MAIN THREAD] ator para se escutar desconhecido. thread_id: %d\n", ts->thread_id);
        fflush(NULL);
        pthread_exit(NULL);
    }

    char *req = tranformar_request_em_string(estado_main);
    char *res_buff;

    t_error err = call_tcp_ip_port(req, ip, porta, res_buff);

    if (err != NO_ERROR)
    {
        printf("[MAIN THREAD] erro %lu no listen_tcp_ip_port() do dependente\n", err);
        fflush(NULL);
        pthread_exit(NULL);
    }

    estado_dep = parse_string_resposta(res_buff);

    pthread_exit(estado_dep);
}

ThreadState *criar_thread_comunicar_dependente(Estado *e)
{
    log_print("[MAIN] criar_thread_comunicar_dependente()\n", LEVEL_DEBUG);

    ThreadState *t = create_thread_state(-1);
    t->response_size = sizeof(Estado);
    t->routine = escuta_dependente;
    t->args = copiar_estado(e);

    log_print("[MAIN] thread criada\n", LEVEL_DEBUG);
    return t;
}

// ================== MODIFICAR ESTADO ==================

int is_todas_as_vagas_andar_ocupadas(Estado *e, int id_andar)
{
    log_print("[MAIN] is_todas_as_vagas_andar_ocupadas()\n", LEVEL_DEBUG);
    int vagas = id_andar == 1 ? e->vagas_andar_1 : e->vagas_andar_2;
    return 0xff == vagas;
}

int is_todas_as_vagas_ocupadas(Estado *e)
{
    log_print("[MAIN] is_todas_as_vagas_ocupadas()\n", LEVEL_DEBUG);
    int nv = 1;
    for (int i = 1; i <= 2; i++)
        nv = nv && is_todas_as_vagas_andar_ocupadas(e, i);
    return nv;
}

int is_sensor_de_presenca_na_saida_ocupado(Estado *e)
{
    log_print("[MAIN] is_sensor_de_presenca_na_saida_ocupado()\n", LEVEL_DEBUG);
    e->motor_cancela_saida_ligado = 0;
}

int get_estado_motor_cancela(time_t sensor_de_presenca, time_t sensor_de_passagem)
{
    log_print("[MAIN] get_estado_motor_cancela()\n", LEVEL_DEBUG);

    // se o sensor de presenca capturou algo ou o sensor de passagem está capturando algo
    if (is_newer(MCS_MAXIMO_CANCELA_ABERTA + sensor_de_presenca) || is_newer(MCS_MAXIMO_CANCELA_ABERTA + sensor_de_passagem))
    {
        // cancela levanta
        return 1;
    }
    // cancela abaixa
    return 0;
}

int decidir_estado_motor_cancela_saida(Estado *e)
{
    log_print("[MAIN] decidir_estado_motor_cancela_saida()\n", LEVEL_DEBUG);

    if (is_newer(MCS_MAXIMO_CANCELA_ABERTA + e->sensor_de_presenca_saida))
    {
        e->motor_cancela_saida_ligado = 1;
    }
    else
    {
        if (!is_newer(MCS_MAXIMO_CANCELA_ABERTA + e->sensor_de_passagem_saida))
        {
            e->motor_cancela_entrada_ligado = 0;
        }
    }
}

// ================== FIM MODIFICAR ESTADO ==================

Estado *controla(Estado *e)
{
    log_print("[MAIN] controla()\n", LEVEL_DEBUG);

    if (is_nova_conexao(e))
    {
        e = inicializar_estado("MAIN", ATOR_MAIN);
        set_entradas_inicial();

        e->t_dep_1 = criar_thread_comunicar_dependente(e);
        e->t_dep_2 = criar_thread_comunicar_dependente(e);
    }
    e->tempo_ultima_execucao = get_time_mcs();

    // ========= CANCELAS

    e->motor_cancela_entrada_ligado = get_estado_motor_cancela(
        e->sensor_de_presenca_entrada,
        e->sensor_de_passagem_entrada);

    e->motor_cancela_saida_ligado = get_estado_motor_cancela(
        e->sensor_de_presenca_saida,
        e->sensor_de_passagem_saida);

    // ========= VAGAS

    int todos_andares_lotados = 1;
    for (int i = 1; i <= e->num_andares; i++)
    {
        char buff[1000];
        sprintf(buff, "[MAIN CONTROLA] Analisando vagas do andar %d\n", i);
        log_print(buff, LEVEL_DEBUG);

        int andar_lotado = is_todas_as_vagas_andar_ocupadas(e, i);
        todos_andares_lotados *= andar_lotado;
    }
    e->estacionamento_lotado = is_todas_as_vagas_ocupadas(e);

    log_print("[MAIN CONTROLA] escuta dependentes\n", LEVEL_DEBUG);

    e->t_dep_1->args = e->t_dep_2->args = e;

    start_thread(e->t_dep_1);
    start_thread(e->t_dep_2);

    log_print("[MAIN CONTROLA] esperando mensagem do dependente 1\n", LEVEL_DEBUG);
    void *res_dep_1_void_p = wait_thread_response_with_deadline(e->t_dep_1, MCS_DEADLINE_RESPOSTA_DEPENDENTE);

    log_print("[MAIN CONTROLA] esperando mensagem do dependente 2\n", LEVEL_DEBUG);
    void *res_dep_2_void_p = wait_thread_response_with_deadline(e->t_dep_2, MCS_DEADLINE_RESPOSTA_DEPENDENTE);

    if (res_dep_1_void_p == NULL)
    {
        log_print("[MAIN CONTROLA] dependente 1 não respondeu\n", LEVEL_ERROR);
        exit(1);
    }

    if (res_dep_2_void_p == NULL)
    {
        log_print("[MAIN CONTROLA] dependente 2 não respondeu\n", LEVEL_ERROR);
        exit(1);
    }

    Estado *res_dep_1 = (Estado *)res_dep_1_void_p;
    Estado *res_dep_2 = (Estado *)res_dep_2_void_p;

    log_print("[MAIN CONTROLA] respostas recebidas, combinando resultados\n", LEVEL_DEBUG);

    // [TODO] join estados

    log_print("[MAIN CONTROLA] resultados combinados, novo estado gerado\n", LEVEL_DEBUG);

    // ========= FIM

    time_t esperar_proximo = get_time_mcs() - e->tempo_ultima_execucao - MCS_PERIODO_MINIMO_EXEC;

    if (esperar_proximo > 0)
    {
        log_print("[MAIN CONTROLA] wait_micro()\n", LEVEL_DEBUG);
        wait_micro(esperar_proximo);
    }

    log_print("[MAIN CONTROLA] fim\n", LEVEL_DEBUG);

    return e;
}
