#ifndef CORTO_PLAZO_H_
#define CORTO_PLAZO_H_

#include <main.h>

void ejecutar_proceso(t_PCB* PCB);
void* planificador_corto_plazo(void* arg);
void actualizar_contexto(t_PCB* PCB, t_paquete* paquete);
void atender_syscall(t_PCB* PCB);

#endif 