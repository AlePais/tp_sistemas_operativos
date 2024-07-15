#ifndef RECURSOS_H_
#define RECURSOS_H_

#include <main.h>

typedef struct {
    int index;
    char* nombre;
    int instancias;
    t_queue* cola_blocked;
    sem_t mx_recurso;
} t_recurso;

extern t_list* lista_recursos;

void iniciar_recursos();
void bloquear_proceso_rec(t_PCB* req_PCB, t_recurso* recurso);
void desbloquear_proceso_rec(t_PCB* req_PCB, t_recurso* recurso);
t_recurso* buscar_recurso(char* nombre);
void asignar_recurso(t_PCB* PCB, t_recurso* recurso);
void desasignar_recurso(t_PCB* PCB, t_recurso* recurso);
void recusos_init(int** array_recursos);
bool queue_pop_element(t_queue* self, void* element);
void desbloquear(t_PCB* PCB);

#endif 