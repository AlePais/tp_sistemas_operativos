#ifndef LARGO_PLAZO_H_
#define LARGO_PLAZO_H_

#include <main.h>

void* crear_proceso(void*);
void* eliminar_proceso(void* arg);
char* string_exit_reason(exit_reasons code);
void enviar_a_exit(t_PCB* PCB, exit_reasons retorno);

#endif 