#include <paginacion.h>

t_log* log_paginacion;

void inicializar_logger_paginacion() {
    log_paginacion = log_create("paginacion.log", "PAGINACION", false, LOG_LEVEL_TRACE);
}

void destruir_logger_paginacion() {
    log_destroy(log_paginacion);
}

void inicializar_memoria_fisica() {
    memoria_fisica = malloc(sizeof(t_memoria_fisica));
    memoria_fisica->memoria = calloc(1, TAMANIO_PAGINA * CANTIDAD_MARCOS);
    void* bitmap_memoria_usuario = malloc(TAMANIO_BITARRAY);
    memset(bitmap_memoria_usuario, 0, TAMANIO_BITARRAY);
    memoria_fisica->marcos_libres = bitarray_create_with_mode(bitmap_memoria_usuario, TAMANIO_BITARRAY, LSB_FIRST);
}

void destruir_memoria_fisica() {
    bitarray_destroy(memoria_fisica->marcos_libres);
    free(memoria_fisica->memoria);
    free(memoria_fisica);
}

t_tabla_paginas inicializar_tabla_paginas(int cantidad_paginas) {
    t_tabla_paginas tabla_paginas = list_create();
    for(int i = 0; i < cantidad_paginas; i++) {
        t_pagina *pagina = malloc(sizeof(t_pagina));
        pagina->numero_marco = -1;
        pagina->presente = false;
        list_add(tabla_paginas, pagina);
    }
    return tabla_paginas;
}

bool hay_n_marcos_disponibles(int cantidad) {
    int libres = 0;
    
    for(int i = 0; i < CANTIDAD_MARCOS; i++) {
        if(!bitarray_test_bit(memoria_fisica->marcos_libres, i)) { //encontro bit en 0 => ASIGNA ESE MARCO
            libres++;

            if(libres >= cantidad) {
                return true;
            }
        }
    }

    return false;
}

bool asignar_pagina_a_marco(t_pagina *pagina) {
    for(int i = 0; i < CANTIDAD_MARCOS; i++) {
        log_debug(log_paginacion, "i: %i", i);
        sem_wait(&mx_memoria);
        if(!bitarray_test_bit(memoria_fisica->marcos_libres, i)) { //encontro bit en 0 => ASIGNA ESE MARCO
            log_debug(log_paginacion, "i encontrado: %i", i);
            bitarray_set_bit(memoria_fisica->marcos_libres, i); //setea el bit en 1
            sem_post(&mx_memoria);
            pagina->numero_marco = i;
            pagina->presente = true;
            return true;
        }
        sem_post(&mx_memoria);
    }

    return false; //no encontro marcos libres
}

bool aumentar_tam_proceso(t_proceso *proceso, int cantidad_actual, int cantidad_final) {
    log_trace(log_paginacion, "aumenta pags proc");
    
    if(!hay_n_marcos_disponibles(cantidad_final - cantidad_actual)) {
        log_error(log_paginacion, "OUT OF MEMORY");
        return false;
    }

    for(int pag = cantidad_actual; pag < cantidad_final; pag++) {
        t_pagina *nueva_pagina = malloc(sizeof(t_pagina));
        nueva_pagina->numero_marco = -1;
        nueva_pagina->presente = false;
        
        if(asignar_pagina_a_marco(nueva_pagina)) {
            log_debug(log_paginacion, "Marco asignado correctamente %i", nueva_pagina->numero_marco);
            list_add(proceso->tabla_paginas, nueva_pagina);
            log_debug(log_paginacion, "pagina agregada (paginas totales %i)", list_size(proceso->tabla_paginas));
        } else {
            log_debug(log_paginacion, "No se pudo asignar el marco");
            free(nueva_pagina);
            return false;
        }
    }

    return true;
}

bool reducir_tam_proceso(t_proceso *proceso, int cantidad_actual, int cantidad_final) {

    if(cantidad_actual == 0) {
        log_error(log_paginacion, "El proceso no tiene páginas");
        return false;
    }

    log_trace(log_paginacion, "reducir pags proc");
    for(int pag = cantidad_actual - 1; pag >= cantidad_final; pag--) {
        t_pagina *pagina_a_borrar = list_get(proceso->tabla_paginas, pag);
        
        //Dejar libre el marco en el bitarray
        bitarray_clean_bit(memoria_fisica->marcos_libres, pagina_a_borrar->numero_marco);
        
        free(pagina_a_borrar);        
        list_remove(proceso->tabla_paginas, pag);
        log_trace(log_paginacion, "pagina removida (paginas totales %i)", list_size(proceso->tabla_paginas));
    }

    return true;
}

int obtener_indice_marco_en_tdp(t_tabla_paginas tdp, int marco) {
    for(int p = 0; p < list_size(tdp); p++) {
        t_pagina *pagina = list_get(tdp, p);
        if(pagina->numero_marco == marco) {
            return p;
        }
    }

    return -1;
}

bool escribir_en_memoria(int direccion_fisica, void *data, int size) {
    int marco = direccion_fisica / TAMANIO_PAGINA;
    //int offset = direccion_fisica % TAMANIO_PAGINA;

    // Copiar los datos en la memoria física
    sem_wait(&mx_memoria);
    memcpy(memoria_fisica->memoria + direccion_fisica, data, size);
    sem_post(&mx_memoria);

    // Marcar el marco como ocupado
    bitarray_set_bit(memoria_fisica->marcos_libres, marco);

    return true;
}

void* leer_de_memoria(int direccion_fisica, int size) {
    if (direccion_fisica < 0 || direccion_fisica >= CANTIDAD_MARCOS * TAMANIO_PAGINA) {
        log_error(log_paginacion, "Error: Dirección física fuera de los límites de la memoria.");
        return NULL;
    }

    void* buffer_dest = malloc(size);
    
    // Copiar los datos en el buffer
    memcpy(buffer_dest, memoria_fisica->memoria + direccion_fisica, size);

    return buffer_dest;
}