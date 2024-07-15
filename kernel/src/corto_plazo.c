#include <corto_plazo.h>

pthread_t thread_interruptor;

void ejecutar_proceso(t_PCB* PCB){
    t_contexto contexto;
    contexto.PID = PCB->PID;
    contexto.motivo = NONE;
    contexto.registros = PCB->registro_cpu;

    cambio_estado(PCB, EXEC);

    t_paquete* paquete = crear_paquete();
    agregar_operacion(paquete, EJECUTAR_PROCESO);
    buffer_add_t_contexto(paquete->buffer, contexto);
    enviar_paquete(paquete,socket_cpu_dispatch);
    eliminar_paquete(paquete);

    if(PCB->Quantum != -1){
        pthread_create(&thread_interruptor, NULL, iniciar_quantum, &PCB->Quantum);
    }
}

void* planificador_corto_plazo(void* arg){
    while(true){

        sem_wait(&sem_cola_ready);
        int* PID;
        sem_wait(&mx_cola_priority_ready);
        sem_wait(&mx_cola_ready);
            if(queue_is_empty(cola_priority_ready)){
                PID = (int*) queue_pop(cola_ready);
            } else {
                PID = (int*) queue_pop(cola_priority_ready);
            }
        sem_post(&mx_cola_ready);
        sem_post(&mx_cola_priority_ready);

        sem_wait(&mx_lista_PCB);
            t_PCB* PCB = list_get(lista_PCB, *PID);
        sem_post(&mx_lista_PCB);
        
        check_pause();

        sem_wait(&PCB->mx_PCB);

        if(PCB->estado != READY){
            sem_post(&PCB->mx_PCB);
            continue;
        }

        do {        
            ejecutar_proceso(PCB);
            sem_post(&PCB->mx_PCB);
                t_paquete* paquete = recibir_paquete(socket_cpu_dispatch);
            sem_wait(&PCB->mx_PCB);
            actualizar_contexto(PCB, paquete);
                eliminar_paquete(paquete);
        } while (PCB->estado == EXEC);

        sem_post(&PCB->mx_PCB);
    }
    return NULL;
}

void actualizar_contexto(t_PCB* PCB, t_paquete* paquete){
    
    if(PCB->Quantum != -1){
        pthread_cancel(thread_interruptor);
        pthread_join(thread_interruptor, NULL);
    }

    if(PCB->estado != EXEC){
        enviar_a_exit(PCB, INTERRUPTED_BY_USER);
        return;
    }
    
    check_pause();

    op_code operacion = obtener_operacion(paquete);
    
    if(operacion != DEVOLUCION_PCB){
        log_error(log_kernel, "PID: %d - Devolución de CPU inválida. (1)", PCB->PID);
        enviar_a_exit(PCB, ERROR);
        return;
    }

    t_contexto contexto = buffer_read_t_contexto(paquete->buffer); 
    
    if(contexto.PID != PCB->PID){
        log_error(log_kernel, "PID: %d - Devolución de CPU inválida. (2)", PCB->PID);
        enviar_a_exit(PCB, ERROR);
        return;
    }
    
    PCB->registro_cpu = contexto.registros;

    switch(contexto.motivo){
        case INTERRUPT:     break;
        case CPU_IO:        atender_syscall(PCB); break;
        case END:           enviar_a_exit(PCB, SUCCESS); break; 
        case OUT_OF_MEMORY: enviar_a_exit(PCB, OUT_OF_MEMORY); break; 
        default: 
            log_error(log_kernel, "PID: %d - Devolución de CPU inválida. (3)", PCB->PID);
            enviar_a_exit(PCB, ERROR);
            break;
    }
    
    if(PCB->estado == EXEC && PCB->Quantum == 0){
        log_info(log_kernel, "PID: %d - Desalojado por fin de Quantum", PCB->PID);
        cambio_estado(PCB, READY);
        PCB->Quantum = config_get_int_value(config,"QUANTUM");
        sem_wait(&mx_cola_ready);
            queue_push(cola_ready, &PCB->PID);
            char* cola = string_cola(cola_ready);
            log_info(log_kernel, "Cola Ready: %s", cola);
            free(cola);
        sem_post(&mx_cola_ready);
        sem_post(&sem_cola_ready);
    }

    return;
}

void atender_syscall(t_PCB* PCB){
    char* nombre_recurso    = NULL;
    char* nombre_interface  = NULL;
    t_recurso* recurso      = NULL;
    t_interface* interface  = NULL;

    t_paquete* paquete = recibir_paquete(socket_cpu_dispatch);

    op_code operacion = obtener_operacion(paquete);
    switch(operacion){
    case WAIT:
        nombre_recurso = buffer_read_string(paquete->buffer);
        recurso =  buscar_recurso(nombre_recurso);
        if(recurso == NULL){
            enviar_a_exit(PCB, INVALID_RESOURCE);
            break;
        }
        sem_wait(&recurso->mx_recurso);
        asignar_recurso(PCB, recurso);
        if(recurso->instancias < 0){
            bloquear_proceso_rec(PCB, recurso);
        }
        sem_post(&recurso->mx_recurso);
        break;

    case SIGNAL:
        nombre_recurso = buffer_read_string(paquete->buffer);
        recurso =  buscar_recurso(nombre_recurso);
        if(recurso == NULL){
            enviar_a_exit(PCB, INVALID_RESOURCE);
            break;
        }
        sem_wait(&recurso->mx_recurso);
        desasignar_recurso(PCB, recurso);
        if(recurso->instancias < 1){
            desbloquear_proceso_rec(PCB, recurso);
        }
        sem_post(&recurso->mx_recurso);
        break;

    default:
        nombre_interface = buffer_read_string(paquete->buffer);
        interface = buscar_interface(nombre_interface);
        if(interface == NULL){
            enviar_a_exit(PCB, INVALID_INTERFACE);
            break; 
        }
        if(!admite_operacion(interface, operacion)){
            sem_post(&interface->mx_interface);
            enviar_a_exit(PCB, INVALID_INTERFACE);
            break; 
        }

        cambio_estado(PCB, BLOCKED);
        log_info(log_kernel, "PID: %d - Bloqueado por: %s", PCB->PID, interface->nombre);

        paquete->buffer->offset = paquete->buffer->stream;
        sem_wait(&interface->mx_cola_blocked);
            queue_push(interface->cola_blocked, &PCB->PID);
            queue_push(interface->cola_blocked, paquete);
        sem_post(&interface->mx_cola_blocked);
        sem_post(&interface->sem_cola_blocked);
        sem_post(&interface->mx_interface);
        paquete = NULL;
        break;
    }

    if(paquete)
        eliminar_paquete(paquete);
    if(nombre_recurso)
        free(nombre_recurso);
    if(nombre_interface)
        free(nombre_interface);
    
    return;
}
