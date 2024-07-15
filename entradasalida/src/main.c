#include <main.h>
#include <commons/error.h>

char *nombre;

t_config_interfaz *config;

int main(int argc, char* argv[]) {

    nombre = argv[1];
    char* tipo_interfaz = argv[2];
    char* archivo_config = argv[3];
    
    log_conexion = log_create("entradasalida.log", "CONEXION", true, LOG_LEVEL_INFO);

    inicializar_log_filesystem();

    if(nombre == NULL || string_is_empty(nombre)) {
        log_error(log_conexion, "Ingrese nombre de la interfaz");
        return EXIT_FAILURE;
    }

    if(tipo_interfaz == NULL || string_is_empty(tipo_interfaz)) {
        log_error(log_conexion, "Ingrese tipo de interfaz");
        return EXIT_FAILURE;
    }

    
    config = cargar_config(tipo_interfaz, archivo_config);

    //log_info(log_conexion, "Se levanto el config con el TUT %i", config->tiempo_unidad_trabajo);

    crear_bitmap();
    generar_archivo_bloques();

    // Conectar a KERNEL
    int socket_kernel = crear_conexion(config->ip_kernel, config->puerto_kernel);
    client_handshake(socket_kernel, IO, KERNEL);

    //Conectar a MEMORIA
    int socket_memoria = crear_conexion(config->ip_memoria, config->puerto_memoria);
    client_handshake(socket_memoria, IO, MEMORIA);

    // Esperar peticiones del KERNEL
    bool connected = true;
    while(connected) {
        t_paquete* paquete_recibido = recibir_paquete(socket_kernel);
        op_code operacion = obtener_operacion(paquete_recibido);
        log_trace(log_conexion, "operacion %i", operacion);

        switch (operacion) {
            case SOLICITUD_NOMBRE_INTERFAZ:
                log_trace(log_conexion, "kernel solicita nombre");
                enviar_nombre_y_tipo_interfaz(nombre, tipo_interfaz ,socket_kernel);                
                break;
            case IO_GEN_SLEEP:
                log_trace(log_conexion, "Kernel envia IO_GEN_SLEEP");
                if(!verificar_nombre(paquete_recibido, nombre)){break;};

                int cantidad_a_dormir = buffer_read_uint32(paquete_recibido->buffer);
                int pid_sleep = buffer_read_uint32(paquete_recibido->buffer);
                int tiempo_unidad_trabajo = config->tiempo_unidad_trabajo;
                log_info(log_conexion, "PID: %i - Operacion: IO_GEN_SLEEP", pid_sleep);               

                dormir(cantidad_a_dormir, tiempo_unidad_trabajo, socket_kernel);
                
                break;
            case IO_STDIN_READ:
                log_trace(log_conexion, "Kernel envia IO_STDIN_READ");
                if(!verificar_nombre(paquete_recibido, nombre)){break;};

                int tamanio_a_escribir = 0;
                t_list *lista_direcciones_escribir = list_create();

                tamanio_a_escribir = determinar_tamanio_a_escribir(paquete_recibido, lista_direcciones_escribir);

                int pid_read = buffer_read_uint32(paquete_recibido->buffer);
                log_trace(log_conexion, "PID: %i - Operacion: IO_STDIN_READ", pid_read);


                char* string_de_consola = leer_de_consola(tamanio_a_escribir);
                
                enviar_para_escribir(lista_direcciones_escribir, string_de_consola, pid_read, socket_memoria);
               
                log_info(log_conexion, "PID: %i - Operacion: IO_STDIN_READ", pid_read);

                free(string_de_consola);
                list_destroy_and_destroy_elements(lista_direcciones_escribir, destroy_direccion_fisica_io);

                finalizo_io(socket_kernel);                               
                break;
            case IO_STDOUT_WRITE:
                log_trace(log_conexion, "Kernel envia IO_STDOUT_WRITE");
                if(!verificar_nombre(paquete_recibido, nombre)){break;};

                int tamanio_a_leer = 0;
                t_list *lista_direcciones_leer = list_create();

                tamanio_a_leer = determinar_tamanio_a_leer(paquete_recibido,lista_direcciones_leer);
                int pid_write = buffer_read_uint32(paquete_recibido->buffer);
                
                char* dato_solicitado = leer_de_memoria(lista_direcciones_leer, pid_write, socket_memoria, tamanio_a_leer);
                
                log_info(log_conexion, "PID: %i - Operacion: IO_STDOUT_WRITE", pid_write);
                printf("Dato leido: %.*s  \n", tamanio_a_leer, dato_solicitado);
               //log_info(log_conexion, "DATO LEIDO %.*s", tamanio_a_leer, dato_solicitado);

                list_destroy(lista_direcciones_leer);
                free(dato_solicitado);
                finalizo_io(socket_kernel);
                break;
            case IO_FS_CREATE:
                log_trace(log_conexion, "Kernel envia IO_FS_CREATE");
                if(!verificar_nombre(paquete_recibido, nombre)){break;};
                usleep(config->tiempo_unidad_trabajo*1000);

                char* nombre_archivo_crear = buffer_read_string(paquete_recibido->buffer);
                int pid_crear = buffer_read_uint32(paquete_recibido -> buffer);
                log_info(log_conexion, "PID: %i - Crear Archivo: <%s>", pid_crear, nombre_archivo_crear);

                t_FCB* fcb = crear_fcb(nombre_archivo_crear);    
                bitmap_marcar_bloque_ocupado(fcb->bloque_inicial);
                crear_archivo(fcb);

                free(nombre_archivo_crear);
                free(fcb);
                finalizo_io(socket_kernel);
                break;
            case IO_FS_DELETE:
                log_trace(log_conexion, "Kernel envia IO_FS_DELETE");
                if(!verificar_nombre(paquete_recibido, nombre)){break;}
                usleep(config->tiempo_unidad_trabajo*1000);

                char* nombre_archivo_borrar = buffer_read_string(paquete_recibido->buffer);
                int pid_borrar = buffer_read_uint32(paquete_recibido->buffer);
                log_info(log_conexion, "PID: %i - Borrar Archivo: <%s>", pid_borrar, nombre_archivo_borrar);

                marcar_bloques_libres(nombre_archivo_borrar);
                eliminar_archivo_metadata(nombre_archivo_borrar);

                free(nombre_archivo_borrar);

                finalizo_io(socket_kernel);
                break;
            case IO_FS_WRITE:
                log_trace(log_conexion, "Kernel envia IO_FS_WRITE");
                if(!verificar_nombre(paquete_recibido, nombre)){ break; }
                usleep(config->tiempo_unidad_trabajo*1000);

                t_list *lista_direcciones_fs_write = list_create();    
                char* nombre_archivo_write = buffer_read_string(paquete_recibido->buffer);
                int tamanio_a_leer_fs = determinar_tamanio_a_leer(paquete_recibido, lista_direcciones_fs_write);
                int offset_archivo_write = buffer_read_uint32(paquete_recibido->buffer); 
                int pid_fs_write = buffer_read_uint32(paquete_recibido->buffer);
                log_info(log_conexion, "PID: <%i> - Escribir Archivo: <%s> - Tamaño a escribir: <%i> - Puntero Archivo: <%i>", pid_fs_write, nombre_archivo_write, tamanio_a_leer_fs, offset_archivo_write);               
               
             
                char* dato_leido_fs_write = leer_de_memoria(lista_direcciones_fs_write, pid_fs_write, socket_memoria, tamanio_a_leer_fs);
                
                t_FCB *fcb_fs_write = leer_metadata(nombre_archivo_write);

                escribir_en_archivo(fcb_fs_write->bloque_inicial, offset_archivo_write, dato_leido_fs_write, tamanio_a_leer_fs);
                
                free(nombre_archivo_write);
                free(fcb_fs_write);
                list_destroy(lista_direcciones_fs_write);
                finalizo_io(socket_kernel);
                break;
            case IO_FS_READ:
                log_trace(log_conexion, "Kernel envia IO_FS_READ");
                if(!verificar_nombre(paquete_recibido, nombre)){break;}
                usleep(config->tiempo_unidad_trabajo*1000);

                t_list *lista_direcciones_fs_read = list_create();    
                char* nombre_archivo_read = buffer_read_string(paquete_recibido->buffer);
                int tamanio_a_escribir_fs = determinar_tamanio_a_leer(paquete_recibido, lista_direcciones_fs_read);
                int offset_archivo_read = buffer_read_uint32(paquete_recibido->buffer); 
                int pid_fs_read = buffer_read_uint32(paquete_recibido->buffer);
                log_info(log_conexion, "PID: <%i> - Escribir Archivo: <%s> - Tamaño a leer: <%i> - Puntero Archivo: <%i>", pid_fs_read, nombre_archivo_read, tamanio_a_escribir_fs, offset_archivo_read);               
                
                t_FCB *fcb_fs_read = leer_metadata(nombre_archivo_read);
            
                char* dato_leido_fs = leer_archivo(tamanio_a_escribir_fs, fcb_fs_read, offset_archivo_read);

                enviar_para_escribir(lista_direcciones_fs_read, dato_leido_fs, pid_fs_read, socket_memoria);
                
                free(dato_leido_fs);
                free(nombre_archivo_read);   
                free(fcb);
                list_destroy(lista_direcciones_fs_read);    
                finalizo_io(socket_kernel);   
                break;
            case IO_FS_TRUNCATE:
                log_trace(log_conexion, "Kernel envia IO_FS_TRUNCATE");
                if(!verificar_nombre(paquete_recibido, nombre)){break;}
                usleep(config->tiempo_unidad_trabajo*1000);

                char* nombre_fs_truncate = buffer_read_string(paquete_recibido->buffer);
                int tamanio_truncate = buffer_read_uint32(paquete_recibido->buffer);
                int pid_truncate = buffer_read_uint32(paquete_recibido->buffer);
                log_info(log_conexion, "PID: %i - Truncar Archivo: <%s>", pid_truncate, nombre_fs_truncate);
                
                t_FCB* fcb_truncate =  leer_metadata(nombre_fs_truncate);
                if(fcb_truncate->size < tamanio_truncate) {
                    log_info(log_conexion, "Se agranda el archivo %s a %i", fcb_truncate->nombre_archivo, tamanio_truncate);
                    agrandar_archivo(fcb_truncate, tamanio_truncate, pid_truncate);
                } else {
                    log_trace(log_conexion, "Se achica el archivo %s a %i", fcb_truncate->nombre_archivo, tamanio_truncate);
                    achicar_archivo(fcb_truncate, tamanio_truncate);
                }

                free(fcb_truncate);

                finalizo_io(socket_kernel);
                break;
            default:
                log_error(log_conexion, "Codigo de operacion invalido: %d", operacion);
                connected = false;
                break;
        }


        eliminar_paquete(paquete_recibido);
    }
    log_destroy(log_conexion);
    log_destroy(log_filesystem);
    liberar_config(config);

    liberar_conexion(socket_kernel);
    liberar_conexion(socket_memoria);

    return EXIT_SUCCESS;
}


void enviar_nombre_y_tipo_interfaz(char *nombre,char* tipo, int socket_cliente) {
    t_paquete *paquete_a_enviar = crear_paquete();
    agregar_operacion(paquete_a_enviar, ENVIO_NOMBRE_INTERFAZ);
    
    buffer_add_string(paquete_a_enviar->buffer, nombre);
    buffer_add_string(paquete_a_enviar->buffer, tipo);

    enviar_paquete(paquete_a_enviar, socket_cliente);

    eliminar_paquete(paquete_a_enviar);
}

void finalizo_io(int socket_kernel) {
    t_paquete *paquete_a_enviar = crear_paquete();
    agregar_operacion(paquete_a_enviar, IO_FIN);
    
    enviar_paquete(paquete_a_enviar, socket_kernel);
    eliminar_paquete(paquete_a_enviar);

    log_info(log_conexion, "Finalizo IO");
} 

void enviar_solicitud_escritura(int pid, char* string, int tamanio, int registro_direccion, int socket){

    t_paquete* paquete_write = crear_paquete();
    agregar_operacion(paquete_write, SOLICITUD_ESCRITURA_MEMORIA);
    buffer_add_uint32(paquete_write->buffer, pid);
    buffer_add_uint32(paquete_write->buffer, tamanio);
    buffer_add_string(paquete_write->buffer, string);    
    buffer_add_uint32(paquete_write->buffer, registro_direccion);
    enviar_paquete(paquete_write, socket);
    eliminar_paquete(paquete_write);
    free(string);
    
}

char* leer_de_consola(int tamanio){

    char* string_a_leer;
    
    string_a_leer = readline("> ");

    if(string_length(string_a_leer) > tamanio){
        log_trace(log_conexion, "La cadena ocupa %i y deberia ocupar %i, se recortara", string_length(string_a_leer), tamanio);
    }
    char* string_recortado = string_substring_until(string_a_leer, tamanio);
    log_trace(log_conexion, "La cadena ingresada es: %s ", string_recortado);
    free(string_a_leer);
    return string_recortado;
}

bool verificar_nombre(t_paquete* paquete, char* nombre){
    char* nombre_requerido = buffer_read_string(paquete->buffer);
    if(strcmp(nombre, nombre_requerido) != 0){ 
        log_trace(log_conexion, "el nombre no es el correcto");
        return false;
    };
    free(nombre_requerido);
    return true;
}

void enviar_solicitud_lectura(int pid, int direccion_fisica, int size, int socket){
    t_paquete* paquete_read = crear_paquete();
    agregar_operacion(paquete_read, SOLICITUD_LECTURA_MEMORIA);
    buffer_add_uint32(paquete_read->buffer, pid);
    buffer_add_uint32(paquete_read->buffer, direccion_fisica);
    buffer_add_uint32(paquete_read->buffer, size);
    enviar_paquete(paquete_read,socket);
    eliminar_paquete(paquete_read);
}

char* mostrar_dato_solicitado(int size, int socket_memoria){
    t_paquete* paquete_recibido_memoria = recibir_paquete(socket_memoria);

    if(obtener_operacion(paquete_recibido_memoria) !=  RESPUESTA_LECTURA_MEMORIA){
        log_error(log_conexion, "Se recibio una operacion de tipo %i", obtener_operacion(paquete_recibido_memoria));
    }

    char* dato_recibido = buffer_read_string(paquete_recibido_memoria->buffer);
    eliminar_paquete(paquete_recibido_memoria);
    return dato_recibido;
}

void dormir(int cantidad_a_dormir, int tiempo_unidad_trabajo, int socket_kernel){
    int tiempo_a_dormir = cantidad_a_dormir * tiempo_unidad_trabajo;
    log_trace(log_conexion, "voy a dormir %d", tiempo_a_dormir);

    if(tiempo_a_dormir > 0){
        usleep(tiempo_a_dormir*1000);
        //Devolver header a kernel
        finalizo_io(socket_kernel);
    }else{
        log_trace(log_conexion, "Error: el tiempo a dormir es invalido");
    }
}

char* concatenar_cadenas_sin_null(char* cadena1, size_t length1, char* cadena2, size_t length2) {
    // Calcular el tamaño total necesario
    size_t total_length = length1 + length2;

    // Reservar memoria para la cadena resultante
    char* resultado = malloc(total_length);
    if (resultado == NULL) {
        perror("Error al asignar memoria");
        exit(1);
    }
    // Copiar la primera cadena
    memcpy(resultado, cadena1, length1);
    // Copiar la segunda cadena justo después de la primera
    memcpy(resultado + length1, cadena2, length2);

    return resultado;
}

int determinar_tamanio_a_escribir(t_paquete* paquete_recibido, t_list* lista_direcciones_escribir){
    int cantidad_direcciones = buffer_read_uint32(paquete_recibido->buffer);
    int tamanio_a_escribir;

    for(int i = 0; i < cantidad_direcciones; i++) {
        t_direccion_fisica_io *df = buffer_read_t_direccion_fisica_io(paquete_recibido->buffer);
        log_trace(log_conexion, "direccion recibida: %i", df->df);
        list_add(lista_direcciones_escribir, df);
        tamanio_a_escribir += df->size;
    }
    return tamanio_a_escribir;
}

void enviar_para_escribir(t_list* lista_direcciones_escribir ,char* string ,int pid_read ,int socket_memoria){
    
    int tamanio_a_sacar = 0;
    
    for(int j = 0; j < list_size(lista_direcciones_escribir); j++ ){
        t_direccion_fisica_io *t_df = list_get(lista_direcciones_escribir, j);
        //char* string_1 = string_substring_from(string, tamanio_a_sacar
        char* string_a_mandar = string_substring(string, tamanio_a_sacar, t_df->size);

        log_trace(log_conexion, "direccion a mandar: %i", t_df->df);
        log_trace(log_conexion, "string que se manda: %s", string_a_mandar);

        enviar_solicitud_escritura(pid_read, string_a_mandar, t_df->size, t_df->df, socket_memoria);
        t_paquete* paquete_recibido_memoria = recibir_paquete(socket_memoria);
        if(obtener_operacion(paquete_recibido_memoria) !=  RESPUESTA_OK_ERROR){
            log_error(log_conexion, "SE recibio una operacionde tipo %i", obtener_operacion(paquete_recibido_memoria));
        }
        tamanio_a_sacar += t_df->size;
        eliminar_paquete(paquete_recibido_memoria);

    }
}

int determinar_tamanio_a_leer(t_paquete* paquete_recibido,t_list* lista_direcciones_leer){
    int cantidad_direcciones_leer = buffer_read_uint32(paquete_recibido->buffer);
    int tamanio_a_leer = 0;
                
    for(int x = 0; x < cantidad_direcciones_leer; x++) {
        t_direccion_fisica_io *df = buffer_read_t_direccion_fisica_io(paquete_recibido->buffer);
        list_add(lista_direcciones_leer, df);
        tamanio_a_leer += df->size;
    }  

    return tamanio_a_leer;
}

char* leer_de_memoria(t_list* lista_direcciones_leer, int pid_write, int socket_memoria, int tamanio_a_leer){
    char *dato_solicitado = malloc(sizeof(tamanio_a_leer));
    int size_dato_solicitado = 0;
    
    for (int k = 0; k < list_size(lista_direcciones_leer); k++) {
        t_direccion_fisica_io *t_df_leer = list_get(lista_direcciones_leer,k);
        enviar_solicitud_lectura(pid_write, t_df_leer->df, t_df_leer->size, socket_memoria);
        char* dato_recibido = mostrar_dato_solicitado(t_df_leer->size, socket_memoria);
        dato_solicitado = concatenar_cadenas_sin_null(dato_solicitado, size_dato_solicitado, dato_recibido, t_df_leer->size);        
        size_dato_solicitado += t_df_leer->size;
        free(dato_recibido);
        free(t_df_leer);
    }

    return dato_solicitado;
}

t_FCB* crear_fcb(char* nombre_archivo){
    t_FCB* fcb = malloc(sizeof(t_FCB));

    if(fcb == NULL) { return NULL; } 

    fcb->nombre_archivo = nombre_archivo;
    fcb->size = 0;
    fcb->bloque_inicial = bitmap_encontrar_bloque_libre();
    log_trace(log_conexion, "EL bloque inicial designado es %i", fcb->bloque_inicial);

    return fcb;
}

void destroy_direccion_fisica_io(void* element) {
    free(element);
}
