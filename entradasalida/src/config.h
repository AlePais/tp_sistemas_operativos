#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>

typedef enum tipo_interfaz{
    GENERICA,
    STDIN,
    STDOUT,
    DIALFS
} tipo_interfaz;

typedef struct config_interfaz {
    tipo_interfaz tipo;
    int tiempo_unidad_trabajo;
    char *ip_kernel;
    char *puerto_kernel;
    char *ip_memoria;
    char *puerto_memoria;
    char *path_base_dialFs;
    int block_size;
    int block_count;
    int retraso_compactacion;
} t_config_interfaz;

t_config_interfaz *cargar_config(char* tipo_interfaz, char* archivo);

void liberar_config(t_config_interfaz *config);

extern t_config_interfaz *config;