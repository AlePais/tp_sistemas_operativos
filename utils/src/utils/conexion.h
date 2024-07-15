#ifndef UTILS_CONEXION_H_
#define UTILS_CONEXION_H_

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
#include <utils/deserialize.h>
#include <utils/serialize.h>


extern t_log* log_conexion;

/**
* @fn    serializar_paquete
* @brief .
*/
void* serializar_paquete(t_paquete* paquete, int bytes);

/**
* @fn    crear_conexion
* @brief .
*/
int crear_conexion(char *ip, char* puerto);

/**
* @fn    crear_buffer
* @brief .
*/
void crear_buffer(t_paquete* paquete);

/**
* @fn    crear_paquete
* @brief .
*/
t_paquete* crear_paquete(void);

/**
* @fn    enviar_paquete
* @brief .
*/
void enviar_paquete(t_paquete* paquete, int socket_cliente);

/**
* @fn    agregar_operacion
* @brief .
*/
void agregar_operacion(t_paquete* paquete, op_code operacion);

/**
* @fn    eliminar_paquete
* @brief .
*/
void eliminar_paquete(t_paquete* paquete);

/**
* @fn    liberar_conexion
* @brief .
*/
void liberar_conexion(int socket_cliente);

/**
* @fn    iniciar_servidor
* @brief .
*/
int iniciar_servidor(char* puerto);

/**
* @fn    esperar_cliente
* @brief .
*/
int esperar_cliente(int socket_servidor);

/**
* @fn    recibir_paquete
* @brief .
*/
t_paquete* recibir_paquete(int socket_cliente);

/**
* @fn    deserializar_paquete
* @brief .
*/
void deserializar_paquete(t_paquete* paquete, int socket_cliente);

/**
* @fn    obtener_operacion
* @brief .
*/
op_code obtener_operacion(t_paquete* paquete);

/**
* @fn    client_handshake
* @brief .
*/
void client_handshake(int socket_servidor, module_name client_name, module_name server_name);

void server_handshake(int socket_cliente, module_name server_name, t_paquete *paquete_recibido);

char* string_op_code(op_code code);

#endif
