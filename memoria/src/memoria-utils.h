#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/memory.h>
#include <readline/readline.h>
#include <utils/conexion.h>
#include <config.h>
#include <paginacion.h>

#include <utils/serialize.h>
#include <utils/deserialize.h>

void* atender_cliente(void* arg);

void inicializar_proceso(t_proceso* proceso);

t_proceso* recibir_proceso(t_paquete *paquete);

void obtener_instrucciones(FILE *archivo, t_list *lista_instrucciones);

void agregar_instruccion_a_paquete(t_paquete *paquete, t_instruccion *instruccion);

t_proceso *buscar_proceso(int pid);

t_instruccion *buscar_instruccion(t_list *lista_instrucciones, int pc);

void finalizar_proceso(t_proceso *proceso);

void agregar_socket_conectado(int socket_cliente);

bool validar_socket_conectado(int socket);

bool supera_grado_multiprogramacion();

bool resize_proceso(t_proceso* proceso, int size);

void enviar_dato_leido(int socket, void* dato);

int obtener_marco_pagina(int pid, int nro_pag);

void responder_marco_pagina(int socket, int marco);

void responder_rta_ok_error(int socket, bool ok);

bool validar_permiso_proceso(int pid, int df);

void inicializar_logger_espacio_memoria();

void destruir_logger_espacio_memoria();

void mostrar_memoria(char* mem);

void destruir_proceso(void* elem);

void destruir_instruccion(void* elem);

void destruir_tabla_de_paginas(void* elem);

extern t_list *lista_procesos;

extern t_list *lista_sockets_conectados;