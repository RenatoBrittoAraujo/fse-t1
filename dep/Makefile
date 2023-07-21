RASP_DO_ESTACIONAMENTO_EH_1_2_OU_3=1

set_env:
# indica ao programa que deveria usar os pins GPIO pra essa rasp
	@export RASP_ESTACIONAMENTO_1_3=$(RASP_DO_ESTACIONAMENTO_EH_1_2_OU_3)

dev:
	make -f Makefile_dev

prod:
	make -f Makefile_prod

run_dev: set_env
	ID_ANDAR=$(ID_ANDAR) make -f Makefile_dev run

run_prod: set_env
	ID_ANDAR=$(ID_ANDAR) make -f Makefile_prod run

all_prod:
	make prod
	ID_ANDAR=$(ID_ANDAR) make run_prod

all_dev:
	make dev
	ID_ANDAR=$(ID_ANDAR) make run_dev

clean:
	@make -f Makefile_prod clean 



dev_test: set_env
	make -f Makefile_dev_test
