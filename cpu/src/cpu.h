#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <commons/config.h>
#include <utils/conexion.h>
#include <utils/estructuras.h>
#include <instruction_processor.h>
#include <cpu_globals.h>

/**
* @fn cargar_tamanio_pagina
* @brief Solicita y carga tamaño de pagina configurado en modulo de memoria.
*/
void cargar_tamanio_pagina();

/**
* @fn procesar_instruccion
* @brief Procesa instruccion de proceso recibido de kernel.
*/
void procesar_instruccion(int socket_kernel, int socket_memoria, t_paquete paquete_recibido, int *exit_reason);

/**
* @fn iniciar_server_interrupt
* @brief Iniciar socket que espera interrupcciones.
*/
void* iniciar_server_interrupt(void* arg);

#endif /* CPU_H_ */