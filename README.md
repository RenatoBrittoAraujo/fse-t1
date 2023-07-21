[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/OJtG4ZlI)
# FSE Trabalho 1

RENATO BRITTO ARAUJO 180027239

O projeto está em C.

O sistema é composto de 3 apis.
- API central
- API dependente 1
- API dependente 2

Sendo a api central o código encontrado na pasta `main` e a dependente no `dep`.

O código foi criado de forma mockada compilando no docker e rodando certas apis.

Note que o sistema ira rodar **como 3 processos na máquina alvo**, que é uma `Raspbian GNU/Linux 10 (buster)`.

O sistema alvo possui docker e docker-compose.

| estacionamento | ip           | placa (hostname) |
| -------------- | ------------ | ---------------- |
| 1              | 164.41.98.15 | rasp43           |
| 2              | 164.41.98.28 | rasp46           |
| 3              | 164.41.98.16 | rasp42           |
| 4 (andar 1)    | 164.41.98.27 | rasp48           |
| 4 (andar 2)    | 164.41.98.29 | rasp47           |

## Como rodar

O app não está funcionando completamente.
Em teoria, o sistema poderia ser levantado via docker-compose com um simples `sudo docker-compose up`.
Porém o servidor alvo da raspaberry pi funcionalmente não tem `docker-compose` (embora o comando `which docker-compose` aponte pro binário) porque ele não pode ser rodado.

### Excutar em dev

`sudo docker-compose up`

### Executar em prod

Em um terminal `make exec_app`
Em outro terminal `make exec_main`

O primeiro terminal rodará 2 andares de instâncias dependentes. 
O segundo rodará o servidor central.

## Problemas dessa solução

Acabei não fazendo tudo que desejava.

É notável que eu não consegui utilizar um debugger. O vscode apresentou 1001 dificuldades, e isso tornou o projeto inteiro bem mais difícil.

Compilação foi um problema, demorou muito até que todos os makefiles e gcc funcionasse sem probleminhas. Mesmo assim algumas partes do código do ambiente estão bem feias e repetitivas.

Existem problemas na comunicação, os servers e callers estão falhando. 

Não existem testes, várias abstrações estão incompletas e outros problemas grandes. Até os nomes ficaram inconsistentes...

## Proximos passos

- Fazer debugger funcionar
- Limpar memória (ponteiros soltos).

## Sistema de arquivos

Resolvi criar uma lib para qualquer futuro projeto em C. Por isso existem tantos arquivos aqui: solução é geral.
| padrão do arquivo                                    | significado                                                                                                             |
| ---------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------- |
| `shared`                                             | código compartilhadas no projeto.                                                                                       |
| `dep`                                                | código dos servidores dependentes.                                                                                      |
| `main`                                               | código do servidor principal.                                                                                           |
| `(shared,main,dep)/inc/`                             | arquivos include `.h`.                                                                                                  |
| `(shared,main,dep)/src/`                             | arquivos `.c`.                                                                                                          |
| `(shared,main,dep)/inc/`                             | arquivos `.obj`.                                                                                                        |
| `(shared,main,dep)/test_inc/`                        | arquivos include `.h` de testes.                                                                                        |
| `(shared,main,dep)/test_src/`                        | arquivos `.c` de testes.                                                                                                |
| `(shared,main,dep)/test_obj/`                        | arquivos `.obj` de testes.                                                                                              |
| `(shared,main,dep)/src/Makefile`                     | makefile para orquestrar compilação daquele projeto. Deve implementar comandos `dev`, `dev_test` e `prod`.              |
| `(shared,main,dep)/src/Makefile_(dev,dev_test,prod)` | makefile específico para compilar o environment em seu nome.                                                            |
| `shared/inc/proto.h`                                 | inspirado no golang, serve para compartilhar structs comuns entre os apps, por exemplo as mensagens que usam (binário). |

### Sistema de testes

Bibliotecas eram muita dor de cabeça pra tornar crossfunctional. Preferi fazer mais simples: os testes sempre devem definir `TEST_MODE=1` usando o preprocessador do C. Com isso, cada arquivo do código fonte à ser testado irá fazer seus imports baseado no valor de `TEST_MODE`, e quanto ele estiver definido, os imports devem ser de funções mockadas (cujo o desenvolvedor faz por conta própria).

Todos os testes devem imprimir o primeiro caractere em uma linha como ou `FAIL <nome_do_teste>` ou `SUCCESS <nome_do_teste>`. Qualquer linha sem esse padrão não é interpretada como o resultado de um caso de teste. A quantidade de linhas deve ser exatamente igual ao número de testes de arquivo específico em `*/test_src/*`.

O propósito é se assemelhar ao testes da linguagem Golang.
