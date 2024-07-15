#include <interruptor.h>

t_temporal* timer;

void* iniciar_quantum(void* arg){
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    int* quantum = (int*) arg;

    timer = temporal_create();
    pthread_cleanup_push(cancelado, arg);
    
    usleep(*quantum * 1000);

    pthread_cleanup_pop(0);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    temporal_destroy(timer);
    *quantum = 0;
    enviar_interrupcion();
    return NULL;
}

void enviar_interrupcion(){
    t_paquete* paquete = crear_paquete();
    agregar_operacion(paquete, INTERRUPCION);
    //buffer_add_uint32(paquete->buffer, 2);
    enviar_paquete(paquete,socket_cpu_interrupt);
    eliminar_paquete(paquete);
}

void cancelado(void* arg){
    int* quantum = (int*) arg;
    int tiempo_restante;
    tiempo_restante = *quantum - temporal_gettime(timer);
    if(tiempo_restante > 0)
        *quantum = tiempo_restante;
    else
        *quantum = 0;
    temporal_destroy(timer);
}