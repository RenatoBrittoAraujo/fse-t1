[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/OJtG4ZlI)
# FSE Trabalho 1 - Versão 2

V2 - Essa é a versão atual do projeto

V1 - Versão inicial, que não compila

## Mudanças da v2 para entrega inicial v1

Removi tudo que não era relevante pra entrega incial, como:
- sistema de teste 
- ambiente de dev e test
- dockerfile e docker-compose

## Requisitos cumpridos

- [x] Interface (Monitoramento): como visto no gif abaixo
- [x] Interface (Comandos):  
- [x] Monitoramento das Vagas
- [x] Contabilização de Pagamentos
- [x] Monitoramento das Vagas
- [x] Controle das Cancelas
- [x] Sensor de Passagem de Carros
- [x] Comunicação TCP/IP
- [x] Persistência dos serviços
- [x] Qualidade do Código / Execução
- [x] Qualidade e usabilidade acima da média.



## Interface

- A esquerda, tem o servidor dependente 1 - entrada
- No centro, tem o servidor dependente 2 - segundo andar
- Na direita, tem o servidor principal

![](figuras/Screenshot%20from%202023-07-25%2017-18-11.png)

## Descrição

O sistema é composto de 3 apis.
- API central
- API dependente 1
- API dependente 2

Sendo a api central o código encontrado na pasta `main` e a dependente no `dep`.

## Como compilar 

`make compile`

## Como rodar

Você precisa de 3 terminais, um para cada serviço:
- SERVIDOR PRINCIPAL: `make principal`
- SERVIDOR DEPENDENTE 1 (terreo): `make andar_1`
- SERVIDOR DEPENDENTE 2 (primeiro andar): `make andar_2`

Exemplo de execução:

![](figuras/simplescreenrecorder-2023-07-25_12.20.22.gif)

Note como os 3 servicos foram executados. A ordem de execução não importa (atendendo o requisito de persistencia).

A Interface possui os seguintes comandos:




## Sistema de arquivos

| padrão do arquivo                     | significado                                                                                                             |
| ------------------------------------- | ----------------------------------------------------------------------------------------------------------------------- |
| `shared`                              | código compartilhadas no projeto.                                                                                       |
| `dep`                                 | código dos servidores dependentes.                                                                                      |
| `main`                                | código do servidor principal.                                                                                           |
| `(shared,main,dep)/inc/`              | arquivos include `.h`.                                                                                                  |
| `(shared,main,dep)/src/`              | arquivos `.c`.                                                                                                          |
| `(shared,main,dep)/inc/`              | arquivos `.obj`.                                                                                                        |
| `(shared,main,dep)/src/Makefile`      | makefile do serviço. comando `prod` compila. comando `run` executa.                                                     |
| `(shared,main,dep)/src/Makefile_prod` | makefile do environment                                                                                                 |
| `shared/inc/proto.h`                  | inspirado no golang, serve para compartilhar structs comuns entre os apps, por exemplo as mensagens que usam (binário). |