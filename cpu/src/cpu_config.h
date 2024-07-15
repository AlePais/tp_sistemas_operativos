#ifndef CPU_CONFIG_H_
#define CPU_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>

typedef struct {
    char* ip_memoria;
    char* puerto_memoria;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_interrupt;
    int cantidad_entradas_tlb;
    char* algoritmo_tlb;
} t_config_cpu;

/**
* @fn cargar_config
* @brief Cargar config de CPU.
*/
void cargar_config(t_config_cpu* config, char * config_path);

/**
* @fn cargar_config
* @brief Cargar config de CPU.
*/
void liberar_config(t_config_cpu* config);

#endif /* CPU_CONFIG_H_ */
