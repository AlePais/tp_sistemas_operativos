#include <filesystem.h>

t_log* log_filesystem;

t_bitmap *bitmap;

void inicializar_log_filesystem() {
    if(log_filesystem == NULL) {
        log_filesystem = log_create("filesystem.log", "FILESYSTEM", true, LOG_LEVEL_INFO);
    }
}

void generar_archivo_bloques(){
    int tamanio_archivo_bloques = config->block_size  * config->block_count;

    int fd_bloques = open("bloques.dat",  O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); 

    if(fd_bloques == -1){
        log_error(log_filesystem , "El archivo de bloques no se creo correctamente");
    }

    if(ftruncate(fd_bloques, tamanio_archivo_bloques) == -1){
        log_error(log_filesystem, "error al truncar el archivo");
    }   

    close(fd_bloques);
}

void crear_bitmap(){
    bitmap = malloc(sizeof(t_bitmap));

    int fd_bitmap = open("bitmap.dat", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_bitmap == -1){
        log_error(log_filesystem, "Error al abrir el archivo bitmap");
    }

    bitmap->tamanio = (config->block_count / 8);
    
    if (ftruncate(fd_bitmap, bitmap->tamanio) == -1) {
        log_error(log_filesystem, "Error al truncar el archivo Bitmap");
    }

    bitmap->direccion = mmap(NULL, bitmap->tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitmap, 0);
    if (bitmap->direccion == MAP_FAILED) {
        log_error(log_filesystem, "Error al mapear el Bitmap");
    }

    bitmap->bitarray = bitarray_create_with_mode(bitmap->direccion, bitmap->tamanio, LSB_FIRST);

    close(fd_bitmap);

}

void bitmap_marcar_bloque_libre(int numero_bloque){

    bitarray_clean_bit(bitmap->bitarray, numero_bloque);
    int i = msync(bitmap->direccion, bitmap->tamanio, MS_SYNC);
    log_trace(log_filesystem, "Se marco el bloque libre %i", numero_bloque);
    if ( i  == -1) {
        log_error(log_filesystem, "Error al sincronizar los cambios en el Bitmap");
    }
    return;
}

void bitmap_marcar_bloques_libres(int bloque_inicial, int bloque_final){

   for(int i = bloque_inicial; i<=bloque_final; i++){ 
    bitmap_marcar_bloque_libre(i);

   }
}

void bitmap_marcar_bloques_ocupados(int bloque_inicial, int bloque_final){

   for(int i = bloque_inicial; i<=bloque_final; i++){ 
    bitmap_marcar_bloque_ocupado(i);

    }
    return;
}

void bitmap_marcar_bloque_ocupado(int numero_bloque){

    bitarray_set_bit(bitmap->bitarray, numero_bloque);
    log_trace(log_filesystem, "Se marco el bloque ocupado %i", numero_bloque);
    if (msync(bitmap->direccion, bitmap->tamanio, MS_SYNC) == -1) {
        log_error(log_filesystem, "Error al sincronizar los cambios en el Bitmap");
    }
}

int bitmap_encontrar_bloques_libres_continuos(int tamanio_archivo) {
    int i, j;
    bool bloque_ocupado;
    int bloques_encontrados = 0;
    int cantidad_bloques_necesarios = calcular_bloques_necesarios(tamanio_archivo);

    for (i = 0; i <= config->block_count - cantidad_bloques_necesarios; i++) {
        bloques_encontrados = 0;

        for (j = 0; j < cantidad_bloques_necesarios; j++) {
            bloque_ocupado = bitarray_test_bit(bitmap->bitarray, i + j);

            if (bloque_ocupado) {
                break;
            } else {
                bloques_encontrados++;
            }
        }
        if (bloques_encontrados == cantidad_bloques_necesarios) {
            return i;
        }
    }

    log_error(log_filesystem, "EL archivo no entra en el filesystem");
    return -1;
}

bool hay_espacio_contiguo(t_FCB* fcb, int bloques_actuales, int bloques_necesarios){
    
    int bloque_final_archivo = fcb->bloque_inicial + bloques_actuales;
    for (int i = bloque_final_archivo; i < bloque_final_archivo + bloques_necesarios; i++){
        bool estado = bitarray_test_bit(bitmap->bitarray, i);
        if(estado){
            return false;
        }
    }
    
    return true;
}

int bitmap_encontrar_bloque_libre(){  

    int i;
    bool bloque_ocupado;
    for (i = 0; i < config->block_count; i++)
    {   
        bloque_ocupado  = bitarray_test_bit(bitmap->bitarray, i);
        if(!bloque_ocupado)
        {
            return i;
            break;
        }
    }
    return -1;
}

void crear_archivo(t_FCB *fcb) {

    char* fullpath = get_fullpath(fcb->nombre_archivo);

    log_trace(log_filesystem, "RUTA ARCHIVO %s", fullpath);

    int fd_archivo = open(fullpath , O_CREAT, S_IRUSR | S_IWUSR | O_RDWR); 

    if(fd_archivo == -1){
        log_error(log_conexion , "El archivo %s no se creo correctamente", fcb->nombre_archivo);
    }

    t_config* config_fcb = config_create(fullpath);

    config_set_value(config_fcb, "NOMBRE_ARCHIVO", fcb->nombre_archivo);
    char* tam_arch = malloc(sizeof(uint32_t));
    sprintf(tam_arch, "%d", fcb->size);
    config_set_value(config_fcb, "TAMANIO_ARCHIVO", tam_arch); 
    char* bloque_incial = malloc(sizeof(uint32_t));
    sprintf(bloque_incial, "%d", fcb->bloque_inicial);
    config_set_value(config_fcb, "BLOQUE_INICIAL", bloque_incial);
    
    config_save(config_fcb);

    free(tam_arch);
    free(bloque_incial);
    free(fullpath);
    config_destroy(config_fcb);
}

t_FCB* leer_metadata(char* nombre_archivo){
    t_FCB *config_fcb = malloc(sizeof(t_FCB));


    char* fullpath = get_fullpath(nombre_archivo);

    t_config* archivo_metadata = config_create(fullpath);

    config_fcb->nombre_archivo = strdup(config_get_string_value(archivo_metadata,"NOMBRE_ARCHIVO"));
    config_fcb->bloque_inicial = atoi(config_get_string_value(archivo_metadata,"BLOQUE_INICIAL"));
    config_fcb->size = atoi(config_get_string_value(archivo_metadata,"TAMANIO_ARCHIVO"));

    config_save(archivo_metadata);

    config_destroy(archivo_metadata);
    free(fullpath);
    

    return config_fcb;
}

int calcular_bloques_necesarios(int size){
    int cantidad_bloques = (int)ceil((double)size / config->block_size);
    return max(cantidad_bloques, 1);
}

void eliminar_archivo_metadata(char* nombre_archivo){

    char* fullpath = get_fullpath(nombre_archivo);

    if (remove(fullpath) == 0){
        log_trace(log_conexion, "El archivo %s se elimino correctamente", fullpath);
    } else {
        log_error(log_conexion, "Error al eliminar el archivo %s", fullpath);
    }

    free(fullpath);
}

void marcar_bloques_libres(char* nombre_archivo){
    t_FCB* fcb = leer_metadata(nombre_archivo);

    int bloques_a_liberar = calcular_bloques_necesarios(fcb->size);

    int bloque_inicial = fcb->bloque_inicial;

    for(int i = 1; i <= bloques_a_liberar; i++){
        bitmap_marcar_bloque_libre(bloque_inicial);
        bloque_inicial++;
    };
    //free(fcb->nombre_archivo);
    free(fcb);
}

char* get_fullpath(char* nombre_archivo) {

    char* fullpath = strdup(config->path_base_dialFs); // Inicializamos con el path base
   
    log_trace(log_filesystem, "Path hasta ahora: %s", fullpath);

    string_append(&fullpath, "/");
    string_append(&fullpath, nombre_archivo);
    
    return fullpath;
}

void escribir_en_archivo(int bloque_inicial, int offset, void *dato, int size) {
    int file_bloques_dat = open("bloques.dat", O_RDWR);

    int offset_total = (bloque_inicial * config->block_size) + offset;
    lseek(file_bloques_dat, offset_total, SEEK_SET);

    write(file_bloques_dat, dato, size);
    
    free(dato);
    
    close(file_bloques_dat);
}

char* leer_archivo(int tamanio, t_FCB *fcb, int offset) {
    char *dato = malloc(tamanio);

    int file_bloques_dat = open("bloques.dat", O_RDWR);

    lseek(file_bloques_dat, (fcb->bloque_inicial * config->block_size) + offset, SEEK_SET);
    ssize_t bytes_leidos = read(file_bloques_dat, dato, tamanio);
    if (bytes_leidos == -1) {
        perror("Error al leer el archivo");
        close(file_bloques_dat);
    }

    close(file_bloques_dat);
    
    return dato;
}

void agrandar_archivo(t_FCB *fcb, int tamanio_nuevo, int pid) {
    int bloques_actuales = calcular_bloques_necesarios(fcb->size);
    int bloques_finales = calcular_bloques_necesarios(tamanio_nuevo);
    int nuevo_bloque_inicial = 0;
    
    if(!hay_espacio_contiguo(fcb, bloques_actuales, bloques_finales - bloques_actuales)){ //esa resta para buscar los bloques necesarios
        log_info(log_conexion, "PID: <%i> - Inicio Compactación.", pid);
        nuevo_bloque_inicial = compactar(fcb);
        usleep(config->retraso_compactacion * 1000);
        log_info(log_conexion, "PID: <%i> - Fin Compactación.", pid); 
	}

    for(int i = bloques_actuales; i < bloques_finales; i++){
        bitmap_marcar_bloque_ocupado(fcb->bloque_inicial + i);
    }
    fcb->size = tamanio_nuevo;
    fcb->bloque_inicial = nuevo_bloque_inicial;
    crear_archivo(fcb);
}

void achicar_archivo(t_FCB *fcb, int tamanio_nuevo) {

    int bloques_actuales = calcular_bloques_necesarios(fcb->size);
    int bloques_finales = calcular_bloques_necesarios(tamanio_nuevo);
    
    for(int i = bloques_actuales - 1; i >= bloques_finales; i--){
        bitmap_marcar_bloque_libre(fcb->bloque_inicial + i);
    }

    fcb->size = tamanio_nuevo;
    crear_archivo(fcb);
}

t_list* leer_directorio(){
    t_list *lista = list_create();
    DIR *d;
	struct dirent *dir;
	d = opendir(config->path_base_dialFs);
	if (d) {
		dir = readdir(d);
		while (dir != NULL) {
			
            if(dir->d_type != DT_REG) {
                dir = readdir(d);
                continue;
            }

            list_add(lista, leer_metadata(dir->d_name));

			dir = readdir(d);
		}
		closedir(d);
	}
    
    list_sort(lista, comparator_fcbs_size);
    log_trace(log_filesystem, "Se leyeron %i", list_size(lista));

    /*PARA DEBUG*/
    for(int i = 0; i < list_size(lista); i++) {
		t_FCB *fcb_l = (t_FCB*) list_get(lista, i);

		log_trace(log_filesystem, "nombre: %s bloque inicial %i", fcb_l->nombre_archivo, fcb_l->bloque_inicial);
	}

    return lista;
}

bool comparator_fcbs_size(void *e1, void* e2) {
    t_FCB *f1 = (t_FCB*) e1; 
    t_FCB *f2 = (t_FCB*) e2; 

    return f1->bloque_inicial < f2->bloque_inicial;
}

int compactar(t_FCB* fcb){
    t_list* lista_fcb = leer_directorio();
    t_FCB* fcb_a_agrandar = buscar_fcb(fcb, lista_fcb);

    // if(list_remove_element(lista_fcb, fcb_a_agrandar)){
	// log_error(log_filesystem, "No se removio el elemento %s", fcb_a_agrandar->nombre_archivo);
    // }

    int tamanio_archivo_agrandar =  calcular_bloques_necesarios(fcb_a_agrandar->size) * config->block_size;
    char* contenido_a_agrandar = malloc(tamanio_archivo_agrandar);
    
    contenido_a_agrandar = buscar_contenido_de(fcb_a_agrandar);
    //log_error(log_filesystem, "EL contenido del archivo a agrandar es %s", contenido_a_agrandar);

    //log_trace(log_filesystem, "EL archivo a agrandar es %s", fcb_a_agrandar->nombre_archivo);
    
    bitmap_marcar_bloques_libres(fcb->bloque_inicial,max(calcular_bloques_necesarios(fcb_a_agrandar->size) + fcb_a_agrandar->bloque_inicial -1, 0));

    for(int i = 0; i<list_size(lista_fcb); i++){
        t_FCB* fcb_a_liberar = list_get(lista_fcb, i);
        if(fcb_a_liberar->bloque_inicial == fcb_a_agrandar->bloque_inicial){continue;}
        int bloque_inicial = fcb_a_liberar->bloque_inicial;
        int bloque_final = max(calcular_bloques_necesarios(fcb_a_liberar->size) + bloque_inicial -1, 0);
        int tamanio = calcular_bloques_necesarios(fcb_a_liberar->size) * config->block_size;
        //log_trace(log_conexion, "Se mueve el archivo %s", fcb_a_liberar->nombre_archivo);

        char* contenido_bloques;
        contenido_bloques = buscar_contenido_de(fcb_a_liberar);
        //log_error(log_filesystem, "EL contenido del archivo a mover es %s", contenido_bloques);

        bitmap_marcar_bloques_libres(bloque_inicial,bloque_final);

        int nuevo_bloque_inicial = copiar_contenido_a(contenido_bloques, tamanio);

        fcb_a_liberar->bloque_inicial = nuevo_bloque_inicial;

        crear_archivo(fcb_a_liberar);
        // if(contenido_bloques){
        //     free(contenido_bloques);
        // }
        
        free(fcb_a_liberar->nombre_archivo);
        free(fcb_a_liberar);
    }
   
    int nuevo_bloque_inicial_archivo_agrandar = copiar_contenido_a(contenido_a_agrandar, tamanio_archivo_agrandar);
    //log_trace(log_filesystem, "El nuevo bloque inicial del archivo que se agrando es %i", nuevo_bloque_inicial_archivo_agrandar);

    list_destroy(lista_fcb);
    
    return nuevo_bloque_inicial_archivo_agrandar;
}

t_FCB* buscar_fcb(t_FCB* fcb_buscado, t_list* lista_fcb){
    bool comparar_fcb(void *fcb) {
        t_FCB *fcb_a_comparar = (t_FCB *) fcb;
        return fcb_a_comparar->bloque_inicial == fcb_buscado->bloque_inicial;
    }

    if(lista_fcb == NULL) {
        log_error(log_conexion, "No hay fcbs");
        return NULL;
    }

    t_FCB *fcb_encontrado = (t_FCB*)list_find(lista_fcb, comparar_fcb);

    return fcb_encontrado;
}

char* buscar_contenido_de(t_FCB* fcb){

    int bloque_inicial = fcb->bloque_inicial;
    int bloques_a_leer = calcular_bloques_necesarios(fcb->size);
    int tamanio_a_leer = bloques_a_leer*config->block_size;

    char* contenido = malloc(tamanio_a_leer);

    contenido = leer_bloques(bloque_inicial, tamanio_a_leer);

    return contenido;
}	

char* leer_bloques(int bloque_inicial, int tamanio) {
    char *dato = malloc(tamanio);

    int file_bloques_dat = open("bloques.dat", O_RDWR);
    int offset = bloque_inicial * config->block_size;

    lseek(file_bloques_dat, offset , SEEK_SET);
    ssize_t bytes_leidos = read(file_bloques_dat, dato, tamanio);
    if (bytes_leidos == -1) {
        perror("Error al leer el archivo");
        close(file_bloques_dat);
    }

    close(file_bloques_dat);
    
    return dato;
}

int copiar_contenido_a(char* contenido, int tamanio){

    int bloque_inicial = bitmap_encontrar_bloque_libre();
    log_trace(log_filesystem, "Se encontro el bloque libre %i", bloque_inicial);

    escribir_en_archivo(bloque_inicial, 0, contenido, tamanio);

    if(calcular_bloques_necesarios(tamanio) == 1){
        bitmap_marcar_bloque_ocupado(bloque_inicial);
    }else{
        bitmap_marcar_bloques_ocupados(bloque_inicial, calcular_bloques_necesarios(tamanio));
    }    
    return bloque_inicial;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}



