#include <utils/serialize.h>

    void buffer_add_uint8(t_buffer *buffer, uint8_t data){
        buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint8_t));
        memcpy(buffer->stream + buffer->size, &data, sizeof(uint8_t));
        buffer->size += sizeof(uint8_t);
    }

    void buffer_add_uint32(t_buffer *buffer, uint32_t data){
        buffer->stream = realloc(buffer->stream, buffer->size + sizeof(uint32_t));
        memcpy(buffer->stream + buffer->size, &data, sizeof(uint32_t));
        buffer->size += sizeof(uint32_t);
    };

    void buffer_add_string(t_buffer *buffer, char *string){
        uint32_t lenght = strlen(string) + 1;
		buffer_add_uint32(buffer, lenght);
        buffer->stream = realloc(buffer->stream, buffer->size + lenght);
		memcpy(buffer->stream + buffer->size, string, lenght);
        buffer->size += lenght;
	}

    void buffer_add_void(t_buffer *buffer, uint32_t lenght, void *string){
		buffer_add_uint32(buffer, lenght);
        buffer->stream = realloc(buffer->stream, buffer->size + lenght);
		memcpy(buffer->stream + buffer->size, string, lenght);
        buffer->size += lenght;
	}

    void buffer_add_t_registro_cpu (t_buffer *buffer, t_registro_cpu  data){
        buffer_add_uint32(buffer, data.PC);
        buffer_add_uint8(buffer, data.AX);
        buffer_add_uint8(buffer, data.BX);
        buffer_add_uint8(buffer, data.CX);
        buffer_add_uint8(buffer, data.DX);
        buffer_add_uint32(buffer, data.EAX);
        buffer_add_uint32(buffer, data.EBX);
        buffer_add_uint32(buffer, data.ECX);
        buffer_add_uint32(buffer, data.EDX);
        buffer_add_uint32(buffer, data.SI);
        buffer_add_uint32(buffer, data.DI);
    };

    void buffer_add_t_contexto(t_buffer *buffer, t_contexto contexto){
        buffer_add_uint32(buffer, contexto.PID);
        buffer_add_uint32(buffer, contexto.motivo);
        buffer_add_t_registro_cpu(buffer, contexto.registros);
    }
    
    void buffer_add_t_instruccion(t_buffer *buffer, t_instruccion *instruccion) {
        buffer_add_string(buffer, instruccion->instruccion);
    }

    void buffer_add_t_direccion_fisica_io(t_buffer* buffer, t_direccion_fisica_io t_df_io) {
        buffer_add_uint32(buffer, t_df_io.df);
        buffer_add_uint32(buffer, t_df_io.size);
    }

    void buffer_add_t_direcciones_fisicas_io(t_buffer *buffer, t_list *lista_dfs) {
        int cantidad_dfs = list_size(lista_dfs);
        buffer_add_uint32(buffer, cantidad_dfs);

        for(int i = 0; i < cantidad_dfs; i++) {
            t_direccion_fisica_io *df = list_get(lista_dfs, i);
            log_trace(log_conexion, "direccion fisica: %i", df->df);
            buffer_add_t_direccion_fisica_io(buffer, *df);
        }
    }