IP = 
USER =  
PORT = 
PRJ_NAME = t1
SERVER_PROJECT_PATH = /home/$(USER)/$(PRJ_NAME)
PATH_MY_OS = /home/renato/Desktop/fse/$(PRJ_NAME)

compile:
	make clean
	make -C shared
	make -C dep prod
	make -C main prod

principal:
	make -C main run

andar_1:
	ID_ANDAR=1 make -C dep run

andar_2:
	ID_ANDAR=2 make -C dep run

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
