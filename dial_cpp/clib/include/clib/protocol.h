#ifndef CLIB_PROTOCOL_H_
#define CLIB_PROTOCOL_H_

#include <clib/types.h>
#include <clib/buffer.h>
#include <clib/array.h>

#pragma pack(push)
#pragma pack(1)

//同MS通信协议头
struct protocolhdr {
    uint8_t version;
    uint16_t cid;
    uint8_t dt;
    uint8_t did;
    uint8_t bt;
    uint16_t sbt;
    uint8_t op_rc;
    uint32_t bl;
    union {
        uint32_t time;
        uint8_t bytes[7];
    } options;
};

#define PROTOCOL_HDR_VERSION  1         //version

//操作码
#define PROTOCOL_HDR_OP_QUERY 1           //查询
#define PROTOCOL_HDR_OP_ADD  2          //条目添加
#define PROTOCOL_HDR_OP_DELETE  3          //条目删除
#define PROTOCOL_HDR_OP_MODIFY  4          //条目修改
#define PROTOCOL_HDR_OP_CLEAR  5          //条目清空
#define PROTOCOL_HDR_OP_SYN  6          //条目同步
#define PROTOCOL_HDR_OP_RESTORE  7          //样本恢复
#define PROTOCOL_HDR_OP_STOP 8          //样本停止
#define PROTOCOL_HDR_OP_SYN_DEL 9       //同步时删除
#define PROTOCOL_HDR_OP_NAP_INIT  200          //初始化NAP
#define PROTOCOL_HDR_OP_REPORT  20          //初始化NAP

//返回码
#define PROTOCOL_HDR_RC_OK  20          //成功。
//3X——格式错误
#define PROTOCOL_HDR_RC_FAIL_OTHER_FMT  30          //失败，其他格式错误。
#define PROTOCOL_HDR_RC_FAIL_VERSION  31          //失败，版本号不匹配，协议版本不支持。
#define PROTOCOL_HDR_RC_FAIL_HEAD  32          //失败，head无效。
#define PROTOCOL_HDR_RC_FAIL_BODY  33          //失败，报文体格式错误
#define PROTOCOL_HDR_RC_FAIL_BL_LEN  34          //失败，报文体过长。
//4X——系统错误
#define PROTOCOL_HDR_RC_FAIL_OTHER  40          //其余系统错误。
#define PROTOCOL_HDR_RC_FAIL_BUSY  41          //系统忙。
#define PROTOCOL_HDR_RC_FAIL_TIMEOUT  42          //操作超时。
#define PROTOCOL_HDR_RC_FAIL_UNSUPPORTED  43        //业务不支持
//5x——配置同步错误
#define PROTOCOL_HDR_RC_FAIL_CLI_CONFLICT  50       //与CLI的配置冲突

//设备类型
#define PROTOCOL_HDR_DT_CRM  1
#define PROTOCOL_HDR_DT_FLOWEXPORT  2

typedef struct _protocol_unit protocol_unit_t;
typedef struct _unit_reader unit_reader_t;

void protocol_unitdestroy_func(void *data);

#pragma pack(pop)


protocol_unit_t* unit_new();
uint16_t unit_get_len(protocol_unit_t*);
void unit_set_sn(protocol_unit_t* , uint32_t sn);
uint32_t unit_get_sn(protocol_unit_t* );
void unit_push_int8(protocol_unit_t*,int8_t);
void unit_push_int16(protocol_unit_t*,int16_t);
void unit_push_int32(protocol_unit_t*,int32_t);
void unit_push_int64(protocol_unit_t*,int64_t);
void unit_push_string(protocol_unit_t*,const char*);
void unit_free(protocol_unit_t* );

unit_reader_t* unit_reader_new(protocol_unit_t*);
int8_t unit_read_int8(unit_reader_t*);
int16_t unit_read_int16(unit_reader_t*);
int32_t unit_read_int32(unit_reader_t*);
int64_t unit_read_int64(unit_reader_t*);
const char* unit_read_string(unit_reader_t*);
void unit_reader_free(unit_reader_t*);

buffer_t* protocol_dump_all(const struct protocolhdr* hdr, ptr_array_t* units);
buffer_t* protocol_dump(const struct protocolhdr* hdr, protocol_unit_t* unit);
ptr_array_t* protocol_load_units(const char* data, uint32_t len);


#endif /* PROTOCOL_H_ */
