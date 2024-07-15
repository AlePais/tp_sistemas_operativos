#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <estructuras.h>
#include <utils/estructuras.h>
#include <utils/conexion.h>
#include <commons/log.h>
#include <semaphore.h>

extern int TAMANIO_PAGINA; 
extern int CANTIDAD_MARCOS;
extern int TAMANIO_BITARRAY;

void inicializar_logger_paginacion();

void destruir_logger_paginacion();

void inicializar_memoria_fisica();

void destruir_memoria_fisica();

t_tabla_paginas inicializar_tabla_paginas(int cantidad_paginas);

bool asignar_pagina_a_marco(t_pagina *pagina);

bool hay_n_marcos_disponibles(int cantidad_marcos);

bool aumentar_tam_proceso(t_proceso *proceso, int cantidad_actual, int cantidad_final);

bool reducir_tam_proceso(t_proceso *proceso, int cantidad_actual, int cantidad_final);

int obtener_indice_marco_en_tdp(t_tabla_paginas tdp, int marco);

bool escritura_proceso(t_proceso *proceso, int direccion_fisica, void *data, int size, int tiempo_a_dormir);

bool escribir_en_memoria(int direccion_fisica, void *data, int size);

void* lectura_proceso(t_proceso *proceso, int direccion_fisica, int size, int tiempo_a_dormir);

void* leer_de_memoria(int direccion_fisica, int size);

extern t_memoria_fisica *memoria_fisica;

extern sem_t mx_memoria;
