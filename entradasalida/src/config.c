#include<string.h>
#include <config.h>
#include <commons/string.h>

t_config_interfaz *cargar_config(char* tipo_interfaz, char* archivo) {
    t_config_interfaz *config = malloc(sizeof(t_config_interfaz));

    t_config *archivo_config;
    
    if (config == NULL) {
        perror("Error al asignar memoria para t_config_interfaz");
        exit(EXIT_FAILURE);
    }

    if (strcmp(tipo_interfaz, "GENERICA") == 0) {
        config->tipo = GENERICA;
        //archivo_config = config_create("generica.config");
    } else if (strcmp(tipo_interfaz, "STDIN") == 0) {
        config->tipo = STDIN;
        //archivo_config = config_create("stdin.config");
    } else if (strcmp(tipo_interfaz, "STDOUT") == 0) {
        config->tipo = STDOUT;
        //archivo_config = config_create("stdout.config");
    } else if (strcmp(tipo_interfaz, "DIALFS") == 0) {
        config->tipo = DIALFS;
        //archivo_config = config_create("dialfs.config");
    } else {
        fprintf(stderr, "Tipo de interfaz desconocido: %s\n", tipo_interfaz);
        free(config);
        exit(EXIT_FAILURE);
    }

    archivo_config = config_create(archivo);

    if (archivo_config == NULL) {
        fprintf(stderr, "No se pudo crear el archivo de configuración para %s\n", tipo_interfaz);
        free(config);
        exit(EXIT_FAILURE);
    }

    config->tiempo_unidad_trabajo = config_get_int_value(archivo_config, "TIEMPO_UNIDAD_TRABAJO");
    config->ip_kernel = strdup(config_get_string_value(archivo_config, "IP_KERNEL"));
    config->puerto_kernel = strdup(config_get_string_value(archivo_config, "PUERTO_KERNEL"));
    config->ip_memoria = strdup(config_get_string_value(archivo_config, "IP_MEMORIA"));
    config->puerto_memoria = strdup(config_get_string_value(archivo_config, "PUERTO_MEMORIA"));
    config->path_base_dialFs = strdup(config_get_string_value(archivo_config, "PATH_BASE_DIALFS"));
    config->block_size = config_get_int_value(archivo_config, "BLOCK_SIZE");
    config->block_count = config_get_int_value(archivo_config, "BLOCK_COUNT");
    config->retraso_compactacion = config_get_int_value(archivo_config, "RETRASO_COMPACTACION");
    
    config_destroy(archivo_config);

    return config;
}

void liberar_config(t_config_interfaz *config) {
    free(config->ip_kernel);
    free(config->puerto_kernel);
    free(config->ip_memoria);
    free(config->puerto_memoria);
    free(config->path_base_dialFs);
    free(config);
}