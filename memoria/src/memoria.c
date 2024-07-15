#include <memoria.h>

t_memoria_fisica *memoria_fisica; 

sem_t mx_memoria;

int CANTIDAD_MARCOS;
int TAMANIO_PAGINA;
int TAMANIO_BITARRAY;

int main(int argc, char* argv[]) {
    char* nombre_config = argv[1];
    
    if(nombre_config == NULL || string_is_empty(nombre_config)) {
        log_error(log_conexion, "Ingresa nombre config");
        return EXIT_FAILURE;
    }

    log_conexion = log_create("memoria.log", "CONEXION", true, LOG_LEVEL_INFO);

    config = malloc(sizeof(t_config_memoria));

    cargar_config(config, nombre_config);

    inicializar_logger_paginacion();
    inicializar_logger_espacio_memoria();

    TAMANIO_PAGINA = config->tam_pagina;
    CANTIDAD_MARCOS = config->tam_memoria / TAMANIO_PAGINA;
    TAMANIO_BITARRAY = CANTIDAD_MARCOS / 8;

    inicializar_memoria_fisica();

    sem_init(&mx_memoria, 0, 1);

    log_info(log_conexion, "TAM_PAG: %i, CANT_MARCOS: %i, TAM_BIT: %i", TAMANIO_PAGINA, CANTIDAD_MARCOS, TAMANIO_BITARRAY);

    log_info(log_conexion, "Puerto %s ", config->puerto_escucha);

    int socket_servidor = iniciar_servidor(config->puerto_escucha);

    // Esperar conexiones
    while(1){
        pthread_t thread;
        int* socket_cliente = malloc(sizeof(int));
        *socket_cliente = esperar_cliente(socket_servidor);
        pthread_create(&thread, NULL, atender_cliente, socket_cliente);
        pthread_detach(thread);
    }
    
    list_destroy(lista_sockets_conectados);
    destruir_memoria_fisica();

    if(lista_procesos) {
        list_destroy_and_destroy_elements(lista_procesos, destruir_proceso);
    }

    if(lista_sockets_conectados) {
        list_destroy_and_destroy_elements(lista_sockets_conectados, free);
    }

    log_destroy(log_conexion);
    destruir_logger_paginacion();
    destruir_logger_espacio_memoria();

    sem_destroy(&mx_memoria);
    liberar_config(config);

    return EXIT_SUCCESS;
}


