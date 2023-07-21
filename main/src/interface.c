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
struct EstadoInterface *__ei = NULL;

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

EstadoInterface *incializar_estado_interface(EstadoEstacionamento *e)
{
    init();
    EstadoInterface *ei = (EstadoInterface *)malloc(sizeof(EstadoInterface));
    ei->e = e;
    return __ei = ei;
}

void desenha_interface(EstadoInterface *ei)
{
    // ====== DISPLAY
    printf("=== GERENCIADOR DE ESTACIONAMENTO ===");

    printf("| andares: %d |\n", ei->e->num_andares);

    printf("| sensor_deteccao_entrada: %ld | sensor_deteccao_saida: %ld |\n", ei->e->entrada->sensor_de_presenca_entrada, ei->e->entrada->sensor_de_presenca_saida);

    printf("| sensor_passagem_entrada: %ld | sensor_passagem_saida: %ld |\n", ei->e->entrada->sensor_de_presenca_entrada, ei->e->entrada->sensor_de_presenca_saida);
    fflush(NULL);
    

    printf("| sensores_de_subida: ");
    
    fflush(NULL);
    for (int i = 0; i < ei->e->num_andares - 1; i++)
    {
        printf("%d ", ei->e->sensor_de_subida_de_andar[i]);
        
        fflush(NULL);
    }
    printf("|\n");
    
    fflush(NULL);

    log_print("sensored_de_subida 174321597g319 \n", LEVEL_DEBUG);

    printf("| sensores_de_descida: ");
    
    fflush(NULL);
    for (int i = 0; i < ei->e->num_andares - 1; i++)
    {
        printf("%d ", ei->e->sensor_de_descida_de_andar[i]);
        
        fflush(NULL);
    }
    printf("|\n");
    
    fflush(NULL);

    log_print("id_andar = \n", LEVEL_DEBUG);

    for (int id_andar = 1; id_andar <= ei->e->num_andares; id_andar++)
    {
        log_print("id_andar = \n", LEVEL_DEBUG);

        EstadoAndar *andar = ei->e->andares[id_andar - 1];

        printf("| ----- | id_andar: %d | vagas_ocupadas:", id_andar);
        fflush(NULL);
        
        for (int i = 0; i < andar->num_vagas; i++)
        {
            printf("%d ", andar->vagas[i]);
            
            fflush(NULL);
        }
        printf("|\n");
        
        fflush(NULL);
    }
    log_print("----------- FIM -----------()\n", LEVEL_DEBUG);
}

EstadoEstacionamento *ler_comando(EstadoInterface *ei)
{
    
    printf("Comandos:\n");
    fflush(NULL);
    printf("%p\n", ei);
    fflush(NULL);
    printf("%p\n", ei->e);
    fflush(NULL);
    printf("%d\n", ei->e->estacionamento_fechado);
    fflush(NULL);
    if (ei->e->estacionamento_fechado)
        printf("\t0 - abrir estacionamento\n");
    else
        printf("\t0 - fechar estacionamento\n");

    for (int id_andar = 1; id_andar <= ei->e->num_andares; id_andar++)
    {
        printf("\t%d - fechar andar %d \n", id_andar, id_andar);
    }

    char c = get_inp_char();
    if (c == '0')
    {
        ei->e->estacionamento_fechado = -(ei->e->estacionamento_fechado - 1);
    }

    if (c >= '1' && c <= '9')
    {
        int i = c - '1';
        int val = ei->e->andares[i]->andar_lotado;
        ei->e->andares[i]->andar_lotado = -(val - 1);
    }

    return ei->e;
}

EstadoEstacionamento *processar_interface(EstadoEstacionamento *e)
{
    if (__ei == NULL)
    {
        __ei = incializar_estado_interface(e);
    }
    desenha_interface(__ei);
    return ler_comando(__ei);
}
