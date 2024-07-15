#ifndef H_ESTRUCTURAS
#define H_ESTRUCTURAS

#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <stdint.h>

typedef t_list* t_tabla_paginas; //tabla de t_pagina
typedef struct proceso {
    int pid;
    char *nombre_archivo;
    t_list *instrucciones;
    void *direccion_memoria;
    t_tabla_paginas tabla_paginas;
} t_proceso;

typedef struct instruccion {
    int ip;
    char* instruccion;
}t_instruccion;

typedef struct 
{
    uint32_t PC;
    uint8_t  AX;
    uint8_t  BX;
    uint8_t  CX;
    uint8_t  DX;
    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
    uint32_t SI;
    uint32_t DI;
} t_registro_cpu;

typedef enum {
    NEW,
    READY,
    BLOCKED,
    EXEC,
    EXIT,
    DELETED
} estado_proceso;

typedef enum {
    NONE,
    INTERRUPT, 
    CPU_IO,
    END,
    ERROR,
    OUT_OF_MEMORY,
    SUCCESS,
    INVALID_RESOURCE,
    INVALID_INTERFACE,
    INTERRUPTED_BY_USER
}exit_reasons;

typedef struct{
    int PID;
    exit_reasons motivo;
    t_registro_cpu registros;
} t_contexto;

typedef enum
{
    DESCONEXION,
	HANDSHAKE,
	CREAR_PROCESO,
	FIN_PROCESO,
	RESIZE_SOLICITUD,
    RESIZE_ENVIO,
	SOLICITUD_INSTRUCCION,
	ENVIO_INSTRUCCION,
	SOLICITUD_NOMBRE_INTERFAZ,
	ENVIO_NOMBRE_INTERFAZ,
	IO_GEN_SLEEP,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_TRUNCATE,
    IO_FS_WRITE,
    IO_FS_READ,
	IO_FIN,
    EJECUTAR_PROCESO,
	INTERRUPCION,
	DEVOLUCION_PCB,
	INSTRUCCION_ENVIO,
    ACCESO_TABLA_PAGINAS,
    RESPUESTA_ACCESO_TABLA_PAGINAS,
    SOLICITUD_LECTURA_MEMORIA,
    RESPUESTA_LECTURA_MEMORIA,
    SOLICITUD_ESCRITURA_MEMORIA,
    RESPUESTA_OK_ERROR,
    WAIT,
    SIGNAL,
    SOLICITUD_LECTURA,
    SOLICITUD_ESCRITURA,
    TAMANIO_PAGINA_SOLICITUD,
    TAMANIO_PAGINA_RESPUESTA
}op_code;

typedef enum {
	CPU,
	KERNEL,
	MEMORIA,
	IO
}module_name;

typedef enum {
	INT8,
	INT32,
}data_type;

typedef struct
{
	int size;
	void* stream;
	void* offset;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef enum {
    RTA_OK,
    RTA_ERROR
}codigo_respuesta;

typedef struct {
    uint32_t pid;
    uint32_t pagina;
    uint32_t marco;
    uint32_t used_count; // Campo adicional para LRU
} t_registro_tlb;

typedef struct {
    t_list* l_registro_tlb;
    int next_fifo; // Para el algoritmo FIFO
} t_tlb;

typedef struct direccion_fisica_io {
    int size;
    int df;
} t_direccion_fisica_io;

#endif