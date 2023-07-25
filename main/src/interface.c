#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ncurses.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#define MAXBYTES 80

#include "shared/inc/shared_util.h"
#include "shared/inc/time.h"
#include "shared/inc/proto.h"
#include "shared/inc/threads.h"

#include "main/inc/interface.h"

#define NONE '\0'
#define TOGGLE_ESTACIONAMENTO '0';
#define DRAW_FREQUENCY 100

int readv;
int finished = 0;

void get_inp_char(ThreadState *ts, void *args)
{
    cbreak();    // Disable line buffering (pass characters immediately to the program)
    noecho();    // Don't echo user input to the screen

    // freopen("/dev/null", "w", stdout);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    // Initialize ncurses again for another session
    initscr();

    int c = getch();
    readv = c;

    refresh(); // Refresh the screen

    // End ncurses for the second session
    endwin();

    finished = 1;

    pthread_exit((void *)c);
}

void desenha_interface(Estado *e)
{
    // ====== DISPLAY
    printf("|------------ GERENCIADOR DE ESTACIONAMENTO ------------|\n");
    printf("        timestamp (microsegundos):  %lu                  \n", get_time_mcs());
    printf("        numero de andares:          %d                   \n", e->num_andares);
    printf("        sensor_abertura_entrada:    %lu    \n", e->sensor_de_presenca_entrada);
    printf("        sensor_fechamento_entrada:  %lu \n", e->sensor_de_passagem_entrada);
    printf("        sensor_abertura_saida:      %lu     \n", e->sensor_de_presenca_saida);
    printf("        sensor_fechamento_saida:    %lu   \n", e->sensor_de_passagem_saida);
    printf("        sensor_de_subida:           %lu   \n", e->sensor_de_subida_de_andar);
    printf("        sensor_de_descida:          %lu  \n", e->sensor_de_descida_de_andar);
    printf("        timestamp_last_entrada:     %lu  \n", e->timestamp_last_entrada);
    printf("        timestamp_last_saida:       %lu  \n", e->timestamp_last_saida);
    printf("        id_last_entrada:            %lu  \n", e->id_last_entrada);
    printf("        id_last_saida:              %lu  \n", e->id_last_saida);

    if (e->estacionou_na_vaga != -1 || 1)
    {

        printf("      ULTIMO CARRO ESTACIONOU NA VAGA %d DO %d ANDAR  \n", e->estacionou_na_vaga % 8+1, e->estacionou_na_vaga / 8 + 1);
    }
    if (e->saiu_da_vaga != -1 || 1)
    {
        printf("      ULTIMO CARRO SAIU NA VAGA %d DO %d ANDAR  \n", e->saiu_da_vaga % 8+1, e->saiu_da_vaga / 8 + 1);
    }
    for (int id_andar = 1; id_andar <= e->num_andares; id_andar++)
    {
        printf("|---------------------- andar %d ------------------------|\n", id_andar);
        int vagas = id_andar == 1 ? e->vagas_andar_1 : e->vagas_andar_2;
        int num_vagas = 8;

        printf("|          vagas_ocupadas: ", id_andar);
        fflush(NULL);

        for (int i = 0; i < num_vagas; i++)
        {
            printf("%d ", !!(vagas & (1 << i)));
        }
        printf("             |\n");
        int fechado = id_andar == 1 ? e->andar_1_fechado : e->andar_2_fechado;
        int lotado = id_andar == 1 ? e->andar_1_lotado : e->andar_2_lotado;
        printf("|          lotado: %d              fechado: %d            |\n", lotado, fechado);
        fflush(NULL);
    }
    printf("|-------------------------------------------------------|\n\n");
}

Estado *ler_comando(Estado *e)
{

    printf(" ------------ Comandos: ------------\n");

    if (e->estacionamento_fechado)
        printf("\t0 - ABRIR estacionamento\n");
    else
        printf("\t0 - FECHAR estacionamento\n");

    for (int id_andar = 1; id_andar <= e->num_andares; id_andar++)
    {
        int fechado = id_andar == 1 ? e->andar_1_fechado : e->andar_2_fechado;
        printf("\t%d - %s andar %d \n", id_andar, fechado ? "ABRIR" : "FECHAR", id_andar);
    }

        printf("\t3 - FORÇAR abertura cancela entrada \n");
        printf("\t4 - FORÇAR abertura cancela saida \n");
        printf("\t5 - FORÇAR fechamento cancela entrada \n");
        printf("\t6 - FORÇAR fechamento cancela saida \n");


    // input esta no readv, se for 'f', não foi lido

    char c = readv;
    int encontrado = 0;

    if (c == '0')
    {
        printf("----- ESTACIONAMENTO FOI %s!\n", e->estacionamento_fechado ? "ABERTO" : "FECHADO");
        e->estacionamento_fechado = -(e->estacionamento_fechado - 1);
        encontrado = 1;
    }

    if (c >= '1' && c <= '9')
    {
        encontrado = 1;
        int i = c - '0';
        if (i == 1)
        {
            printf("----- ESTACIONAMENTO 1 FOI %s!\n", e->andar_1_fechado ? "ABERTO" : "FECHADO");
            e->andar_1_fechado = 1 - e->andar_1_fechado;
        }

        if (i == 2)
        {
            printf("----- ESTACIONAMENTO 2 FOI %s!\n", e->andar_2_fechado ? "ABERTO" : "FECHADO");
            e->andar_2_fechado = 1 - e->andar_2_fechado;
        }
    }

        if (c >= '3' && c <= '6')
    {
        encontrado = 1;
        int i = c - '0';
        if (i == 3)
        {
            printf("----- CANCELA ENTRADA ABERTA");
            e->override_motor_cancela_entrada = get_time_mcs();
        }
        if (i == 4)
        {
            printf("----- CANCELA SAIDA ABERTA");
            e->override_motor_cancela_saida = get_time_mcs();
        }

        if (i == 5)
        {
            printf("----- CANCELA ENTRADA FECHADA");
            e->override_motor_0_cancela_entrada = get_time_mcs();
        }
        if (i == 6)
        {
            printf("----- CANCELA SAIDA FECHADA");
            e->override_motor_0_cancela_saida = get_time_mcs();
        }
    }

    if (c == '\n' || c == 'r')
    {
        return e;
    }

    // if (!encontrado)
    // {
    //     printf("-----  Comando invalido '%c' %d\n", c, c);
    // }

    return e;
}

void read_input()
{
    readv = 'f';
    finished = 0;

    ThreadState *t = create_thread_state(-1);
    t->routine = get_inp_char;

    start_thread(t);
    wait_micro(100 * MILLI);
    // printf("THREAD END\n");
    // fflush(NULL);
    char c;
    // Request the cancellation of the thread
    // printf("pthread_cancel\n");
    // fflush(NULL);
    pthread_cancel(t->thread_id);
    // printf("pthread_join\n");
    // fflush(NULL);
    pthread_join(t->thread_id, NULL);

    // freopen("/dev/tty", "w", stdout);

    // printf("JOINED! with c = %d\n", readv);
    // fflush(NULL);
    if (!finished)
    {
        refresh(); // Refresh the screen
        // End ncurses for the second session
        endwin();
    }
    finished = 0;
}

int inicial = 1;

int t = 0;
Estado *processar_interface(Estado *e)
{
    desenha_interface(e);
    e = ler_comando(e);

    return e;
}
