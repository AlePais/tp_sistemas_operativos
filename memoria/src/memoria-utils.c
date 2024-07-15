#include <memoria-utils.h>

t_list *lista_procesos;

t_list *lista_sockets_conectados;

t_config_memoria *config;

t_log *log_memoria;

void* atender_cliente(void* arg)
{
    int* socket_cliente = (int*) arg;
    bool connected = true;
    
    while (connected) {
        t_paquete* paquete_recibido = recibir_paquete(*socket_cliente);
        op_code operacion = obtener_operacion(paquete_recibido);

        log_trace(log_conexion, "Socket que llega %i", *socket_cliente);

        if((operacion != HANDSHAKE) && (!validar_socket_conectado(*socket_cliente))){
            log_error(log_conexion, "Debes hacer el handshake");
            break;
        }

        switch (operacion) {
            case HANDSHAKE:
                server_handshake(*socket_cliente, MEMORIA, paquete_recibido);
                agregar_socket_conectado(*socket_cliente);
                break;
            case CREAR_PROCESO:
                t_proceso *proceso = recibir_proceso(paquete_recibido); 

                //Inicializa listas y cosas necesarias
                inicializar_proceso(proceso);

                char* fullpath = malloc(strlen(config->path_instrucciones) + 1);

                strcpy(fullpath, config->path_instrucciones);

                // if(!string_ends_with(fullpath, "/")) {
                //     string_append(&fullpath, "/");
                // }

                string_append(&fullpath, proceso->nombre_archivo);
                
                log_trace(log_conexion, "Busca archivo en %s", fullpath);

                FILE *archivo = fopen(fullpath, "r");
                free(fullpath); 

                obtener_instrucciones(archivo, proceso->instrucciones);

                if (lista_procesos == NULL){
                    lista_procesos = list_create();
                }

                list_add(lista_procesos, proceso);

                responder_rta_ok_error(*socket_cliente, true);

                fclose(archivo);
                break;
            case SOLICITUD_INSTRUCCION:
                int pid_solicitado = buffer_read_uint32(paquete_recibido->buffer);
                int pc_solicitado = buffer_read_uint32(paquete_recibido->buffer);

                log_trace(log_conexion, "enviar la instruccion: %d", pc_solicitado);

                t_paquete* paquete_a_enviar = crear_paquete();
                agregar_operacion(paquete_a_enviar, INSTRUCCION_ENVIO);
                t_proceso *proceso_recibido = buscar_proceso(pid_solicitado);
                t_instruccion *instruccion = buscar_instruccion(proceso_recibido->instrucciones, pc_solicitado);


                if(instruccion == NULL) {
                    instruccion = malloc(sizeof(instruccion));
                    instruccion->instruccion = malloc(sizeof("") + 1); 
                    instruccion->ip = 0;
                    instruccion->instruccion = "";
                }
                
                buffer_add_t_instruccion(paquete_a_enviar->buffer, instruccion);

                enviar_paquete(paquete_a_enviar, *socket_cliente);

                eliminar_paquete(paquete_a_enviar);
                break;
            case FIN_PROCESO:
                int pid_finalizar = buffer_read_uint32(paquete_recibido->buffer);
                t_proceso* proceso_a_finalizar = buscar_proceso(pid_finalizar);
                
                if(proceso_a_finalizar == NULL) {
                    log_error(log_conexion, "Proceso no encontrado");
                    break;
                }

                finalizar_proceso(proceso_a_finalizar);
                log_info(log_conexion, "Proceso Finalizado: PID: <%i>", pid_finalizar);
                break;
            case TAMANIO_PAGINA_SOLICITUD:
                log_trace(log_conexion, "Se recibe peticion TAM_PAG. TAM_PAG = %i", config->tam_pagina);

                t_paquete* paquete_tam_pag = crear_paquete();
                agregar_operacion(paquete_tam_pag, TAMANIO_PAGINA_RESPUESTA);
                buffer_add_uint32(paquete_tam_pag->buffer, config->tam_pagina);
                enviar_paquete(paquete_tam_pag, *socket_cliente);
                eliminar_paquete(paquete_tam_pag);

                break;
            case RESIZE_SOLICITUD:
                int pid_proceso_a_ajustar = buffer_read_uint32(paquete_recibido->buffer);
                int nuevo_tamanio = buffer_read_uint32(paquete_recibido->buffer);

                log_trace(log_conexion, "LLega un ajustar proceso PID <%i> : %i", pid_proceso_a_ajustar, nuevo_tamanio);

                t_proceso *proceso_a_ajustar = buscar_proceso(pid_proceso_a_ajustar);

                if(proceso_a_ajustar == NULL) {
                    log_error(log_conexion, "Proceso no encontrado");
                    responder_rta_ok_error(*socket_cliente, false);
                    break;
                }
                
                bool resize_ok = resize_proceso(proceso_a_ajustar, nuevo_tamanio);

                responder_rta_ok_error(*socket_cliente, resize_ok);

                break;
            case ACCESO_TABLA_PAGINAS:
                int pid_acceso_tdp = buffer_read_uint32(paquete_recibido->buffer);
                int pagina_consultada_acceso_tdp = buffer_read_uint32(paquete_recibido->buffer);

                int marco_pagina_consultada = obtener_marco_pagina(pid_acceso_tdp, pagina_consultada_acceso_tdp);
                
                log_info(log_conexion, "PID: <%i> - Página <%i> - Marco <%i>", pid_acceso_tdp, pagina_consultada_acceso_tdp, marco_pagina_consultada);

                responder_marco_pagina(*socket_cliente, marco_pagina_consultada);

                break;
            case SOLICITUD_LECTURA_MEMORIA:
                int pid_solicitud_lectura = buffer_read_uint32(paquete_recibido->buffer);
                int df_a_leer = buffer_read_uint32(paquete_recibido->buffer);
                int size_a_leer = buffer_read_uint32(paquete_recibido->buffer);
                
                log_info(log_conexion, "PID: <%i> - Accion: LEER - Direccion Fisica <%i> - Tamaño <%d>", pid_solicitud_lectura, df_a_leer, size_a_leer);

                void* dato_leido = leer_de_memoria(df_a_leer, size_a_leer);

                log_trace(log_conexion, "DATO LEIDO %.*s", size_a_leer, (char*) dato_leido);

                enviar_dato_leido(*socket_cliente, dato_leido);

                free(dato_leido);

                break;
            case SOLICITUD_ESCRITURA_MEMORIA:
                int pid_solicitud_escritura = buffer_read_uint32(paquete_recibido->buffer);
                int size = buffer_read_uint32(paquete_recibido->buffer);
                void* dato = buffer_read_string(paquete_recibido->buffer);
                int df_a_escribir = buffer_read_uint32(paquete_recibido->buffer);
                
                log_info(log_conexion, "PID: <%i> - Accion: ESCRIBIR - Direccion Fisica <%i> - Tamaño <%d>", pid_solicitud_escritura, df_a_escribir, size); 
                
                bool escritura_ok = escribir_en_memoria(df_a_escribir, dato, size);

                char* memoria_string = mem_hexstring(memoria_fisica->memoria, TAMANIO_PAGINA * CANTIDAD_MARCOS);
                mostrar_memoria(memoria_string);
                free(memoria_string);
                free(dato);

                responder_rta_ok_error(*socket_cliente, escritura_ok);

                break;
            case DESCONEXION:
                log_trace(log_conexion, "El cliente se desconecto");
                list_remove_element(lista_sockets_conectados, socket_cliente);
                connected = false;
                break;
            default:
                log_trace(log_conexion, "Codigo de operacion invalido: %d", operacion);
                connected = false;
                break;
        }
        eliminar_paquete(paquete_recibido);
        usleep(config->retardo_respuesta * 1000);
    }
    liberar_conexion(*socket_cliente);
    free(arg);
    pthread_exit(NULL);
}

void inicializar_proceso(t_proceso* proceso) {
    proceso->direccion_memoria = &proceso;
    proceso->instrucciones = list_create();
    proceso->tabla_paginas = inicializar_tabla_paginas(0); 
    log_info(log_conexion, "Proceso creado <PID: %i>", proceso->pid);
    log_info(log_conexion, "PID: <%i> - Tamaño: %i", proceso->pid, 0);
}

bool resize_proceso(t_proceso* proceso, int size) {
    int cant_paginas_finales = (int) ceil((double)size / TAMANIO_PAGINA); 
    log_trace(log_conexion, "Cant paginas finales: %i", cant_paginas_finales);

    int cant_paginas_actuales = list_size(proceso->tabla_paginas);
    log_trace(log_conexion, "Cant paginas actuales: %i", cant_paginas_actuales);

    if(cant_paginas_finales > cant_paginas_actuales) {
        int tamanio_diferencia = cant_paginas_finales * TAMANIO_PAGINA - cant_paginas_actuales * TAMANIO_PAGINA;
        log_info(log_conexion, "PID: <%i> - Tamaño Actual: <%i> - Tamaño a Ampliar: <%i>", proceso->pid, cant_paginas_actuales * TAMANIO_PAGINA, tamanio_diferencia);
        return aumentar_tam_proceso(proceso, cant_paginas_actuales, cant_paginas_finales);
    } else {
        int tamanio_diferencia = cant_paginas_actuales * TAMANIO_PAGINA - cant_paginas_finales * TAMANIO_PAGINA;
        log_info(log_conexion, "PID: <%i> - Tamaño Actual: <%i> - Tamaño a Reducir: <%i>", proceso->pid, cant_paginas_actuales * TAMANIO_PAGINA, tamanio_diferencia);
        return reducir_tam_proceso(proceso, cant_paginas_actuales, cant_paginas_finales);
    }
}

t_proceso* recibir_proceso(t_paquete *paquete) {
    t_proceso *proceso = malloc(sizeof(t_proceso));
    
    proceso->pid = buffer_read_uint32(paquete->buffer);
    proceso->nombre_archivo = buffer_read_string(paquete->buffer);

    return proceso;
}

void obtener_instrucciones(FILE *archivo, t_list *lista_instrucciones) {
    char *linea = NULL;
    size_t longitud = 0;

    int i = 0;

    // Leer línea por línea hasta el final del archivo
    while (getline(&linea, &longitud, archivo) != -1) {
        t_instruccion *instruccion = malloc(sizeof(t_instruccion));
        size_t tamanio = strlen(linea);
        if (linea[tamanio - 1] == '\n'){
            linea[tamanio - 1] = '\0';
        }
        instruccion->instruccion = malloc(tamanio + 1);
        instruccion->ip = i;
        strcpy(instruccion->instruccion, linea);
        
        list_add(lista_instrucciones, instruccion);

        i++;
    }
    
    free(linea);
}

t_proceso *buscar_proceso(int pid) {
    bool comparar_proceso(void *proceso) {
        t_proceso *proc = (t_proceso *) proceso;
        return proc->pid == pid;
    }

    if(lista_procesos == NULL) {
        log_error(log_conexion, "No hay procesos");
        return NULL;
    }

    t_proceso *proceso_encontrado = (t_proceso*)list_find(lista_procesos, comparar_proceso);

    return proceso_encontrado;
}

t_instruccion *buscar_instruccion(t_list *lista_instrucciones, int ip) {
    bool comparar_instruccion(void *instruccion) {
        t_instruccion *instr = (t_instruccion *) instruccion;
        return instr->ip == ip;
    }

    if(lista_instrucciones == NULL) {
        log_error(log_conexion, "No hay instrucciones");
        return NULL;
    }

    t_instruccion *resultado = (t_instruccion*)list_find(lista_instrucciones, comparar_instruccion);
    return resultado;
}


void agregar_socket_conectado(int socket_cliente) {
    if(lista_sockets_conectados == NULL) {
        lista_sockets_conectados = list_create();
    }

    int* socket_nuevo = malloc(sizeof(int));

    *socket_nuevo = socket_cliente;

    list_add(lista_sockets_conectados, socket_nuevo);
}

bool validar_socket_conectado(int socket) {
    
    if(lista_sockets_conectados == NULL) {return false;}

    for(int i = 0; i < list_size(lista_sockets_conectados); i++) {
        int* valor_actual = (int*) list_get(lista_sockets_conectados, i);

        if(*valor_actual == socket) {
            return true;
        }
    }

    return false;
}

void finalizar_proceso(t_proceso *proceso) {
    if(!list_remove_element(lista_procesos, proceso)) {
        log_error(log_conexion, "Proceso %d no encontrado", proceso->pid);
        return;
    }

    log_info(log_conexion, "PID: <%i> - Tamaño: %i", proceso->pid, list_size(proceso->tabla_paginas));

    destruir_proceso(proceso);
}

void destruir_proceso(void* elem) {
    t_proceso *proceso = (t_proceso*) elem;

    free(proceso->nombre_archivo);
    list_destroy_and_destroy_elements(proceso->instrucciones, destruir_instruccion);
    list_destroy_and_destroy_elements(proceso->tabla_paginas, destruir_tabla_de_paginas);
    
    free(proceso);
} 

void destruir_instruccion(void* elem) {
    t_instruccion* instruccion = (t_instruccion*) elem;
    free(instruccion->instruccion);
    free(instruccion);
}

void destruir_tabla_de_paginas(void* elem) {
    t_pagina* pagina = (t_pagina*) elem;
    free(pagina);
}

void enviar_dato_leido(int socket, void* dato) {
    t_paquete* paquete = crear_paquete();
    agregar_operacion(paquete, RESPUESTA_LECTURA_MEMORIA);
    buffer_add_string(paquete->buffer, dato);

    enviar_paquete(paquete, socket);
    eliminar_paquete(paquete);
}

int obtener_marco_pagina(int pid, int nro_pag) {
    t_proceso *proceso = buscar_proceso(pid);
    if(proceso == NULL) {
        log_error(log_conexion, "Proceso no encontrado");
        return -1;
    }

    if(nro_pag >= list_size(proceso->tabla_paginas)) {
        log_error(log_conexion, "Pagina no encontrada en proceso");
        return -1;    
    }

    t_pagina *pagina = list_get(proceso->tabla_paginas, nro_pag);

    if(pagina == NULL) {
        log_error(log_conexion, "Pagina no encontrada");
        return -1;
    }

    return pagina->numero_marco;
}

void responder_marco_pagina(int socket, int marco) {
    t_paquete* paquete = crear_paquete();

    agregar_operacion(paquete, RESPUESTA_ACCESO_TABLA_PAGINAS);
    buffer_add_uint32(paquete->buffer, marco);
    enviar_paquete(paquete, socket);
    eliminar_paquete(paquete);
}

void responder_rta_ok_error(int socket, bool ok) {
    t_paquete* paquete = crear_paquete();

    codigo_respuesta rta_code;
    
    rta_code = ok ? RTA_OK : RTA_ERROR;

    agregar_operacion(paquete, RESPUESTA_OK_ERROR);
    buffer_add_uint32(paquete->buffer, rta_code);
    enviar_paquete(paquete, socket);
    eliminar_paquete(paquete);
}

bool validar_permiso_proceso(int pid, int df) {
    t_proceso *proceso = buscar_proceso(pid);
    if(proceso == NULL) { return false; }

    int marco = df / TAMANIO_PAGINA;

    int i = obtener_indice_marco_en_tdp(proceso->tabla_paginas, marco);

    return (i != -1);
}

void inicializar_logger_espacio_memoria() {
    log_memoria = log_create("espacio_memoria.log", "MEMORIA", false, LOG_LEVEL_TRACE);
}

void destruir_logger_espacio_memoria() {
    log_destroy(log_memoria);
}

void mostrar_memoria(char* mem) {
    log_trace(log_memoria, "\n%s", mem);
}
