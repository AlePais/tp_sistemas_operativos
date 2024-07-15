#ifndef MAIN_H_
#define MAIN_H_

#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/conexion.h>
#include <commons/config.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <filesystem.h>

void enviar_nombre_y_tipo_interfaz(char *nombre,char* tipo, int socket_cliente);

void finalizo_io(int socket_kernel);

void enviar_solicitud_escritura(int pid, char* string, int tamanio, int registro_direccion, int socket);

char* leer_de_consola(int tamanio);

bool verificar_nombre(t_paquete* paqute, char* nombre);

void enviar_solicitud_lectura(int pid, int direccion_fisica, int size, int socket);

char* mostrar_dato_solicitado(int size, int socket_memoria);

void dormir(int cantidad_a_dormir, int tiempo_unidad_trabajo, int socker_kernel);

char* concatenar_cadenas_sin_null( char* cadena1, size_t length1, char* cadena2, size_t length2);

int determinar_tamanio_a_escribir(t_paquete* paquete_recibido, t_list*lista_direcciones_escribir);

void enviar_para_escribir(t_list* lista_direcciones_escribir ,char* string ,int pid_read ,int socket_memoria);

char* leer_de_memoria(t_list* lista_direcciones_leer, int pid_write, int socket_memoria, int tamanio_a_leer);

int determinar_tamanio_a_leer(t_paquete* paquete_recibido,t_list* lista_direcciones_leer);

void destroy_direccion_fisica_io(void* element);

#endif /* SERVER_H_ */