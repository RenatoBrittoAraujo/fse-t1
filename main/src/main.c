#include <stdlib.h>

#include "shared/inc/proto.h"
#include "shared/inc/shared_util.h"
#include "shared/inc/time.h"

#include "main/inc/interface.h"
#include "main/inc/controle.h"

int main()
{
    set_env();
    set_level(LEVEL_DEBUG);
    set_time_wait_ignore(0);

    EstadoEstacionamento *e = NULL;
    while (1)
    {
        e = controla(e);
        e = processar_interface(e);
    }
}
