#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <estructuras.h>
#include<string.h>

typedef struct {
    char* puerto_escucha;
    int tam_memoria;
    int tam_pagina;
    char* path_instrucciones;
    int retardo_respuesta;
} t_config_memoria;

void cargar_config(t_config_memoria *config, char* nombre);

void liberar_config(t_config_memoria *config);

extern t_config_memoria *config;