#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <main.h>
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <pthread.h>
#include <utils/conexion.h>
#include <utils/estructuras.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <readline/history.h>

typedef struct {
    char *name;
    int (*func)();
} COMMAND;

void* consola(void* arg);

COMMAND *find_command(char *name);
int execute_line(char *line);

int exec(char *path);
int proc_start(char *path);
int proc_end(char *char_PID);
int plan_end();
int plan_start();
int multiprog(char *char_valor);
int proc_status();
int cons_exit();

#endif