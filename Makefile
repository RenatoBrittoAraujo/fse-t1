IP = 164.41.98.15
USER = renatoaraujo
PASS = 180027239
PORT = 13508
PRJ_NAME = t1_v3
SERVER_PROJECT_PATH = /home/$(USER)/$(PRJ_NAME)
PATH_MY_OS = /home/renato/Desktop/fse/$(PRJ_NAME)

all:
	make clean
	make -C shared
	make -C dep
	make -C main
	make -C main run_prod

all_dep:
	make clean
	make -C shared
	make -C dep
	make -C main
	make -C main run_prod

#  CONFIGURACOES DE DEPLOY
deps_dev:
	sudo apt install docker-compose
	sudo apt install sshpass

deps_prod:
# sudo apt-get install

ssh:
	sshpass -p $(PASS) ssh $(USER)@$(IP) -p $(PORT) 

ssh_nopass:
	ssh $(USER)@$(IP) -p $(PORT) 

install:
	rsync -av --exclude=".git" -e 'sshpass -p $(PASS) ssh -p $(PORT)' . $(USER)@$(IP):$(SERVER_PROJECT_PATH)
	@echo "app installed on target:$(SERVER_PROJECT_PATH)"

pull:
	rsync -av --exclude=".git" -e 'sshpass -p $(PASS) ssh -p $(PORT)' $(USER)@$(IP):$(SERVER_PROJECT_PATH) .. 
	@echo "app installed on target:$(SERVER_PROJECT_PATH)"


clean:
	make -C shared clean
	make -C main clean
	@echo " find . -type f -name '*.o' -delete"; find . -type f -name '*.o' -delete

commita:
	@git add .
	@git commit -m "$(shell date)"
	@git push materia main
	@git push origin master

.PHONY: all