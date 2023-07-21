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

#define SEGUNDO 1000 // milissegundos
#define TEMPO_MAXIMO_CANCELA_ABERTA 10 * SEGUNDO *MILLI
#define TEMPO_MAXIMO_DE_DESCONEXAO 10 * SEGUNDO *MILLI
#define PERIODO_MINIMO_EXEC 50

#define DEADLINE_RESPOSTA_DEPENDENTE 500 * MILLI

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
    if (e->tempo_ultima_execucao < get_time_mcs() - TEMPO_MAXIMO_DE_DESCONEXAO)
    {
        return true;
    }
    return false;
}

void envia_mensagem_pra_dependente(ThreadState *ts, void *args)
{
    log_print("[MAIN THREAD] envia_mensagem_pra_dependente()\n", LEVEL_DEBUG);
    EstadoEstacionamento *e = (EstadoEstacionamento *)args;

    MensagemIn *req = monta_request(e);

    char *req_str = tranformar_request_em_string(req);
    char *ip;
    int porta;

    if (ts->thread_id == e->t_dep_1->thread_id)
    {
        ip = e->andares[0]->endereco->ip;
        porta = e->andares[0]->endereco->porta;
    }
    else if (ts->thread_id == e->t_dep_2->thread_id)
    {
        ip = e->andares[1]->endereco->ip;
        porta = e->andares[1]->endereco->porta;
    }
    else
    {
        log_print("[MAIN THREAD] ip e porta da mensagem não encontrado\n", LEVEL_ERROR);
        exit(1);
    }

    char *res_str = listen_tcp_ip_port(req_str, ip, porta);
    MensagemOut *res = parse_string_resposta(res_str);

    if (ts->thread_id == e->t_dep_1->thread_id)
    {
        log_print("[MAIN THREAD] resposta do andar 1\n", LEVEL_DEBUG);
        void *t_res = wait_thread_response_with_deadline(e->t_dep_1, DEADLINE_RESPOSTA_DEPENDENTE);
        memcpy(t_res, (void *)res->e, sizeof(EstadoEstacionamento));
    }
    else if (ts->thread_id == e->t_dep_2->thread_id)
    {
        log_print("[MAIN THREAD] resposta do andar 2\n", LEVEL_DEBUG);
        void *t_res = wait_thread_response_with_deadline(e->t_dep_2, DEADLINE_RESPOSTA_DEPENDENTE);
        memcpy(t_res, (void *)res->e, sizeof(EstadoEstacionamento));
    }
    else
    {
        log_print("[ERROR] [MAIN THREAD] endereco para esrever resposta não encontrado\n", LEVEL_ERROR);
        exit(1);
    }
}

ThreadState *criar_thread_comunicar_dependente(EstadoEstacionamento *e)
{
    log_print("[MAIN] criar_thread_comunicar_dependente()\n", LEVEL_DEBUG);

    ThreadState *t = create_thread_state(-1);
    t->response_size = sizeof(EstadoEstacionamento);
    t->routine = envia_mensagem_pra_dependente;
    t->args = copiar_estado(e);

    log_print("[MAIN] thread criada\n", LEVEL_DEBUG);
    return t;
}

void set_thread_args(ThreadState *t, EstadoEstacionamento *e)
{
    log_print("[MAIN] set_thread_args()\n", LEVEL_DEBUG);
    printf("e->tdep_1 e->tdpe2%p %p\n", e->t_dep_1, e->t_dep_2);
    fflush(NULL);
    t->args = (void *)malloc(sizeof(EstadoEstacionamento));
    log_print("[MAIN] set_thread_args 2\n", LEVEL_DEBUG);
    memcpy(t->args, e, sizeof(EstadoEstacionamento));
    log_print("[MAIN] set_thread_args ret\n", LEVEL_DEBUG);
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
    if (is_newer(TEMPO_MAXIMO_CANCELA_ABERTA + sensor_de_presenca) || is_newer(TEMPO_MAXIMO_CANCELA_ABERTA + sensor_de_passagem))
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

    if (is_newer(TEMPO_MAXIMO_CANCELA_ABERTA + e->entrada->sensor_de_presenca_saida))
    {
        e->entrada->motor_saida_ligado = 1;
    }
    else
    {
        if (!is_newer(TEMPO_MAXIMO_CANCELA_ABERTA + e->entrada->sensor_de_passagem_saida))
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
        e = inicializar_estado("MAIN");
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

    log_print("[MAIN CONTROLA] enviando mensagem pra dependente 1\n", LEVEL_DEBUG);
    e->t_dep_1->args = e;
    start_thread(e->t_dep_1);

    log_print("[MAIN CONTROLA] enviando mensagem pra dependente 2\n", LEVEL_DEBUG);
    e->t_dep_2->args = e;
    start_thread(e->t_dep_2);

    // ========= FIM

    time_t esperar_proximo = get_time_mcs() - e->tempo_ultima_execucao - PERIODO_MINIMO_EXEC;

    if (esperar_proximo > 0)
    {
        log_print("[MAIN CONTROLA] wait_micro()\n", LEVEL_DEBUG);
        wait_micro(esperar_proximo);
    }

    log_print("[MAIN CONTROLA] fim\n", LEVEL_DEBUG);

    return e;
}
