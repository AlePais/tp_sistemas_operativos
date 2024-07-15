#include <config.h>

void cargar_config(t_config_memoria* config_memoria, char* nombre_config) {
    // t_config *config = config_create("memoria.config");
    t_config *config = config_create(nombre_config);
    config_memoria->puerto_escucha = strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
    config_memoria->tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
    config_memoria->tam_pagina = config_get_int_value(config, "TAM_PAGINA");
    config_memoria->path_instrucciones = strdup(config_get_string_value(config, "PATH_INSTRUCCIONES"));
    config_memoria->retardo_respuesta = config_get_int_value(config, "RETARDO_RESPUESTA");

    config_destroy(config);
}

void liberar_config(t_config_memoria *config) {
    free(config->puerto_escucha);
    free(config->path_instrucciones);
    free(config);
}