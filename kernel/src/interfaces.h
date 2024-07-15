#ifndef INTERFACES_H_
#define INTERFACES_H_

#include <main.h>

typedef struct {
    bool connected;
    int socket;

    char* tipo;
    char* nombre;
    t_PCB* occupant;

    t_queue* cola_blocked;
    sem_t sem_cola_blocked;
    sem_t mx_cola_blocked;

    sem_t sem_occupied;
    sem_t mx_interface;
} t_interface;

void* atender_interface_IN(void* arg);
void* atender_interface_OUT(void* arg);
t_interface* buscar_interface(char* nombre);
bool admite_operacion(t_interface* interface, op_code operacion);

#endif 