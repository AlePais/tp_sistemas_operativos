#include <cpu.h>

int main(int argc, char* argv[]) {
    log_conexion = log_create("cpu.log", "CONEXION", true, LOG_LEVEL_INFO);

    char * config_path = argv[1];

    config = malloc(sizeof(t_config_cpu));
    cargar_config(config, config_path);
    
    // Conectar a MEMORIA
    socket_memoria = crear_conexion(config->ip_memoria, config->puerto_memoria);
    client_handshake(socket_memoria, CPU, MEMORIA);

    //SOLICITAR TAMAÑO DE PAGINA A MEMORIA
    cargar_tamanio_pagina();

    // Inicializo TLB
    tlb.l_registro_tlb = list_create();
    tlb.next_fifo = 0;

    // Iniciar Servidor CPU_INTERRUPT en otro Thread
    pthread_t thread;
    pthread_create(&thread, NULL, iniciar_server_interrupt, NULL);
    pthread_detach(thread);

    // Iniciar Servidor CPU_DISPATCH
    int socket_servidor= iniciar_servidor(config->puerto_escucha_dispatch);

    // Espero a KERNEL
    socket_kernel = esperar_cliente(socket_servidor);

    bool connected = false;
    while (!connected) {
        t_paquete* paquete_recibido = recibir_paquete(socket_kernel);
        op_code operacion = obtener_operacion(paquete_recibido);
        if (operacion == HANDSHAKE) {
            server_handshake(socket_kernel, CPU, paquete_recibido);
            connected = true;
        }
        else{
            log_trace(log_conexion, "Error en handshake");
        }
        eliminar_paquete(paquete_recibido);
    }

    while (connected) {
        interrupt = 1;

        t_paquete* paquete_recibido = recibir_paquete(socket_kernel);
        op_code operacion = obtener_operacion(paquete_recibido);
        switch (operacion) {
            case EJECUTAR_PROCESO:
                instruction_process(paquete_recibido);
                break;
            case DESCONEXION:
                log_trace(log_conexion, "El cliente se desconecto");
                connected = false;
                break;
            default:
                log_trace(log_conexion, "Codigo de operacion invalido: %d", operacion);
                connected = false;
                break;
        }
        eliminar_paquete(paquete_recibido);
    }

    free(config_path);
    liberar_conexion(socket_kernel);
    liberar_conexion(socket_memoria);
    liberar_config(config);
    return EXIT_SUCCESS;
}


void* iniciar_server_interrupt(void* arg)
{
    int socket_servidor_interrupt  = iniciar_servidor(config->puerto_escucha_interrupt);

    // Espero a KERNEL
    int socket_kernel_interrupt = esperar_cliente(socket_servidor_interrupt);

    bool connected_interrupt  = false;
    while (!connected_interrupt ) {
        t_paquete* paquete_recibido = recibir_paquete(socket_kernel_interrupt);
        op_code operacion = obtener_operacion(paquete_recibido);
        if (operacion == HANDSHAKE) {
            server_handshake(socket_kernel_interrupt, CPU, paquete_recibido);
            connected_interrupt  = true;
            log_trace(log_conexion, "Handshake Exitoso interrupt ");
        }
        else{
            log_trace(log_conexion, "Error en handshake");
        }
        eliminar_paquete(paquete_recibido);
    }

    while (connected_interrupt) {
        t_paquete* paquete_recibido = recibir_paquete(socket_kernel_interrupt);
        op_code operacion = obtener_operacion(paquete_recibido);
        switch (operacion) {
            case INTERRUPCION:
                interrupt = 0;
                break;
            case DESCONEXION:
                log_trace(log_conexion, "El cliente se desconecto");
                connected_interrupt = false;
                break;
            default:
                log_trace(log_conexion, "Codigo de operacion invalido: %d", operacion);
                connected_interrupt = false;
                break;
        }
        eliminar_paquete(paquete_recibido);
    }
    liberar_conexion(socket_kernel_interrupt);

    pthread_exit(NULL);
}

void cargar_tamanio_pagina()
{
    t_paquete* paquete_a_memoria = crear_paquete();
    agregar_operacion(paquete_a_memoria, TAMANIO_PAGINA_SOLICITUD);
    buffer_add_uint32(paquete_a_memoria->buffer, 1);
	enviar_paquete(paquete_a_memoria, socket_memoria);
	eliminar_paquete(paquete_a_memoria);

    t_paquete* paquete_recibido_memoria = recibir_paquete(socket_memoria);
    op_code operacion = obtener_operacion(paquete_recibido_memoria);
    if (operacion == TAMANIO_PAGINA_RESPUESTA) {
        tamanio_pagina = buffer_read_uint32(paquete_recibido_memoria->buffer);
    }
	eliminar_paquete(paquete_recibido_memoria);
}
