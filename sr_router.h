/*-----------------------------------------------------------------------------
 * File: sr_router.h
 * Date: ?
 * Authors: Guido Apenzeller, Martin Casado, Virkam V.
 * Contact: casado@stanford.edu
 *
 *---------------------------------------------------------------------------*/

#ifndef SR_ROUTER_H
#define SR_ROUTER_H

#include <netinet/in.h>
#include <sys/time.h>
#include <stdio.h>

#include "sr_protocol.h"
#include "sr_arpcache.h"
#include "sr_nat.h"
#include "sr_rt.h"

/* we dont like this debug , but what to do for varargs ? */
#ifdef _DEBUG_
#define Debug(x, args...) printf(x, ## args)
#define DebugMAC(x) \
  do { int ivyl; for(ivyl=0; ivyl<5; ivyl++) printf("%02x:", \
  (unsigned char)(x[ivyl])); printf("%02x",(unsigned char)(x[5])); } while (0)
#else
#define Debug(x, args...) do{}while(0)
#define DebugMAC(x) do{}while(0)
#endif

#define INIT_TTL 255
#define PACKET_DUMP_SIZE 1024

/* forward declare */
struct sr_if;
struct sr_rt;

/* ----------------------------------------------------------------------------
 * struct sr_instance
 *
 * Encapsulation of the state for a single virtual router.
 *
 * -------------------------------------------------------------------------- */

struct sr_instance
{
    int  sockfd;   /* socket to server */
    char user[32]; /* user name */
    char host[32]; /* host name */ 
    char template[30]; /* template name if any */
    unsigned short topo_id;
    struct sockaddr_in sr_addr; /* address to server */
    struct sr_if* if_list; /* list of interfaces */
    struct sr_rt* routing_table; /* routing table */
    struct sr_arpcache cache;   /* ARP cache */
    struct sr_nat* nat; /**< Pointer to NAT state structure. */

    pthread_attr_t attr;
    FILE* logfile;
};

/* -- sr_main.c -- */
int sr_verify_routing_table(struct sr_instance* sr);

/* -- sr_vns_comm.c -- */
int sr_send_packet(struct sr_instance* , uint8_t* , unsigned int , const char*);
int sr_connect_to_server(struct sr_instance* ,unsigned short , char* );
int sr_read_from_server(struct sr_instance* );
int sr_arp_req_not_for_us(struct sr_instance*, uint8_t *, unsigned int, char*);

/* -- sr_router.c -- */
void sr_init(struct sr_instance* );
void sr_handlepacket(struct sr_instance* , uint8_t * , unsigned int , char* );
void arp_handlepacket(struct sr_instance*, uint8_t *, unsigned int, char *);
void sr_add_ethernet_send(struct sr_instance *sr, uint8_t *packet,
        unsigned int len, uint32_t dip, enum sr_ethertype type); 
void build_arp_reply(struct sr_instance *, struct sr_arp_hdr *arp_hdr, struct sr_if *r_iface);
void ip_handlepacket(struct sr_instance*, uint8_t *, unsigned int, char *);
void ip_handlepacketforme(struct sr_instance *sr, sr_ip_hdr_t *ip_hdr, char *interface);
void ip_forwardpacket(struct sr_instance *sr, sr_ip_hdr_t *ip_hdr, unsigned int len, char *interface);
void sr_handle_arpreq(struct sr_instance *sr, struct sr_arpreq *req);
int arp_validpacket(uint8_t *packet, unsigned int len);
int ip_validpacket(uint8_t *packet, unsigned int len);
int icmp_validpacket(struct sr_ip_hdr *ip_hdr);
int tcp_validpacket(struct sr_ip_hdr *ip_hdr);
int sr_packet_is_for_me(struct sr_instance* sr, uint32_t ip_dst);
void arp_boardcast(struct sr_instance* sr, struct sr_arpreq *req);
struct sr_rt* longest_prefix_matching(struct sr_instance *sr, uint32_t ip_dest);
void sr_icmp_with_payload(struct sr_instance *sr, sr_ip_hdr_t *ip_hdr, char *interface, uint8_t icmp_type, uint8_t icmp_code);
struct sr_icmp_t3_hdr icmp_send_error_packet(struct sr_ip_hdr *ip_hdr, int code_num);


/* -- sr_if.c -- */
void sr_add_interface(struct sr_instance* , const char* );
void sr_set_ether_ip(struct sr_instance* , uint32_t );
void sr_set_ether_addr(struct sr_instance* , const unsigned char* );
void sr_print_if_list(struct sr_instance* );

/* -- sr_nat.c -- */

void nat_handle_ippacket(struct sr_instance *sr, sr_ip_hdr_t *ipPacket,
                              unsigned int length, struct sr_if *r_interface);

#endif /* SR_ROUTER_H */
