/**
 *Author:           wuyangchun
 *Date:             2012-07-26
 *Description:      dns数据包相关
 *
 *Notice:
 **/


#ifndef CLIB_DNS_H_
#define CLIB_DNS_H_

#include <endian.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push)
#pragma pack(1)

#define MAX_DNAME_LEN 256

//DNS 头定义
struct dnshdr {
    uint16_t id; /*%< query identification number */

#if  BYTE_ORDER == BIG_ENDIAN
    //#error big
    /* fields in third byte */
    unsigned qr :1; /*%< response flag, 如果是 response, 那么是 1 */
    unsigned opcode :4; /*%< purpose of message. 此包的目的. __ns_opcode */
    unsigned aa :1; /*%< authoritive answer */
    unsigned tc :1; /*%< truncated message. 如果应答过大, udp 放不下, 就会被 truncate */
    unsigned rd :1; /*%< recursion desired. 是否希望要递归查询. */
    /* fields in fourth byte */
    unsigned ra :1; /*%< recursion available. 返回包中, 服务器告知自己能否做递归查询 */
    unsigned unused :1; /*%< unused bits (MBZ as of 4.9.3a3) */
    unsigned ad :1; /*%< authentic data from named */
    unsigned cd :1; /*%< checking disabled by resolver */
    unsigned rcode :4; /*%< response code, __ns_rcode 例如服务器失败是 0010 */
#endif
#if  BYTE_ORDER == LITTLE_ENDIAN || BYTE_ORDER == PDP_ENDIAN
    //#error little
    /* fields in third byte */
    unsigned rd :1; /*%< recursion desired */
    unsigned tc :1; /*%< truncated message */
    unsigned aa :1; /*%< authoritive answer */
    unsigned opcode :4; /*%< purpose of message */
    unsigned qr :1; /*%< response flag */
    /* fields in fourth byte */
    unsigned rcode :4; /*%< response code */
    unsigned cd :1; /*%< checking disabled by resolver */
    unsigned ad :1; /*%< authentic data from named */
    unsigned unused :1; /*%< unused bits (MBZ as of 4.9.3a3) */
    unsigned ra :1; /*%< recursion available */
#endif
    /* remaining bytes */
    uint16_t qdcount; /*%< number of question entries */
    uint16_t ancount; /*%< number of answer entries */
    uint16_t nscount; /*%< number of authority entries */
    uint16_t arcount; /*%< number of resource entries */
//} HEADER;
};

typedef enum __ns_type {
    ns_t_invalid = 0, /*%< Cookie. */
    ns_t_a = 1, /*%< Host address. */
    ns_t_ns = 2, /*%< Authoritative server. */
    ns_t_md = 3, /*%< Mail destination. */
    ns_t_mf = 4, /*%< Mail forwarder. */
    ns_t_cname = 5, /*%< Canonical name. */
    ns_t_soa = 6, /*%< Start of authority zone. */
    ns_t_mb = 7, /*%< Mailbox domain name. */
    ns_t_mg = 8, /*%< Mail group member. */
    ns_t_mr = 9, /*%< Mail rename name. */
    ns_t_null = 10, /*%< Null resource record. */
    ns_t_wks = 11, /*%< Well known service. */
    ns_t_ptr = 12, /*%< Domain name pointer. */
    ns_t_hinfo = 13, /*%< Host information. */
    ns_t_minfo = 14, /*%< Mailbox information. */
    ns_t_mx = 15, /*%< Mail routing information. */
    ns_t_txt = 16, /*%< Text strings. */
    ns_t_rp = 17, /*%< Responsible person. */
    ns_t_afsdb = 18, /*%< AFS cell database. */
    ns_t_x25 = 19, /*%< X_25 calling address. */
    ns_t_isdn = 20, /*%< ISDN calling address. */
    ns_t_rt = 21, /*%< Router. */
    ns_t_nsap = 22, /*%< NSAP address. */
    ns_t_nsap_ptr = 23, /*%< Reverse NSAP lookup (deprecated). */
    ns_t_sig = 24, /*%< Security signature. */
    ns_t_key = 25, /*%< Security key. */
    ns_t_px = 26, /*%< X.400 mail mapping. */
    ns_t_gpos = 27, /*%< Geographical position (withdrawn). */
    ns_t_aaaa = 28, /*%< Ip6 Address. */
    ns_t_loc = 29, /*%< Location Information. */
    ns_t_nxt = 30, /*%< Next domain (security). */
    ns_t_eid = 31, /*%< Endpoint identifier. */
    ns_t_nimloc = 32, /*%< Nimrod Locator. */
    ns_t_srv = 33, /*%< Server Selection. */
    ns_t_atma = 34, /*%< ATM Address */
    ns_t_naptr = 35, /*%< Naming Authority PoinTeR */
    ns_t_kx = 36, /*%< Key Exchange */
    ns_t_cert = 37, /*%< Certification record */
    ns_t_a6 = 38, /*%< IPv6 address (deprecates AAAA) */
    ns_t_dname = 39, /*%< Non-terminal DNAME (for IPv6) */
    ns_t_sink = 40, /*%< Kitchen sink (experimentatl) */
    ns_t_opt = 41, /*%< EDNS0 option (meta-RR) */
    ns_t_apl = 42, /*%< Address prefix list (RFC3123) */
    ns_t_tkey = 249, /*%< Transaction key */
    ns_t_tsig = 250, /*%< Transaction signature. */
    ns_t_ixfr = 251, /*%< Incremental zone transfer. */
    ns_t_axfr = 252, /*%< Transfer zone of authority. */
    ns_t_mailb = 253, /*%< Transfer mailbox records. */
    ns_t_maila = 254, /*%< Transfer mail agent records. */
    ns_t_any = 255, /*%< Wildcard match. */
    ns_t_zxfr = 256, /*%< BIND-specific, nonstandard. */
    ns_t_max = 65536
} ns_type;

typedef enum __ns_code { /* rcode */
    ns_c_noerror = 0,
    ns_c_formerr = 1,
    ns_c_servfail = 2,
    ns_c_nxdomain = 3,
    ns_c_notimp = 4,
    ns_c_refuse = 5,
} ns_code;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif
#endif /* CLIB_DNS_H_ */
