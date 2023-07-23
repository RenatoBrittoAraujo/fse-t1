#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

#include "shared/inc/shared_util.h"
#include "shared/inc/proto.h"

#include "main/inc/interface.h"

#define NONE '\0'
#define TOGGLE_ESTACIONAMENTO '0';

int initialized = 0;

void init()
{
    if (!initialized)
    {
        initialized = 1;
        // transforma leitura de input em non-blocking
        fcntl(0, F_SETFL, O_NONBLOCK);
    }
}

// lê o buffer de input. Retorna NONE se nada for lido.
// note que apenas 1 caractere é lido, não deve ser problema.
char get_inp_char()
{
    char buff[1];
    if (read(0, buff, 1) < 1)
        return NONE;
    return buff[0];
}

void desenha_interface(EstadoEstacionamento *e)
{
    // ====== DISPLAY
    printf("=== GERENCIADOR DE ESTACIONAMENTO ===");

    printf("| andares: %d |\n", e->num_andares);

    printf("| sensor_deteccao_entrada: %ld | sensor_deteccao_saida: %ld |\n", e->sensor_de_presenca_entrada, e->sensor_de_presenca_saida);

    printf("| sensor_passagem_entrada: %ld | sensor_passagem_saida: %ld |\n", e->sensor_de_presenca_entrada, e->sensor_de_presenca_saida);
    fflush(NULL);

    printf("| sensor_de_subida: %d | sensor_de_descida: %d |\n", e->sensor_de_subida_de_andar, e->sensor_de_descida_de_andar);

    for (int id_andar = 1; id_andar <= e->num_andares; id_andar++)
    {
        log_print("id_andar = \n", LEVEL_DEBUG);
        int vagas = id_andar == 1 ? e->vagas_andar_1 : e->vagas_andar_2;
        int num_vagas = 8;

        printf("| ----- | id_andar: %d | vagas_ocupadas:", id_andar);
        fflush(NULL);

        for (int i = 0; i < num_vagas; i++)
        {
            printf("%d ", !!(vagas & (1 << i)));
        }
        printf("|\n");
        fflush(NULL);
    }
    log_print("----------- FIM -----------()\n", LEVEL_DEBUG);
}

EstadoEstacionamento *ler_comando(EstadoEstacionamento *e)
{

    printf("Comandos:\n");

    if (e->estacionamento_fechado)
        printf("\t0 - abrir estacionamento\n");
    else
        printf("\t0 - fechar estacionamento\n");

    for (int id_andar = 1; id_andar <= e->num_andares; id_andar++)
    {
        printf("\t%d - fechar andar %d \n", id_andar, id_andar);
    }

    char c = get_inp_char();
    if (c == '0')
    {
        e->estacionamento_fechado = -(e->estacionamento_fechado - 1);
    }

    if (c >= '1' && c <= '9')
    {
        int i = c - '0';
        if (i == 1)
        {

            e->andar_1_fechado = 1 - e->andar_1_fechado;
        }
        else if (i == 2)
        {
            e->andar_2_fechado = 1 - e->andar_1_fechado;
        }
        else
        {
            printf("Comando invalido, andar %d desconhecido\n", i);
        }
    }

    return e;
}

EstadoEstacionamento *processar_interface(EstadoEstacionamento *e)
{
    desenha_interface(e);
    e = ler_comando(e);
    return e;
}
