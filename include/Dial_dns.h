#ifndef __DNS_H__
#define __DNS_H__

#include <string.h>
#include <linux/types.h>
#include <asm/byteorder.h>


/* DNS payload overview
 *
 *              DNS FLAG:
 *
 *              0                   7 8                15
 *              +--+--------+--+--+--+--+------+--------+
 *              |QR| opcode |AA|TC|RD|RA|(zero)|  rcode |
 *              +--+--------+--+--+--+--+------+--------+
 *              ^                                       ^
 *              low                                     high
 *              |________________
 *                              |
 *      0                     14 15                    31
 *      +-----------------------+-----------------------+  <-----------
 *      |       SID             |       FLAG            |
 *      +-----------------------+-----------------------+
 *      |       QUERY_NR        |       RR_NR           |    dnshdr_s
 *      +-----------------------+-----------------------+
 *      |       SOA_R_NR        |       ADD_R_NR        |
 *      +-----------------------+-----------------------+  <-----------      <---------------
 *      |            QUESTION NAME (Variable-length)    |
 *      |                                               |
 *      |               .................               |                       question X (1...N)
 *      +-----------------------+-----------------------+  <-----------
 *      |         Q_TYPE        |        Q_CLASS        |    dns_tail_s
 *      +-----------------------+-----------------------+  <-----------      <---------------
 *      |       RESPONSE NAME (Variable-length)         |
 *      |                                               |
 *      |             .................                 |
 *      +-----------------------+-----------------------+  <-----------
 *      |          R_TYPE       |       R_CLASS         |                       answer X (0...N)
 *      +-----------------------+-----------------------+
 *      |                      TTL                      |    dns_tail2_s
 *      +-----------------------+-----------------------+
 *      |      DATA LENGTH      |    RR (Variable-      |
 *      +-----------------------+                       |  <-----------
 *      |     length)   .................               |
 *      +-----------------------------------------------+                    <---------------
 *
 */


/* DNS HEADER struct */

typedef struct dnshdr_s{
        __be16 id;

        union{
#if defined(__BIG_ENDIAN_BITFIELD)   

                struct {
                        __be16

                        qr:1,
                        opc:4,
                        aa:1,
                        tc:1,
                        rd:1,

                        ra:1,
                        zpad:3,
                        rcode:4;
                }flags;
#elif defined(__LITTLE_ENDIAN_BITFIELD)
                struct {
                        __be16
			
                        rd:1,
                        tc:1,
                        aa:1,
                        opc:4,
                        qr:1,

                        rcode:4,
                        zpad:3,
                        ra:1;
                }flags;
#else
#error "Adjust your <asm/byteorder.h> defines" 
#endif
		__be16 flag;
	};
        __be16 q_nr;
        __be16 an_nr;
        __be16 soa_nr;
        __be16 addtion_nr;
}dnshdr_t;


/* tail struct follow QUESTION NAME field */

typedef struct dns_tail_s{
	__be16 dtype;
	__be16 dclass;
}dns_tail_t;


/* tail struct follow RESPONSE field */

typedef struct dns_tail2_s{
	struct dns_tail_s dns_rtail;
	__be32 ttl;
	__be16 data_len;
}dns_tail2_t;


/* optcode */


enum{
	NS_O_QUERY = 0,  	/*%< Standard query. */
	NS_O_IQUERY = 1, 	/*%< Inverse query (deprecated/unsupported). */
	NS_O_STATUS = 2, 	/*%< Name server status query (unsupported). */
/* Opcode 3 is undefined/reserved. */
	NS_O_NOTIFY = 4, 	/*%< Zone change notification. */
	NS_O_UPDATE = 5, 	/*%< Zone update message. */
	NS_O_MAX = 6
};

/* rcode */
enum{
	NS_R_NOERROR = 0, 	/*%< No error occurred. */
	NS_R_FORMERR = 1, 	/*%< Format error. */
	NS_R_SERVFAIL = 2, 	/*%< Server failure. */
	NS_R_NXDOMAIN = 3, 	/*%< Name error. */
	NS_R_NOTIMPL = 4, 	/*%< Unimplemented. */
	NS_R_REFUSED = 5, 	/*%< Operation refused. */
/* these are for BIND_UPDATE */
	NS_R_YXDOMAIN = 6,	/*%< Name exists */
	NS_R_YXRRSET = 7, 	/*%< RRset exists */
	NS_R_NXRRSET = 8, 	/*%< RRset does not exist */
	NS_R_NOTAUTH = 9, 	/*%< Not authoritative for zone */
	NS_R_NOTZONE = 10, 	/*%< Zone of record different from zone section */
	NS_R_MAX = 11,
	NS_R_BADKEY = 17,
	NS_R_BADTIME = 18

};

/* flag */
enum{
	NS_FLAG_QR=0x8000,
	NS_FLAG_OPC=0x7800,
	NS_FLAG_AA=0x0400,
	NS_FLAG_TC=0x0200,
	NS_FLAG_RD=0x0100,
	NS_FLAG_RA=0x0080,
	NS_FLAG_ZPAD=0x0070,
	NS_FLAG_RCODE=0x000f
};

/* type */

enum{
	NS_TYPE_A= 1,
	/*NS_TYPE_NS=__cpu_to_be16(2),
	NS_TYPE_CNAME=__cpu_to_be16(5),
	NS_TYPE_SOA=__cpu_to_be16(6),
	NS_TYPE_PTR=__cpu_to_be16(12),
	NS_TYPE_HINFO=__cpu_to_be16(13),
	NS_TYPE_MX=__cpu_to_be16(15),
	NS_TYPE_TXT=__cpu_to_be16(16),
	NS_TYPE_AAAA=__cpu_to_be16(28),
	NS_TYPE_SRV=__cpu_to_be16(33),
	NS_TYPE_A6=__cpu_to_be16(38),
	NS_TYPE_ANY=__cpu_to_be16(255),*/
};

/* class */

enum{
	NS_CLASS_IN=1,
	NS_CLASS_ANY=255,
};


/* response name is compressed  */
static const char CMPZ=0xc0;

#define name_offset(hdr,p)				\
	do{						\
		uint8_t hi=(*p)&~CMPZ;			\
		uint8_t lo=*(++p);			\
		p=hi*0x100+lo+(char *)hdr;		\
	}while(0)


#endif

extern const char domain_charset[64];

/* assign simple dns query name (ex www.google.com) to dnshdr_s
 *
 * @dnshdr:	dnshdr_s pointer
 * @dname:	simple dns query name, must end with '\0'
 *
 * return size of query name in dnshdr_s
 */

static size_t dns_q_set(struct dnshdr_s *dnshdr, char *dname)
{
	char dst_offset, *dst, *src;

	if(!dnshdr)
		return 0;

	src = dname;

	dst = (char *)dnshdr + sizeof(struct dnshdr_s);
	dst_offset = 1;

	while(1){
		if(*src){
			if(*src == '.'){
				*dst = dst_offset-1;
				dst = dst+dst_offset;
				dst_offset = 0;
			}else{
				*(dst+dst_offset) = *src;
			}
			dst_offset++;
			src++;
		}else{
			*dst = dst_offset-1;
			dst = dst+dst_offset;
			*dst = 0;
			break;
		}
	}
	return strlen(dname)+2;
}

/* full fill dns query by sample way
 *
 * @dnshdr:	dnshdr_s pointer
 * @dname:	simple dns query name, must end with '\0'
 * @id:		seq id in dnshdr_s
 * @flags:	flag in dnshdr_s
 * @dclass:	class in dnshdr_s
 * @dtype:	type in dnshdr_s
 *
 * return size of dns query
 */

static size_t dns_fill_query(struct dnshdr_s *dnshdr, char *dname, __be16 id, __be16 flags, __be16 dclass, __be16 dtype)
{
	struct dns_tail_s *dns_tail;
	dnshdr->id = __cpu_to_be16(id);
	dnshdr->flag = flags;
	dnshdr->q_nr = __cpu_to_be16(1);
	dnshdr->an_nr = __cpu_to_be16(0);
	dnshdr->soa_nr = __cpu_to_be16(0);
	dnshdr->addtion_nr = __cpu_to_be16(0);
	dns_tail = (struct dns_tail_s *)((char *)dnshdr + sizeof(struct dnshdr_s) + dns_q_set(dnshdr, dname));
	dns_tail->dclass = dclass;
	dns_tail->dtype= dtype;
	return (char *)dns_tail+sizeof(struct dns_tail_s)-(char *)dnshdr;
}
