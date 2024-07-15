#ifndef INSTRUCCION_PROCESSOR_H_
#define INSTRUCCION_PROCESSOR_H_

#include <utils/estructuras.h>
#include <utils/serialize.h>
#include <utils/deserialize.h>
#include <utils/conexion.h>
#include <registrosCpu.h>
#include <math.h>
#include <cpu_config.h>
#include <cpu_globals.h>
#include <mmu.h>

/**
* @fn instruction_process
* @brief Ejecuta proceso solicitado por kernel.
*/
void instruction_process(t_paquete *paquete_recibido);

/**
* @fn instruction_execute
* @brief Función para liberar la memoria asignada a tokens en los que separe instruccion
*/
void free_tokens(char **tokens);

/**
* @fn instruction_fetch
* @brief Solicita instruccion a memoria.
*/
void instruction_fetch(char** instruccion);
/**
* @fn instruction_decode
* @brief Decodificar la instrucción en tokens.
*/
void instruction_decode(char *instruccion, char ***tokens);

/**
* @fn instruction_execute
* @brief Ejecutar la instrucción y obtener el paquete a kernel IO si corresponde
*/
t_paquete* instruction_execute(char **instruccion);

/**
* @fn SET
* @brief Asigna al registro el valor pasado como parámetro.
*/
void SET(char* registro, int valor);

/**
* @fn MOV_IN
* @brief  Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra en el Registro Dirección y lo almacena en el RegistroDatos.
*/
void MOV_IN(char* registroDatos, char* registroDirección);

/**
* @fn MOV_OUT
* @brief  Lee el valor del Registro Datos y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección.
*/
void MOV_OUT(char* registroDirección, char* registroDatos);

/**
* @fn SUM
* @brief Suma al Registro Destino el Registro Origen y deja el resultado en el Registro Destino.
*/
void SUM(char* destino, char* origen);

/**
* @fn SUB
* @brief Resta al Registro Destino el Registro Origen y deja el resultado en el Registro Destino.
*/
void SUB(char* destino, char* origen);

/**
* @fn JNZ
* @brief Si el valor del registro es distinto de cero, actualiza el program counter al número de instrucción pasada por parámetro.
*/
void JNZ(char* registro, int instruccion);

/**
* @fn RESIZE
* @brief Solicitará a la Memoria ajustar el tamaño del proceso al tamaño pasado por parámetro. En caso de que la respuesta de la memoria sea Out of Memory, se deberá devolver el contexto de ejecución al Kernel informando de esta situación.
*/
void RESIZE(int tamanio);

/**
* @fn COPY_STRING
* @brief  Toma del string apuntado por el registro SI y copia la cantidad de bytes indicadas en el parámetro tamaño a la posición de memoria apuntada por el registro DI.
*/
void COPY_STRING(uint32_t tamanio);

/**
* @fn generic_semaforos
* @brief  Funcion generica para construir paquetes a a I/O de SIGNAL y WAIT
*/
t_paquete* generic_semaforos(op_code operacion, char* recurso);

/**
* @fn INS_WAIT
* @brief  Esta instrucción solicita al Kernel que se asigne una instancia del recurso indicado por parámetro.
*/
t_paquete* INS_WAIT(char* recurso);

/**
* @fn INS_SIGNAL
* @brief  Esta instrucción solicita al Kernel que se libere una instancia del recurso indicado por parámetro.
*/
t_paquete* INS_SIGNAL(char* recurso);

/**
* @fn INS_IO_GEN_SLEEP
* @brief Solicita al Kernel que se envíe a una interfaz de I/O a que realice un sleep por una cantidad de unidades de trabajo.
*/
t_paquete* INS_IO_GEN_SLEEP(char* interfaz, uint32_t tiempo);

/**
* @fn generic_stdin_stdout
* @brief Funcion generica para construir paquetes a a I/O de STDIN y STDOUT
*/
t_paquete* generic_stdin_stdout(char* interfaz, char* registroDireccion, char* registroTamanio, op_code operacion);

/**
* @fn INS_IO_STDIN_READ
* @brief Esta instrucción solicita al Kernel que mediante la interfaz ingresada se lea desde el STDIN (Teclado) un valor cuyo tamaño está delimitado por el valor del Registro Tamaño y el mismo se guarde a partir de la Dirección Lógica almacenada en el Registro Dirección.
*/
t_paquete* INS_IO_STDIN_READ(char* interfaz, char* registroDireccion, char* registroTamanio);

/**
* @fn INS_IO_STDOUT_WRITE
* @brief Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se lea desde la posición de memoria indicada por la Dirección Lógica almacenada en el Registro Dirección, un tamaño indicado por el Registro Tamaño y se imprima por pantalla.
*/                          
t_paquete* INS_IO_STDOUT_WRITE(char* interfaz, char* registroDireccion, char* registroTamanio);

/**
* @fn INS_IO_FS_CREATE
* @brief Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se cree un archivo en el FS montado en dicha interfaz.
*/                          
t_paquete* INS_IO_FS_CREATE(char* interfaz, char* nombreArchivo);

/**
* @fn INS_IO_FS_DELETE
* @brief Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se elimine un archivo en el FS montado en dicha interfaz.
*/                          
t_paquete* INS_IO_FS_DELETE(char* interfaz, char* nombreArchivo);

/**
* @fn INS_IO_FS_TRUNCATE
* @brief Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se modifique el tamaño del archivo en el FS montado en dicha interfaz, actualizando al valor que se encuentra en el registro indicado por Registro Tamaño.
*/
t_paquete* INS_IO_FS_TRUNCATE(char* interfaz, char* nombreArchivo, char* registroTamanio);

/**
* @fn INS_IO_FS_WRITE
* @brief Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se lea desde Memoria la cantidad de bytes indicadas por el Registro Tamaño a partir de la dirección lógica que se encuentra en el Registro Dirección y se escriban en el archivo a partir del valor del Registro Puntero Archivo.
*/                          
t_paquete* INS_IO_FS_WRITE(char* interfaz, char* nombreArchivo, char* registroDireccion, char* registroTamanio, char* registroPunteroArchivo);

/**
* @fn INS_IO_FS_READ
* @brief Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se lea desde el archivo a partir del valor del Registro Puntero Archivo la cantidad de bytes indicada por Registro Tamaño y se escriban en la Memoria a partir de la dirección lógica indicada en el Registro Dirección.
*/                          
t_paquete* INS_IO_FS_READ(char* interfaz, char* nombreArchivo, char* registroDireccion, char* registroTamanio, char* registroPunteroArchivo);

/**
* @fn INCREMENTAR_PC
* @brief Si el valor del registro es distinto de cero, actualiza el program counter al número de instrucción pasada por parámetro.
*/
void INCREMENTAR_PC(data_type tipo_dato, void* registro);

/**
* @fn INS_EXIT
* @brief Devuelve el Contexto de Ejecución actualizado al Kernel para su finalización.
*/
void INS_EXIT();

#endif /* INSTRUCCION_PROCESSOR_H_ */

// // Es capaz de resolver las operaciones: MOV_IN, MOV_OUT, IO_STDIN_READ, IO_STDOUT_WRITE.
