#ifndef MULTIPROGRAMACION_H_
#define MULTIPROGRAMACION_H_

#include <main.h>

typedef struct {
    int running;
    int grado;
    sem_t sem;
    sem_t mx;
} t_prog;

void iniciar_multiprogramacion();
void add_prog();
void remove_prog();
void modificar_grado(int valor);

void check_pause();
void detener_planificacion();
void iniciar_planificacion();

#endif