#include <largo_plazo.h>

void* crear_proceso(void* arg){
    while(1){
        sem_wait(&sem_cola_new);
        add_prog();
        check_pause();
        sem_wait(&mx_cola_new);
        int* PID = (int*) queue_pop(cola_new);
        sem_post(&mx_cola_new);

        sem_wait(&mx_lista_PCB);
        t_PCB* PCB = list_get(lista_PCB, *PID);
        sem_post(&mx_lista_PCB);

        sem_wait(&PCB->mx_PCB);

        if(PCB->estado == NEW){

            t_paquete* paquete = crear_paquete();
            agregar_operacion(paquete, CREAR_PROCESO);
            buffer_add_uint32(paquete->buffer, PCB->PID);
            buffer_add_string(paquete->buffer, PCB->archivo);
            enviar_paquete(paquete,socket_memoria);
            eliminar_paquete(paquete);

            //OK de Memoria
            paquete = recibir_paquete(socket_memoria);
            eliminar_paquete(paquete);

            //moverlo a ready y cambiar estado
            cambio_estado(PCB, READY);
            sem_wait(&mx_cola_ready);
                queue_push(cola_ready, &PCB->PID);
                char* cola = string_cola(cola_ready);
                log_info(log_kernel, "Cola Ready: %s", cola);
                free(cola);
            sem_post(&mx_cola_ready);
            sem_post(&sem_cola_ready);
        }
        else{
            remove_prog();
        }

        sem_post(&PCB->mx_PCB);
    }
    return NULL;
}

void* eliminar_proceso(void* arg){
    while(1){
        sem_wait(&sem_cola_exit);

        sem_wait(&mx_cola_exit);
            int* PID = (int*) queue_pop(cola_exit);
        sem_post(&mx_cola_exit);
        
        sem_wait(&mx_lista_PCB);
            t_PCB* PCB = list_get(lista_PCB, *PID);
        sem_post(&mx_lista_PCB);

        check_pause();
        sem_wait(&PCB->mx_PCB);

        switch (PCB->estado) {
            case NEW:
                cambio_estado(PCB, EXIT);
                PCB->retorno = INTERRUPTED_BY_USER;
                log_info(log_kernel, "Finaliza el proceso %d - Motivo: %s", PCB->PID, string_exit_reason(PCB->retorno));
                cambio_estado(PCB, DELETED);
                break;
            case EXEC:
                cambio_estado(PCB, EXIT);
                enviar_interrupcion();
                break;
            case READY:
            case BLOCKED:
                enviar_a_exit(PCB, INTERRUPTED_BY_USER);
                break;
            case EXIT:
                //Baja de Memoria
                t_paquete* paquete = crear_paquete();
                agregar_operacion(paquete, FIN_PROCESO);
                buffer_add_uint32(paquete->buffer, PCB->PID);
                enviar_paquete(paquete,socket_memoria);
                eliminar_paquete(paquete);

                //Liberar recursos
                int cant_rec = list_size(lista_recursos);
                for (int i = 0; i < cant_rec; i++) {

                    t_recurso* recurso = list_get(lista_recursos, i);
                    sem_wait(&recurso->mx_recurso);
                    int rec_asing = PCB->recursos_asignados[i];
                    for (int j = 0; j < rec_asing; j++){

                        desasignar_recurso(PCB, recurso);
                        if(recurso->instancias < 1){
                            desbloquear_proceso_rec(PCB, recurso);
                        }

                    }
                    sem_post(&recurso->mx_recurso);
                }

                cambio_estado(PCB, DELETED);
                remove_prog();
                break;
            default: 
                break;
        }
        sem_post(&PCB->mx_PCB);
    }
    return NULL;
}

char* string_exit_reason(exit_reasons code){
	switch (code) {
		case SUCCESS: return "SUCCESS";
		case INVALID_RESOURCE: return "INVALID_RESOURCE";
		case INVALID_INTERFACE: return "INVALID_INTERFACE";
		case OUT_OF_MEMORY: return "OUT_OF_MEMORY";
		case INTERRUPTED_BY_USER: return "INTERRUPTED_BY_USER";
		default: return "UNKNOWN";
	}
}

void enviar_a_exit(t_PCB* PCB, exit_reasons retorno){
    cambio_estado(PCB, EXIT);
    PCB->retorno = retorno;

    sem_wait(&mx_cola_exit);
        queue_push(cola_exit, &PCB->PID);
    sem_post(&mx_cola_exit);
    sem_post(&sem_cola_exit);

    log_info(log_kernel, "Finaliza el proceso %d - Motivo: %s", PCB->PID, string_exit_reason(PCB->retorno));
}