#include <instruction_processor.h>

void* registroDestino;
data_type tipo_datoOrigen;
data_type tipo_datoDestino;
void* registroOrigen;
void* registroDestino;
void* punteroRegistroDatos;
void* punteroRegistroDireccion;
void* punteroRegistroDireccionSI;
void* punteroRegistroDireccionDI;

void instruction_process(t_paquete *paquete_recibido)
{
    char*  instruccion;
    char** tokens;

    t_paquete* paquete_a_kernel_io = NULL;

    contexto = buffer_read_t_contexto(paquete_recibido->buffer);

    while(interrupt)    
    {
        instruccion = NULL;
        tokens = NULL;

        instruction_fetch(&instruccion);
        if(strcmp(instruccion, "") == 0){
            free(instruccion);
            break;
        }

        instruction_decode(instruccion, &tokens);
        paquete_a_kernel_io = instruction_execute(tokens);

        free(instruccion);
        free_tokens(tokens);
    }

    t_paquete* paquete_a_kernel = crear_paquete();
    agregar_operacion(paquete_a_kernel, DEVOLUCION_PCB);
    buffer_add_t_contexto(paquete_a_kernel->buffer, contexto);
	enviar_paquete(paquete_a_kernel, socket_kernel);
	eliminar_paquete(paquete_a_kernel);

    if (paquete_a_kernel_io != NULL){
        buffer_add_uint32(paquete_a_kernel_io->buffer, contexto.PID);
        enviar_paquete(paquete_a_kernel_io, socket_kernel);
        eliminar_paquete(paquete_a_kernel_io);
    }
}

void instruction_fetch(char** instruccion)
{
    log_info(log_conexion, "PID: %d - FETCH - Program Counter: %d", contexto.PID, contexto.registros.PC);

    t_paquete* paquete_a_memoria = crear_paquete();
    agregar_operacion(paquete_a_memoria, SOLICITUD_INSTRUCCION);
    buffer_add_uint32(paquete_a_memoria->buffer, contexto.PID);
    buffer_add_uint32(paquete_a_memoria->buffer, contexto.registros.PC);
	enviar_paquete(paquete_a_memoria, socket_memoria);
	eliminar_paquete(paquete_a_memoria);

    t_paquete* paquete_recibido_memoria = recibir_paquete(socket_memoria);
    op_code operacion = obtener_operacion(paquete_recibido_memoria);
    if (operacion == INSTRUCCION_ENVIO) {
        *instruccion = buffer_read_string(paquete_recibido_memoria->buffer);
    }
	eliminar_paquete(paquete_recibido_memoria);
}

void instruction_decode(char *instruccion, char ***tokens) {
    *tokens = string_n_split(instruccion, 6, " ");
}

void free_tokens(char **tokens) {
    for (int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

void df_destroy(t_direccion_fisica_io * df_element)
{
    free(df_element);
}

t_paquete* instruction_execute(char **instruccion)
{
    t_paquete* paquete_a_kernel_io = NULL;

    char* tipo_instruccion = instruccion[0];
    char* parametros = string_new();
    char* parametro1 = NULL;
    char* parametro2 = NULL;
    char* parametro3 = NULL;
    char* parametro4 = NULL;
    char* parametro5 = NULL;
    tipo_datoOrigen  = INT32;
    tipo_datoDestino = INT32;
    registroOrigen   = NULL;
    registroDestino  = NULL;

    punteroRegistroDatos     = NULL;
    punteroRegistroDireccion = NULL;

    contexto.motivo = INTERRUPT;

    int continuar = 1;

    if(instruccion[1] != NULL && continuar){
        parametro1  = instruccion[1];
        string_append(&parametros, parametro1);
        string_append(&parametros, " ");
    } else {continuar = 0;};
    if(instruccion[2] != NULL && continuar){
        parametro2 = instruccion[2];
        string_append(&parametros, parametro2);
        string_append(&parametros, " ");
    } else {continuar = 0;}
    if(instruccion[3] != NULL && continuar){
        parametro3 = instruccion[3];
        string_append(&parametros, parametro3);
        string_append(&parametros, " ");
    } else {continuar = 0;}
    if(instruccion[4] != NULL && continuar){
        parametro4 = instruccion[4];
        string_append(&parametros, parametro4);
        string_append(&parametros, " ");
    } else {continuar = 0;}
    if(instruccion[5] != NULL && continuar){
        parametro5 = instruccion[5];
        string_append(&parametros, parametro5);
        string_append(&parametros, " ");
    } else {continuar = 0;}

    if (strcmp(tipo_instruccion, "SET") == 0){
        int valor = atoi(parametro2);
        SET(parametro1, valor);
    } else if (strcmp(tipo_instruccion, "MOV_IN") == 0){
        MOV_IN(parametro1, parametro2);
    } else if (strcmp(tipo_instruccion, "MOV_OUT") == 0){
        MOV_OUT(parametro1, parametro2);
    } else if (strcmp(tipo_instruccion, "SUM") == 0){
        SUM(parametro1, parametro2);
    } else if (strcmp(tipo_instruccion, "SUB") == 0){
        SUB(parametro1, parametro2);
    } else if (strcmp(tipo_instruccion, "JNZ") == 0){
        int valor = atoi(parametro2);
        JNZ(parametro1, valor);
    } else if (strcmp(tipo_instruccion, "RESIZE") == 0){
        int valor = atoi(parametro1);
        RESIZE(valor);
    } else if (strcmp(tipo_instruccion, "COPY_STRING") == 0){
        int valor = atoi(parametro1);
        COPY_STRING(valor);
    } else if (strcmp(tipo_instruccion, "WAIT") == 0){
        paquete_a_kernel_io = INS_WAIT(parametro1);
        contexto.motivo = CPU_IO;
    } else if (strcmp(tipo_instruccion, "SIGNAL") == 0){
        paquete_a_kernel_io = INS_SIGNAL(parametro1);
        contexto.motivo = CPU_IO;
    } else if (strcmp(tipo_instruccion, "IO_GEN_SLEEP") == 0) {
        int valor = atoi(parametro2);
        paquete_a_kernel_io = INS_IO_GEN_SLEEP(parametro1, valor);
        contexto.motivo = CPU_IO;
    } else if (strcmp(tipo_instruccion, "IO_STDIN_READ") == 0) {
        paquete_a_kernel_io = INS_IO_STDIN_READ(parametro1, parametro2, parametro3);
        contexto.motivo = CPU_IO;
    } else if (strcmp(tipo_instruccion, "IO_STDOUT_WRITE") == 0) {
        paquete_a_kernel_io = INS_IO_STDOUT_WRITE(parametro1, parametro2, parametro3);
        contexto.motivo = CPU_IO;
    } else if (strcmp(tipo_instruccion, "IO_FS_CREATE") == 0) {
        paquete_a_kernel_io = INS_IO_FS_CREATE(parametro1, parametro2);
        contexto.motivo = CPU_IO;
    } else if (strcmp(tipo_instruccion, "IO_FS_DELETE") == 0) {
        paquete_a_kernel_io = INS_IO_FS_DELETE(parametro1, parametro2);
        contexto.motivo = CPU_IO;
    } else if (strcmp(tipo_instruccion, "IO_FS_TRUNCATE") == 0) {
        paquete_a_kernel_io = INS_IO_FS_TRUNCATE(parametro1, parametro2, parametro3);
        contexto.motivo = CPU_IO;
    } else if (strcmp(tipo_instruccion, "IO_FS_WRITE") == 0) {
        paquete_a_kernel_io = INS_IO_FS_WRITE(parametro1, parametro2, parametro3, parametro4, parametro5);
        contexto.motivo = CPU_IO;
    } else if (strcmp(tipo_instruccion, "IO_FS_READ") == 0) {
        paquete_a_kernel_io = INS_IO_FS_READ(parametro1, parametro2, parametro3, parametro4, parametro5);
        contexto.motivo = CPU_IO;
    } else if (strcmp(tipo_instruccion, "EXIT") == 0) {
        INS_EXIT();
        contexto.motivo = END;
    }

    registroDestino = obtener_puntero_registro(&(contexto.registros), "PC", &tipo_datoDestino);
    if (registroDestino != NULL) {
        INCREMENTAR_PC(tipo_datoDestino, registroDestino);
    }

    log_info(log_conexion, "PID: %d - Ejecutando: %s - %s ", contexto.PID, tipo_instruccion, parametros);
    free(parametros);
    return paquete_a_kernel_io;
}

void SET(char* registro, int valor)
{
    registroDestino = obtener_puntero_registro(&(contexto.registros), registro, &tipo_datoDestino);
    if (registroDestino != NULL) 
    {
        switch (tipo_datoDestino) {
            case INT8:
                uint8_t* aux_8 = (uint8_t *)registroDestino;
                *aux_8  = (uint8_t)valor;
                break;
            case INT32:
                uint32_t* aux_32 = (uint32_t *)registroDestino;
                *aux_32  = (uint32_t)valor;
                break;
        }
    }
}

void MOV_IN(char* registroDatos, char* registroDireccion)
{
    int direccionLogica = obtenerValorRegistro(registroDireccion);
    punteroRegistroDatos = obtener_puntero_registro(&(contexto.registros), registroDatos, &tipo_datoDestino);
    int tamanio_lectura = tamanio_tipo_dato(tipo_datoDestino);

    t_list* df_list = listaDireccionesFisicas(direccionLogica, tamanio_lectura);
    char *contenido = string_new();
    leerMemoria(df_list, &contenido); 

    switch (tipo_datoDestino) {
        case INT8:
            uint8_t valor = *(uint8_t *)contenido;
            SET(registroDatos, valor);
            log_info(log_conexion, "PID: %d - Acción: LEER - Valor Leido: %d", contexto.PID, valor);
            break;
        case INT32:
            uint32_t valor2 = *(uint32_t *)contenido;
            SET(registroDatos, valor2);
            log_info(log_conexion, "PID: %d - Acción: LEER - Valor Leido: %d", contexto.PID, valor2);
            break;
    }

    list_destroy_and_destroy_elements(df_list,  (void*) df_destroy);
    free(contenido);
}

void MOV_OUT(char* registroDireccion, char* registroDatos)
{
    data_type tipoDato;
    int tamanio_escritura;

    void * ptr;
    int direccionLogica = obtenerValorRegistro(registroDireccion);
    punteroRegistroDatos = obtener_puntero_registro(&(contexto.registros), registroDatos, &tipoDato);

    switch (tipoDato) {
        case INT8:
            ptr = (uint8_t *)punteroRegistroDatos;
            tamanio_escritura = sizeof(uint8_t);
            break;
        case INT32:
            ptr = (uint32_t *)punteroRegistroDatos;
            tamanio_escritura = sizeof(uint32_t);
            break;
        default:
            break;
    }
    t_list* df_list = listaDireccionesFisicas(direccionLogica, tamanio_escritura);
    escribirMemoria(df_list, ptr);
    list_destroy_and_destroy_elements(df_list,  (void*) df_destroy);
}

void SUM(char* destino, char* origen)
{
    registroDestino = obtener_puntero_registro(&(contexto.registros), destino, &tipo_datoDestino);
    registroOrigen  = obtener_puntero_registro(&(contexto.registros), origen, &tipo_datoOrigen);

    switch (tipo_datoDestino) { 
        case INT8:
            *((uint8_t*)registroDestino) += (tipo_datoOrigen == INT8) ? *((uint8_t*)registroOrigen) : (uint8_t)*((int32_t*)registroOrigen);
            break;
        case INT32:
            *((uint32_t*)registroDestino) += (tipo_datoOrigen == INT32) ? *((int32_t*)registroOrigen) : (uint32_t)*((int8_t*)registroOrigen);
            break;
    }
}

void SUB(char* destino, char* origen)
{
    registroDestino = obtener_puntero_registro(&(contexto.registros), destino, &tipo_datoDestino);
    registroOrigen  = obtener_puntero_registro(&(contexto.registros), origen, &tipo_datoOrigen);

    switch (tipo_datoDestino) { 
        case INT8:
            *((int8_t*)registroDestino) -= (tipo_datoOrigen == INT8) ? *((int8_t*)registroOrigen) : (int8_t)*((int32_t*)registroOrigen);
            break;
        case INT32:
            *((int32_t*)registroDestino) -= (tipo_datoOrigen == INT32) ? *((int32_t*)registroOrigen) : (int32_t)*((int8_t*)registroOrigen);
            break;
    }
}

void JNZ(char* registro, int instruccion)
{
    registroDestino = obtener_puntero_registro(&(contexto.registros), "PC", &tipo_datoDestino);
    registroOrigen = obtener_puntero_registro(&(contexto.registros), registro, &tipo_datoOrigen);

    registroDestino = obtener_puntero_registro(&(contexto.registros), registro, &tipo_datoDestino);
    if (registroDestino != NULL) 
    {
        switch (tipo_datoOrigen) {
            case INT8:
                uint8_t * aux_8 = registroOrigen;
                if (*aux_8 == 0){
                    SET(registro, instruccion);
                }
                break;
            case INT32:
                uint32_t * aux_32 = registroOrigen;
                if (*aux_32 == 0){
                    SET(registro, instruccion);
                }
                break;
        }
    }
}

void RESIZE(int tamanio)
{
    codigo_respuesta rta_code;
    t_paquete* paquete_a_memoria = crear_paquete();
    agregar_operacion(paquete_a_memoria, RESIZE_SOLICITUD);
    buffer_add_uint32(paquete_a_memoria->buffer, contexto.PID);
    buffer_add_uint32(paquete_a_memoria->buffer, tamanio);
	enviar_paquete(paquete_a_memoria, socket_memoria);
	eliminar_paquete(paquete_a_memoria);
    
    t_paquete* paquete_recibido_memoria = recibir_paquete(socket_memoria);
    op_code operacion = obtener_operacion(paquete_recibido_memoria);
    if (operacion == RESPUESTA_OK_ERROR)
    {
        rta_code = buffer_read_uint32(paquete_recibido_memoria->buffer);
        //Logica para tratar errores.
        if (rta_code == RTA_ERROR)
        {
            interrupt = 0;
            contexto.motivo = OUT_OF_MEMORY;
        }
    }
	eliminar_paquete(paquete_recibido_memoria);
}

void COPY_STRING(uint32_t tamanio)
{
    t_list* df_list;

    int direccionLogicaSI = obtenerValorRegistro("SI");
    df_list = listaDireccionesFisicas(direccionLogicaSI, tamanio);
    char* contenido = string_new();
    leerMemoria(df_list, &contenido); 
    list_destroy_and_destroy_elements(df_list,  (void*) df_destroy);

    int direccionLogicaDI = obtenerValorRegistro("DI");
    df_list = listaDireccionesFisicas(direccionLogicaDI, tamanio);
    escribirMemoria(df_list, contenido);
    list_destroy_and_destroy_elements(df_list,  (void*) df_destroy);

    free(contenido);
}

t_paquete* generic_semaforos(op_code operacion, char* recurso){
    t_paquete* paquete_a_kernel_io = crear_paquete();
    agregar_operacion(paquete_a_kernel_io, operacion);
    buffer_add_string(paquete_a_kernel_io->buffer, recurso);
    
    interrupt = 0;

    return paquete_a_kernel_io;
}

t_paquete* INS_WAIT(char* recurso)
{
    return generic_semaforos(WAIT, recurso);
}

t_paquete* INS_SIGNAL(char* recurso)
{
    return generic_semaforos(SIGNAL, recurso);
}

t_paquete* INS_IO_GEN_SLEEP(char* interfaz, uint32_t tiempo)
{
    t_paquete* paquete_a_kernel_io = crear_paquete();
    agregar_operacion(paquete_a_kernel_io, IO_GEN_SLEEP);
    buffer_add_string(paquete_a_kernel_io->buffer, interfaz);
    buffer_add_uint32(paquete_a_kernel_io->buffer, tiempo);
    INS_EXIT();
    return paquete_a_kernel_io;
}

t_paquete* generic_stdin_stdout(char* interfaz, char* registroDireccion, char* registroTamanio, op_code operacion)
{
    int direccionLogica = obtenerValorRegistro(registroDireccion);
    int tamanio = obtenerValorRegistro(registroTamanio);

    t_list* df_list = listaDireccionesFisicas(direccionLogica, tamanio);
    
    t_paquete* paquete_a_kernel_io = crear_paquete();
    agregar_operacion(paquete_a_kernel_io, operacion);
    buffer_add_string(paquete_a_kernel_io->buffer, interfaz);
    buffer_add_t_direcciones_fisicas_io(paquete_a_kernel_io->buffer, df_list);

    list_destroy_and_destroy_elements(df_list,  (void*) df_destroy);
    INS_EXIT();
    return paquete_a_kernel_io;
}

t_paquete* INS_IO_STDIN_READ(char* interfaz, char* registroDireccion, char* registroTamanio){
    return generic_stdin_stdout(interfaz, registroDireccion, registroTamanio, IO_STDIN_READ);
}

t_paquete* INS_IO_STDOUT_WRITE(char* interfaz, char* registroDireccion, char* registroTamanio){
    return generic_stdin_stdout(interfaz, registroDireccion, registroTamanio, IO_STDOUT_WRITE);
}

t_paquete* INS_IO_FS_CREATE(char* interfaz, char* nombreArchivo){
    t_paquete* paquete_a_kernel_io = crear_paquete();
    agregar_operacion(paquete_a_kernel_io, IO_FS_CREATE);
    buffer_add_string(paquete_a_kernel_io->buffer, interfaz);
    buffer_add_string(paquete_a_kernel_io->buffer, nombreArchivo);
    INS_EXIT();
    return paquete_a_kernel_io;
}

t_paquete* INS_IO_FS_DELETE(char* interfaz, char* nombreArchivo){
    t_paquete* paquete_a_kernel_io = crear_paquete();
    agregar_operacion(paquete_a_kernel_io, IO_FS_DELETE);
    buffer_add_string(paquete_a_kernel_io->buffer, interfaz);
    buffer_add_string(paquete_a_kernel_io->buffer, nombreArchivo);
    INS_EXIT();
    return paquete_a_kernel_io;
}

t_paquete* INS_IO_FS_TRUNCATE(char* interfaz, char* nombreArchivo, char* registroTamanio){

    int tamanio = obtenerValorRegistro(registroTamanio);

    t_paquete* paquete_a_kernel_io = crear_paquete();
    agregar_operacion(paquete_a_kernel_io, IO_FS_TRUNCATE);
    buffer_add_string(paquete_a_kernel_io->buffer, interfaz);
    buffer_add_string(paquete_a_kernel_io->buffer, nombreArchivo);
    buffer_add_uint32(paquete_a_kernel_io->buffer, tamanio);
    INS_EXIT();
    return paquete_a_kernel_io;
}

t_paquete* generic_io_fs_write_read(char* interfaz, char* nombreArchivo, char* registroDireccion, char* registroTamanio, char* registroPunteroArchivo, op_code operacion){
    int tamanio = obtenerValorRegistro(registroTamanio);
    int direccionLogica = obtenerValorRegistro(registroDireccion);
    t_list* df_list = listaDireccionesFisicas(direccionLogica, tamanio);
    
    t_paquete* paquete_a_kernel_io = crear_paquete();
    agregar_operacion(paquete_a_kernel_io, operacion);
    buffer_add_string(paquete_a_kernel_io->buffer, interfaz);
    buffer_add_string(paquete_a_kernel_io->buffer, nombreArchivo);
    buffer_add_t_direcciones_fisicas_io(paquete_a_kernel_io->buffer, df_list);
    buffer_add_uint32(paquete_a_kernel_io->buffer, atoi(registroPunteroArchivo));

    list_destroy_and_destroy_elements(df_list,  (void*) df_destroy);
    INS_EXIT();
    return paquete_a_kernel_io;
}

t_paquete* INS_IO_FS_WRITE(char* interfaz, char* nombreArchivo, char* registroDireccion, char* registroTamanio, char* registroPunteroArchivo){
    return generic_io_fs_write_read(interfaz, nombreArchivo, registroDireccion, registroTamanio, registroPunteroArchivo, IO_FS_WRITE);
};

t_paquete* INS_IO_FS_READ(char* interfaz, char* nombreArchivo, char* registroDireccion, char* registroTamanio, char* registroPunteroArchivo){
    return generic_io_fs_write_read(interfaz, nombreArchivo, registroDireccion, registroTamanio, registroPunteroArchivo, IO_FS_READ);
};

void INS_EXIT(){
    interrupt = 0;
}

void INCREMENTAR_PC(data_type tipo_dato, void* registro){
    if (tipo_dato == INT8){
        uint8_t * aux = registro;
        *aux  += 1;
    }
    else if (tipo_dato == INT32){
        uint32_t * aux = registro;
        *aux  += 1;
    }
}