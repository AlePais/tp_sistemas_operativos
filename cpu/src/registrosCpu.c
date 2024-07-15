#include <registrosCpu.h>

void* obtener_puntero_registro(t_registro_cpu* registro, char *nombre_campo, data_type* tipo_dato) {
    *tipo_dato = INT32;
    if (strcmp(nombre_campo, "PC") == 0) {
        return &(registro->PC);
    } else if (strcmp(nombre_campo, "AX") == 0) {
        *tipo_dato = INT8;
        return &(registro->AX);
    } else if (strcmp(nombre_campo, "BX") == 0) {
        *tipo_dato = INT8;
        return &(registro->BX);
    } else if (strcmp(nombre_campo, "CX") == 0) {
        *tipo_dato = INT8;
        return &(registro->CX);
    } else if (strcmp(nombre_campo, "DX") == 0) {
        *tipo_dato = INT8;
        return &(registro->DX);
    } else if (strcmp(nombre_campo, "EAX") == 0) {
        return &(registro->EAX);
    } else if (strcmp(nombre_campo, "EBX") == 0) {
        return &(registro->EBX);
    } else if (strcmp(nombre_campo, "ECX") == 0) {
        return &(registro->ECX);
    } else if (strcmp(nombre_campo, "EDX") == 0) {
        return &(registro->EDX);
    } else if (strcmp(nombre_campo, "SI") == 0) {
        return &(registro->SI);
    } else if (strcmp(nombre_campo, "DI") == 0) {
        return &(registro->DI);
    } else {
        return NULL;
    }
}

int obtenerValorRegistroYTipo(char * registro, data_type * tipoDato) {
    int valorRegistro;
    void* punteroRegistro = obtener_puntero_registro(&(contexto.registros), registro, tipoDato);
    switch (*tipoDato) {
        case INT8:
            valorRegistro = *(uint8_t *)punteroRegistro;
            break;
        case INT32:
            valorRegistro = *(uint32_t *)punteroRegistro;
            break;
        default:
            break;
    }
    return valorRegistro;
}

int obtenerValorRegistro(char * registro){
    data_type tipoDato;
    int valorRegistro = obtenerValorRegistroYTipo(registro, &tipoDato);

    return valorRegistro;
}

int tamanio_tipo_dato(data_type tipoDato){
    int tamanioTipoDato = 0;

    switch (tipoDato) {
        case INT8:
            tamanioTipoDato = sizeof(uint8_t);
            break;
        case INT32:
            tamanioTipoDato = sizeof(uint32_t);
            break;
        default:
            // -1 para indicar un tipo de dato no definido
            tamanioTipoDato = -1;
            break;
    }

    return tamanioTipoDato;
}