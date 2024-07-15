#include <cpu_config.h>

void cargar_config(t_config_cpu* config_cpu, char * config_path) {
    t_config *config = config_create(config_path);

    config_cpu->ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    config_cpu->puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    config_cpu->puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    config_cpu->puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    config_cpu->cantidad_entradas_tlb = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
    config_cpu->algoritmo_tlb = config_get_string_value(config, "ALGORITMO_TLB");

}

void liberar_config(t_config_cpu* config) {
    free(config->ip_memoria);
    free(config->puerto_memoria);
    free(config->puerto_escucha_dispatch);
    free(config->puerto_escucha_interrupt);
    free(config->algoritmo_tlb);
    free(config);
}