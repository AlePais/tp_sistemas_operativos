#ifndef UTILS_DESERIALIZE_H_
#define UTILS_DESERIALIZE_H_

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
#include<utils/conexion.h>

/**
* @fn buffer_read_uint8_t
* @brief Lee un uint8_t del buffer y avanza el offset.
*/
	uint8_t buffer_read_uint8_t(t_buffer *buffer);

/**
* @fn buffer_read_uint32
* @brief Lee un uint32_t del buffer y avanza el offset.
*/
	uint32_t buffer_read_uint32(t_buffer *buffer);

/**
* @fn buffer_read_string
* @brief Lee un string, con su longitud, del buffer y avanza el offset.
*/
	char *buffer_read_string(t_buffer *buffer);

/**
* @fn buffer_read_t_registro_cpu
* @brief Lee un t_registro_cpu al buffer y avanza el offset.
*/
	t_registro_cpu buffer_read_t_registro_cpu (t_buffer *buffer);

/**
* @fn buffer_read_t_proceso
* @brief Lee un t_proceso del buffer y avanza el offset.
*/
	t_proceso* buffer_read_t_proceso(t_buffer *buffer);

/**
* @fn buffer_read_t_contexto
* @brief Lee un t_contexto del buffer y avanza el offset.
*/
	t_contexto buffer_read_t_contexto(t_buffer *buffer);

/**
* @fn buffer_read_t_direccion_fisica_io
* @brief Lee una t_direccion_fisica_io del buffer y avanza el offset.
*/
	t_direccion_fisica_io* buffer_read_t_direccion_fisica_io(t_buffer *buffer);

/**
* @fn buffer_read_t_direcciones_fisicas_io
* @brief Devuelve una lista con multiples t_direccion_fisica_io y avanza el offset.
*/
	t_list* buffer_read_t_direcciones_fisicas_io(t_buffer *buffer);
#endif