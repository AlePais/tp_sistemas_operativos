#include<cpu_globals.h>

int interrupt = 0;
int socket_kernel;
int socket_memoria;
int tamanio_pagina;
t_config_cpu* config;
t_tlb tlb;
t_contexto contexto;
int vecesUsado;
