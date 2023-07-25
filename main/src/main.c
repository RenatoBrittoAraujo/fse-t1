#include <ncurses.h>
#include <stdlib.h>

#include "shared/inc/proto.h"
#include "shared/inc/shared_util.h"
#include "shared/inc/time.h"

#include "main/inc/interface.h"
#include "main/inc/controle.h"

int main()
{
    set_level(LEVEL_DEBUG);

    Estado *e = NULL;
    while (1)
    {
        read_input();
        // printf("\033[2J\033[1;1H");

        e = controla(e);
        printf("\n");
        fflush(NULL);
        e = processar_interface(e);
        // separa os logs de todas as iterações do loop principal
        printf("\n\n");
        fflush(NULL);
    }
}
