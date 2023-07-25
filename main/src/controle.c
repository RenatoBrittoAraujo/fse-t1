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

#define DEBUG 0

void set_entradas_inicial()
{
    IF_DEBUG log_print("[MAIN] set_entradas_inicial()\n", LEVEL_DEBUG);
}

bool is_nova_conexao(Estado *e)
{
    IF_DEBUG log_print("[MAIN] is_nova_conexao()\n", LEVEL_DEBUG);
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

void chama_dependente(ThreadState *ts, void *args)
{
    IF_DEBUG log_print("[MAIN THREAD] chama_dependente()\n", LEVEL_DEBUG);

    Estado *estado_main = (Estado *)args;
    Estado *estado_dep = NULL;

    char *ip;
    int porta;

    int dependente = -1;
    if (ts->thread_id == estado_main->t_dep_2->thread_id)
        dependente = ATOR_DEP2;
    if (ts->thread_id == estado_main->t_dep_1->thread_id)
        dependente = ATOR_DEP1;

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
        IF_DEBUG printf("[MAIN THREAD] ator para se escutar desconhecido. thread_id: %d\n", ts->thread_id);
        fflush(NULL);
        pthread_exit(NULL);
    }
    int id_andar = dependente == ATOR_DEP1 ? 1 : 2;

    // IF_DEBUG printf("[MAIN THREAD] I SENT ESTACIONAMENTO %d O ESTADO:\n", id_andar);
    // IF_DEBUG print_estado(estado_main);
    char *req = transforma_estado_em_string(estado_main);

    char res_buff[MAX_FRAME_SIZE];

    IF_DEBUG printf("[MAIN THREAD] chamando dependente no endereço: %s:%d\n", ip, porta);
    fflush(NULL);

    t_error err = call_tcp_ip_port(req, sizeof(Estado), ip, porta, res_buff);
    if (err == SHARED_TCP_IP_ERROR_CONNECTION_FAILED)
    {
        IF_DEBUG printf("[MAIN THREAD] servidor do andar %d não respondeu\n", id_andar);
        pthread_exit(NULL);
    }

    IF_DEBUG printf("[MAIN THREAD] call finalizada!\n");
    fflush(NULL);

    free(req);
    IF_DEBUG printf("[MAIN THREAD] request limpa da memoria\n");
    fflush(NULL);

    if (err != NO_ERROR)
    {
        IF_DEBUG printf("[MAIN THREAD] erro %lu no listen_tcp_ip_port() do dependente\n", err);
        fflush(NULL);
        pthread_exit(NULL);
    }

    IF_DEBUG printf("resposta sem erro = %p\n", res_buff);
    fflush(NULL);

    estado_dep = parse_string_estado(res_buff);

    IF_DEBUG printf("[MAIN THREAD] resposta parseada = %p\n", estado_dep);
    fflush(NULL);

    pthread_exit(estado_dep);
}

ThreadState *criar_thread_comunicar_dependente(Estado *e)
{
    IF_DEBUG log_print("[MAIN] criar_thread_comunicar_dependente()\n", LEVEL_DEBUG);

    ThreadState *t = create_thread_state(-1);
    t->response_size = sizeof(Estado);
    t->routine = chama_dependente;

    IF_DEBUG log_print("[MAIN] thread criada\n", LEVEL_DEBUG);
    return t;
}

// ================== MODIFICAR ESTADO ==================

int is_todas_as_vagas_andar_ocupadas(Estado *e, int id_andar)
{
    IF_DEBUG log_print("[MAIN] is_todas_as_vagas_andar_ocupadas()\n", LEVEL_DEBUG);
    int vagas = id_andar == 1 ? e->vagas_andar_1 : e->vagas_andar_2;
    return 0xff == vagas;
}

int is_todas_as_vagas_ocupadas(Estado *e)
{
    IF_DEBUG log_print("[MAIN] is_todas_as_vagas_ocupadas()\n", LEVEL_DEBUG);
    int nv = 1;
    for (int i = 1; i <= 2; i++)
        nv = nv && is_todas_as_vagas_andar_ocupadas(e, i);
    return nv;
}

int is_sensor_de_presenca_na_saida_ocupado(Estado *e)
{
    IF_DEBUG log_print("[MAIN] is_sensor_de_presenca_na_saida_ocupado()\n", LEVEL_DEBUG);
    e->motor_cancela_saida_ligado = 0;
}

int get_estado_motor_cancela(unsigned long sensor_de_presenca, unsigned long sensor_de_passagem)
{
    IF_DEBUG log_print("[MAIN] get_estado_motor_cancela()\n", LEVEL_DEBUG);

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
    IF_DEBUG log_print("[MAIN] decidir_estado_motor_cancela_saida()\n", LEVEL_DEBUG);

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
    IF_DEBUG log_print("[MAIN] controla()\n", LEVEL_DEBUG);

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

    if (e->sensor_de_presenca_entrada > e->sensor_de_passagem_entrada)
    {
        e->nova_presenca_entrada = 1;
    }
    else
    {
        if (e->nova_presenca_entrada == 1)
        {
            // novo carro entrou no estacionamento

            e->timestamp_last_entrada = get_time_mcs();
            e->id_last_entrada++;
            e->entrada_pendente = 1;

            // salva id do carro
            // salva horario entrada carro
            // salva que sistema esta esperando nova vaga ser ocupada
        }
        e->nova_presenca_entrada = 0;
    }

    if (e->sensor_de_presenca_saida > e->sensor_de_passagem_saida)
    {
        e->nova_presenca_saida = 1;
    }
    else
    {
        if (e->nova_presenca_saida == 1)
        {
            // carro saiu do estacionamento
            e->timestamp_last_saida = get_time_mcs();
            e->id_last_saida =
                e->saida_pendente = 1;
        }
        e->nova_presenca_saida = 0;
    }

    int vagas_mudaram = (e->vagas_andar_1 != e->last_vagas_andar_1) || (e->vagas_andar_2 != e->last_vagas_andar_2);

    int num_vagas_last = 0, num_vagas_now = 0;

    for (int i = 0; i < 8; i++)
        if (e->vagas_andar_1 & (1 << i))
            num_vagas_now++;
    for (int i = 0; i < 8; i++)
        if (e->vagas_andar_2 & (1 << i))
            num_vagas_now++;
    for (int i = 0; i < 8; i++)
        if (e->last_vagas_andar_1 & (1 << i))
            num_vagas_last++;
    for (int i = 0; i < 8; i++)
        if (e->last_vagas_andar_2 & (1 << i))
            num_vagas_last++;

    if (e->entrada_pendente && num_vagas_last + 1 == num_vagas_now)
    {
        int vaga_i;
        int andar;

        for (int i = 0; i < 8; i++)
            if (!!(e->vagas_andar_1 & (1 << i)) != !!(e->last_vagas_andar_1 & (1 << i)))
            {
                vaga_i = i;
                andar = 1;
            }

        for (int i = 0; i < 8; i++)
            if (!!(e->vagas_andar_2 & (1 << i)) != !!(e->last_vagas_andar_2 & (1 << i)))
            {
                vaga_i = i;
                andar = 2;
            }

        if (andar == 2)
            vaga_i += 8;

        if (e->id_vagas[vaga_i] != -1)
        {
            log_print("[MAIN] vaga de estacionamento estava ocupada!", LEVEL_ERROR);
        }

        e->last_entrada_i = vaga_i;

        e->id_vagas[vaga_i] = e->id_last_entrada;
        e->entrada_time[vaga_i] = get_time_mcs();
        e->estacionou_na_vaga = vaga_i;

        e->entrada_pendente = 0;
    }

    if (num_vagas_last - 1 == num_vagas_now)
    {
        int vaga_i;
        int andar;

        for (int i = 0; i < 8; i++)
            if (!!(e->vagas_andar_1 & (1 << i)) != !!(e->last_vagas_andar_1 & (1 << i)))
            {
                vaga_i = i;
                andar = 1;
            }

        for (int i = 0; i < 8; i++)
            if (!!(e->vagas_andar_2 & (1 << i)) != !!(e->last_vagas_andar_2 & (1 << i)))
            {
                vaga_i = i;
                andar = 2;
            }

        if (andar == 2)
            vaga_i += 8;
        e->last_saida_i = vaga_i;
    }

    if (e->saida_pendente)
    {
        if (e->last_saida_i == -1)
        {
            log_print("[MAIN] vaga de saida nao pode ser detectada!", LEVEL_ERROR);
        }
        else
        {
            int vaga_i = e->last_saida_i;
            if (e->id_vagas[vaga_i] == -1)
            {
                log_print("[MAIN] vaga de saida estava vazia!", LEVEL_ERROR);
            }

            int id = e->id_vagas[vaga_i];
            e->saiu_da_vaga = vaga_i;
            e->id_vagas[vaga_i] = -1;
            int tempo_segundos = (get_time_mcs() - e->entrada_time[vaga_i]) / 1e6;
            int preco = tempo_segundos * e->preco_por_segundo;
            e->preco_pago_last_carro = preco;
        }

        e->last_saida_i = -1;
        e->saida_pendente = 0;
    }

    e->last_vagas_andar_1 = e->vagas_andar_1;
    e->last_vagas_andar_2 = e->vagas_andar_2;

    // ========= VAGAS

    int todos_andares_lotados = 1;
    for (int i = 1; i <= e->num_andares; i++)
    {
        char buff[1000];
        IF_DEBUG sprintf(buff, "[MAIN CONTROLA] Analisando vagas do andar %d\n", i);
        IF_DEBUG log_print(buff, LEVEL_DEBUG);

        int andar_lotado = is_todas_as_vagas_andar_ocupadas(e, i);
        todos_andares_lotados *= andar_lotado;
    }
    e->estacionamento_lotado = is_todas_as_vagas_ocupadas(e);

    IF_DEBUG log_print("[MAIN CONTROLA] enviando request pros dependentes\n", LEVEL_DEBUG);
    e->t_dep_1->args = e->t_dep_2->args = e;
    start_thread(e->t_dep_1);
    start_thread(e->t_dep_2);

    IF_DEBUG log_print("[MAIN CONTROLA] esperando resposta do dependente 1\n", LEVEL_DEBUG);
    void *res_dep_1_void_p = wait_thread_response_with_deadline(e->t_dep_1, MCS_DEADLINE_RESPOSTA_DEPENDENTE);

    IF_DEBUG log_print("[MAIN CONTROLA] esperando resposta do dependente 2\n", LEVEL_DEBUG);
    void *res_dep_2_void_p = wait_thread_response_with_deadline(e->t_dep_2, MCS_DEADLINE_RESPOSTA_DEPENDENTE);

    if (res_dep_1_void_p == NULL)
    {
        log_print("[MAIN CONTROLA] dependente 1 não está respondendo...\n", LEVEL_ERROR);
    }
    else
    {
        IF_DEBUG log_print("[MAIN CONTROLA] combinando estado servidor dependente 1\n", LEVEL_DEBUG);
        Estado *res_dep_1 = (Estado *)res_dep_1_void_p;

        e->andar_1_lotado = res_dep_1->andar_1_lotado;
        e->vagas_andar_1 = res_dep_1->vagas_andar_1;
        e->sensor_de_presenca_entrada = res_dep_1->sensor_de_presenca_entrada;
        e->sensor_de_presenca_saida = res_dep_1->sensor_de_presenca_saida;
        e->sensor_de_passagem_entrada = res_dep_1->sensor_de_passagem_entrada;
        e->sensor_de_passagem_saida = res_dep_1->sensor_de_passagem_saida;
    }

    if (res_dep_2_void_p == NULL)
    {
        log_print("[MAIN CONTROLA] dependente 2 não está respondendo...\n", LEVEL_ERROR);
    }
    else
    {
        Estado *res_dep_2 = (Estado *)res_dep_2_void_p;

        IF_DEBUG log_print("[MAIN CONTROLA] combinando estado servidor dependente 2\n", LEVEL_DEBUG);

        e->andar_2_lotado = res_dep_2->andar_2_lotado;
        e->vagas_andar_2 = res_dep_2->vagas_andar_2;
        e->sensor_de_subida_de_andar = res_dep_2->sensor_de_subida_de_andar;
        e->sensor_de_descida_de_andar = res_dep_2->sensor_de_descida_de_andar;
    }

    IF_DEBUG log_print("[MAIN CONTROLA] resultados combinados, novo estado gerado\n", LEVEL_DEBUG);

    // ========= FIM
    unsigned long wait_time = MCS_PERIODO_MINIMO_EXEC - get_time_mcs() + e->tempo_ultima_execucao;

    if (MCS_PERIODO_MINIMO_EXEC > get_time_mcs() - e->tempo_ultima_execucao)
    {
        IF_DEBUG log_print("[MAIN CONTROLA] wait_micro()\n", LEVEL_DEBUG);
        wait_micro(wait_time);
    }

    IF_DEBUG log_print("[MAIN CONTROLA] fim\n", LEVEL_DEBUG);

    return e;
}
