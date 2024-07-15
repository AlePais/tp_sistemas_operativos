#include <mmu.h>

void convertirDL(int direccionLogica, uint32_t* pagina, uint32_t* desplazamiento){
    *pagina = (int)((intptr_t) direccionLogica / tamanio_pagina);
    *desplazamiento = (int) ((intptr_t) direccionLogica % tamanio_pagina);
}

uint32_t consultar_TLB(uint32_t pagina){
    uint32_t marco = -1;

    bool marcoEncontrado(void* arg){
        t_registro_tlb * registroComparado = (t_registro_tlb*) arg;
        return registroComparado->pid == contexto.PID && registroComparado->pagina == pagina;
    }

    t_registro_tlb * registro = list_find(tlb.l_registro_tlb, marcoEncontrado);
    if (registro != NULL)
    {
        if (strcmp(config->algoritmo_tlb, "LRU") == 0)
        {
            list_remove_element(tlb.l_registro_tlb, registro);
            list_add(tlb.l_registro_tlb, registro);
        }

        marco = registro->marco;
        log_info(log_conexion, "PID: %d - TLB HIT - Pagina: %d", contexto.PID, pagina);
    }

    return marco;
}

void agregar_TLB(uint32_t marco, uint32_t pagina)
{
    t_registro_tlb * registro;
    vecesUsado = 0;

    registro = malloc(sizeof(t_registro_tlb));

    registro->pid = contexto.PID;
    registro->pagina = pagina;
    registro->marco = marco;
    registro->used_count = 1;

    if (list_size(tlb.l_registro_tlb) == config->cantidad_entradas_tlb)
    {
        free(list_remove(tlb.l_registro_tlb, 0));
    }

    list_add(tlb.l_registro_tlb, registro);

    if (tlb.next_fifo < config->cantidad_entradas_tlb - 1){
        tlb.next_fifo++; 
    } else {
        tlb.next_fifo = 0;
    }
}

uint32_t solicitar_marco(uint32_t pagina){ 
    uint32_t marco = -1;

    marco = consultar_TLB(pagina);

    if (marco == -1){
        log_info(log_conexion, "PID: %d - TLB MISS - Pagina: %d", contexto.PID, pagina);

        t_paquete* paquete_a_memoria = crear_paquete();
        agregar_operacion(paquete_a_memoria, ACCESO_TABLA_PAGINAS);
        buffer_add_uint32(paquete_a_memoria->buffer, contexto.PID);
        buffer_add_uint32(paquete_a_memoria->buffer, pagina);
        enviar_paquete(paquete_a_memoria, socket_memoria);
        eliminar_paquete(paquete_a_memoria);

        t_paquete* paquete_recibido_memoria = recibir_paquete(socket_memoria);
        op_code operacion = obtener_operacion(paquete_recibido_memoria);
        if (operacion == RESPUESTA_ACCESO_TABLA_PAGINAS) {
            marco = buffer_read_uint32(paquete_recibido_memoria->buffer);
        }
        eliminar_paquete(paquete_recibido_memoria);

        if (marco !=-1){
            log_info(log_conexion, "PID: %d -- OBTENER MARCO - Página: %d - Marco: %d”.", contexto.PID, pagina, marco);
            if (config->cantidad_entradas_tlb > 0){
                agregar_TLB(marco, pagina);
            }
        }
    }
    return marco;
}

t_list * listaDireccionesFisicas(int direccionLogica, int tamanio_restante){
    
    uint32_t pagina;
    uint32_t desplazamiento;
    uint32_t tamanio = 0;
    uint32_t tamanio_definido = 0;
    uint32_t disponible;
    uint32_t marco;
    uint32_t direccionFisica;

    t_list* df_list = list_create();

    while(tamanio_restante > 0){
        convertirDL(direccionLogica, &pagina, &desplazamiento);

        disponible = tamanio_pagina - desplazamiento;

        if ( disponible < tamanio_restante){
            tamanio = disponible;
        }else{
            tamanio = tamanio_restante;
        }

        marco = solicitar_marco(pagina);
        direccionFisica = marco * tamanio_pagina + desplazamiento;

        t_direccion_fisica_io * df_element;
        df_element = malloc(sizeof(t_direccion_fisica_io));

        df_element->df = direccionFisica;
        df_element->size = tamanio;

        list_add(df_list, df_element);

        direccionLogica += tamanio;
        tamanio_restante -= tamanio;
        tamanio_definido += tamanio;
    }

    return df_list;
}

char* solicitarLecturaMemoria(uint32_t direccionFisica, uint32_t tamanio){
    char * contenido;
    t_paquete* paquete_a_memoria = crear_paquete();
    agregar_operacion(paquete_a_memoria, SOLICITUD_LECTURA_MEMORIA);
    buffer_add_uint32(paquete_a_memoria->buffer, contexto.PID);
    buffer_add_uint32(paquete_a_memoria->buffer, direccionFisica);
    buffer_add_uint32(paquete_a_memoria->buffer, tamanio);
    enviar_paquete(paquete_a_memoria, socket_memoria);
    eliminar_paquete(paquete_a_memoria);

    t_paquete* paquete_recibido_memoria = recibir_paquete(socket_memoria);
    op_code operacion = obtener_operacion(paquete_recibido_memoria);
    if (operacion == RESPUESTA_LECTURA_MEMORIA) {
        contenido = buffer_read_string(paquete_recibido_memoria->buffer);
        log_info(log_conexion, "PID: %d - Acción: LEER - Dirección Física: %d", contexto.PID, direccionFisica);
    }
    eliminar_paquete(paquete_recibido_memoria);

    return contenido;
}

void leerMemoria(t_list* df_list, char ** contenido){

    int tamanioContenido = 0;

    for(int i = 0; i < list_size(df_list); i++) {
        t_direccion_fisica_io * df_element = list_get(df_list, i);
        char * cadena = string_substring_until(solicitarLecturaMemoria(df_element->df, df_element->size),  df_element->size);
        *contenido = concatenar_cadenas_sin_null(*contenido, tamanioContenido, cadena, df_element->size);
        tamanioContenido += df_element->size;
        free(cadena);
    }

    string_append(contenido, "\0");
}


void escribirMemoria(t_list* df_list, void * valorEscribir){
    codigo_respuesta rta_code;

    int inicio = 0;

    for(int i = 0; i < list_size(df_list); i++) {
        
        t_direccion_fisica_io * df_element = list_get(df_list, i);
        void * valor = malloc(df_element->size);

        memcpy(valor, valorEscribir + inicio, df_element->size);

        t_paquete* paquete_a_memoria = crear_paquete();
        agregar_operacion(paquete_a_memoria, SOLICITUD_ESCRITURA_MEMORIA);
        buffer_add_uint32(paquete_a_memoria->buffer, contexto.PID);
        buffer_add_uint32(paquete_a_memoria->buffer, df_element->size);
        buffer_add_void(paquete_a_memoria->buffer, df_element->size, valor);
        buffer_add_uint32(paquete_a_memoria->buffer, df_element->df);
        enviar_paquete(paquete_a_memoria, socket_memoria);
        eliminar_paquete(paquete_a_memoria);

        t_paquete* paquete_recibido_memoria = recibir_paquete(socket_memoria);
        op_code operacion = obtener_operacion(paquete_recibido_memoria);
        if (operacion == RESPUESTA_OK_ERROR)
        {
            log_info(log_conexion, "PID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %s", contexto.PID, df_element->df, (char*)valor);
            rta_code = buffer_read_uint32(paquete_recibido_memoria->buffer);
            if (rta_code == RTA_ERROR)
            {
                interrupt = 0;
                contexto.motivo = OUT_OF_MEMORY;
            }
        }
        eliminar_paquete(paquete_recibido_memoria);
        inicio += df_element->size;
        free(valor);
    }
}

t_paquete * leerDesdeInterfaz(t_list* df_ios){
    t_paquete* paquete_a_kernel_io = crear_paquete();
    agregar_operacion(paquete_a_kernel_io, IO_STDIN_READ);
    buffer_add_t_direcciones_fisicas_io(paquete_a_kernel_io->buffer, df_ios);
    buffer_add_uint32(paquete_a_kernel_io->buffer, contexto.PID);
    free(df_ios);
    return paquete_a_kernel_io;
}

char* concatenar_cadenas_sin_null(char* cadena1, size_t length1, char* cadena2, size_t length2) {
    // Calcular el tamaño total necesario
    size_t total_length = length1 + length2;

    // Reservar memoria para la cadena resultante
    char* resultado = calloc(1,total_length);
    if (resultado == NULL) {
        perror("Error al asignar memoria");
        exit(1);
    }

    // Copiar la primera cadena
    memcpy(resultado, cadena1, length1);

    // Copiar la segunda cadena justo después de la primera
    memcpy(resultado + length1, cadena2, length2);

    return resultado;
}