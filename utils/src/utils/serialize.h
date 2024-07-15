#ifndef UTILS_SERIALIZE_H_
#define UTILS_SERIALIZE_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include <utils/estructuras.h>
#include <utils/conexion.h>

/**
* @fn buffer_add_uint8
* @brief Agrega un uint8_t al buffer.
*/
	void buffer_add_uint8(t_buffer *buffer, uint8_t data);

/**
* @fn buffer_add_uint32
* @brief Agrega un uint32_t al buffer.
*/
	void buffer_add_uint32(t_buffer *buffer, uint32_t data);

/**
* @fn buffer_add_string
* @brief Agrega string al buffer (longitud la toma implicitamente del lenght de string).
*/
    void buffer_add_string(t_buffer *buffer, char *string);

/**
* @fn buffer_add_void
* @brief Agrega void al buffer con un uint32_t adelante indicando su longitud.
*/
void buffer_add_void(t_buffer *buffer, uint32_t lenght, void *string);

/**
* @fn buffer_add_t_instruccion
* @brief Agrega un t_instruccion al buffer.
*/
	void buffer_add_t_instruccion(t_buffer *buffer, t_instruccion *instruccion);

/**
* @fn buffer_add_t_contexto
* @brief Agrega un t_contexto al buffer.
*/
	void buffer_add_t_contexto(t_buffer *buffer, t_contexto contexto);

/**
* @fn buffer_add_t_direccion_fisica_io
* @brief Agrega una t_direccion_fisica_io al buffer.
*/
	void buffer_add_t_direccion_fisica_io(t_buffer* buffer, t_direccion_fisica_io t_df_io);

/**
* @fn buffer_add_t_direcciones_fisicas_io
* @brief Agrega multiples t_direccion_fisica_io al buffer a partir de una t_list.
*/
	void buffer_add_t_direcciones_fisicas_io(t_buffer* buffer, t_list *lista_dfs);

#endif