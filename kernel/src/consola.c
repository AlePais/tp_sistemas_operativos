#include <consola.h>

COMMAND commands[] = {
    {"EJECUTAR_SCRIPT", exec},
    {"INICIAR_PROCESO", proc_start},
    {"FINALIZAR_PROCESO", proc_end},
    {"DETENER_PLANIFICACION", plan_end},
    {"INICIAR_PLANIFICACION", plan_start},
    {"MULTIPROGRAMACION", multiprog},
    {"PROCESO_ESTADO", proc_status},
    {"EXIT", cons_exit},
    { (char *)NULL, (void *)NULL}
};

int MAX_COMMAND_LENGTH = 100;
int PID_COUNTER = 0;

t_log *logger;

void* consola(void* arg) {
    char *linea;
    int result;
    
    logger = log_create("consola.log", "CONSOLA", false, LOG_LEVEL_DEBUG);
    
    bool done;
    while(!done) {
        linea = readline("> ");

        if(!linea) break;

        string_trim(&linea);

        log_info(logger, "Usuario ingresó línea: %s", linea);
        
        if(*linea) {
            add_history(linea);            
            result = execute_line(linea);
            if(result == 1) {
                log_info(logger, "Cerrando consola.");
                done = true;
            }
        }

        free(linea);
    } 

    return NULL;
}

int execute_line(char *line) {
    register int i;
    COMMAND *command;
    char *word;

    i = 0;
    while(line[i] && whitespace(line[i])) {
        i++;
    }

    word = line + i;

    while(line[i] && !whitespace(line[i])) {
        i++;
    }

    if(line[i]) { 
        line[i++] = '\0'; 
    }
    
    command = find_command(word);

    if(!command) {
        log_error(logger, "Comando no reconocido.");
        return 0;
    }

    //Argumentos del comando
    while(whitespace(line[i])) {
        i++;
    }

    word = line + i;

    //Llamar a la funcion
    return (*command->func)(word);
    
}

COMMAND *find_command(char *name) {
    int i;

    for(i = 0; commands[i].name; i++) {
        if(strcmp(name, commands[i].name) == 0) return (&commands[i]);
    }

    return NULL;
}

/***********************************************************************************/
/*                         FUNCIONES DE COMMANDS                                   */
/***********************************************************************************/

int exec(char *path)
{
    if(*path == '/'){
        path++;
    }
    FILE* archivo = fopen(path, "r");
    if (archivo == NULL) {
        log_error(logger, "Archivo no encontrado.");
        return 0;
    }

    log_info(logger, "Archivo abierto con exito.");

    // Leer y ejecutar cada comando del archivo
    char comando[MAX_COMMAND_LENGTH];
    while (fgets(comando, sizeof(comando), archivo) != NULL) {

        // Eliminar el salto de línea del final del comando (si existe)
        size_t longitud = strlen(comando);
        if (comando[longitud - 1] == '\n') {
            comando[longitud - 1] = '\0';
        }

        log_info(logger, "Archivo ingresó línea: %s", comando);
        execute_line(comando);
    }

    fclose(archivo);
    return 0;
}

int proc_start(char *path)
{
    t_PCB* PCB = calloc(1, sizeof(t_PCB));

    PCB->PID = PID_COUNTER++;

    char* algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    if (strcmp(algoritmo, "FIFO") == 0)
        PCB->Quantum = -1;
    else{
        PCB->Quantum = config_get_int_value(config,"QUANTUM");
    }

    PCB->estado = NEW;
    PCB->retorno = NONE;

    PCB->archivo = malloc(strlen(path) + 1);
    strcpy(PCB->archivo, path);
    recusos_init(&PCB->recursos_asignados);

    sem_init(&PCB->mx_PCB, 0, 1);

    sem_wait(&mx_lista_PCB);
    list_add_in_index(lista_PCB, PCB->PID, PCB);
    sem_post(&mx_lista_PCB);

    sem_wait(&mx_cola_new);
    queue_push(cola_new, &PCB->PID);
    sem_post(&mx_cola_new);
    
    sem_post(&sem_cola_new);

    log_info(log_kernel, "Se crea el proceso %d en NEW", PCB->PID);

    return 0;
}

int proc_end(char *char_PID)
{
    int PID = atoi(char_PID);
    bool buscar_por_index(void* arg){
        t_PCB* registro_PCB = (t_PCB*) arg;
        bool coinciden = (registro_PCB->PID == PID);
        return coinciden;
    }

    sem_wait(&mx_lista_PCB);
    t_PCB* PCB = list_find(lista_PCB, buscar_por_index);
    sem_post(&mx_lista_PCB);

    if(PCB == NULL || PCB->estado == EXIT || PCB->estado == DELETED){
        //TODO ERROR
        return 0;
    }

    sem_wait(&mx_cola_exit);
        queue_push(cola_exit, &PCB->PID);
    sem_post(&mx_cola_exit);
    
    sem_post(&sem_cola_exit);

    return 0;
}

int multiprog(char *char_valor)
{
    int valor = atoi(char_valor);
    modificar_grado(valor);
    return 0;
}

int plan_start()
{
    iniciar_planificacion();
    return 0;
}

bool ordenar(void* arg_1, void* arg_2){
    t_PCB* PCB_1 = (t_PCB*) arg_1;
    t_PCB* PCB_2 = (t_PCB*) arg_2;
    return PCB_1->estado <= PCB_2->estado;
}

int proc_status()
{
    sem_wait(&mx_lista_PCB);
        t_list *lista_ordenada = list_duplicate(lista_PCB);
    sem_post(&mx_lista_PCB);

    list_sort(lista_ordenada, ordenar);
    int count = list_size(lista_ordenada);
    for (int i = 0; i < count; i++)
    {
        t_PCB* PCB = list_get(lista_ordenada, i);

        switch (PCB->estado) {
            case NEW:       printf("Estado: %-7s - ", "NEW");       break;
            case READY:     printf("Estado: %-7s - ", "READY");     break;
            case BLOCKED:   printf("Estado: %-7s - ", "BLOCKED");   break;
            case EXEC:      printf("Estado: %-7s - ", "EXEC");      break;
            case EXIT:      printf("Estado: %-7s - ", "EXIT");      break;
            case DELETED:   printf("Estado: %-7s - ", "DELETED");   break;
        }
        
        printf("Proceso: %2d - ", PCB->PID);
        printf("Quantum: %4d - ", PCB->Quantum);
        
        int cant_rec = list_size(lista_recursos);
        printf("[");
        for (int i = 0; i < cant_rec; i++) {
            printf("%d", PCB->recursos_asignados[i]);
            if (i != cant_rec - 1) printf(",");
        }
        printf("] - ");
        
        printf("Archivo: %s\n", PCB->archivo);
    }
    list_destroy(lista_ordenada);
    return 0;
}

int plan_end()
{
    detener_planificacion();
    return 0;
}

int cons_exit()
{
return 1;
}