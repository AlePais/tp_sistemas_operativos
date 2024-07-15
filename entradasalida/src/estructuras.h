#include <stdlib.h>
#include <stdio.h>
typedef struct t_bitmap
{
    char *direccion;
    uint32_t tamanio;
    t_bitarray *bitarray;
} t_bitmap;

typedef struct fcb
{
    char* nombre_archivo;
    uint32_t size;
    uint32_t bloque_inicial;
} t_FCB;