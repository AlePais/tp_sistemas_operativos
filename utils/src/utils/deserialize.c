#include <utils/deserialize.h>


    uint8_t buffer_read_uint8(t_buffer *buffer){
        uint8_t data;
        memcpy(&data, buffer->offset, sizeof(uint8_t));
        buffer->offset += sizeof(uint8_t);
        return data;
    };

    uint32_t buffer_read_uint32(t_buffer *buffer){
        uint32_t data;
        memcpy(&data, buffer->offset, sizeof(uint32_t));
        buffer->offset += sizeof(uint32_t);
        return data;
    };
	
    char *buffer_read_string(t_buffer *buffer){
		char *data;
        uint32_t lenght= buffer_read_uint32(buffer);
		data = malloc(lenght);
        memcpy(data, buffer->offset, lenght);
        buffer->offset += lenght;
        return data;
	}

    t_contexto buffer_read_t_contexto(t_buffer *buffer){
        t_contexto contexto;
        contexto.PID = buffer_read_uint32(buffer);
        contexto.motivo = buffer_read_uint32(buffer);
        contexto.registros = buffer_read_t_registro_cpu(buffer);
        return contexto;
    }

    t_registro_cpu buffer_read_t_registro_cpu (t_buffer *buffer){
        t_registro_cpu data;
        data.PC = buffer_read_uint32(buffer);
        data.AX = buffer_read_uint8(buffer);
        data.BX = buffer_read_uint8(buffer);
        data.CX = buffer_read_uint8(buffer);
        data.DX = buffer_read_uint8(buffer);
        data.EAX = buffer_read_uint32(buffer);
        data.EBX = buffer_read_uint32(buffer);
        data.ECX = buffer_read_uint32(buffer);
        data.EDX = buffer_read_uint32(buffer);
        data.SI = buffer_read_uint32(buffer);
        data.DI = buffer_read_uint32(buffer);
        return data;
    }

    t_direccion_fisica_io* buffer_read_t_direccion_fisica_io(t_buffer *buffer) {
        t_direccion_fisica_io *t_df_io = malloc(sizeof(t_direccion_fisica_io));
        t_df_io->df = buffer_read_uint32(buffer);
        t_df_io->size = buffer_read_uint32(buffer);
        return t_df_io;
    }

    t_list* buffer_read_t_direcciones_fisicas_io(t_buffer *buffer) {
        t_list *lista = list_create();
        int cantidad_dfs = buffer_read_uint32(buffer);

        for(int i = 0; i < cantidad_dfs; i++) {
            t_direccion_fisica_io *df = buffer_read_t_direccion_fisica_io(buffer);
            list_add(lista, df);
        }

        return lista;
    }

    
