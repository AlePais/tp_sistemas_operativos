#include <interfaces.h>

void* atender_interface_IN(void* arg)
{
    t_interface* interface = (t_interface*) arg;
    interface->nombre = NULL;
    interface->connected = true;

    while (interface->connected) {
        t_paquete* paquete_recibido = recibir_paquete(interface->socket);
        check_pause();
        op_code operacion = obtener_operacion(paquete_recibido);
        switch (operacion) {
            case HANDSHAKE:
                server_handshake(interface->socket, KERNEL, paquete_recibido);
                //Enviar Solicitud de Nombre
                t_paquete* paquete = crear_paquete();
                agregar_operacion(paquete, SOLICITUD_NOMBRE_INTERFAZ);
                enviar_paquete(paquete, interface->socket);
                eliminar_paquete(paquete);
                break;
            case ENVIO_NOMBRE_INTERFAZ:
                interface->nombre = buffer_read_string(paquete_recibido->buffer);
                interface->tipo = buffer_read_string(paquete_recibido->buffer);
                interface->occupant = NULL;
                interface->cola_blocked = queue_create();
                sem_init(&interface->sem_cola_blocked, 0, 0);
                sem_init(&interface->mx_cola_blocked, 0, 1);
                sem_init(&interface->sem_occupied, 0, 0);
                sem_init(&interface->mx_interface, 0, 1);

                sem_wait(&mx_lista_interfaces);
                    list_add(lista_interfaces, interface);
                sem_post(&mx_lista_interfaces);

                pthread_t thread;
                pthread_create(&thread, NULL, atender_interface_OUT, interface);
                pthread_detach(thread);

                sem_post(&interface->sem_occupied);
                break;
            case IO_FIN:
                sem_wait(&interface->mx_interface);
                    desbloquear(interface->occupant);
                    sem_post(&interface->occupant->mx_PCB);
                    interface->occupant = NULL;
                    sem_post(&interface->sem_occupied);
                sem_post(&interface->mx_interface);
                break;
            case DESCONEXION:
                sem_wait(&interface->mx_interface);
                    interface->connected = false;
                    if(interface->occupant != NULL){
                        enviar_a_exit(interface->occupant, INVALID_INTERFACE);
                        sem_post(&interface->occupant->mx_PCB);
                        interface->occupant = NULL;
                        sem_post(&interface->sem_occupied);
                    }
                sem_post(&interface->mx_interface);
                
                sem_wait(&interface->mx_cola_blocked);
                    int* PID = malloc(sizeof(int));
                    *PID = -1;
                    queue_push(interface->cola_blocked, PID);
                sem_post(&interface->mx_cola_blocked);
                sem_post(&interface->sem_cola_blocked);
                break;
            default:
                if(interface->nombre)
                    log_error(log_kernel, "INTERFACE: %s - Código de operación inválido.", interface->nombre);
                else
                    log_error(log_kernel, "INTERFACE - Código de operación inválido.");
                break;
        }
        eliminar_paquete(paquete_recibido); 
    }
    return NULL;
}

void* atender_interface_OUT(void* arg)
{
    t_interface* interface = (t_interface*) arg;

    while (true) {
        sem_wait(&interface->sem_cola_blocked);
        check_pause();
        sem_wait(&interface->mx_cola_blocked);
            int* PID = (int*) queue_pop(interface->cola_blocked);
            if(*PID == -1){
                free(PID);
                sem_post(&interface->mx_cola_blocked);
                break;
            }
            t_paquete* paquete_operacion = (t_paquete*) queue_pop(interface->cola_blocked);
        sem_post(&interface->mx_cola_blocked);

        sem_wait(&mx_lista_PCB);
            t_PCB* PCB = list_get(lista_PCB, *PID);
        sem_post(&mx_lista_PCB);

        sem_wait(&interface->sem_occupied);
        sem_wait(&interface->mx_interface);
        sem_wait(&PCB->mx_PCB);

            if(PCB->estado != BLOCKED){
                sem_post(&PCB->mx_PCB);
                sem_post(&interface->mx_interface);
                sem_post(&interface->sem_occupied);
                continue;
            }

            if(!interface->connected){
                enviar_a_exit(PCB, INVALID_INTERFACE);
                sem_post(&PCB->mx_PCB);
                sem_post(&interface->mx_interface);
                sem_post(&interface->sem_occupied);
                continue;
            }

            interface->occupant = PCB;
            enviar_paquete(paquete_operacion, interface->socket);
            eliminar_paquete(paquete_operacion);
        sem_post(&interface->mx_interface);
    }

    liberar_conexion(interface->socket);

    sem_wait(&mx_lista_interfaces);
        list_remove_element(lista_interfaces, interface);
    sem_post(&mx_lista_interfaces);

    //Eliminar interface
    free(interface->nombre);
    free(interface->tipo);
    queue_destroy(interface->cola_blocked);
    sem_destroy(&interface->sem_cola_blocked);
    sem_destroy(&interface->mx_cola_blocked);
    sem_destroy(&interface->sem_occupied);
    sem_destroy(&interface->mx_interface);
    free(interface);

    return NULL;
}

t_interface* buscar_interface(char* nombre){

    bool interface_conectada(void* arg){
        t_interface* arg_interface = (t_interface*) arg;
        sem_wait(&arg_interface->mx_interface);
        bool existe = strcmp(arg_interface->nombre, nombre) == 0;
        bool conectada = arg_interface->connected;
        if(!existe || !conectada){
            sem_post(&arg_interface->mx_interface);
            return false;
        }
        return true;
    };

    t_interface* interface;
    sem_wait(&mx_lista_interfaces);
        interface = (t_interface*) list_find(lista_interfaces, interface_conectada);
    sem_post(&mx_lista_interfaces);
    return interface;
}

bool admite_operacion(t_interface* interface, op_code operacion){
    bool admite;
    switch (operacion) {
        case IO_GEN_SLEEP:
            admite = strcmp(interface->tipo, "GENERICA") == 0;
            break;
        case IO_STDIN_READ:
            admite = strcmp(interface->tipo, "STDIN") == 0;
            break;
        case IO_STDOUT_WRITE:
            admite = strcmp(interface->tipo, "STDOUT") == 0;
            break;
        case IO_FS_CREATE:
        case IO_FS_DELETE:
        case IO_FS_TRUNCATE:
        case IO_FS_WRITE:
        case IO_FS_READ:
            admite = strcmp(interface->tipo, "DIALFS") == 0;
            break;
        default:
            admite = false;
    }
    return admite;
}

