#include <multiprogramacion.h>

t_prog prog;

bool paused;
sem_t mx_pause;

void iniciar_multiprogramacion(){
    prog.running = 0;
    prog.grado = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
    sem_init(&prog.sem, 0, prog.grado);
    sem_init(&prog.mx, 0, 1);

    paused = false;
    sem_init(&mx_pause, 0, 1);
}

void add_prog(){
    while (true) {
        sem_wait(&prog.sem);
        sem_wait(&prog.mx);
            if(prog.grado > prog.running){
                prog.running += 1;
                sem_post(&prog.mx);
                return;
            }
        sem_post(&prog.mx);
    }
}

void remove_prog(){
    sem_wait(&prog.mx);
        prog.running -= 1;
        if(prog.grado > prog.running){
            sem_post(&prog.sem);
        }
    sem_post(&prog.mx);
}

void modificar_grado(int valor){
    sem_wait(&prog.mx);
        if(valor > prog.grado){
            int dif = valor - prog.grado;
            for (int i = 0; i < dif; i++)
            {
                sem_post(&prog.sem);
            }
        }
        prog.grado = valor;
    sem_post(&prog.mx);
}


void check_pause(){
    sem_wait(&mx_pause);
    sem_post(&mx_pause);
}

void detener_planificacion(){
    if(!paused){
        paused = true;
        sem_wait(&mx_pause);
    }
}

void iniciar_planificacion(){
    if(paused){
        paused = false;
        sem_post(&mx_pause);
    }
}