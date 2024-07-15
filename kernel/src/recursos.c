#include <recursos.h>

t_list* lista_recursos;

void iniciar_recursos(){

    lista_recursos = list_create();

    char** recursos = config_get_array_value(config, "RECURSOS");
    char** instancias = config_get_array_value(config, "INSTANCIAS_RECURSOS");

    for (int i = 0; recursos[i] != NULL; i++) {

        t_recurso* recurso = malloc(sizeof(t_recurso));

        recurso->index = i;
        recurso->nombre = recursos[i];
        recurso->instancias = atoi(instancias[i]);
        recurso->cola_blocked = queue_create();
        sem_init(&recurso->mx_recurso, 0, 1);

        list_add_in_index(lista_recursos, i, recurso);
    }

    free(recursos);
    free(instancias);
}

void bloquear_proceso_rec(t_PCB* req_PCB, t_recurso* recurso){
    cambio_estado(req_PCB, BLOCKED);
    log_info(log_kernel, "PID: %d - Bloqueado por: %s", req_PCB->PID, recurso->nombre);
    queue_push(recurso->cola_blocked, &req_PCB->PID);
}

void desbloquear_proceso_rec(t_PCB* req_PCB, t_recurso* recurso){
    bool eliminado = queue_pop_element(recurso->cola_blocked, &req_PCB->PID);
    if(!eliminado){
        int* PID = (int*) queue_pop(recurso->cola_blocked);

        sem_wait(&mx_lista_PCB);
        t_PCB* PCB = list_get(lista_PCB, *PID);
        sem_post(&mx_lista_PCB);

        sem_wait(&PCB->mx_PCB);
            desbloquear(PCB);
        sem_post(&PCB->mx_PCB);
    }
}

t_recurso* buscar_recurso(char* nombre){
    bool buscar_por_nombre(void* arg){
        t_recurso* rec = (t_recurso*) arg;
        bool coinciden = strcmp(rec->nombre, nombre);
        return !coinciden;
    };
    t_recurso* recurso = (t_recurso*) list_find(lista_recursos, buscar_por_nombre);
    return recurso;
}

void asignar_recurso(t_PCB* PCB, t_recurso* recurso){
    recurso->instancias -= 1;
    int* asignados = &PCB->recursos_asignados[recurso->index];
    *asignados += 1;
}

void desasignar_recurso(t_PCB* PCB, t_recurso* recurso){
    recurso->instancias += 1;
    int* asignados = &PCB->recursos_asignados[recurso->index];
    if(*asignados > 0){
        *asignados -= 1;
    }
}

void recusos_init(int** array_recursos){
    int cant_rec = list_size(lista_recursos);
    *array_recursos = (int *)malloc(cant_rec * sizeof(int));
    for (int i = 0; i < cant_rec; i++) {
        (*array_recursos)[i] = 0;
    }
}

bool queue_pop_element(t_queue* self, void* element) {
	return list_remove_element(self->elements, element);
}

void desbloquear(t_PCB* PCB){
    if(PCB->estado == BLOCKED){
        cambio_estado(PCB, READY);

        char* algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

        if(PCB->Quantum > 0 && strcmp(algoritmo, "VRR") == 0){
            sem_wait(&mx_cola_priority_ready);
                queue_push(cola_priority_ready, &PCB->PID);
                char* cola = string_cola(cola_priority_ready);
                log_info(log_kernel, "Cola Ready Prioridad: %s", cola);
                free(cola);
            sem_post(&mx_cola_priority_ready);
        }

        else{
            sem_wait(&mx_cola_ready);
                queue_push(cola_ready, &PCB->PID);
                char* cola = string_cola(cola_ready);
                log_info(log_kernel, "Cola Ready: %s", cola);
                free(cola);
            sem_post(&mx_cola_ready);
            if(PCB->Quantum >= 0){
                PCB->Quantum = config_get_int_value(config,"QUANTUM");
            }
        }

        sem_post(&sem_cola_ready);
    }
}
