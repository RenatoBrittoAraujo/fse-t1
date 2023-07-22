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

bool is_nova_conexao(EstadoEstacionamento *e)
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
    MensagemOut *request_dependente = parse_string_resposta((char *)req);
    EstadoEstacionamento *novo_estado_dependente = (EstadoEstacionamento *)request_dependente->e;
    memcpy(req, novo_estado_dependente, sizeof(EstadoEstacionamento));

    // envia a resposta do servidor principal
    MensagemIn *res = monta_request((EstadoEstacionamento *)res_data);
    char *res_str = tranformar_request_em_string(res);
    return res_str;
}

void escuta_dependente(ThreadState *ts, void *args)
{
    log_print("[MAIN THREAD] escuta_dependente()\n", LEVEL_DEBUG);

    EstadoEstacionamento *estado_main = (EstadoEstacionamento *)args;
    EstadoEstacionamento *estado_dep = NULL;

    char *ip;
    int porta;

    int dependente = -1;
    dependente = ts->thread_id == estado_main->t_dep_1->thread_id ? ATOR_DEP1 : dependente;
    dependente = ts->thread_id == estado_main->t_dep_2->thread_id ? ATOR_DEP2 : dependente;

    switch (dependente)
    {
    case ATOR_DEP1:
        ip = estado_main->andares[0]->endereco->ip;
        porta = estado_main->andares[0]->endereco->porta;
        break;
    case ATOR_DEP2:
        ip = estado_main->andares[1]->endereco->ip;
        porta = estado_main->andares[1]->endereco->porta;
        break;
    default:
        printf("[MAIN THREAD] ator para se escutar desconhecido. thread_id: %d\n", ts->thread_id);
        fflush(NULL);
        pthread_exit(NULL);
    }

    t_error err = listen_tcp_ip_port(get_response, ip, porta, (void *)estado_main, (void *)estado_dep);

    if (err != NO_ERROR)
    {
        printf("[MAIN THREAD] erro %lu no listen_tcp_ip_port() do dependente\n", err);
        fflush(NULL);
        pthread_exit(NULL);
    }

    pthread_exit(estado_dep);
}

ThreadState *criar_thread_comunicar_dependente(EstadoEstacionamento *e)
{
    log_print("[MAIN] criar_thread_comunicar_dependente()\n", LEVEL_DEBUG);

    ThreadState *t = create_thread_state(-1);
    t->response_size = sizeof(EstadoEstacionamento);
    t->routine = escuta_dependente;
    t->args = copiar_estado(e);

    log_print("[MAIN] thread criada\n", LEVEL_DEBUG);
    return t;
}

// ================== MODIFICAR ESTADO ==================

int is_todas_as_vagas_andar_ocupadas(EstadoAndar *a)
{
    log_print("[MAIN] is_todas_as_vagas_andar_ocupadas()\n", LEVEL_DEBUG);
    int nv = 0;
    for (int i = 0; i < a->num_vagas; i++)
    {
        nv += a->vagas[i];
    }
    return nv == a->num_vagas;
}

int is_todas_as_vagas_ocupadas(EstadoEstacionamento *e)
{
    log_print("[MAIN] is_todas_as_vagas_ocupadas()\n", LEVEL_DEBUG);
    printf("e->num_andares = %d\n", e->num_andares);
    fflush(NULL);
    int nv = 0;
    for (int i = 0; i < e->num_andares; i++)
    {
        nv += is_todas_as_vagas_andar_ocupadas(e->andares[i]);
    }
    return nv == e->num_andares;
}

int is_sensor_de_presenca_na_saida_ocupado(EstadoEstacionamento *e)
{
    log_print("[MAIN] is_sensor_de_presenca_na_saida_ocupado()\n", LEVEL_DEBUG);
    e->entrada->motor_saida_ligado = 0;
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

int decidir_estado_motor_cancela_saida(EstadoEstacionamento *e)
{
    log_print("[MAIN] decidir_estado_motor_cancela_saida()\n", LEVEL_DEBUG);

    if (is_newer(MCS_MAXIMO_CANCELA_ABERTA + e->entrada->sensor_de_presenca_saida))
    {
        e->entrada->motor_saida_ligado = 1;
    }
    else
    {
        if (!is_newer(MCS_MAXIMO_CANCELA_ABERTA + e->entrada->sensor_de_passagem_saida))
        {
            e->entrada->motor_saida_ligado = 0;
        }
    }
}

// ================== FIM MODIFICAR ESTADO ==================

EstadoEstacionamento *controla(EstadoEstacionamento *e)
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

    e->entrada->motor_entrada_ligado = get_estado_motor_cancela(
        e->entrada->sensor_de_presenca_entrada,
        e->entrada->sensor_de_passagem_entrada);

    e->entrada->motor_saida_ligado = get_estado_motor_cancela(
        e->entrada->sensor_de_presenca_saida,
        e->entrada->sensor_de_passagem_saida);

    // ========= VAGAS

    int todos_andares_lotados = 1;
    for (int i = 1; i <= e->num_andares; i++)
    {
        char buff[1000];
        sprintf(buff, "[MAIN CONTROLA] Analisando vagas do andar %d\n", i);
        log_print(buff, LEVEL_DEBUG);

        EstadoAndar *a = e->andares[i - 1];
        int andar_lotado = is_todas_as_vagas_andar_ocupadas(a);
        todos_andares_lotados *= andar_lotado;
        a->andar_lotado = andar_lotado;
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

    EstadoEstacionamento *res_dep_1 = (EstadoEstacionamento *)res_dep_1_void_p;
    EstadoEstacionamento *res_dep_2 = (EstadoEstacionamento *)res_dep_2_void_p;

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
