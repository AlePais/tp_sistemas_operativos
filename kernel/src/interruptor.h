#ifndef H_INTERRUPTOR
#define H_INTERRUPTOR

#include <main.h>

void* iniciar_quantum(void* arg);
void enviar_interrupcion();

void cancelado(void* arg);

#endif