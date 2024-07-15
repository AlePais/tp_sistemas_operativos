#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/conexion.h>
#include <commons/string.h>
#include <estructuras.h>
#include <math.h>
#include <commons/config.h>
#include <config.h>
#include <dirent.h>

void inicializar_log_filesystem();

void generar_archivo_bloques();

void crear_bitmap();

void bitmap_marcar_bloque_libre(int numeroBloque);

void bitmap_marcar_bloques_ocupados(int bloque_inicial, int bloque_final);

void bitmap_marcar_bloques_libres(int bloque_inicial, int bloque_final);

void bitmap_marcar_bloque_ocupado(int numeroBloque);

int bitmap_encontrar_bloque_libre();

t_FCB* crear_fcb(char* nombre_archivo);

void crear_archivo(t_FCB *fcb);

void eliminar_archivo_metadata(char* nombre_archivo);

t_FCB* leer_metadata(char* nombre_archivo);

int calcular_bloques_necesarios(int size);

void marcar_bloques_libres(char* nombre_archivo);

char* get_fullpath(char* nombre_archivo);

char* leer_archivo(int tamanio, t_FCB *fcb, int offset);

void escribir_en_archivo(int bloque_inicial, int offset, void *dato, int size);

void agrandar_archivo(t_FCB *fcb, int tamanio_nuevo, int pid);

void achicar_archivo(t_FCB *fcb, int tamanio_nuevo);

bool hay_espacio_contiguo(t_FCB* fcb,int bloques_actuales, int bloques_necesarios);

t_list* leer_directorio();

bool comparator_fcbs_size(void *e1, void* e2);

t_FCB* buscar_fcb(t_FCB* fcb_buscado, t_list* lista_fcb);

int compactar(t_FCB* fcb); 

char* buscar_contenido_de(t_FCB* fcb);

char* leer_bloques(int bloque_inicial, int tamanio);

int copiar_contenido_a(char* contenido, int tamanio);

int max(int a, int b);

void destroy_fcb(void* element);

extern t_log* log_filesystem;

extern t_bitmap *bitmap;
