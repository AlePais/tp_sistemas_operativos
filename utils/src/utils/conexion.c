#include <utils/conexion.h>

t_log* log_conexion;

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family,
                         		server_info->ai_socktype,
                         		server_info->ai_protocol);

	if(setsockopt(socket_cliente, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed");

	int error = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
    while(error != 0)
    {   
		log_trace(log_conexion, "Reintentando conexion con: %s:%s", ip, puerto);
		sleep(5);
		error = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
    }
	log_trace(log_conexion, "Conexión exitosa con: %s:%s", ip, puerto);
    freeaddrinfo(server_info);
    return socket_cliente;
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	crear_buffer(paquete);
	return paquete;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	int error = send(socket_cliente, a_enviar, bytes, 0);
    if(error == bytes)
    {
        log_trace(log_conexion, "Paquete enviado: %s", string_op_code(paquete->codigo_operacion));
        //log_trace(log_conexion, "Tamaño de buffer: %d", (int)(error - 2 * sizeof(int)));
    }
	free(a_enviar);
}

void agregar_operacion(t_paquete* paquete, op_code operacion)
{
    paquete->codigo_operacion = operacion;
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
    log_trace(log_conexion, "Se cerro la conexion");
}

int iniciar_servidor(char* puerto)
{
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	getaddrinfo(NULL, puerto, &hints, &servinfo);

	socket_servidor = socket(servinfo->ai_family,
                        	 servinfo->ai_socktype,
							 servinfo->ai_protocol);

	if(setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed");

	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	int error = listen(socket_servidor, SOMAXCONN);
    if(error == 0)
    {
        log_trace(log_conexion, "Se inicio servidor en puerto: %s", puerto);
    }
	freeaddrinfo(servinfo);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	int socket_cliente;
	socket_cliente = accept(socket_servidor, NULL, NULL);
    if(socket_cliente != -1)
    {
        log_trace(log_conexion, "Nueva conexion aceptada");
    }
	return socket_cliente;
}

t_paquete* recibir_paquete(int socket_cliente)
{
	t_paquete* paquete = crear_paquete();
    deserializar_paquete(paquete, socket_cliente);
    log_trace(log_conexion, "Paquete recibido: %s", string_op_code(paquete->codigo_operacion));
	return paquete;
}

void deserializar_paquete(t_paquete* paquete, int socket_cliente)
{
    if(recv(socket_cliente, &(paquete->codigo_operacion), sizeof(op_code), MSG_WAITALL) > 0)
    {
        recv(socket_cliente, &(paquete->buffer->size), sizeof(int), MSG_WAITALL);
	    paquete->buffer->stream = malloc(paquete->buffer->size);
		if (paquete->buffer->size != 0){
		    recv(socket_cliente, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);
		}
		paquete->buffer->offset = paquete->buffer->stream;
    }
	else
	{
		paquete->codigo_operacion = DESCONEXION;
        close(socket_cliente);
	}
}

op_code obtener_operacion(t_paquete* paquete)
{
    return paquete->codigo_operacion;
}

void client_handshake(int socket_servidor, module_name client_name, module_name server_name)
{
    t_paquete* paquete;
    op_code operacion;

    //Enviar Handshake
    paquete = crear_paquete();
    agregar_operacion(paquete, HANDSHAKE);
	buffer_add_uint32(paquete->buffer, client_name);
    enviar_paquete(paquete,socket_servidor);
    eliminar_paquete(paquete);

    //Recibir Handshake
    paquete = recibir_paquete(socket_servidor);
    operacion = obtener_operacion(paquete);
    if(operacion == HANDSHAKE)
    {   
        module_name nombre = buffer_read_uint32(paquete->buffer);
        if(nombre == server_name)
        {
            switch (nombre)
            {
            case CPU:
                log_info(log_conexion, "Handshake con CPU exitoso");
                break;
            case MEMORIA:
                log_info(log_conexion, "Handshake con MEMORIA exitoso");
                break;
            case IO:
                log_info(log_conexion, "Handshake con ENTRADA_SALIDA exitoso");
                break;
            case KERNEL:
                log_info(log_conexion, "Handshake con KERNEL exitoso");
                break;
            default:
                break;
            }
        }
    }
    eliminar_paquete(paquete);
}

void server_handshake(int socket_cliente, module_name server_name, t_paquete *paquete_recibido) {
	module_name nombre = buffer_read_uint32(paquete_recibido->buffer);

	t_paquete* paquete_a_enviar = crear_paquete();
    agregar_operacion(paquete_a_enviar, HANDSHAKE);
	buffer_add_uint32(paquete_a_enviar->buffer, server_name);

	//Enviar Handshake
	switch (nombre)
	{
		case CPU:
			log_info(log_conexion, "Handshake con CPU exitoso");
			break;
		case MEMORIA:
			log_info(log_conexion, "Handshake con MEMORIA exitoso");
			break;
		case IO:
			log_info(log_conexion, "Handshake con ENTRADA_SALIDA exitoso");
			break;
		case KERNEL:
			log_info(log_conexion, "Handshake con KERNEL exitoso");
			break;
		default:
			break;
	}

	enviar_paquete(paquete_a_enviar, socket_cliente);

	eliminar_paquete(paquete_a_enviar);
}

char* string_op_code(op_code code){
	switch (code) {
		case DESCONEXION: return "DESCONEXION";
		case HANDSHAKE: return "HANDSHAKE";
		case CREAR_PROCESO: return "CREAR_PROCESO";
		case FIN_PROCESO: return "FIN_PROCESO";
		case RESIZE_SOLICITUD: return "RESIZE_SOLICITUD";
		case RESIZE_ENVIO: return "RESIZE_ENVIO";
		case SOLICITUD_INSTRUCCION: return "SOLICITUD_INSTRUCCION";
		case ENVIO_INSTRUCCION: return "ENVIO_INSTRUCCION";
		case SOLICITUD_NOMBRE_INTERFAZ: return "SOLICITUD_NOMBRE_INTERFAZ";
		case ENVIO_NOMBRE_INTERFAZ: return "ENVIO_NOMBRE_INTERFAZ";
		case IO_GEN_SLEEP: return "IO_GEN_SLEEP";
		case IO_STDIN_READ: return "IO_STDIN_READ";
		case IO_STDOUT_WRITE: return "IO_STDOUT_WRITE";
		case IO_FS_CREATE: return "IO_FS_CREATE";
		case IO_FS_DELETE: return "IO_FS_DELETE";
		case IO_FS_TRUNCATE: return "IO_FS_TRUNCATE";
		case IO_FS_WRITE: return "IO_FS_WRITE";
		case IO_FS_READ: return "IO_FS_READ";
		case IO_FIN: return "IO_FIN";
		case EJECUTAR_PROCESO: return "EJECUTAR_PROCESO";
		case INTERRUPCION: return "INTERRUPCION";
		case DEVOLUCION_PCB: return "DEVOLUCION_PCB";
		case INSTRUCCION_ENVIO: return "INSTRUCCION_ENVIO";
		case ACCESO_TABLA_PAGINAS: return "ACCESO_TABLA_PAGINAS";
		case RESPUESTA_ACCESO_TABLA_PAGINAS: return "RESPUESTA_ACCESO_TABLA_PAGINAS";
		case SOLICITUD_LECTURA_MEMORIA: return "SOLICITUD_LECTURA_MEMORIA";
		case RESPUESTA_LECTURA_MEMORIA: return "RESPUESTA_LECTURA_MEMORIA";
		case SOLICITUD_ESCRITURA_MEMORIA: return "SOLICITUD_ESCRITURA_MEMORIA";
		case RESPUESTA_OK_ERROR: return "RESPUESTA_OK_ERROR";
		case WAIT: return "WAIT";
		case SIGNAL: return "SIGNAL";
		case SOLICITUD_LECTURA: return "SOLICITUD_LECTURA";
		case SOLICITUD_ESCRITURA: return "SOLICITUD_ESCRITURA";
		case TAMANIO_PAGINA_SOLICITUD: return "TAMANIO_PAGINA_SOLICITUD";
		case TAMANIO_PAGINA_RESPUESTA: return "TAMANIO_PAGINA_RESPUESTA";
		default: return "UNKNOWN";
	}
}
