#ifndef UTILS_GENERAL_H_
#define UTILS_GENERAL_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<utils/estructuras.h>
#include<cpu_globals.h>

/**
* @fn obtener_puntero_registro
* @brief Recibe un string con nombre de registro de CPU y devuelve su puntero y tipo.
*/
void* obtener_puntero_registro(t_registro_cpu* registro, char *nombre_campo, data_type* tipo_dato);

/**
* @fn obtenerValorRegistroYTipo
* @brief Recibe nombre de un registro de CPU y devuelve su valor y tipo.
*/
int obtenerValorRegistroYTipo(char * registro, data_type * tipoDato);

/**
* @fn obtenerValorRegistro
* @brief Recibe nombre de un registro de CPU y devuelve su valor.
*/
int obtenerValorRegistro(char * registro);

/**
* @fn tamanio_tipo_dato
* @brief Recibe un string tipo de dato y devuelve su tamaño.
*/
int tamanio_tipo_dato(data_type tipoDato);


#endif