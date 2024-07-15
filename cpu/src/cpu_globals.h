#ifndef CPU_GLOBALS_H_
#define CPU_GLOBALS_H_

#include<utils/estructuras.h>
#include<cpu_config.h>

extern int interrupt;
extern int socket_kernel;
extern int socket_memoria;
extern int tamanio_pagina;
extern t_config_cpu* config;
extern t_tlb tlb;
extern t_contexto contexto;
extern int vecesUsado;

#endif /* CPU_GLOBALS_H_ */
