#include <main.h>

//GLOBAL SEMAPHORE
sem_t mx_cola_new;
sem_t sem_cola_new;
sem_t mx_cola_ready;
sem_t sem_cola_ready;
sem_t mx_cola_exit;
sem_t sem_cola_exit;
sem_t mx_lista_PCB;
sem_t mx_lista_interfaces;
sem_t mx_cola_priority_ready;

//GLOBAL VARIABLE
t_queue* cola_new;
t_queue* cola_ready;
t_queue* cola_exit;
t_queue* cola_priority_ready;
t_list* lista_PCB;
t_list* lista_interfaces;

t_config *config;

int socket_memoria;
int socket_cpu_dispatch;
int socket_cpu_interrupt;

t_log* log_kernel;

int main(int argc, char* argv[]) {
    log_kernel = log_create("kernel.log", "KERNEL", false, LOG_LEVEL_INFO);
    log_conexion = log_create("conexion.log", "CONEXION", false, LOG_LEVEL_TRACE);
    config = config_create(argv[1]);

    cola_new = queue_create();
    cola_ready = queue_create();
    cola_exit = queue_create();
    cola_priority_ready = queue_create();
    lista_PCB = list_create();
    lista_interfaces = list_create();

    //START SEMAPHORE 
    sem_init(&mx_lista_PCB, 0, 1);
    sem_init(&mx_lista_interfaces, 0, 1);
    sem_init(&mx_cola_new, 0, 1);
    sem_init(&mx_cola_ready, 0, 1);
    sem_init(&mx_cola_exit, 0, 1);
    sem_init(&mx_cola_priority_ready, 0, 1);
    sem_init(&sem_cola_new, 0, 0);
    sem_init(&sem_cola_ready, 0, 0);
    sem_init(&sem_cola_exit, 0, 0);

    pthread_t thread_largo_plazo;
    pthread_create(&thread_largo_plazo, NULL, crear_proceso, NULL);
    pthread_detach(thread_largo_plazo);

    pthread_t thread_consola;
    pthread_create(&thread_consola, NULL, consola, NULL);
    pthread_detach(thread_consola);

    pthread_t thread_corto_plazo;
    pthread_create(&thread_corto_plazo, NULL, planificador_corto_plazo, NULL);
    pthread_detach(thread_corto_plazo);

    pthread_t thread_largo_plazo_2;
    pthread_create(&thread_largo_plazo_2, NULL, eliminar_proceso, NULL);
    pthread_detach(thread_largo_plazo_2);

    iniciar_recursos();
    iniciar_multiprogramacion();

    // Conectar a CPU_DISPATCH
    char *ip_cpu = config_get_string_value(config, "IP_CPU");
    char *puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    socket_cpu_dispatch = crear_conexion(ip_cpu, puerto_cpu_dispatch);
    client_handshake(socket_cpu_dispatch, KERNEL, CPU);

    // Conectar a CPU_INTERRUPT
    char *puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
    socket_cpu_interrupt = crear_conexion(ip_cpu, puerto_cpu_interrupt);
    client_handshake(socket_cpu_interrupt, KERNEL, CPU);

    // Conectar a MEMORIA
    char *ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char *puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    socket_memoria = crear_conexion(ip_memoria, puerto_memoria);
    client_handshake(socket_memoria, KERNEL, MEMORIA);

    // Iniciar Servidor KERNEL
    char* puerto = config_get_string_value(config, "PUERTO_ESCUCHA");
    int socket_servidor = iniciar_servidor(puerto);

    // Esperar conexiones
    while(1){
        pthread_t thread;
        t_interface* interface = malloc(sizeof(t_interface));
        interface->socket = esperar_cliente(socket_servidor);
        pthread_create(&thread, NULL, atender_interface_IN, interface);
        pthread_detach(thread);
    }

    liberar_conexion(socket_memoria);
    liberar_conexion(socket_cpu_dispatch);
    liberar_conexion(socket_cpu_interrupt);

    sem_destroy(&mx_lista_PCB);

    log_destroy(log_kernel);
    log_destroy(log_conexion);
    config_destroy(config);

    return EXIT_SUCCESS;
}

void cambio_estado(t_PCB* PCB, estado_proceso nuevo_estado){
    if(PCB->estado != nuevo_estado){
        log_info(log_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: %s",
            PCB->PID,
            string_estado_proceso(PCB->estado),
            string_estado_proceso(nuevo_estado));
        PCB->estado = nuevo_estado;
    }
}

char* string_estado_proceso(estado_proceso estado){
    switch (estado) {
        case NEW:       return "NEW";
        case READY:     return "READY";
        case BLOCKED:   return "BLOCKED";
        case EXEC:      return "EXEC";
        case EXIT:      return "EXIT";
        case DELETED:   return "DELETED";
        default:        return "UNKNOWN";
    }
}


char* string_cola(t_queue* cola){
    char* lista = string_new();
    string_append(&lista, "[");

    int size = queue_size(cola);
    for (int i = 0; i < size; i++)
    {
        int* PID = list_get(cola->elements, i);
        char* string_PID = string_itoa(*PID);
        string_append(&lista, string_PID);
        free(string_PID);
        if (i != size - 1) string_append(&lista, ",");
    }

    string_append(&lista, "]");
    return lista;
}
