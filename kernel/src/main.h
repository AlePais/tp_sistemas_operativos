#ifndef MAIN_H_
#define MAIN_H_

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <pthread.h>
#include <utils/hello.h>
#include <utils/conexion.h>
#include <commons/config.h>
#include <commons/temporal.h>
#include <commons/collections/list.h>
#include <consola.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <utils/serialize.h>
#include <utils/deserialize.h>
#include <utils/estructuras.h>
#include <interruptor.h>

//EXTERN GLOBAL SEMAPHORE
extern sem_t mx_cola_new;
extern sem_t sem_cola_new;
extern sem_t mx_cola_ready;
extern sem_t sem_cola_ready;
extern sem_t mx_cola_exit;
extern sem_t sem_cola_exit;
extern sem_t mx_lista_PCB;
extern sem_t mx_lista_interfaces;
extern sem_t mx_cola_priority_ready;

//GLOBAL VARIABLE
extern t_queue* cola_new;
extern t_queue* cola_ready;
extern t_queue* cola_exit;
extern t_queue* cola_priority_ready;
extern t_list* lista_PCB;
extern t_list* lista_interfaces;

extern t_config *config;
extern t_log* log_kernel;

extern int socket_memoria;
extern int socket_cpu_dispatch;
extern int socket_cpu_interrupt;

typedef struct {
    int PID;
    int Quantum;
    sem_t mx_PCB;
    estado_proceso estado;
    t_registro_cpu registro_cpu;
    char* archivo;
    int* recursos_asignados;
    exit_reasons retorno;
} t_PCB;

#include <largo_plazo.h>
#include <recursos.h>
#include <interfaces.h>
#include <corto_plazo.h>
#include <multiprogramacion.h>

void* atender_cliente(void*);
void cambio_estado(t_PCB* PCB, estado_proceso nuevo_estado);
char* string_estado_proceso(estado_proceso estado);
char* string_cola(t_queue* cola);

#endif /* SERVER_H_ */