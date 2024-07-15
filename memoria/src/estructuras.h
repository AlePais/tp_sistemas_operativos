#ifndef H_ESTRUCTURAS_MEMORIA
#define H_ESTRUCTURAS_MEMORIA

#include <commons/bitarray.h>
#include <utils/estructuras.h>

typedef struct memoria_fisica {
    void *memoria; 
    t_bitarray *marcos_libres; 
} t_memoria_fisica;

typedef struct pagina {
    int numero_marco; // Número de marco en la memoria física
    bool presente; // Indica si la página está presente en memoria física
} t_pagina;

#endif /* H_ESTRUCTURAS_MEMORIA */