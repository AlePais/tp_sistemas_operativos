#ifndef MMU_H_
#define MMU_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <utils/conexion.h>
#include <utils/estructuras.h>
#include <cpu_globals.h>


/**
* @fn convertirDL
* @brief Recibe una direccio logica y devuelve la pagina y desplazamiento que le corresponden.
*/
char* concatenar_cadenas_sin_null(char* cadena1, size_t length1, char* cadena2, size_t length2);

/**
* @fn convertirDL
* @brief Recibe una direccio logica y devuelve la pagina y desplazamiento que le corresponden.
*/
void convertirDL(int direccionLogica, uint32_t* pagina, uint32_t* desplazamiento);

/**
* @fn consultar_TLB
* @brief Consultar TLB para obtener el marco correspondiente a la página

*/
uint32_t consultar_TLB(uint32_t page_number);

/**
* @fn agregar_TLB
* @brief agrega un registro en TLB que asocie el marco con la pagina correspondiente
*/
void agregar_TLB(uint32_t marco, uint32_t pagina);

/**
* @fn solicitar_marco
* @brief Solicita a modulo de memoria el marco que le corresponde a una pagina
*/
uint32_t solicitar_marco(uint32_t pagina);

/**
* @fn listaDireccionesFisicas
* @brief Devuelve una lista de direcciones fisicas partiendo de una direccion logica y un tamaño para leer/escribir
*/
t_list * listaDireccionesFisicas(int direccionLogica, int tamanio_restante);

/**
* @fn solicitarLecturaMemoria
* @brief Solicita a modulo de memoria la lectura de la cantidad de bytes tamanio de una direccion fisica
*/
char* solicitarLecturaMemoria(uint32_t direccionFisica, uint32_t tamanio);

/**
* @fn leerMemoria
* @brief Aplica solicitarLecturaMemoria a una lista de direcciones fisicas y concatena respuesta en contenido
*/
void leerMemoria(t_list* df_list, char ** contenido);

/**
* @fn escribirMemoria
* @brief Escribe en memoria un valorEscribir en una lista de direcciones fisicas recibida
*/
void escribirMemoria(t_list* df_list, void * valorEscribir);

#endif /* MMU_H_ */