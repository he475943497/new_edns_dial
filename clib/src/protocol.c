
#include <clib/protocol.h>
#include <clib/socket.h>
#include <clib/memory.h>
#include <clib/log.h>
#include <string.h>

struct _protocol_unit {
    uint32_t sn;
    buffer_t* body;
};

struct _unit_reader{
	protocol_unit_t* unit;
	uint16_t offset;
};

void protocol_unitdestroy_func(void *data){
	struct _protocol_unit* unit = (struct _protocol_unit*)data;
	if(unit->body != NULL){
		buffer_free(unit->body);
	}
	mem_free(unit);
}

protocol_unit_t* unit_new(){
	struct _protocol_unit* unit = (struct _protocol_unit*)mem_alloc(sizeof(struct _protocol_unit));
	unit->sn = 0;
	unit->body = buffer_new(256);
	return unit;
}

uint16_t unit_get_len(protocol_unit_t* unit){
	return sizeof(unit->sn) + buffer_length(unit->body);
}
void unit_set_sn(protocol_unit_t* unit, uint32_t sn){
	unit->sn = sn;
}

uint32_t unit_get_sn(protocol_unit_t* unit){
	return unit->sn;
}

void unit_push_int8(protocol_unit_t* unit,int8_t data){
	buffer_append_int8(unit->body, data);
}

void unit_push_int16(protocol_unit_t* unit,int16_t data){
	buffer_append_int16(unit->body, htons(data));
}

void unit_push_int32(protocol_unit_t* unit,int32_t data){
	buffer_append_int32(unit->body, htonl(data));
}

void unit_push_int64(protocol_unit_t* unit,int64_t data){
	buffer_append_int64(unit->body, htonll(data));
}

void unit_push_string(protocol_unit_t* unit,const char* data){
	buffer_append(unit->body, data, strlen(data)+1);
}

void unit_free(protocol_unit_t* unit){
	protocol_unitdestroy_func(unit);
}

unit_reader_t* unit_reader_new(protocol_unit_t* unit){
	struct _unit_reader* reader = (struct _unit_reader*)mem_alloc(sizeof(struct _unit_reader));
	reader->offset = 0;
	reader->unit = unit;
	return reader;
}

int8_t unit_read_int8(unit_reader_t* reader){
	buffer_t* buffer = reader->unit->body;
	uint buflen = buffer_length(buffer);
	if (buflen < reader->offset + sizeof(uint8_t)) { //out of range
		return 0;
	}

    const char* data = buffer_data(buffer) + reader->offset;
    reader->offset += sizeof(uint8_t);

    return *data;
}

int16_t unit_read_int16(unit_reader_t* reader) {
	buffer_t* buffer = reader->unit->body;
	uint buflen = buffer_length(buffer);
	if (buflen < reader->offset + sizeof(uint16_t)) { //out of range
		return 0;
	}

	const char* data = buffer_data(buffer) + reader->offset;
	reader->offset += sizeof(uint16_t);

	return ntohs(*((int16_t*) data));
}

int32_t unit_read_int32(unit_reader_t* reader) {
	buffer_t* buffer = reader->unit->body;
	uint buflen = buffer_length(buffer);
	if (buflen < reader->offset + sizeof(uint32_t)) { //out of range
		return 0;
	}

	const char* data = buffer_data(buffer) + reader->offset;
	reader->offset += sizeof(uint32_t);

	return ntohl(*((int32_t*) data));
}

int64_t unit_read_int64(unit_reader_t* reader){
	buffer_t* buffer = reader->unit->body;
	uint buflen = buffer_length(buffer);
	if (buflen < reader->offset + sizeof(uint64_t)) { //out of range
		return 0;
	}

	const char* data = buffer_data(buffer) + reader->offset;
	reader->offset += sizeof(uint64_t);

	return ntohll(*((int64_t*) data));
}

const char* unit_read_string(unit_reader_t* reader){
	buffer_t* buffer = reader->unit->body;
	uint buflen = buffer_length(buffer);
	if (buflen <= reader->offset) { //out of range
		return 0;
	}

	const char* data = buffer_data(buffer) + reader->offset;
	reader->offset += strlen(data)+1;

	return data;
}

void unit_reader_free(unit_reader_t* reader){
	mem_free(reader);
}

buffer_t* protocol_dump_all(const struct protocolhdr* hdr, ptr_array_t* units){
	buffer_t* buffer = buffer_new(512);
	buffer_append(buffer, hdr, sizeof(struct protocolhdr));
	uint32_t protocol_len = 0;
	if(units != NULL){
		for(uint i = 0; i < ptr_array_size(units); i++){
			struct _protocol_unit* unit = ptr_array_at(units, i);
			buffer_append_int16(buffer, htons(unit_get_len((protocol_unit_t*)unit)));
			buffer_append_int32(buffer, htonl(unit->sn));
			buffer_append_buffer(buffer, unit->body);
			protocol_len += unit_get_len((protocol_unit_t*)unit) + sizeof(uint16_t);
		}
	}
	struct protocolhdr* temp_hdr = (struct protocolhdr*)buffer_data(buffer);
	temp_hdr->bl = htonl(protocol_len);
	temp_hdr->cid = htons(temp_hdr->cid);
	temp_hdr->sbt = htons(temp_hdr->sbt);

	return buffer;
}

buffer_t* protocol_dump(const struct protocolhdr* hdr, protocol_unit_t* unit){
	buffer_t* buffer = buffer_new(512);
	buffer_append(buffer, hdr, sizeof(struct protocolhdr));
	uint32_t protocol_len = 0;
	if (unit != NULL) {

		buffer_append_int16(buffer,
				htons(unit_get_len((protocol_unit_t*) unit)));
		buffer_append_int32(buffer, htonl(unit->sn));
		buffer_append_buffer(buffer, unit->body);
		protocol_len += unit_get_len((protocol_unit_t*) unit)
				+ sizeof(uint16_t);
	}
	struct protocolhdr* temp_hdr = (struct protocolhdr*) buffer_data(buffer);
	temp_hdr->bl = htonl(protocol_len);
	temp_hdr->cid = htons(temp_hdr->cid);
	temp_hdr->sbt = htons(temp_hdr->sbt);

	return buffer;
}

#pragma pack(push)
#pragma pack(1)
struct unit {
    uint16_t data_len;   //该长度不包含SN
    uint32_t sn;
    char data[0];
};
#pragma pack(pop)

ptr_array_t* protocol_load_units(const char* data, uint32_t len) {
	ptr_array_t *array = ptr_array_new_full(4, protocol_unitdestroy_func);
	uint32_t left_size = len;
	while (left_size > 0) {
		protocol_unit_t* unit = unit_new();
		struct unit *temp_unit = (struct unit *) (data + len - left_size);
		uint16_t unit_len = ntohs(temp_unit->data_len);
		if (unit_len + sizeof(unit_len) > left_size) {
			LOG(LOG_LEVEL_WARNING, "unit len error");
			ptr_array_free(array);
			return NULL;
		}

		unit_set_sn(unit, ntohl(temp_unit->sn));
		buffer_append(unit->body, &temp_unit->data, unit_len);

		ptr_array_append(array, unit);

		left_size -= unit_len + sizeof(unit_len);
	}

	return array;
}
