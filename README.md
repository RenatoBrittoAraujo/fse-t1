[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/OJtG4ZlI)
# FSE Trabalho 1 - Versão 2

V2 - Essa é a versão atual do projeto
V1 - Versão inicial, que não compila

## Mudanças da v2 para entrega inicial v1

Removi tudo que não era relevante pra entrega incial, como:
- sistema de teste (kkk)
- ambiente de dev e test (kkkkkk)
- dockerfile e docker-compose

## Descrição

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

## Como compilar 

`make all`

## Como rodar

Em outro terminal `make all_dep`
Em um terminal `make all_main`

O primeiro terminal rodará 2 andares de instâncias dependentes. 
O segundo rodará o servidor central.

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
| `(shared,main,dep)/src/Makefile_prod` | makefile específico para compilar o environment em seu nome.                                                            |
| `shared/inc/proto.h`                                 | inspirado no golang, serve para compartilhar structs comuns entre os apps, por exemplo as mensagens que usam (binário). |
